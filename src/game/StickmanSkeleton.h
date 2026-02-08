#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "Assets.h"
#include "RenderContext.h"

class StickmanSkeleton {
public:
    struct AtlasRegion {
        SDL_Rect src{0, 0, 0, 0};
        int orig_w = 0;
        int orig_h = 0;
        int offset_x = 0;
        int offset_y = 0;
        bool rotated = false;
    };

    bool Load(const std::string& json_path, const std::string& atlas_path);
    bool IsLoaded() const { return loaded_; }
    bool SetAnimation(const std::string& name, bool loop = true, bool restart = true);
    bool HasAnimation(const std::string& name) const;
    void Update(float delta_seconds);
    const std::string& CurrentAnimation() const { return current_animation_; }

    void Draw(SDL_Renderer* renderer, const RenderContext& ctx, const TextureAsset& texture,
              float x, float y, float scale, SDL_Color color, bool flip_x = false) const;

private:
    struct Bone {
        std::string name;
        int parent_index = -1;
        float x = 0.0f;
        float y = 0.0f;
        float rotation = 0.0f;
        float scale_x = 1.0f;
        float scale_y = 1.0f;
    };

    struct BonePose {
        // 2x2 world matrix (no translation): [a b; c d]
        float a = 1.0f;
        float b = 0.0f;
        float c = 0.0f;
        float d = 1.0f;
        float world_x = 0.0f;
        float world_y = 0.0f;
    };

    struct Slot {
        std::string name;
        int bone_index = -1;
        std::string attachment_name;
    };

    struct Attachment {
        std::string name;
        std::string slot_name;
        float x = 0.0f;
        float y = 0.0f;
        float rotation = 0.0f;
        float scale_x = 1.0f;
        float scale_y = 1.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    struct FloatKeyframe {
        float time = 0.0f;
        float value = 0.0f;
        bool stepped = false;
    };

    struct Vec2Keyframe {
        float time = 0.0f;
        float x = 0.0f;
        float y = 0.0f;
        bool stepped = false;
    };

    struct AttachmentKeyframe {
        float time = 0.0f;
        std::string name;
    };

    struct Animation {
        float duration = 0.0f;
        std::unordered_map<int, std::vector<FloatKeyframe>> bone_rotate;
        std::unordered_map<int, std::vector<Vec2Keyframe>> bone_translate;
        std::unordered_map<int, std::vector<Vec2Keyframe>> bone_scale;
        std::unordered_map<int, std::vector<AttachmentKeyframe>> slot_attachment;
    };

    bool loaded_ = false;
    std::vector<Bone> bones_;
    std::vector<Slot> slots_;
    std::unordered_map<std::string, Attachment> attachments_;
    std::unordered_map<std::string, AtlasRegion> regions_;
    std::unordered_map<std::string, Animation> animations_;
    std::string current_animation_;
    float animation_time_ = 0.0f;
    bool animation_loop_ = true;

    const Attachment* FindAttachment(const Slot& slot) const;
    const AtlasRegion* FindRegion(const std::string& name) const;

    static std::string MakeAttachmentKey(const std::string& slot_name, const std::string& attachment_name);
};
