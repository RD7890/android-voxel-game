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
    vec3 top    = vec3(0.39, 0.64, 0.93);
    vec3 bottom = vec3(0.70, 0.84, 0.98);
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

    // Spawn player above terrain center
    camera.x = 8.0f; camera.z = 8.0f; camera.y = 60.0f;
    camera.pitch = -20.0f; // Look slightly down on spawn
    lastTime = getTime();

    // Generate initial chunks so we can find ground
    world.update(camera.x, camera.z);

    // Place player on actual terrain surface
    int spawnH = world.getSpawnHeight((int)camera.x, (int)camera.z);
    camera.y = (float)spawnH + 1.62f;

    LOGI("Renderer init OK %dx%d  spawnY=%d  cameraY=%.1f", width, height, spawnH, camera.y);
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

    // Find ground beneath player
    int groundY = 0;
    int px = (int)camera.x, pz = (int)camera.z;
    for (int y = (int)(camera.y + 1); y >= 0; y--) {
        BlockType bt = world.getBlock(px, y, pz);
        if (bt != BlockType::AIR && bt != BlockType::WATER) {
            groundY = y + 1; break;
        }
    }
    float minY = (float)groundY + 1.62f;
    if (camera.y < minY) {
        camera.y = minY;
        velY = 0.0f;
        onGround = true;
    } else {
        onGround = false;
    }

    // Horizontal movement
    camera.move(joyRight, 0, joyForward, dt);

    // Update world chunks
    world.update(camera.x, camera.z);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float aspect = (float)screenW / (float)screenH;
    auto vp = camera.getVPMatrix(aspect);

    drawSky();

    blockShader.use();
    blockShader.setMat4("uVP", vp.data());
    blockShader.setVec3("uFogColor", 0.70f, 0.84f, 0.98f);
    float fogStart = (world.renderDistance - 1) * 16.0f * 0.6f;
    float fogEnd   = world.renderDistance * 16.0f * 0.85f;
    blockShader.setFloat("uFogStart", fogStart);
    blockShader.setFloat("uFogEnd",   fogEnd);
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
    if (onGround) { velY = 9.0f; onGround = false; }
}

RaycastResult Renderer::getTarget() const {
    float fx, fy, fz;
    camera.getForward(fx, fy, fz);
    return world.raycast(camera.x, camera.y, camera.z, fx, fy, fz, 5.0f);
}

void Renderer::breakBlock() {
    auto res = getTarget();
    if (res.hit) {
        world.setBlockWorld(res.bx, res.by, res.bz, BlockType::AIR);
        LOGI("Break block at %d %d %d", res.bx, res.by, res.bz);
    }
}

void Renderer::placeBlock(int blockType) {
    if (blockType <= 0 || blockType > 8) return;
    auto res = getTarget();
    if (!res.hit) return;
    int bpx = res.bx + res.nx;
    int bpy = res.by + res.ny;
    int bpz = res.bz + res.nz;
    if (bpy < 0 || bpy >= CHUNK_HEIGHT) return;
    // Don't place inside player body
    float feetY = camera.y - 1.62f;
    float eyeY  = camera.y + 0.3f;
    int ipx = (int)floorf(camera.x);
    int ipz = (int)floorf(camera.z);
    if (bpx == ipx && bpz == ipz && (float)bpy >= feetY && (float)bpy <= eyeY) return;
    world.setBlockWorld(bpx, bpy, bpz, (BlockType)blockType);
    LOGI("Place block %d at %d %d %d", blockType, bpx, bpy, bpz);
}

void Renderer::destroy() {
    blockShader.destroy(); skyShader.destroy();
    if (skyVAO) { glDeleteVertexArrays(1,&skyVAO); glDeleteBuffers(1,&skyVBO); skyVAO=skyVBO=0; }
}
