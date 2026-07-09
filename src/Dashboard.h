#pragma once

#include "common.h"
#include <SDL.h>

struct DashboardData {
    int playerX = 0;
    int playerY = 0;
    int playerSpeed = 0;
    Lane playerLane = Lane::L1;

    int obstacleX = 0;
    int obstacleY = 0;
    int obstacleSpeed = 0;
    Lane obstacleLane = Lane::L0;

    int collisionCount = 0;
    int heartbeat = 0;
    int rxFrameCount = 0;
    int txFrameCount = 0;
    int leftRpm = 0;
    int rightRpm = 0;
    int vddaMv = 0;
    int mcuStatus = 0;
    bool telemetryFresh = false;
    bool serialConnected = false;
    bool lastRxOk = false;
    bool lastTxOk = false;
};

class Dashboard {
public:
    void draw(SDL_Renderer* renderer, const DashboardData& data) const;

private:
    void drawSection(SDL_Renderer* renderer, int x, int y, int w, int h) const;
    void drawText(SDL_Renderer* renderer, int x, int y, const char* text, int scale,
                  SDL_Color color) const;
    void drawChar(SDL_Renderer* renderer, int x, int y, char c, int scale,
                  SDL_Color color) const;
    void drawNumber(SDL_Renderer* renderer, int x, int y, int value, int scale,
                    SDL_Color color) const;
    void drawLane(SDL_Renderer* renderer, int x, int y, Lane lane) const;
    void drawBar(SDL_Renderer* renderer, int x, int y, int w, int h, int value,
                 int maxValue, SDL_Color color) const;
    void drawStatusLed(SDL_Renderer* renderer, int x, int y, SDL_Color color) const;
    void drawMiniMap(SDL_Renderer* renderer, int x, int y, int w, int h,
                     const DashboardData& data) const;

    void fillRect(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) const;
    void drawRect(SDL_Renderer* renderer, SDL_Rect rect, SDL_Color color) const;
};
