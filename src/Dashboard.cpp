#include "Dashboard.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>

namespace {
constexpr int PANEL_X = DASHBOARD_X;
constexpr int PANEL_MARGIN = 10;
constexpr int SECTION_X = PANEL_X + PANEL_MARGIN;
constexpr int SECTION_W = DASHBOARD_WIDTH - PANEL_MARGIN * 2;

constexpr SDL_Color BG{8, 12, 18, 255};
constexpr SDL_Color SECTION{16, 25, 36, 255};
constexpr SDL_Color BORDER{42, 70, 88, 255};
constexpr SDL_Color MUTED{84, 109, 128, 255};
constexpr SDL_Color TEXT{210, 235, 242, 255};
constexpr SDL_Color CYAN{58, 214, 255, 255};
constexpr SDL_Color GREEN{78, 234, 152, 255};
constexpr SDL_Color AMBER{255, 190, 74, 255};
constexpr SDL_Color RED{255, 78, 86, 255};

std::array<unsigned char, 7> glyphFor(char c) {
    switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c)))) {
        case '0': return {14, 17, 19, 21, 25, 17, 14};
        case '1': return {4, 12, 4, 4, 4, 4, 14};
        case '2': return {14, 17, 1, 2, 4, 8, 31};
        case '3': return {30, 1, 1, 14, 1, 1, 30};
        case '4': return {2, 6, 10, 18, 31, 2, 2};
        case '5': return {31, 16, 30, 1, 1, 17, 14};
        case '6': return {6, 8, 16, 30, 17, 17, 14};
        case '7': return {31, 1, 2, 4, 8, 8, 8};
        case '8': return {14, 17, 17, 14, 17, 17, 14};
        case '9': return {14, 17, 17, 15, 1, 2, 12};
        case 'A': return {14, 17, 17, 31, 17, 17, 17};
        case 'B': return {30, 17, 17, 30, 17, 17, 30};
        case 'C': return {14, 17, 16, 16, 16, 17, 14};
        case 'D': return {30, 17, 17, 17, 17, 17, 30};
        case 'E': return {31, 16, 16, 30, 16, 16, 31};
        case 'F': return {31, 16, 16, 30, 16, 16, 16};
        case 'G': return {14, 17, 16, 23, 17, 17, 14};
        case 'H': return {17, 17, 17, 31, 17, 17, 17};
        case 'I': return {14, 4, 4, 4, 4, 4, 14};
        case 'J': return {7, 2, 2, 2, 18, 18, 12};
        case 'K': return {17, 18, 20, 24, 20, 18, 17};
        case 'L': return {16, 16, 16, 16, 16, 16, 31};
        case 'M': return {17, 27, 21, 21, 17, 17, 17};
        case 'N': return {17, 25, 21, 19, 17, 17, 17};
        case 'O': return {14, 17, 17, 17, 17, 17, 14};
        case 'P': return {30, 17, 17, 30, 16, 16, 16};
        case 'Q': return {14, 17, 17, 17, 21, 18, 13};
        case 'R': return {30, 17, 17, 30, 20, 18, 17};
        case 'S': return {15, 16, 16, 14, 1, 1, 30};
        case 'T': return {31, 4, 4, 4, 4, 4, 4};
        case 'U': return {17, 17, 17, 17, 17, 17, 14};
        case 'V': return {17, 17, 17, 17, 17, 10, 4};
        case 'W': return {17, 17, 17, 21, 21, 21, 10};
        case 'X': return {17, 17, 10, 4, 10, 17, 17};
        case 'Y': return {17, 17, 10, 4, 4, 4, 4};
        case 'Z': return {31, 1, 2, 4, 8, 16, 31};
        case '-': return {0, 0, 0, 31, 0, 0, 0};
        case ':': return {0, 4, 4, 0, 4, 4, 0};
        case '/': return {1, 1, 2, 4, 8, 16, 16};
        case '.': return {0, 0, 0, 0, 0, 12, 12};
        default:  return {0, 0, 0, 0, 0, 0, 0};
    }
}

int laneIndex(Lane lane) {
    return static_cast<int>(lane);
}

int clampInt(int value, int minValue, int maxValue) {
    return std::max(minValue, std::min(value, maxValue));
}
}

