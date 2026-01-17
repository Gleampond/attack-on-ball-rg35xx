#pragma once

#include <SDL2/SDL.h>

#include "RenderContext.h"
#include "Assets.h"

inline void DrawTexture(SDL_Renderer* renderer, const RenderContext& ctx, const TextureAsset& asset,
                        float x, float y, float scale_x, float scale_y, SDL_Color color, float alpha = 1.0f) {
    if (!asset.texture) {
        return;
    }
    SDL_SetTextureColorMod(asset.texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(asset.texture, static_cast<Uint8>(alpha * 255.0f));
    SDL_FRect dst{ctx.offset_x + x * ctx.scale,
                 ctx.offset_y + y * ctx.scale,
                 static_cast<float>(asset.width) * scale_x * ctx.scale,
                 static_cast<float>(asset.height) * scale_y * ctx.scale};
    SDL_RenderCopyF(renderer, asset.texture, nullptr, &dst);
}

inline void DrawTextureCentered(SDL_Renderer* renderer, const RenderContext& ctx, const TextureAsset& asset,
                                float x, float y, float scale_x, float scale_y, SDL_Color color, float alpha = 1.0f) {
    if (!asset.texture) {
        return;
    }
    SDL_SetTextureColorMod(asset.texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(asset.texture, static_cast<Uint8>(alpha * 255.0f));
    SDL_FRect dst{ctx.offset_x + (x - static_cast<float>(asset.width) * 0.5f * scale_x) * ctx.scale,
                 ctx.offset_y + (y - static_cast<float>(asset.height) * 0.5f * scale_y) * ctx.scale,
                 static_cast<float>(asset.width) * scale_x * ctx.scale,
                 static_cast<float>(asset.height) * scale_y * ctx.scale};
    SDL_RenderCopyF(renderer, asset.texture, nullptr, &dst);
}

inline void DrawTextureSubrect(SDL_Renderer* renderer, const RenderContext& ctx, const TextureAsset& asset,
                               const SDL_Rect& src, float x, float y, float scale_x, float scale_y,
                               SDL_Color color, float alpha = 1.0f) {
    if (!asset.texture) {
        return;
    }
    SDL_SetTextureColorMod(asset.texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(asset.texture, static_cast<Uint8>(alpha * 255.0f));
    SDL_FRect dst{ctx.offset_x + x * ctx.scale,
                 ctx.offset_y + y * ctx.scale,
                 static_cast<float>(src.w) * scale_x * ctx.scale,
                 static_cast<float>(src.h) * scale_y * ctx.scale};
    SDL_RenderCopyF(renderer, asset.texture, &src, &dst);
}
