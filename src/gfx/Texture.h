#pragma once
#include <string>

class Texture {
public:
    Texture() = default;
    ~Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    bool load(const std::string& path);   // PNG sa alfa kanalom
    void bind(unsigned unit = 0) const;
    int width() const { return w_; }
    int height() const { return h_; }

private:
    unsigned id_ = 0;
    int w_ = 0, h_ = 0;
};