void Dashboard::draw(SDL_Renderer* renderer, const DashboardData& data) const {
    fillRect(renderer, {PANEL_X, 0, DASHBOARD_WIDTH, WIN_HEIGHT}, BG);
    fillRect(renderer, {PANEL_X, 0, 2, WIN_HEIGHT}, BORDER);

    drawText(renderer, SECTION_X + 8, 14, "CAR TWIN", 2, CYAN);
    drawText(renderer, SECTION_X + 9, 34, "TELEMETRY HUD", 1, MUTED);

    drawSection(renderer, SECTION_X, 54, SECTION_W, 74);
    drawText(renderer, SECTION_X + 10, 66, "LINK", 1, TEXT);
    drawStatusLed(renderer, SECTION_X + 62, 63, data.serialConnected ? GREEN : RED);
    char portText[16] = "SCAN";
    if (data.serialConnected && data.serialPort > 0) {
        std::snprintf(portText, sizeof(portText), "COM%d", data.serialPort);
    }
    drawText(renderer, SECTION_X + 86, 64, portText, 1, data.serialConnected ? GREEN : RED);
    drawText(renderer, SECTION_X + 10, 86, "MCU", 1, TEXT);
    drawStatusLed(renderer, SECTION_X + 62, 83, data.mcuStatus ? GREEN : MUTED);
    drawText(renderer, SECTION_X + 86, 84, data.mcuStatus ? "RUN" : "IDLE", 1,
             data.mcuStatus ? GREEN : MUTED);
    drawText(renderer, SECTION_X + 10, 108, "HB", 1, MUTED);
    drawNumber(renderer, SECTION_X + 44, 106, data.heartbeat, 1, CYAN);
    drawText(renderer, SECTION_X + 106, 108, "RX", 1, MUTED);
    drawNumber(renderer, SECTION_X + 132, 106, data.rxFrameCount, 1, GREEN);

    drawSection(renderer, SECTION_X, 138, SECTION_W, 114);
    drawText(renderer, SECTION_X + 10, 150, "POWER", 1, TEXT);
    char voltage[16] = "----";
    if (data.vddaMv > 0) {
        std::snprintf(voltage, sizeof(voltage), "%d.%02dV",
                      data.vddaMv / 1000, (data.vddaMv % 1000) / 10);
    }
    drawText(renderer, SECTION_X + 92, 148, voltage, 2, AMBER);
    drawText(renderer, SECTION_X + 10, 180, "RPM L", 1, MUTED);
    drawNumber(renderer, SECTION_X + 76, 178, data.leftRpm, 2, GREEN);
    drawText(renderer, SECTION_X + 10, 214, "RPM R", 1, MUTED);
    drawNumber(renderer, SECTION_X + 76, 212, data.rightRpm, 2, GREEN);

    drawSection(renderer, SECTION_X, 262, SECTION_W, 74);
    drawText(renderer, SECTION_X + 10, 274, "MODE", 1, TEXT);
    drawStatusLed(renderer, SECTION_X + 62, 271,
                  data.autoMode ? GREEN : (data.modeCommandPending ? AMBER : MUTED));
    fillRect(renderer, {MODE_TOGGLE_X, MODE_TOGGLE_Y, MODE_TOGGLE_W, MODE_TOGGLE_H},
             data.autoMode ? GREEN : SDL_Color{25, 39, 52, 255});
    drawRect(renderer, {MODE_TOGGLE_X, MODE_TOGGLE_Y, MODE_TOGGLE_W, MODE_TOGGLE_H},
             data.modeCommandPending ? AMBER : BORDER);
    drawText(renderer, MODE_TOGGLE_X + 14, MODE_TOGGLE_Y + 7,
             data.autoMode ? "AUTO" : "MAN", 1, data.autoMode ? BG : TEXT);
    drawText(renderer, SECTION_X + 10, 306, "LANE", 1, TEXT);
    drawLane(renderer, SECTION_X + 44, 298, data.playerLane);

    drawSection(renderer, SECTION_X, 346, SECTION_W, 62);
    drawText(renderer, SECTION_X + 10, 360, "SPEED", 1, TEXT);
    drawBar(renderer, SECTION_X + 76, 362, 86, 12, data.playerSpeed, 20, GREEN);
    drawNumber(renderer, SECTION_X + 166, 356, data.playerSpeed, 1, AMBER);
    drawText(renderer, SECTION_X + 10, 388, "TX", 1, MUTED);
    drawNumber(renderer, SECTION_X + 44, 386, data.txFrameCount, 1, GREEN);
    drawText(renderer, SECTION_X + 106, 388, "CR", 1, MUTED);
    drawNumber(renderer, SECTION_X + 132, 386, data.collisionCount, 1, RED);

    drawSection(renderer, SECTION_X, 418, SECTION_W, 82);
    drawText(renderer, SECTION_X + 10, 430, "CAR", 1, TEXT);
    drawText(renderer, SECTION_X + 10, 454, "X", 1, CYAN);
    drawNumber(renderer, SECTION_X + 32, 452, data.playerX, 1, CYAN);
    drawText(renderer, SECTION_X + 98, 454, "Y", 1, CYAN);
    drawNumber(renderer, SECTION_X + 120, 452, data.playerY, 1, CYAN);
    drawText(renderer, SECTION_X + 10, 480, "OBS", 1, TEXT);
    drawText(renderer, SECTION_X + 48, 480, "X", 1, AMBER);
    drawNumber(renderer, SECTION_X + 70, 478, data.obstacleX, 1, AMBER);
    drawText(renderer, SECTION_X + 118, 480, "Y", 1, AMBER);
    drawNumber(renderer, SECTION_X + 140, 478, data.obstacleY, 1, AMBER);

    drawSection(renderer, SECTION_X, 510, SECTION_W, 70);
    drawMiniMap(renderer, SECTION_X + 18, 526, 150, 38, data);
}

