#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    Shader() = default;
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool load(const std::string& vertPath, const std::string& fragPath);
    void use() const;

    void set(const char* name, int v);
    void set(const char* name, float v);
    void set(const char* name, bool v);
    void set(const char* name, const glm::vec2& v);
    void set(const char* name, const glm::vec3& v);
    void set(const char* name, const glm::vec4& v);
    void set(const char* name, const glm::mat4& v);

private:
    int loc(const char* name);
    unsigned id_ = 0;
    std::unordered_map<std::string, int> cache_;
};
