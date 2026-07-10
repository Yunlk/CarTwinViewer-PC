#include "Car.h"

void Car::load(SDL_Renderer* renderer) {
    m_img.load(renderer, IMG_CAR);
    m_imgOutline.load(renderer, IMG_CAR_OUT);
}

void Car::draw(SDL_Renderer* renderer) const {
    m_imgOutline.draw(renderer, x, y);
    m_img.draw(renderer, x, y);
}

void Car::setLane(Lane l) {
    lane = l;
    targetX = laneToX(l);
}

void Car::update() {
    if (x == targetX) return;

    constexpr int STEP = 8;
    if (x < targetX) {
        x = (x + STEP > targetX) ? targetX : x + STEP;
    } else {
        x = (x - STEP < targetX) ? targetX : x - STEP;
    }
    lane = laneFromX(x);
}

void Car::reset() {
    x = LANE_1_X;
    targetX = LANE_1_X;
    y = CAR_START_Y;
    speed = CAR_SPEED;
    lane = Lane::L1;
}
