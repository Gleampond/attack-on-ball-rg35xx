#include "Assets.h"

#include <SDL2/SDL_image.h>

bool Assets::Init(SDL_Renderer* renderer) {
    renderer_ = renderer;
    return true;
}

void Assets::Shutdown() {
    FreeFonts();
    FreeSounds();
    FreeTextures();
    renderer_ = nullptr;
}

bool Assets::LoadTexture(const std::string& key, const std::string& path) {
    if (!renderer_) {
        return false;
    }
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        return false;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    TextureAsset asset;
    asset.texture = texture;
    asset.width = surface->w;
    asset.height = surface->h;
    SDL_FreeSurface(surface);
    if (!texture) {
        return false;
    }
    textures_[key] = asset;
    return true;
}

TextureAsset Assets::GetTexture(const std::string& key) const {
    auto it = textures_.find(key);
    if (it == textures_.end()) {
        return {};
    }
    return it->second;
}

bool Assets::LoadFont(const std::string& key, const std::string& image_path, const std::string& xml_path) {
    BitmapFont font;
    if (!font.Load(renderer_, image_path, xml_path)) {
        return false;
    }
    fonts_[key] = std::move(font);
    return true;
}

const BitmapFont* Assets::GetFont(const std::string& key) const {
    auto it = fonts_.find(key);
    if (it == fonts_.end()) {
        return nullptr;
    }
    return &it->second;
}

bool Assets::LoadSound(const std::string& key, const std::string& path) {
    Mix_Chunk* chunk = Mix_LoadWAV(path.c_str());
    if (!chunk) {
        return false;
    }
    sounds_[key] = chunk;
    return true;
}

Mix_Chunk* Assets::GetSound(const std::string& key) const {
    auto it = sounds_.find(key);
    if (it == sounds_.end()) {
        return nullptr;
    }
    return it->second;
}

void Assets::PlaySound(const std::string& key, int volume) {
    Mix_Chunk* chunk = GetSound(key);
    if (!chunk) {
        return;
    }
    Mix_VolumeChunk(chunk, volume);
    Mix_PlayChannel(-1, chunk, 0);
}

void Assets::FreeTextures() {
    for (auto& entry : textures_) {
        if (entry.second.texture) {
            SDL_DestroyTexture(entry.second.texture);
            entry.second.texture = nullptr;
        }
    }
    textures_.clear();
}

void Assets::FreeFonts() {
    for (auto& entry : fonts_) {
        entry.second.Unload();
    }
    fonts_.clear();
}

void Assets::FreeSounds() {
    for (auto& entry : sounds_) {
        Mix_FreeChunk(entry.second);
    }
    sounds_.clear();
}
