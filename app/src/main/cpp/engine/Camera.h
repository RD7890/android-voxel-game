#pragma once
#include <array>
#include <cmath>

class Camera {
public:
    float x = 8.0f, y = 32.0f, z = 8.0f;
    float yaw = 0.0f, pitch = 0.0f;
    float fov = 70.0f;
    float sensitivity = 0.15f;
    float moveSpeed = 6.0f;

    void move(float rightAxis, float upAxis, float forwardAxis, float dt);
    void rotate(float dyaw, float dpitch);

    void getForward(float& fx, float& fy, float& fz) const;
    void getRight(float& rx, float& ry, float& rz) const;

    std::array<float, 16> getViewMatrix() const;
    std::array<float, 16> getProjectionMatrix(float aspect) const;
    std::array<float, 16> getVPMatrix(float aspect) const;
};
