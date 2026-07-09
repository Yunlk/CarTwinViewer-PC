#pragma once

#include <SDL.h>

// ========== 屏幕 / 窗口 ==========
constexpr int WIN_WIDTH   = 560;
constexpr int WIN_HEIGHT  = 600;
constexpr int FPS         = 60;
constexpr int FRAME_DELAY = 1000 / FPS;

// ========== 道路参数 ==========
constexpr int ROAD_WIDTH    = 350;
constexpr int ROAD_HEIGHT   = 600;
constexpr int ROAD_SPEED    = 5;
constexpr int DASHBOARD_WIDTH = WIN_WIDTH - ROAD_WIDTH;
constexpr int DASHBOARD_X     = ROAD_WIDTH;
constexpr int MODE_TOGGLE_X   = DASHBOARD_X + 104;
constexpr int MODE_TOGGLE_Y   = 270;
constexpr int MODE_TOGGLE_W   = 76;
constexpr int MODE_TOGGLE_H   = 24;

// ========== 主车参数 ==========
constexpr int CAR_WIDTH     = 98;
constexpr int CAR_HEIGHT    = 180;
constexpr int CAR_SPEED     = 10;
constexpr int CAR_START_Y   = 420;

// ========== 障碍车参数 ==========
constexpr int OBSTACLE_SPEED     = 9;
constexpr int OBSTACLE_START_Y   = -100;
constexpr int OBSTACLE_RESET_Y   = 610;

// ========== 三车道 X 坐标 ==========
constexpr int LANE_0_X = 10;
constexpr int LANE_1_X = 130;
constexpr int LANE_2_X = 240;

// ========== 车道类型 ==========
enum class Lane { L0 = 0, L1 = 1, L2 = 2 };

// ========== 图片路径 ==========
constexpr const char* IMG_ROAD       = "assets/road1.png";
constexpr const char* IMG_CAR        = "assets/Car.png";
constexpr const char* IMG_CAR_OUT    = "assets/Car0.png";
constexpr const char* IMG_OBSTACLE   = "assets/Car2.png";
constexpr const char* IMG_OBSTACLE_OUT = "assets/Car01.png";

// ========== 串口 ==========
constexpr int  COM_BAUD       = 9600;
constexpr int  COM_BITS       = 8;
constexpr int  COM_STOPBITS   = 1;
constexpr int  COM_PARITY     = 0;
constexpr int  COM_TIME_MS    = 5;
constexpr int  COM_RESCAN_MS  = 500;
constexpr int  COM_BUF_SIZE   = 128;
constexpr int  OBSTACLE_FRAME_SIZE = 10;
constexpr int  DATA_OCT       = 10;

constexpr char SOFT_CMD_BEGIN = '%';
constexpr char SOFT_CMD_END   = '$';
constexpr char SOFT_CMD_RET   = '\0';

constexpr int CONTROL_MODE_AUTO   = 0;
constexpr int CONTROL_MODE_MANUAL = 1;
constexpr int MCU_STATUS_AUTO_MODE = 0x04;

// ========== 点是否在矩形内 ==========
inline bool pointInRect(int px, int py, int rx, int ry, int rw, int rh) {
    return (px >= rx && px <= rx + rw && py >= ry && py <= ry + rh);
}

// ========== 根据车道号获取 X 坐标 ==========
inline int laneToX(Lane lane) {
    switch (lane) {
        case Lane::L0: return LANE_0_X;
        case Lane::L1: return LANE_1_X;
        case Lane::L2: return LANE_2_X;
    }
    return LANE_1_X;
}
