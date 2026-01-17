#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>

#include "RenderContext.h"

struct Glyph {
    SDL_Rect src;
    int xoffset = 0;
    int yoffset = 0;
    int xadvance = 0;
};

class BitmapFont {
public:
    bool Load(SDL_Renderer* renderer, const std::string& image_path, const std::string& xml_path);
    void Unload();

    void Draw(SDL_Renderer* renderer, const RenderContext& ctx, const std::string& text, float x, float y, float scale,
              SDL_Color color, int* out_width = nullptr) const;

    int LineHeight() const { return line_height_; }

private:
    SDL_Texture* texture_ = nullptr;
    int texture_w_ = 0;
    int texture_h_ = 0;
    int line_height_ = 0;
    std::unordered_map<int, Glyph> glyphs_;

    static bool ParseCharAttribute(const std::string& line, const std::string& key, int* value);
};
