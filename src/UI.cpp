#include "UI.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {
bool keyEquals(const char* key, int len, const char* expected) {
    return static_cast<int>(std::strlen(expected)) == len &&
           std::strncmp(key, expected, len) == 0;
}

Lane laneFromX(int x) {
    const int d0 = std::abs(x - LANE_0_X);
    const int d1 = std::abs(x - LANE_1_X);
    const int d2 = std::abs(x - LANE_2_X);

    if (d0 <= d1 && d0 <= d2) {
        return Lane::L0;
    }
    if (d1 <= d0 && d1 <= d2) {
        return Lane::L1;
    }
    return Lane::L2;
}

bool pointInModeToggle(int x, int y) {
    return x >= MODE_TOGGLE_X && x <= MODE_TOGGLE_X + MODE_TOGGLE_W &&
           y >= MODE_TOGGLE_Y && y <= MODE_TOGGLE_Y + MODE_TOGGLE_H;
}
}

bool UI::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        return false;
    }

    m_window = SDL_CreateWindow("CarTwinViewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!m_window) return false;

    m_renderer = SDL_CreateRenderer(m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer) return false;

    m_road.load(m_renderer);
    m_player.load(m_renderer);
    m_obstacle.load(m_renderer);

    // 启动串口线程
    m_running = true;
    m_serialThread = std::thread(&UI::serialLoop, this);

    return true;
}

UI::~UI() {
    m_running = false;
    if (m_serialThread.joinable())
        m_serialThread.join();

    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    IMG_Quit();
    SDL_Quit();
}

void UI::serialLoop() {
    memset(m_sendBuf, 0, COM_BUF_SIZE);
    memset(m_recvBuf, 0, COM_BUF_SIZE);
    memset(m_frameBuf, 0, COM_BUF_SIZE);
    m_frameLen = 0;

    while (m_running) {
        PortHandle port = nullptr;
        SerialPortInfo activePort;
        const auto ports = serialListPorts();

        for (const auto& candidate : ports) {
            if (!m_running) {
                break;
            }

            port = serialOpen(candidate.index, COM_BAUD, COM_BITS, COM_STOPBITS,
                              COM_PARITY, false);
            if (port) {
                activePort = candidate;
                m_serialPort = activePort.index;
                m_serialConnected = true;
                m_lastRxOk = false;
                m_lastTxOk = false;
                printf("Serial connected: COM%d %s\n", activePort.index,
                       activePort.name.c_str());
                break;
            }
        }

        if (!port) {
            m_serialConnected = false;
            m_serialPort = 0;
            m_lastRxOk = false;
            m_lastTxOk = false;
            m_telemetryFresh = false;
            SDL_Delay(COM_RESCAN_MS);
            continue;
        }

        while (m_running && port) {
            const int rxLen = serialRecv(port, m_recvBuf, COM_BUF_SIZE - 1);
            if (rxLen < 0) {
                printf("Serial read failed on COM%d, reconnecting.\n", activePort.index);
                break;
            }
            m_lastRxOk = rxLen > 0;
            if (rxLen > 0) {
                processSerialBytes(m_recvBuf, rxLen);
            }
            SDL_Delay(COM_TIME_MS);

            m_obstacle.packSendBuf(m_sendBuf);
            const int txLen = serialSend(port, m_sendBuf, OBSTACLE_FRAME_SIZE);
            m_lastTxOk = txLen == OBSTACLE_FRAME_SIZE;
            if (txLen == OBSTACLE_FRAME_SIZE) {
                ++m_txFrameCount;
            } else {
                printf("Serial write failed on COM%d, reconnecting.\n", activePort.index);
                break;
            }

            const int pendingMode = m_pendingModeCommand.load();
            if (pendingMode == CONTROL_MODE_AUTO || pendingMode == CONTROL_MODE_MANUAL) {
                char commandBuf[COM_BUF_SIZE] = {};
                const int commandLen = std::snprintf(commandBuf, sizeof(commandBuf),
                                                     "%cMODE=%d%c",
                                                     SOFT_CMD_BEGIN, pendingMode,
                                                     SOFT_CMD_END);
                const int commandTxLen = serialSend(port, commandBuf, commandLen);
                m_lastTxOk = commandTxLen == commandLen;
                if (commandTxLen == commandLen) {
                    int expected = pendingMode;
                    m_pendingModeCommand.compare_exchange_strong(expected, -1);
                    ++m_txFrameCount;
                } else {
                    printf("Serial mode command failed on COM%d, reconnecting.\n",
                           activePort.index);
                    break;
                }
            }
            SDL_Delay(COM_TIME_MS);
        }

        serialClose(port);
        m_serialConnected = false;
        m_serialPort = 0;
        m_lastRxOk = false;
        m_lastTxOk = false;
        m_telemetryFresh = false;
        m_mcuStatus = 0;
        m_frameLen = 0;
        memset(m_frameBuf, 0, COM_BUF_SIZE);
        SDL_Delay(COM_RESCAN_MS);
    }
}

