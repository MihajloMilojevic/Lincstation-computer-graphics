#include "Shader.h"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

static bool readFile(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) { std::cerr << "Cannot open file: " << path << "\n"; return false; }
    std::stringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

static unsigned compile(unsigned type, const std::string& src, const std::string& name) {
    unsigned s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    int ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof log, nullptr, log);
        std::cerr << "Shader compile error (" << name << "):\n" << log << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

Shader::~Shader() {
    if (id_) glDeleteProgram(id_);
}

bool Shader::load(const std::string& vertPath, const std::string& fragPath) {
    std::string vs, fs;
    if (!readFile(vertPath, vs) || !readFile(fragPath, fs)) return false;
    unsigned v = compile(GL_VERTEX_SHADER, vs, vertPath);
    unsigned f = compile(GL_FRAGMENT_SHADER, fs, fragPath);
    if (!v || !f) return false;

    id_ = glCreateProgram();
    glAttachShader(id_, v);
    glAttachShader(id_, f);
    glLinkProgram(id_);
    glDeleteShader(v);
    glDeleteShader(f);

    int ok = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(id_, sizeof log, nullptr, log);
        std::cerr << "Shader link error:\n" << log << "\n";
        return false;
    }
    return true;
}

void Shader::use() const { glUseProgram(id_); }

int Shader::loc(const char* name) {
    auto it = cache_.find(name);
    if (it != cache_.end()) return it->second;
    int l = glGetUniformLocation(id_, name);
    cache_[name] = l;
    return l;
}

void Shader::set(const char* n, int v)              { glUniform1i(loc(n), v); }
void Shader::set(const char* n, float v)            { glUniform1f(loc(n), v); }
void Shader::set(const char* n, bool v)             { glUniform1i(loc(n), v ? 1 : 0); }
void Shader::set(const char* n, const glm::vec2& v) { glUniform2fv(loc(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::vec3& v) { glUniform3fv(loc(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::vec4& v) { glUniform4fv(loc(n), 1, glm::value_ptr(v)); }
void Shader::set(const char* n, const glm::mat4& v) { glUniformMatrix4fv(loc(n), 1, GL_FALSE, glm::value_ptr(v)); }
