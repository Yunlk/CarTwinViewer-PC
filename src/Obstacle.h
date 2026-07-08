#pragma once

#include "common.h"
#include "Texture.h"
#include <random>

class Car;

// ========== 障碍车类 ==========
class Obstacle {
public:
    int  x = LANE_0_X,  y = OBSTACLE_START_Y;
    int  speed = OBSTACLE_SPEED;
    Lane lane = Lane::L0;

    void load(SDL_Renderer* renderer);
    void draw(SDL_Renderer* renderer) const;
    void update(const Car& player);
    void reset();

    // 串口数据打包
    void packSendBuf(char* buf) const;

private:
    Texture m_img;
    Texture m_imgOutline;
    std::mt19937 m_rng{ std::random_device{}() };
};