void UI::processSerialBytes(const char* data, int len) {
    for (int i = 0; i < len; ++i) {
        const char ch = data[i];
        if (ch == SOFT_CMD_BEGIN) {
            m_frameLen = 0;
        }

        if (m_frameLen >= COM_BUF_SIZE - 1) {
            m_frameLen = 0;
        }

        m_frameBuf[m_frameLen++] = ch;

        if (ch == SOFT_CMD_END) {
            handleSerialFrame(m_frameBuf, m_frameLen);
            m_frameLen = 0;
            memset(m_frameBuf, 0, COM_BUF_SIZE);
        }
    }
}

void UI::handleSerialFrame(const char* frame, int len) {
    if (len <= 0 || frame[0] != SOFT_CMD_BEGIN) {
        return;
    }

    ++m_rxFrameCount;
    if (parseTelemetryFrame(frame, len)) {
        m_lastRxOk = true;
        return;
    }

    m_road.unpackRecvBuf(frame, m_player);
    m_player.lane = laneFromX(m_player.x);
}

bool UI::parseTelemetryFrame(const char* frame, int len) {
    bool hasKeyValue = false;
    for (int i = 0; i < len; ++i) {
        if (frame[i] == '=') {
            hasKeyValue = true;
            break;
        }
    }
    if (!hasKeyValue) {
        return false;
    }

    int index = 1;
    while (index < len && frame[index] != SOFT_CMD_END) {
        if (frame[index] == ',') {
            ++index;
            continue;
        }

        const int keyStart = index;
        while (index < len && frame[index] != '=' && frame[index] != ',' &&
               frame[index] != SOFT_CMD_END) {
            ++index;
        }
        if (index >= len || frame[index] != '=') {
            while (index < len && frame[index] != ',' && frame[index] != SOFT_CMD_END) {
                ++index;
            }
            continue;
        }

        const int keyLen = index - keyStart;
        ++index;

        char* end = nullptr;
        const long value = std::strtol(&frame[index], &end, 10);
        if (end == &frame[index]) {
            while (index < len && frame[index] != ',' && frame[index] != SOFT_CMD_END) {
                ++index;
            }
            continue;
        }

        if (keyEquals(&frame[keyStart], keyLen, "X")) {
            m_player.x = static_cast<int>(value);
            m_player.lane = laneFromX(m_player.x);
        } else if (keyEquals(&frame[keyStart], keyLen, "Y")) {
            m_player.y = static_cast<int>(value);
        } else if (keyEquals(&frame[keyStart], keyLen, "HB")) {
            m_heartbeat = static_cast<int>(value);
        } else if (keyEquals(&frame[keyStart], keyLen, "RX")) {
            // STM32-side count of frames received from the PC.
        } else if (keyEquals(&frame[keyStart], keyLen, "TX")) {
            // STM32-side count of frames sent to the PC.
        } else if (keyEquals(&frame[keyStart], keyLen, "RL")) {
            m_leftRpm = static_cast<int>(value);
        } else if (keyEquals(&frame[keyStart], keyLen, "RR")) {
            m_rightRpm = static_cast<int>(value);
        } else if (keyEquals(&frame[keyStart], keyLen, "V")) {
            m_vddaMv = static_cast<int>(value);
        } else if (keyEquals(&frame[keyStart], keyLen, "ST")) {
            m_mcuStatus = static_cast<int>(value);
            m_autoMode = (static_cast<int>(value) & MCU_STATUS_AUTO_MODE) != 0;
        }

        index = static_cast<int>(end - frame);
        while (index < len && frame[index] != ',' && frame[index] != SOFT_CMD_END) {
            ++index;
        }
    }

    m_telemetryFresh = true;
    return true;
}

