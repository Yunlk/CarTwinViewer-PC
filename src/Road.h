#pragma once

#include "common.h"
#include "Texture.h"

class Car;

// ========== 道路（滚动背景） ==========
class Road {
public:
    void load(SDL_Renderer* renderer);
    void draw(SDL_Renderer* renderer);

    // 串口数据接收
    void unpackRecvBuf(const char* buf, Car& car);

    // 碰撞检测
    static bool checkCollision(const Car& player, int ox, int oy);

private:
    Texture m_img;
    int     m_scrollOffset = 0;
};
