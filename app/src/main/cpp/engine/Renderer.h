#pragma once
#include "Shader.h"
#include "Camera.h"
#include "World.h"

class Renderer {
public:
    Camera camera;
    World  world;

    bool init(int width, int height);
    void resize(int width, int height);
    void draw();
    void destroy();

    void onJoystick(float rightAxis, float forwardAxis);
    void onLook(float dyaw, float dpitch);
    void onJump();
    void setSensitivity(float s) { camera.sensitivity = s; }
    void setRenderDistance(int d) { world.setRenderDistance(d); }

    void breakBlock();
    void placeBlock(int blockType);
    RaycastResult getTarget() const;

private:
    Shader blockShader;
    Shader skyShader;
    int    screenW = 0, screenH = 0;

    GLuint skyVAO = 0, skyVBO = 0;

    float joyRight = 0, joyForward = 0;
    float velY = 0.0f;
    bool  onGround = false;
    double lastTime = 0.0;

    void initSky();
    void drawSky();
    double getTime() const;
};
