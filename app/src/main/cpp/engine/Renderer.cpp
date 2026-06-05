#include "Renderer.h"
#include <GLES3/gl3.h>
#include <android/log.h>
#include <time.h>
#include <cmath>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"MCME",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"MCME",__VA_ARGS__)

static const char* BLOCK_VERT = R"GLSL(
#version 300 es
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
uniform mat4 uVP;
out vec3 vColor;
void main() {
    gl_Position = uVP * vec4(aPos, 1.0);
    vColor = aColor;
}
)GLSL";

static const char* BLOCK_FRAG = R"GLSL(
#version 300 es
precision mediump float;
in vec3 vColor;
uniform vec3 uFogColor;
uniform float uFogStart;
uniform float uFogEnd;
out vec4 fragColor;
void main() {
    float depth = gl_FragCoord.z / gl_FragCoord.w;
    float fog = clamp((depth - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
    vec3 color = mix(vColor, uFogColor, fog);
    fragColor = vec4(color, 1.0);
}
)GLSL";

static const char* SKY_VERT = R"GLSL(
#version 300 es
layout(location = 0) in vec2 aPos;
out vec2 vUV;
void main() {
    gl_Position = vec4(aPos, 0.999, 1.0);
    vUV = aPos * 0.5 + 0.5;
}
)GLSL";

static const char* SKY_FRAG = R"GLSL(
#version 300 es
precision mediump float;
in vec2 vUV;
out vec4 fragColor;
void main() {
    vec3 top    = vec3(0.39f, 0.64f, 0.93f);
    vec3 bottom = vec3(0.70f, 0.84f, 0.98f);
    fragColor = vec4(mix(bottom, top, vUV.y), 1.0);
}
)GLSL";

double Renderer::getTime() const {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void Renderer::initSky() {
    static const float quad[] = {-1,-1, 1,-1, 1,1, -1,-1, 1,1, -1,1};
    glGenVertexArrays(1, &skyVAO); glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void Renderer::drawSky() {
    glDepthMask(GL_FALSE);
    skyShader.use();
    glBindVertexArray(skyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
}

bool Renderer::init(int width, int height) {
    screenW = width; screenH = height;
    glViewport(0, 0, width, height);
    glClearColor(0.39f, 0.64f, 0.93f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    if (!blockShader.compile(BLOCK_VERT, BLOCK_FRAG)) {
        LOGE("Failed to compile block shader"); return false;
    }
    if (!skyShader.compile(SKY_VERT, SKY_FRAG)) {
        LOGE("Failed to compile sky shader"); return false;
    }
    initSky();

    // Spawn player above terrain
    camera.x = 8; camera.z = 8; camera.y = 40;
    lastTime = getTime();
    world.update(camera.x, camera.z);
    LOGI("Renderer init OK %dx%d", width, height);
    return true;
}

void Renderer::resize(int width, int height) {
    screenW = width; screenH = height;
    glViewport(0, 0, width, height);
}

void Renderer::draw() {
    double now = getTime();
    float dt = (float)(now - lastTime);
    if (dt > 0.1f) dt = 0.1f;
    lastTime = now;

    // Gravity
    velY -= 20.0f * dt;
    camera.y += velY * dt;
    // Simple ground collision
    int groundY = 0;
    for (int y = (int)camera.y; y >= 0; y--) {
        if (world.getBlock((int)camera.x, y, (int)camera.z) != BlockType::AIR &&
            world.getBlock((int)camera.x, y, (int)camera.z) != BlockType::WATER) {
            groundY = y + 1; break;
        }
    }
    if (camera.y <= groundY + 1.6f) {
        camera.y = groundY + 1.6f;
        velY = 0; onGround = true;
    } else { onGround = false; }

    // Horizontal movement
    camera.move(joyRight, 0, joyForward, dt);

    // Update world chunks
    world.update(camera.x, camera.z);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = (float)screenW / (float)screenH;
    auto vp = camera.getVPMatrix(aspect);

    // Draw sky
    drawSky();

    // Draw blocks
    blockShader.use();
    blockShader.setMat4("uVP", vp.data());
    blockShader.setVec3("uFogColor", 0.70f, 0.84f, 0.98f);
    blockShader.setFloat("uFogStart", (world.renderDistance - 1) * 16.0f * 0.7f);
    blockShader.setFloat("uFogEnd",   (world.renderDistance) * 16.0f * 0.9f);
    world.render(blockShader);
}

void Renderer::onJoystick(float rightAxis, float forwardAxis) {
    joyRight   = rightAxis;
    joyForward = forwardAxis;
}

void Renderer::onLook(float dyaw, float dpitch) {
    camera.rotate(dyaw, dpitch);
}

void Renderer::onJump() {
    if (onGround) { velY = 8.0f; onGround = false; }
}

void Renderer::destroy() {
    blockShader.destroy(); skyShader.destroy();
    if (skyVAO) { glDeleteVertexArrays(1,&skyVAO); glDeleteBuffers(1,&skyVBO); skyVAO=skyVBO=0; }
}
