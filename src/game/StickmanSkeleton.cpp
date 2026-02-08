#include "StickmanSkeleton.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>

#include "Json.h"

namespace {
constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;
constexpr float kYFlip = -1.0f;
constexpr float kLegSlotYOffset = 9.0f;
constexpr float kHandSlotYOffset = 3.0f;
constexpr float kHandBackXOffset = 3.0f;

float GetNumber(const JsonValue& obj, const std::string& key, float fallback) {
    const JsonValue* value = obj.Get(key);
    if (!value || !value->IsNumber()) {
        return fallback;
    }
    return static_cast<float>(value->number_value);
}

std::string GetString(const JsonValue& obj, const std::string& key, const std::string& fallback) {
    const JsonValue* value = obj.Get(key);
    if (!value || !value->IsString()) {
        return fallback;
    }
    return value->string_value;
}

bool IsSteppedFrame(const JsonValue& keyframe_obj) {
    const JsonValue* curve = keyframe_obj.Get("curve");
    if (!curve) {
        return false;
    }
    return curve->IsString() && curve->string_value == "stepped";
}

bool IsLegSlot(const std::string& slot_name) {
    return slot_name == "Lag" || slot_name == "Lag2";
}

bool IsHandSlot(const std::string& slot_name) {
    return slot_name == "Hand" || slot_name == "Hand2";
}

bool ReadFile(const std::string& path, std::string* out_contents) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    *out_contents = buffer.str();
    return true;
}

std::string Trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        start++;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        end--;
    }
    return value.substr(start, end - start);
}

bool ParseAtlas(const std::string& path, std::unordered_map<std::string, StickmanSkeleton::AtlasRegion>* regions) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    std::string line;
    std::string current_name;
    bool in_regions = false;
    StickmanSkeleton::AtlasRegion current_region;

    auto CommitRegion = [&]() {
        if (!current_name.empty()) {
            (*regions)[current_name] = current_region;
        }
    };

    while (std::getline(file, line)) {
        const std::string trimmed = Trim(line);
        if (trimmed.empty()) {
            continue;
        }
        if (trimmed.find(':') == std::string::npos && trimmed.find("Stickman.png") == std::string::npos) {
            CommitRegion();
            current_name = trimmed;
            current_region = StickmanSkeleton::AtlasRegion{};
            in_regions = true;
            continue;
        }
        if (!in_regions) {
            continue;
        }
        const size_t colon = trimmed.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        const std::string key = Trim(trimmed.substr(0, colon));
        const std::string value = Trim(trimmed.substr(colon + 1));
        if (key == "rotate") {
            current_region.rotated = (value == "true");
        } else if (key == "xy") {
            std::sscanf(value.c_str(), "%d,%d", &current_region.src.x, &current_region.src.y);
        } else if (key == "size") {
            std::sscanf(value.c_str(), "%d,%d", &current_region.src.w, &current_region.src.h);
        } else if (key == "orig") {
            std::sscanf(value.c_str(), "%d,%d", &current_region.orig_w, &current_region.orig_h);
        } else if (key == "offset") {
            std::sscanf(value.c_str(), "%d,%d", &current_region.offset_x, &current_region.offset_y);
        }
    }
    CommitRegion();
    return true;
}
}  // namespace

std::string StickmanSkeleton::MakeAttachmentKey(const std::string& slot_name, const std::string& attachment_name) {
    return slot_name + "::" + attachment_name;
}

const StickmanSkeleton::Attachment* StickmanSkeleton::FindAttachment(const Slot& slot) const {
    const std::string key = MakeAttachmentKey(slot.name, slot.attachment_name);
    auto it = attachments_.find(key);
    if (it == attachments_.end()) {
        return nullptr;
    }
    return &it->second;
}

const StickmanSkeleton::AtlasRegion* StickmanSkeleton::FindRegion(const std::string& name) const {
    auto it = regions_.find(name);
    if (it == regions_.end()) {
        return nullptr;
    }
    return &it->second;
}

