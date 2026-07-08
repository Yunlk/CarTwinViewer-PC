#include "UI.h"
#include <cstdio>

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
    PortHandle port = serialOpen(COM_PORT, COM_BAUD, COM_BITS, COM_STOPBITS, COM_PARITY);
    memset(m_sendBuf, 0, COM_BUF_SIZE);
    memset(m_recvBuf, 0, COM_BUF_SIZE);

    while (m_running) {
        serialRecv(port, m_recvBuf, COM_BUF_SIZE);
        m_road.unpackRecvBuf(m_recvBuf, m_player);
        SDL_Delay(COM_TIME_MS);

        m_obstacle.packSendBuf(m_sendBuf);
        serialSend(port, m_sendBuf, COM_BUF_SIZE);
        SDL_Delay(COM_TIME_MS);
    }

    serialClose(port);
}

void UI::handleInput() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            m_running = false;
        }
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_1: m_player.setLane(Lane::L0); break;
                case SDLK_2: m_player.setLane(Lane::L1); break;
                case SDLK_3: m_player.setLane(Lane::L2); break;
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

void UI::run() {
    while (m_running) {
        handleInput();

        m_obstacle.update(m_player);

        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_RenderClear(m_renderer);

        m_road.draw(m_renderer);
        m_player.draw(m_renderer);
        m_obstacle.draw(m_renderer);

        if (Road::checkCollision(m_player, m_obstacle.x, m_obstacle.y)) {
            drawCollisionBox();
            SDL_RenderPresent(m_renderer);
            printf("Collision detected! Reset.\n");
            m_player.reset();
            m_obstacle.reset();
            continue;
        }

        SDL_RenderPresent(m_renderer);
        SDL_Delay(FRAME_DELAY);
    }
}
