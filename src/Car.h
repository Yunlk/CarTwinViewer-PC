#pragma once

#include "common.h"
#include "Texture.h"

// ========== 主车类 ==========
class Car {
public:
    int  x = LANE_1_X,  y = CAR_START_Y;
    int  speed = CAR_SPEED;
    Lane lane  = Lane::L1;
    int  targetX = LANE_1_X;

    void load(SDL_Renderer* renderer);
    void draw(SDL_Renderer* renderer) const;
    void setLane(Lane l);
    void update();
    void reset();

private:
    Texture m_img;
    Texture m_imgOutline;
};
