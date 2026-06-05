#include "Camera.h"
#include <cstring>
#include <algorithm>

static constexpr float PI      = 3.14159265358979f;
static constexpr float DEG2RAD = PI / 180.0f;

void Camera::getForward(float& fx, float& fy, float& fz) const {
    float yr = yaw * DEG2RAD, pr = pitch * DEG2RAD;
    fx =  cosf(pr) * sinf(yr);
    fy = -sinf(pr);
    fz = -cosf(pr) * cosf(yr);
}

void Camera::getRight(float& rx, float& ry, float& rz) const {
    float yr = yaw * DEG2RAD;
    rx = cosf(yr); ry = 0.0f; rz = sinf(yr);
}

void Camera::move(float rightAxis, float upAxis, float forwardAxis, float dt) {
    float fx, fy, fz, rx, ry, rz;
    getForward(fx, fy, fz);
    getRight(rx, ry, rz);
    float spd = moveSpeed * dt;
    // Horizontal only for joystick
    x += (fx * forwardAxis + rx * rightAxis) * spd;
    z += (fz * forwardAxis + rz * rightAxis) * spd;
    y += upAxis * spd;
}

void Camera::rotate(float dyaw, float dpitch) {
    yaw   += dyaw   * sensitivity;
    pitch += dpitch * sensitivity;
    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

static void matMul(const float* a, const float* b, float* out) {
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++) {
            float s = 0;
            for (int k = 0; k < 4; k++) s += a[k*4+row] * b[col*4+k];
            out[col*4+row] = s;
        }
}

std::array<float,16> Camera::getViewMatrix() const {
    float fx, fy, fz, rx, ry, rz;
    getForward(fx, fy, fz); getRight(rx, ry, rz);
    // up = right × forward (then normalize)
    float ux = ry*fz - rz*fy, uy = rz*fx - rx*fz, uz = rx*fy - ry*fx;
    float ul = sqrtf(ux*ux + uy*uy + uz*uz);
    if (ul > 1e-5f) { ux/=ul; uy/=ul; uz/=ul; }
    return {
        rx, ux, -fx, 0,
        ry, uy, -fy, 0,
        rz, uz, -fz, 0,
        -(rx*x+ry*y+rz*z), -(ux*x+uy*y+uz*z), fx*x+fy*y+fz*z, 1
    };
}

std::array<float,16> Camera::getProjectionMatrix(float aspect) const {
    float f = 1.0f / tanf(fov * DEG2RAD * 0.5f);
    float n = 0.1f, fa = 500.0f;
    std::array<float,16> m{};
    m[0]=f/aspect; m[5]=f;
    m[10]=(fa+n)/(n-fa); m[11]=-1.0f;
    m[14]=(2*fa*n)/(n-fa);
    return m;
}

std::array<float,16> Camera::getVPMatrix(float aspect) const {
    auto v = getViewMatrix(), p = getProjectionMatrix(aspect);
    std::array<float,16> r;
    matMul(p.data(), v.data(), r.data());
    return r;
}