void Dashboard::drawSection(SDL_Renderer* renderer, int x, int y, int w, int h) const {
    fillRect(renderer, {x, y, w, h}, SECTION);
    drawRect(renderer, {x, y, w, h}, BORDER);
}

void Dashboard::drawText(SDL_Renderer* renderer, int x, int y, const char* text, int scale,
                         SDL_Color color) const {
    int cursor = x;
    for (const char* p = text; *p; ++p) {
        drawChar(renderer, cursor, y, *p, scale, color);
        cursor += 6 * scale;
    }
}

void Dashboard::drawChar(SDL_Renderer* renderer, int x, int y, char c, int scale,
                         SDL_Color color) const {
    const auto glyph = glyphFor(c);
    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
            if ((glyph[row] & (1 << (4 - col))) == 0) {
                continue;
            }
            fillRect(renderer, {x + col * scale, y + row * scale, scale, scale}, color);
        }
    }
}

void Dashboard::drawNumber(SDL_Renderer* renderer, int x, int y, int value, int scale,
                           SDL_Color color) const {
    char buffer[16] = {};
    std::snprintf(buffer, sizeof(buffer), "%d", value);
    drawText(renderer, x, y, buffer, scale, color);
}

void Dashboard::drawLane(SDL_Renderer* renderer, int x, int y, Lane lane) const {
    const int selected = laneIndex(lane);
    for (int i = 0; i < 3; ++i) {
        const SDL_Rect box{x + i * 56, y, 42, 24};
        const bool active = i == selected;
        fillRect(renderer, box, active ? CYAN : SDL_Color{25, 39, 52, 255});
        drawRect(renderer, box, active ? TEXT : BORDER);
        char label[2] = {static_cast<char>('0' + i), '\0'};
        drawText(renderer, box.x + 16, box.y + 7, label, 2, active ? BG : MUTED);
    }
}

void Dashboard::drawBar(SDL_Renderer* renderer, int x, int y, int w, int h, int value,
                        int maxValue, SDL_Color color) const {
    const int clamped = clampInt(value, 0, maxValue);
    const int fillWidth = maxValue > 0 ? (w - 4) * clamped / maxValue : 0;
    drawRect(renderer, {x, y, w, h}, BORDER);
    fillRect(renderer, {x + 2, y + 2, fillWidth, h - 4}, color);
}

void Dashboard::drawStatusLed(SDL_Renderer* renderer, int x, int y, SDL_Color color) const {
    fillRect(renderer, {x, y, 12, 12}, color);
    drawRect(renderer, {x - 2, y - 2, 16, 16}, BORDER);
}

void Dashboard::drawMiniMap(SDL_Renderer* renderer, int x, int y, int w, int h,
                            const DashboardData& data) const {
    fillRect(renderer, {x, y, w, h}, SDL_Color{6, 10, 14, 255});
    drawRect(renderer, {x, y, w, h}, BORDER);

    const int laneW = w / 3;
    for (int i = 1; i < 3; ++i) {
        fillRect(renderer, {x + i * laneW, y + 4, 1, h - 8}, MUTED);
    }

    auto drawDot = [&](int worldX, int worldY, SDL_Color color) {
        const int centerX = clampInt(worldX + CAR_WIDTH / 2, 0, ROAD_WIDTH);
        const int centerY = clampInt(worldY + CAR_HEIGHT / 2, 0, ROAD_HEIGHT);
        const int px = x + centerX * w / ROAD_WIDTH;
        const int py = y + centerY * h / ROAD_HEIGHT;
        fillRect(renderer, {px - 3, py - 3, 6, 6}, color);
    };

    drawDot(data.playerX, data.playerY, GREEN);
    drawDot(data.obstacleX, data.obstacleY, RED);
}

void Dashboard::fillRect(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
}

void Dashboard::drawRect(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &rect);
}
