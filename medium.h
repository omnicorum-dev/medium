//
// Created by Nico Russo on 4/15/26.
//

#ifndef MEDIUM_H
#define MEDIUM_H

#include <base.h>
#include <graphite.h>
using namespace omni::basic;

class Medium {
public:
    u32* backBuffer;

    u32 SCREEN_WIDTH;
    u32 SCREEN_HEIGHT;
    u32 GAME_WIDTH;
    u32 GAME_HEIGHT;

    Graphite::Canvas canvas;

    virtual ~Medium() = default;

    void mediumInit(const u32 consoleWidth, const u32 consoleHeight, const u32 gameWidth, const u32 gameHeight) {
        SCREEN_WIDTH = consoleWidth;
        SCREEN_HEIGHT = consoleHeight;
        GAME_WIDTH = gameWidth;
        GAME_HEIGHT = gameHeight;
    }

    virtual u32 mediumStartup() = 0;
    virtual void mediumRun(std::function<Graphite::Canvas&(f32)> gameUpdate) = 0;
    virtual u32 mediumShutdown() = 0;
};

#endif //MEDIUM_H
