//
// Created by Nico Russo on 4/15/26.
//

#ifndef MEDIUM_H
#define MEDIUM_H

#include <base.h>
#include <graphite.h>

#include <keyCodes.h>
#include <mouseButtons.h>
#include <gamepadInputs.h>

using namespace omni::basic;

class Medium {
public:
    u32* backBuffer = nullptr;

    u32 SCREEN_WIDTH = 0;
    u32 SCREEN_HEIGHT = 0;
    u32 GAME_WIDTH = 0;
    u32 GAME_HEIGHT = 0;

    std::string windowName = "Graphite";

    Graphite::Canvas canvas = {};

    Medium() = default;
    explicit Medium(const std::string& _windowName) {
        windowName = _windowName;
    }
    virtual ~Medium() = default;

    void mediumInit(const u32 consoleWidth, const u32 consoleHeight, const u32 gameWidth, const u32 gameHeight) {
        SCREEN_WIDTH = consoleWidth;
        SCREEN_HEIGHT = consoleHeight;
        GAME_WIDTH = gameWidth;
        GAME_HEIGHT = gameHeight;
    }

    void setWindowName(const std::string& _windowName) {
        windowName = _windowName;
    }

    virtual u32 mediumStartup() = 0;
    virtual void mediumRun(std::function<void(f32)> gameUpdate) = 0;
    virtual u32 mediumShutdown() = 0;

    [[nodiscard]] virtual void* getNativeWindow() const = 0;
};

#endif //MEDIUM_H
