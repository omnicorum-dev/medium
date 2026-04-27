//
// Created by Nico Russo on 4/15/26.
//

#ifndef MEDIUM_H
#define MEDIUM_H

#include <base.h>
#include <graphite.h>

#include <utility>

using namespace omni::basic;

class Medium {
public:
    u32 SCREEN_WIDTH = 0;
    u32 SCREEN_HEIGHT = 0;
    u32 GAME_WIDTH = 0;
    u32 GAME_HEIGHT = 0;

    float deltaTime = 0.0f;

    std::string windowName = "Graphite";

    Medium(const u32 consoleWidth,
        const u32 consoleHeight,
        const u32 gameWidth,
        const u32 gameHeight,
        std::string windowName_ = "Graphite")
    : SCREEN_WIDTH(consoleWidth), SCREEN_HEIGHT(consoleHeight), GAME_WIDTH(gameWidth), GAME_HEIGHT(gameHeight), windowName(std::move(windowName_)) {}

    virtual ~Medium() = default;

    virtual void setWindowName(const std::string& _windowName) {
        windowName = _windowName;
        LOG_ERROR("setWindowName() not implemented for current platform");
    }

    virtual u32 mediumStartup() = 0;
    virtual void mediumRun(std::function<void(f32)> gameUpdate) = 0;
    virtual u32 mediumShutdown() = 0;

    virtual std::filesystem::path getAssetRoot() = 0;
    virtual std::filesystem::path getSaveRoot()  = 0;

    virtual void renderCanvas(const Graphite::Canvas& canvas,
        u32 customShader = 0, int x = 0, int y = 0, int w = -1, int h = -1) {
        LOG_ERROR("renderCanvas() not implemented for current platform");
    }

    virtual void setScreenShader(u32 shader) {
        LOG_ERROR("setScreenShader() not implemented for current platform");
    }

    virtual void cursorHide() {
        LOG_ERROR("cursorHide() not implemented for current platform");
    }

    virtual void cursorShow() {
        LOG_ERROR("cursorShow() not implemented for current platform");
    }

    virtual void cursorCapture() {
        LOG_ERROR("cursorCapture() not implemented for current platform");
    }

    [[nodiscard]] virtual void* getNativeWindow() const = 0;

private:
    virtual void present() {
        LOG_ERROR("present() not implemented for current platform");
    }

    virtual void applyRect(u32 program, int x, int y, int w, int h) {
        LOG_ERROR("applyRect() not implemented for current platform");
    }

};

#endif //MEDIUM_H