bool StickmanSkeleton::HasAnimation(const std::string& name) const {
    return animations_.find(name) != animations_.end();
}

bool StickmanSkeleton::SetAnimation(const std::string& name, bool loop, bool restart) {
    auto it = animations_.find(name);
    if (it == animations_.end()) {
        return false;
    }
    if (restart || current_animation_ != name) {
        animation_time_ = 0.0f;
    }
    current_animation_ = name;
    animation_loop_ = loop;
    return true;
}

void StickmanSkeleton::Update(float delta_seconds) {
    if (current_animation_.empty()) {
        return;
    }
    auto it = animations_.find(current_animation_);
    if (it == animations_.end()) {
        return;
    }
    const float duration = it->second.duration;
    if (duration <= 0.0f) {
        animation_time_ = 0.0f;
        return;
    }
    animation_time_ += delta_seconds;
    if (animation_loop_) {
        animation_time_ = std::fmod(animation_time_, duration);
        if (animation_time_ < 0.0f) {
            animation_time_ += duration;
        }
    } else if (animation_time_ > duration) {
        animation_time_ = duration;
    }
}

bool StickmanSkeleton::Load(const std::string& json_path, const std::string& atlas_path) {
    std::string json_text;
    if (!ReadFile(json_path, &json_text)) {
        return false;
    }
    JsonValue root;
    std::string error;
    if (!ParseJson(json_text, &root, &error)) {
        return false;
    }
    const JsonValue* bones = root.Get("bones");
    const JsonValue* slots = root.Get("slots");
    const JsonValue* skins = root.Get("skins");
    if (!bones || !bones->IsArray() || !slots || !slots->IsArray() || !skins || !skins->IsObject()) {
        return false;
    }

    bones_.clear();
    slots_.clear();
    attachments_.clear();
    regions_.clear();
    animations_.clear();
    current_animation_.clear();
    animation_time_ = 0.0f;
    animation_loop_ = true;

    std::unordered_map<std::string, int> bone_indices;
    std::vector<std::string> bone_parents;
    for (const auto& bone_value : bones->array_value) {
        if (!bone_value.IsObject()) {
            continue;
        }
        Bone bone;
        bone.name = GetString(bone_value, "name", "");
        const std::string parent_name = GetString(bone_value, "parent", "");
        bone.x = GetNumber(bone_value, "x", 0.0f);
        bone.y = GetNumber(bone_value, "y", 0.0f) * kYFlip;
        bone.rotation = GetNumber(bone_value, "rotation", 0.0f) * kYFlip;
        bone.scale_x = GetNumber(bone_value, "scaleX", 1.0f);
        bone.scale_y = GetNumber(bone_value, "scaleY", 1.0f);
        bone.parent_index = -1;
        const int index = static_cast<int>(bones_.size());
        bones_.push_back(bone);
        bone_parents.push_back(parent_name);
        bone_indices[bone.name] = index;
    }

    for (size_t i = 0; i < bone_parents.size(); ++i) {
        const std::string& parent_name = bone_parents[i];
        if (parent_name.empty()) {
            continue;
        }
        auto it = bone_indices.find(parent_name);
        if (it != bone_indices.end()) {
            bones_[i].parent_index = it->second;
        }
    }

    for (const auto& slot_value : slots->array_value) {
        if (!slot_value.IsObject()) {
            continue;
        }
        Slot slot;
        slot.name = GetString(slot_value, "name", "");
        const std::string bone_name = GetString(slot_value, "bone", "");
        slot.attachment_name = GetString(slot_value, "attachment", "");
        auto it = bone_indices.find(bone_name);
        if (it != bone_indices.end()) {
            slot.bone_index = it->second;
        }
        slots_.push_back(slot);
    }
    std::unordered_map<std::string, int> slot_indices;
    for (size_t i = 0; i < slots_.size(); ++i) {
        slot_indices[slots_[i].name] = static_cast<int>(i);
    }

    const JsonValue* default_skin = skins->Get("default");
    if (default_skin && default_skin->IsObject()) {
        for (const auto& slot_entry : default_skin->object_value) {
            const std::string& slot_name = slot_entry.first;
            const JsonValue& attachments = slot_entry.second;
            if (!attachments.IsObject()) {
                continue;
            }
            for (const auto& attachment_entry : attachments.object_value) {
                const std::string& attachment_name = attachment_entry.first;
                const JsonValue& attachment_value = attachment_entry.second;
                if (!attachment_value.IsObject()) {
                    continue;
                }
                Attachment attachment;
                attachment.name = attachment_name;
                attachment.slot_name = slot_name;
                attachment.x = GetNumber(attachment_value, "x", 0.0f);
                attachment.y = GetNumber(attachment_value, "y", 0.0f) * kYFlip;
                attachment.rotation = GetNumber(attachment_value, "rotation", 0.0f) * kYFlip;
                attachment.scale_x = GetNumber(attachment_value, "scaleX", 1.0f);
                attachment.scale_y = GetNumber(attachment_value, "scaleY", 1.0f);
                attachment.width = GetNumber(attachment_value, "width", 0.0f);
                attachment.height = GetNumber(attachment_value, "height", 0.0f);
                attachments_[MakeAttachmentKey(slot_name, attachment_name)] = attachment;
            }
        }
    }

    const JsonValue* animations = root.Get("animations");
    if (animations && animations->IsObject()) {
        for (const auto& animation_entry : animations->object_value) {
            const std::string& animation_name = animation_entry.first;
            const JsonValue& animation_value = animation_entry.second;
            if (!animation_value.IsObject()) {
                continue;
            }

            Animation animation;
            float duration = 0.0f;

            const JsonValue* slot_animations = animation_value.Get("slots");
            if (slot_animations && slot_animations->IsObject()) {
                for (const auto& slot_entry : slot_animations->object_value) {
                    auto slot_it = slot_indices.find(slot_entry.first);
                    if (slot_it == slot_indices.end() || !slot_entry.second.IsObject()) {
                        continue;
                    }
                    const JsonValue* attachment_timeline = slot_entry.second.Get("attachment");
                    if (!attachment_timeline || !attachment_timeline->IsArray()) {
                        continue;
                    }
                    std::vector<AttachmentKeyframe> frames;
                    frames.reserve(attachment_timeline->array_value.size());
                    for (const auto& frame_value : attachment_timeline->array_value) {
                        if (!frame_value.IsObject()) {
                            continue;
                        }
                        AttachmentKeyframe frame;
                        frame.time = GetNumber(frame_value, "time", 0.0f);
                        frame.name = GetString(frame_value, "name", "");
                        frames.push_back(frame);
                        duration = std::max(duration, frame.time);
                    }
                    if (!frames.empty()) {
                        animation.slot_attachment[slot_it->second] = std::move(frames);
                    }
                }
            }

            const JsonValue* bone_animations = animation_value.Get("bones");
            if (bone_animations && bone_animations->IsObject()) {
                for (const auto& bone_entry : bone_animations->object_value) {
                    auto bone_it = bone_indices.find(bone_entry.first);
                    if (bone_it == bone_indices.end() || !bone_entry.second.IsObject()) {
                        continue;
                    }
                    const int bone_index = bone_it->second;

                    const JsonValue* rotate_timeline = bone_entry.second.Get("rotate");
                    if (rotate_timeline && rotate_timeline->IsArray()) {
                        std::vector<FloatKeyframe> frames;
                        frames.reserve(rotate_timeline->array_value.size());
                        for (const auto& frame_value : rotate_timeline->array_value) {
                            if (!frame_value.IsObject()) {
                                continue;
                            }
                            FloatKeyframe frame;
                            frame.time = GetNumber(frame_value, "time", 0.0f);
                            frame.value = GetNumber(frame_value, "angle", 0.0f) * kYFlip;
                            frame.stepped = IsSteppedFrame(frame_value);
                            frames.push_back(frame);
                            duration = std::max(duration, frame.time);
                        }
                        if (!frames.empty()) {
                            animation.bone_rotate[bone_index] = std::move(frames);
                        }
                    }

                    const JsonValue* translate_timeline = bone_entry.second.Get("translate");
                    if (translate_timeline && translate_timeline->IsArray()) {
                        std::vector<Vec2Keyframe> frames;
                        frames.reserve(translate_timeline->array_value.size());
                        for (const auto& frame_value : translate_timeline->array_value) {
                            if (!frame_value.IsObject()) {
                                continue;
                            }
                            Vec2Keyframe frame;
                            frame.time = GetNumber(frame_value, "time", 0.0f);
                            frame.x = GetNumber(frame_value, "x", 0.0f);
                            frame.y = GetNumber(frame_value, "y", 0.0f) * kYFlip;
                            frame.stepped = IsSteppedFrame(frame_value);
                            frames.push_back(frame);
                            duration = std::max(duration, frame.time);
                        }
                        if (!frames.empty()) {
                            animation.bone_translate[bone_index] = std::move(frames);
                        }
                    }

                    const JsonValue* scale_timeline = bone_entry.second.Get("scale");
                    if (scale_timeline && scale_timeline->IsArray()) {
                        std::vector<Vec2Keyframe> frames;
                        frames.reserve(scale_timeline->array_value.size());
                        for (const auto& frame_value : scale_timeline->array_value) {
                            if (!frame_value.IsObject()) {
                                continue;
                            }
                            Vec2Keyframe frame;
                            frame.time = GetNumber(frame_value, "time", 0.0f);
                            frame.x = GetNumber(frame_value, "x", 1.0f);
                            frame.y = GetNumber(frame_value, "y", 1.0f);
                            frame.stepped = IsSteppedFrame(frame_value);
                            frames.push_back(frame);
                            duration = std::max(duration, frame.time);
                        }
                        if (!frames.empty()) {
                            animation.bone_scale[bone_index] = std::move(frames);
                        }
                    }
                }
            }

            animation.duration = duration;
            animations_[animation_name] = std::move(animation);
        }
    }

    if (!ParseAtlas(atlas_path, &regions_)) {
        return false;
    }

    loaded_ = true;
    SetAnimation("Idle", true, true);
    return true;
}

