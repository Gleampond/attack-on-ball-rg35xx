#include "PreloadState.h"

#include <string>

#include "game/Game.h"
#include "game/states/MenuState.h"

namespace {
bool LoadTextures(Assets& assets) {
    bool ok = true;
    ok &= assets.LoadTexture("bg", "assets/Bg.png");
    ok &= assets.LoadTexture("blood", "assets/Blood.png");
    ok &= assets.LoadTexture("colon", "assets/Colon.png");
    ok &= assets.LoadTexture("dot", "assets/Dot.png");
    ok &= assets.LoadTexture("effectHit", "assets/EffectHit.png");
    ok &= assets.LoadTexture("empty", "assets/Empty.png");
    ok &= assets.LoadTexture("gauge", "assets/Gauge.png");
    ok &= assets.LoadTexture("gaugeHead", "assets/GaugeHead.png");
    ok &= assets.LoadTexture("landWhite", "assets/LandWhite.png");
    ok &= assets.LoadTexture("shadow", "assets/Shadow.png");
    ok &= assets.LoadTexture("title", "assets/Title.png");
    ok &= assets.LoadTexture("topSign", "assets/TopSign.png");
    ok &= assets.LoadTexture("touchToPlay", "assets/TouchToPlay.png");
    ok &= assets.LoadTexture("white4", "assets/White4.png");
    ok &= assets.LoadTexture("wordBest", "assets/WordBest.png");
    ok &= assets.LoadTexture("wordYour", "assets/WordYour.png");
    ok &= assets.LoadTexture("stickman", "assets/Stickman.png");

    for (int i = 0; i <= 1; ++i) {
        ok &= assets.LoadTexture("buttonGamecenter" + std::to_string(i), "assets/ButtonGamecenter" + std::to_string(i) + ".png");
        ok &= assets.LoadTexture("buttonPlay" + std::to_string(i), "assets/ButtonPlay" + std::to_string(i) + ".png");
        ok &= assets.LoadTexture("buttonRate" + std::to_string(i), "assets/ButtonRate" + std::to_string(i) + ".png");
        ok &= assets.LoadTexture("buttonShare" + std::to_string(i), "assets/ButtonShare" + std::to_string(i) + ".png");
    }
    for (int i = 1; i <= 4; ++i) {
        ok &= assets.LoadTexture("numberItem" + std::to_string(i), "assets/NumberItem" + std::to_string(i) + ".png");
    }
    for (int i = 0; i <= 4; ++i) {
        ok &= assets.LoadTexture("ball" + std::to_string(i), "assets/Ball" + std::to_string(i) + ".png");
    }
    for (int i = 0; i <= 5; ++i) {
        ok &= assets.LoadTexture("land" + std::to_string(i), "assets/Land" + std::to_string(i) + ".png");
    }
    for (int i = 0; i <= 7; ++i) {
        ok &= assets.LoadTexture("deadParts" + std::to_string(i), "assets/DeadParts" + std::to_string(i) + ".png");
    }
    for (int i = 0; i <= 10; ++i) {
        ok &= assets.LoadTexture("effectBlood" + std::to_string(i), "assets/EffectBlood" + std::to_string(i) + ".png");
    }

    return ok;
}

bool LoadFonts(Assets& assets) {
    bool ok = true;
    ok &= assets.LoadFont("numberTime", "assets/NumberTime.png", "assets/NumberTime.xml");
    ok &= assets.LoadFont("numberScoreMain", "assets/NumberScoreMain.png", "assets/NumberScoreMain.xml");
    ok &= assets.LoadFont("numberScoreEnd", "assets/NumberScoreEnd.png", "assets/NumberScoreEnd.xml");
    return ok;
}

bool LoadSounds(Assets& assets) {
    bool ok = true;
    ok &= assets.LoadSound("foot", "assets/sound/foot.ogg");
    ok &= assets.LoadSound("toss", "assets/sound/toss_03.ogg");
    ok &= assets.LoadSound("numberGet", "assets/sound/numberGet_00.ogg");
    ok &= assets.LoadSound("numberGetGauge", "assets/sound/numberGet_gauge00.ogg");
    ok &= assets.LoadSound("uiButton", "assets/sound/ui_button.ogg");
    ok &= assets.LoadSound("dead02", "assets/sound/dead02.ogg");
    ok &= assets.LoadSound("dead03", "assets/sound/dead03.ogg");
    ok &= assets.LoadSound("die00", "assets/sound/die00.ogg");
    ok &= assets.LoadSound("clock", "assets/sound/clock00.ogg");
    return ok;
}
}

void PreloadState::Enter(Game& game) {
    Assets& assets = game.GetAssets();
    LoadTextures(assets);
    LoadFonts(assets);
    LoadSounds(assets);
    game.ChangeState(std::make_unique<MenuState>());
}

void PreloadState::Exit(Game& game) {
    (void)game;
}

void PreloadState::HandleEvent(Game& game, const SDL_Event& event) {
    (void)game;
    (void)event;
}

void PreloadState::Update(Game& game, float delta_seconds) {
    (void)game;
    (void)delta_seconds;
}

void PreloadState::Render(Game& game) {
    (void)game;
}
