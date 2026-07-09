#include "Obstacle.h"
#include "Car.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

void Obstacle::load(SDL_Renderer* renderer) {
    m_img.load(renderer, IMG_OBSTACLE);
    m_imgOutline.load(renderer, IMG_OBSTACLE_OUT);
}

void Obstacle::draw(SDL_Renderer* renderer) const {
    m_imgOutline.draw(renderer, x, y);
    m_img.draw(renderer, x, y);
}

void Obstacle::update(const Car& player) {
    int delta = player.speed - speed;
    y += delta;

    if (y > OBSTACLE_RESET_Y) {
        y = OBSTACLE_START_Y;
        lane = static_cast<Lane>(std::uniform_int_distribution<int>(0, 2)(m_rng));
        x = laneToX(lane);
    }
}

void Obstacle::reset() {
    x = LANE_0_X;
    y = OBSTACLE_START_Y;
    speed = OBSTACLE_SPEED;
    lane = Lane::L0;
}

void Obstacle::packSendBuf(char* buf) const {
    buf[0] = SOFT_CMD_BEGIN;
    snprintf(&buf[1], 4, "%03d", x);
    buf[4] = SOFT_CMD_RET;
    snprintf(&buf[5], 5, "%4d", y);
    buf[9] = SOFT_CMD_END;
    buf[10] = '\0';
}