void StickmanSkeleton::Draw(SDL_Renderer* renderer, const RenderContext& ctx, const TextureAsset& texture,
                            float x, float y, float scale, SDL_Color color, bool flip_x) const {
    if (!loaded_ || !texture.texture) {
        return;
    }

    auto EvaluateFloatTimeline = [](const std::vector<FloatKeyframe>& timeline, float time, float fallback) {
        if (timeline.empty()) {
            return fallback;
        }
        if (time <= timeline.front().time) {
            return timeline.front().value;
        }
        for (size_t i = 0; i + 1 < timeline.size(); ++i) {
            const FloatKeyframe& a = timeline[i];
            const FloatKeyframe& b = timeline[i + 1];
            if (time < b.time) {
                if (a.stepped || b.time <= a.time) {
                    return a.value;
                }
                const float t = (time - a.time) / (b.time - a.time);
                return a.value + (b.value - a.value) * t;
            }
        }
        return timeline.back().value;
    };

    auto EvaluateVec2Timeline = [](const std::vector<Vec2Keyframe>& timeline, float time, SDL_FPoint fallback) {
        if (timeline.empty()) {
            return fallback;
        }
        if (time <= timeline.front().time) {
            return SDL_FPoint{timeline.front().x, timeline.front().y};
        }
        for (size_t i = 0; i + 1 < timeline.size(); ++i) {
            const Vec2Keyframe& a = timeline[i];
            const Vec2Keyframe& b = timeline[i + 1];
            if (time < b.time) {
                if (a.stepped || b.time <= a.time) {
                    return SDL_FPoint{a.x, a.y};
                }
                const float t = (time - a.time) / (b.time - a.time);
                return SDL_FPoint{
                    a.x + (b.x - a.x) * t,
                    a.y + (b.y - a.y) * t
                };
            }
        }
        return SDL_FPoint{timeline.back().x, timeline.back().y};
    };

    auto EvaluateAttachmentTimeline = [](const std::vector<AttachmentKeyframe>& timeline, float time, const std::string& fallback) {
        if (timeline.empty()) {
            return fallback;
        }
        std::string attachment = timeline.front().name;
        for (const auto& frame : timeline) {
            if (frame.time <= time) {
                attachment = frame.name;
                continue;
            }
            break;
        }
        return attachment;
    };

    std::vector<Bone> animated_bones = bones_;
    std::vector<std::string> slot_attachments;
    slot_attachments.reserve(slots_.size());
    for (const auto& slot : slots_) {
        slot_attachments.push_back(slot.attachment_name);
    }

    auto animation_it = animations_.find(current_animation_);
    if (animation_it != animations_.end()) {
        const Animation& animation = animation_it->second;
        for (const auto& entry : animation.bone_rotate) {
            if (entry.first < 0 || static_cast<size_t>(entry.first) >= animated_bones.size()) {
                continue;
            }
            animated_bones[entry.first].rotation += EvaluateFloatTimeline(entry.second, animation_time_, 0.0f);
        }
        for (const auto& entry : animation.bone_translate) {
            if (entry.first < 0 || static_cast<size_t>(entry.first) >= animated_bones.size()) {
                continue;
            }
            const SDL_FPoint offset = EvaluateVec2Timeline(entry.second, animation_time_, SDL_FPoint{0.0f, 0.0f});
            animated_bones[entry.first].x += offset.x;
            animated_bones[entry.first].y += offset.y;
        }
        for (const auto& entry : animation.bone_scale) {
            if (entry.first < 0 || static_cast<size_t>(entry.first) >= animated_bones.size()) {
                continue;
            }
            const SDL_FPoint scale_value = EvaluateVec2Timeline(entry.second, animation_time_, SDL_FPoint{1.0f, 1.0f});
            animated_bones[entry.first].scale_x *= scale_value.x;
            animated_bones[entry.first].scale_y *= scale_value.y;
        }
        for (const auto& entry : animation.slot_attachment) {
            if (entry.first < 0 || static_cast<size_t>(entry.first) >= slot_attachments.size()) {
                continue;
            }
            slot_attachments[entry.first] = EvaluateAttachmentTimeline(entry.second, animation_time_, slot_attachments[entry.first]);
        }
    }

    std::vector<BonePose> poses(animated_bones.size());
    for (size_t i = 0; i < animated_bones.size(); ++i) {
        const Bone& bone = animated_bones[i];
        BonePose pose;
        const float lr = bone.rotation * kDegToRad;
        const float cos_r = std::cos(lr);
        const float sin_r = std::sin(lr);
        const float la = cos_r * bone.scale_x;
        const float lb = -sin_r * bone.scale_y;
        const float lc = sin_r * bone.scale_x;
        const float ld = cos_r * bone.scale_y;
        if (bone.parent_index < 0) {
            pose.world_x = bone.x;
            pose.world_y = bone.y;
            pose.a = la;
            pose.b = lb;
            pose.c = lc;
            pose.d = ld;
        } else {
            const BonePose& parent = poses[bone.parent_index];
            pose.world_x = parent.world_x + bone.x * parent.a + bone.y * parent.b;
            pose.world_y = parent.world_y + bone.x * parent.c + bone.y * parent.d;
            pose.a = parent.a * la + parent.b * lc;
            pose.b = parent.a * lb + parent.b * ld;
            pose.c = parent.c * la + parent.d * lc;
            pose.d = parent.c * lb + parent.d * ld;
        }
        poses[i] = pose;
    }

    SDL_SetTextureColorMod(texture.texture, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(texture.texture, color.a);

    for (size_t slot_index = 0; slot_index < slots_.size(); ++slot_index) {
        const Slot& slot = slots_[slot_index];
        if (slot.bone_index < 0) {
            continue;
        }
        const std::string key = MakeAttachmentKey(slot.name, slot_attachments[slot_index]);
        auto attachment_it = attachments_.find(key);
        if (attachment_it == attachments_.end()) {
            continue;
        }
        const Attachment* attachment = &attachment_it->second;
        if (!attachment) {
            continue;
        }
        const AtlasRegion* region = FindRegion(attachment->name);
        if (!region) {
            continue;
        }
        const BonePose& bone = poses[slot.bone_index];

        const float orig_w = (region->orig_w > 0) ? static_cast<float>(region->orig_w) : static_cast<float>(region->src.w);
        const float orig_h = (region->orig_h > 0) ? static_cast<float>(region->orig_h) : static_cast<float>(region->src.h);
        const float base_w = (attachment->width > 0.0f) ? attachment->width : orig_w;
        const float base_h = (attachment->height > 0.0f) ? attachment->height : orig_h;
        const float region_scale_x = (orig_w > 0.0f) ? (base_w / orig_w) : 1.0f;
        const float region_scale_y = (orig_h > 0.0f) ? (base_h / orig_h) : 1.0f;

        const float src_w = static_cast<float>(region->src.w);
        const float src_h = static_cast<float>(region->src.h);

        const float draw_w = src_w * region_scale_x * attachment->scale_x * scale;
        const float draw_h = src_h * region_scale_y * attachment->scale_y * scale;
        const float center_offset_x = (static_cast<float>(region->offset_x) + src_w * 0.5f - orig_w * 0.5f) * region_scale_x * attachment->scale_x;
        const float center_offset_y = (orig_h * 0.5f - static_cast<float>(region->offset_y) - src_h * 0.5f) * region_scale_y * attachment->scale_y;

        const float ar = attachment->rotation * kDegToRad;
        const float ar_cos = std::cos(ar);
        const float ar_sin = std::sin(ar);
        const float attachment_offset_x = center_offset_x * ar_cos - center_offset_y * ar_sin;
        const float attachment_offset_y = center_offset_x * ar_sin + center_offset_y * ar_cos;
        const float attachment_local_x = attachment->x + attachment_offset_x;
        const float leg_y_adjust = IsLegSlot(slot.name) ? kLegSlotYOffset : 0.0f;
        const float hand_y_adjust = IsHandSlot(slot.name) ? kHandSlotYOffset : 0.0f;
        const float attachment_local_y = attachment->y + attachment_offset_y + leg_y_adjust + hand_y_adjust;

        float center_x = x + bone.world_x + attachment_local_x * bone.a + attachment_local_y * bone.b;
        const float center_y = y + bone.world_y + attachment_local_x * bone.c + attachment_local_y * bone.d;
        float angle = std::atan2(bone.c, bone.a) / kDegToRad + attachment->rotation;
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        if (flip_x) {
            center_x = x - (center_x - x);
            angle = -angle;
            flip = SDL_FLIP_HORIZONTAL;
        }
        if (IsHandSlot(slot.name)) {
            center_x += flip_x ? kHandBackXOffset : -kHandBackXOffset;
        }

        SDL_FRect dst{
            ctx.offset_x + (center_x - draw_w * 0.5f) * ctx.scale,
            ctx.offset_y + (center_y - draw_h * 0.5f) * ctx.scale,
            draw_w * ctx.scale,
            draw_h * ctx.scale
        };
        SDL_FPoint center{dst.w * 0.5f, dst.h * 0.5f};
        SDL_RenderCopyExF(renderer, texture.texture, &region->src, &dst, angle, &center, flip);
    }
}
