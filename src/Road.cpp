#include "Road.h"
#include "Car.h"
#include <cstdlib>

void Road::load(SDL_Renderer* renderer) {
    m_img.load(renderer, IMG_ROAD);
}

void Road::draw(SDL_Renderer* renderer) {
    m_scrollOffset = (m_scrollOffset + ROAD_SPEED) % ROAD_HEIGHT;

    // 两张图首尾相接实现无缝滚动
    m_img.draw(renderer, 0, m_scrollOffset);
    m_img.draw(renderer, 0, m_scrollOffset - ROAD_HEIGHT);
}

void Road::unpackRecvBuf(const char* buf, Car& car) {
    if (buf[0] == SOFT_CMD_BEGIN) {
        car.x = atoi(&buf[1]);
        car.y = atoi(&buf[5]);
    }
}

bool Road::checkCollision(const Car& player, int ox, int oy) {
    return pointInRect(player.x,              player.y,              ox, oy, CAR_WIDTH, CAR_HEIGHT) ||
           pointInRect(player.x + CAR_WIDTH,  player.y,              ox, oy, CAR_WIDTH, CAR_HEIGHT) ||
           pointInRect(player.x,              player.y + CAR_HEIGHT, ox, oy, CAR_WIDTH, CAR_HEIGHT) ||
           pointInRect(player.x + CAR_WIDTH,  player.y + CAR_HEIGHT, ox, oy, CAR_WIDTH, CAR_HEIGHT);
}
