#pragma once

#include "math3d.hpp"
#include <SDL3/SDL.h>

namespace dong::utils {

/**
 * 第一人称相机控制器
 * 
 * 功能：
 * - 右键按住：控制视角（鼠标移动）
 * - WASD：前后左右移动
 * - QE：上下移动
 * - 空格：上升
 * - Shift：加速
 */
class FPSCamera {
public:
    // 相机位置
    Vec3 position{0, 1.6f, 5};
    
    // 欧拉角（弧度）
    float yaw = -3.14159f;    // 水平旋转（绕 Y 轴）
    float pitch = 0.0f;       // 垂直旋转（绕 X 轴）
    
    // 移动参数
    float move_speed = 5.0f;
    float mouse_sensitivity = 0.01f;
    float sprint_multiplier = 2.0f;
    
    // 视角参数
    float fov = 60.0f;        // 视场角（度）
    float near_plane = 0.1f;
    float far_plane = 1000.0f;

    // 限制
    float pitch_limit = 1.5f; // 约 85 度

    FPSCamera() = default;

    // 获取前方向
    Vec3 getForward() const {
        return Vec3{
            std::cos(pitch) * std::sin(yaw),
            std::sin(pitch),
            std::cos(pitch) * std::cos(yaw)
        };
    }

    // 获取右方向（水平面上）
    Vec3 getRight() const {
        // 右向量 = 前向量绕Y轴顺时针旋转90度
        // 前向量: {sin(yaw), 0, cos(yaw)}
        // 右向量: {cos(yaw), 0, -sin(yaw)} 但鼠标yaw是反向的
        // 所以实际右向量需要取反
        return Vec3{
            -std::cos(yaw),
            0,
            std::sin(yaw)
        };
    }

    // 获取上方向
    Vec3 getUp() const {
        return Vec3{0, 1, 0};
    }

    // 获取视图矩阵
    Mat4 getViewMatrix() const {
        Vec3 target = position + getForward();
        return Mat4::lookAt(position, target, getUp());
    }

    // 获取投影矩阵
    Mat4 getProjectionMatrix(float aspect) const {
        return Mat4::perspective(radians(fov), aspect, near_plane, far_plane);
    }

    // 获取 VP 矩阵
    Mat4 getViewProjectionMatrix(float aspect) const {
        return getProjectionMatrix(aspect) * getViewMatrix();
    }

    // 从屏幕坐标生成射线（screen_x, screen_y 范围为 [-1, 1]）
    Ray screenToRay(float screen_x, float screen_y, float aspect) const {
        // 计算射线方向
        float half_fov = radians(fov * 0.5f);
        float tan_fov = std::tan(half_fov);
        
        Vec3 forward = getForward();
        Vec3 right = getRight();
        Vec3 up = right.cross(forward).normalized();

        // 射线方向 = forward + right * x * tan_fov * aspect + up * y * tan_fov
        Vec3 dir = forward + right * (screen_x * tan_fov * aspect) + up * (screen_y * tan_fov);
        
        return Ray{position, dir.normalized()};
    }

    // 从像素坐标生成射线
    Ray pixelToRay(int px, int py, int screen_width, int screen_height) const {
        float x = (2.0f * px / screen_width - 1.0f);
        float y = (1.0f - 2.0f * py / screen_height);  // Y 轴翻转
        float aspect = static_cast<float>(screen_width) / screen_height;
        return screenToRay(x, y, aspect);
    }

    // 更新相机状态（每帧调用）
    void update(float delta_time, const bool* keys, bool right_mouse_down, 
                int mouse_delta_x, int mouse_delta_y) {
        // 视角控制（右键按住时）
        if (right_mouse_down) {
            yaw -= mouse_delta_x * mouse_sensitivity;
            pitch -= mouse_delta_y * mouse_sensitivity;
            pitch = clamp(pitch, -pitch_limit, pitch_limit);
        }

        // 移动控制
        float speed = move_speed * delta_time;
        if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]) {
            speed *= sprint_multiplier;
        }

        Vec3 forward_flat{std::sin(yaw), 0, std::cos(yaw)};  // 水平面上的前方向
        Vec3 right = getRight();

        Vec3 move{0, 0, 0};
        if (keys[SDL_SCANCODE_W]) move += forward_flat;
        if (keys[SDL_SCANCODE_S]) move -= forward_flat;
        if (keys[SDL_SCANCODE_D]) move += right;
        if (keys[SDL_SCANCODE_A]) move -= right;
        if (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_E]) move.y += 1;
        if (keys[SDL_SCANCODE_Q] || keys[SDL_SCANCODE_LCTRL]) move.y -= 1;

        if (move.lengthSquared() > 0.001f) {
            position += move.normalized() * speed;
        }
    }
};

/**
 * 简化的输入状态管理
 */
struct InputState {
    bool keys[SDL_SCANCODE_COUNT] = {false};
    int mouse_x = 0, mouse_y = 0;
    int mouse_delta_x = 0, mouse_delta_y = 0;
    bool left_mouse_down = false;
    bool right_mouse_down = false;
    bool middle_mouse_down = false;
    bool left_mouse_pressed = false;   // 本帧按下
    bool left_mouse_released = false;  // 本帧释放
    float wheel_delta = 0.0f;
    float mouse_wheel_x = 0.0f;
    float mouse_wheel_y = 0.0f;

    void handleEvent(const SDL_Event& event) {
        switch (event.type) {
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    keys[event.key.scancode] = true;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if (event.key.scancode < SDL_SCANCODE_COUNT) {
                    keys[event.key.scancode] = false;
                }
                break;
            case SDL_EVENT_MOUSE_MOTION:
                mouse_delta_x = static_cast<int>(event.motion.xrel);
                mouse_delta_y = static_cast<int>(event.motion.yrel);
                mouse_x = static_cast<int>(event.motion.x);
                mouse_y = static_cast<int>(event.motion.y);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    left_mouse_down = true;
                    left_mouse_pressed = true;
                }
                if (event.button.button == SDL_BUTTON_RIGHT) right_mouse_down = true;
                if (event.button.button == SDL_BUTTON_MIDDLE) middle_mouse_down = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    left_mouse_down = false;
                    left_mouse_released = true;
                }
                if (event.button.button == SDL_BUTTON_RIGHT) right_mouse_down = false;
                if (event.button.button == SDL_BUTTON_MIDDLE) middle_mouse_down = false;
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                wheel_delta = event.wheel.y;
                mouse_wheel_x = event.wheel.x;
                mouse_wheel_y = event.wheel.y;
                break;
            default:
                break;
        }
    }

    void resetFrameState() {
        mouse_delta_x = 0;
        mouse_delta_y = 0;
        wheel_delta = 0.0f;
        mouse_wheel_x = 0.0f;
        mouse_wheel_y = 0.0f;
        left_mouse_pressed = false;
        left_mouse_released = false;
    }
};

} // namespace dong::utils
