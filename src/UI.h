#pragma once

#include <SDL.h>
#include <thread>
#include <atomic>
#include "Car.h"
#include "Road.h"
#include "Obstacle.h"
#include "Serial.h"

class UI {
public:
    bool init();
    void run();
    ~UI();

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    Road     m_road;
    Car      m_player;
    Obstacle m_obstacle;

    // 串口线程
    std::thread  m_serialThread;
    std::atomic<bool> m_running{ false };
    char m_sendBuf[COM_BUF_SIZE] = {};
    char m_recvBuf[COM_BUF_SIZE] = {};

    void serialLoop();
    void handleInput();
    void drawCollisionBox() const;
};
