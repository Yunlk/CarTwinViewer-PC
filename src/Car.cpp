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
    x = laneToX(l);
}

void Car::reset() {
    x = LANE_1_X;
    y = CAR_START_Y;
    speed = CAR_SPEED;
    lane = Lane::L1;
}
