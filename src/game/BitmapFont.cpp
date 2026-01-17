#include "BitmapFont.h"

#include <SDL2/SDL_image.h>
#include <fstream>
#include <sstream>

bool BitmapFont::ParseCharAttribute(const std::string& line, const std::string& key, int* value) {
    const std::string token = key + "=\"";
    const std::size_t start = line.find(token);
    if (start == std::string::npos) {
        return false;
    }
    const std::size_t value_start = start + token.size();
    const std::size_t value_end = line.find('"', value_start);
    if (value_end == std::string::npos) {
        return false;
    }
    *value = std::stoi(line.substr(value_start, value_end - value_start));
    return true;
}

bool BitmapFont::Load(SDL_Renderer* renderer, const std::string& image_path, const std::string& xml_path) {
    Unload();

    SDL_Surface* surface = IMG_Load(image_path.c_str());
    if (!surface) {
        return false;
    }
    texture_ = SDL_CreateTextureFromSurface(renderer, surface);
    texture_w_ = surface->w;
    texture_h_ = surface->h;
    SDL_FreeSurface(surface);
    if (!texture_) {
        return false;
    }

    std::ifstream file(xml_path);
    if (!file.is_open()) {
        Unload();
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("lineHeight") != std::string::npos) {
            ParseCharAttribute(line, "lineHeight", &line_height_);
        }
        if (line.find("<char ") == std::string::npos) {
            continue;
        }
        int id = 0;
        Glyph glyph;
        if (!ParseCharAttribute(line, "id", &id)) {
            continue;
        }
        ParseCharAttribute(line, "x", &glyph.src.x);
        ParseCharAttribute(line, "y", &glyph.src.y);
        ParseCharAttribute(line, "width", &glyph.src.w);
        ParseCharAttribute(line, "height", &glyph.src.h);
        ParseCharAttribute(line, "xoffset", &glyph.xoffset);
        ParseCharAttribute(line, "yoffset", &glyph.yoffset);
        ParseCharAttribute(line, "xadvance", &glyph.xadvance);
        glyphs_[id] = glyph;
    }

    return true;
}

void BitmapFont::Unload() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    texture_w_ = 0;
    texture_h_ = 0;
    line_height_ = 0;
    glyphs_.clear();
}

void BitmapFont::Draw(SDL_Renderer* renderer, const RenderContext& ctx, const std::string& text, float x, float y, float scale,
                      SDL_Color color, int* out_width) const {
    if (!texture_) {
        if (out_width) {
            *out_width = 0;
        }
        return;
    }

    SDL_SetTextureColorMod(texture_, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture_, color.a);

    float cursor = x;
    for (char ch : text) {
        auto it = glyphs_.find(static_cast<int>(ch));
        if (it == glyphs_.end()) {
            cursor += 8.0f * scale;
            continue;
        }
        const Glyph& glyph = it->second;
        SDL_FRect dst{
            ctx.offset_x + (cursor + static_cast<float>(glyph.xoffset) * scale) * ctx.scale,
            ctx.offset_y + (y + static_cast<float>(glyph.yoffset) * scale) * ctx.scale,
            static_cast<float>(glyph.src.w) * scale * ctx.scale,
            static_cast<float>(glyph.src.h) * scale * ctx.scale};
        SDL_RenderCopyF(renderer, texture_, &glyph.src, &dst);
        cursor += static_cast<float>(glyph.xadvance) * scale;
    }

    if (out_width) {
        *out_width = static_cast<int>(cursor - x);
    }
}