void UI::requestMode(int mode) {
    if (mode != CONTROL_MODE_AUTO && mode != CONTROL_MODE_MANUAL) {
        return;
    }

    m_pendingModeCommand = mode;
    m_autoMode = mode == CONTROL_MODE_AUTO;
}

void UI::requestModeToggle() {
    requestMode(m_autoMode ? CONTROL_MODE_MANUAL : CONTROL_MODE_AUTO);
}

void UI::handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            m_running = false;
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            if (pointInModeToggle(e.button.x, e.button.y)) {
                requestModeToggle();
            }
        }
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_1:
                case SDLK_KP_1:
                    m_player.setLane(Lane::L0);
                    break;
                case SDLK_2:
                case SDLK_KP_2:
                    m_player.setLane(Lane::L1);
                    break;
                case SDLK_3:
                case SDLK_KP_3:
                    m_player.setLane(Lane::L2);
                    break;
                case SDLK_m:
                    requestModeToggle();
                    break;
                case SDLK_ESCAPE: m_running = false; break;
            }
        }
    }
}

void UI::drawCollisionBox() const {
    // 碰撞时画红框（主车）+ 绿框（障碍车）
    SDL_Rect r1 = { m_player.x, m_player.y, CAR_WIDTH, CAR_HEIGHT };
    SDL_Rect r2 = { m_obstacle.x, m_obstacle.y, CAR_WIDTH, CAR_HEIGHT };

    SDL_SetRenderDrawColor(m_renderer, 255, 0, 0, 255);
    SDL_RenderDrawRect(m_renderer, &r1);
    SDL_SetRenderDrawColor(m_renderer, 0, 255, 0, 255);
    SDL_RenderDrawRect(m_renderer, &r2);
}

DashboardData UI::makeDashboardData() const {
    DashboardData data;
    data.playerX = m_player.x;
    data.playerY = m_player.y;
    data.playerSpeed = m_player.speed;
    data.playerLane = m_player.lane;
    data.obstacleX = m_obstacle.x;
    data.obstacleY = m_obstacle.y;
    data.obstacleSpeed = m_obstacle.speed;
    data.obstacleLane = m_obstacle.lane;
    data.collisionCount = m_collisionCount;
    data.heartbeat = m_heartbeat;
    data.rxFrameCount = m_rxFrameCount;
    data.txFrameCount = m_txFrameCount;
    data.leftRpm = m_leftRpm;
    data.rightRpm = m_rightRpm;
    data.vddaMv = m_vddaMv;
    data.mcuStatus = m_mcuStatus;
    data.serialPort = m_serialPort;
    data.autoMode = m_autoMode;
    data.modeCommandPending = m_pendingModeCommand != -1;
    data.telemetryFresh = m_telemetryFresh;
    data.serialConnected = m_serialConnected;
    data.lastRxOk = m_lastRxOk;
    data.lastTxOk = m_lastTxOk;
    return data;
}

void UI::run() {
    while (m_running) {
        handleInput();

        m_obstacle.update(m_player);

        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderClear(m_renderer);

        m_road.draw(m_renderer);
        m_player.draw(m_renderer);
        m_obstacle.draw(m_renderer);

        const bool collided = Road::checkCollision(m_player, m_obstacle.x, m_obstacle.y);
        if (collided) {
            ++m_collisionCount;
            drawCollisionBox();
        }

        m_dashboard.draw(m_renderer, makeDashboardData());
        SDL_RenderPresent(m_renderer);

        if (collided) {
            printf("Collision detected! Reset.\n");
            m_player.reset();
            m_obstacle.reset();
            continue;
        }

        SDL_Delay(FRAME_DELAY);
    }
}
