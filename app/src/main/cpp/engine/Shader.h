#pragma once
#include <GLES3/gl3.h>
#include <string>

class Shader {
public:
    GLuint program = 0;

    bool compile(const char* vertSrc, const char* fragSrc);
    void use() const;
    void destroy();

    void setMat4(const char* name, const float* mat) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setFloat(const char* name, float v) const;
    void setInt(const char* name, int v) const;

private:
    GLuint compileShader(GLenum type, const char* src);
};
