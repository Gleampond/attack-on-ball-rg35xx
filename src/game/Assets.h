#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <unordered_map>

#include "BitmapFont.h"

struct TextureAsset {
    SDL_Texture* texture = nullptr;
    int width = 0;
    int height = 0;
};

class Assets {
public:
    bool Init(SDL_Renderer* renderer);
    void Shutdown();

    bool LoadTexture(const std::string& key, const std::string& path);
    TextureAsset GetTexture(const std::string& key) const;

    bool LoadFont(const std::string& key, const std::string& image_path, const std::string& xml_path);
    const BitmapFont* GetFont(const std::string& key) const;

    bool LoadSound(const std::string& key, const std::string& path);
    Mix_Chunk* GetSound(const std::string& key) const;
    void PlaySound(const std::string& key, int volume = MIX_MAX_VOLUME);

private:
    SDL_Renderer* renderer_ = nullptr;
    std::unordered_map<std::string, TextureAsset> textures_;
    std::unordered_map<std::string, BitmapFont> fonts_;
    std::unordered_map<std::string, Mix_Chunk*> sounds_;

    void FreeTextures();
    void FreeSounds();
    void FreeFonts();
};
