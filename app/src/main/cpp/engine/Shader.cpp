#include "Shader.h"
#include <android/log.h>
#define TAG "MCME"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

GLuint Shader::compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512]; glGetShaderInfoLog(s, 512, nullptr, log);
        LOGE("Shader compile error: %s", log);
        glDeleteShader(s); return 0;
    }
    return s;
}

bool Shader::compile(const char* vertSrc, const char* fragSrc) {
    GLuint v = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (!v || !f) return false;
    program = glCreateProgram();
    glAttachShader(program, v); glAttachShader(program, f);
    glLinkProgram(program);
    GLint ok; glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512]; glGetProgramInfoLog(program, 512, nullptr, log);
        LOGE("Shader link error: %s", log);
        glDeleteProgram(program); program = 0;
    }
    glDeleteShader(v); glDeleteShader(f);
    return ok;
}

void Shader::use() const { glUseProgram(program); }
void Shader::destroy() { if (program) { glDeleteProgram(program); program = 0; } }

void Shader::setMat4(const char* n, const float* m) const {
    glUniformMatrix4fv(glGetUniformLocation(program, n), 1, GL_FALSE, m);
}
void Shader::setVec3(const char* n, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(program, n), x, y, z);
}
void Shader::setFloat(const char* n, float v) const {
    glUniform1f(glGetUniformLocation(program, n), v);
}
void Shader::setInt(const char* n, int v) const {
    glUniform1i(glGetUniformLocation(program, n), v);
}
