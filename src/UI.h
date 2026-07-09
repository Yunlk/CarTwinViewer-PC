#pragma once

#include <SDL.h>
#include <thread>
#include <atomic>
#include "Car.h"
#include "Road.h"
#include "Dashboard.h"
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
    Dashboard m_dashboard;

    // 串口线程
    std::thread  m_serialThread;
    std::atomic<bool> m_running{ false };
    char m_sendBuf[COM_BUF_SIZE] = {};
    char m_recvBuf[COM_BUF_SIZE] = {};
    char m_frameBuf[COM_BUF_SIZE] = {};
    int m_frameLen = 0;

    std::atomic<bool> m_serialConnected{ false };
    std::atomic<bool> m_lastRxOk{ false };
    std::atomic<bool> m_lastTxOk{ false };
    std::atomic<bool> m_telemetryFresh{ false };
    std::atomic<int> m_rxFrameCount{ 0 };
    std::atomic<int> m_txFrameCount{ 0 };
    std::atomic<int> m_heartbeat{ 0 };
    std::atomic<int> m_leftRpm{ 0 };
    std::atomic<int> m_rightRpm{ 0 };
    std::atomic<int> m_vddaMv{ 0 };
    std::atomic<int> m_mcuStatus{ 0 };
    std::atomic<int> m_serialPort{ 0 };
    std::atomic<int> m_pendingModeCommand{ -1 };
    std::atomic<bool> m_autoMode{ false };
    int m_collisionCount = 0;

    void serialLoop();
    void processSerialBytes(const char* data, int len);
    void handleSerialFrame(const char* frame, int len);
    bool parseTelemetryFrame(const char* frame, int len);
    void requestModeToggle();
    void requestMode(int mode);
    void handleInput();
    void drawCollisionBox() const;
    DashboardData makeDashboardData() const;
};
