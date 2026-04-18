//
// Created by Nico Russo on 4/18/26.
//

#ifndef MEDIUM_INPUT_H
#define MEDIUM_INPUT_H

#include <base.h>

#include <medium.h>

class Input {
public:
    Medium* medium;
    Input(Medium* medium) : medium(medium) {}

    virtual ~Input () = default;

    static bool isKeyPressed(const int keycode) { return instance->isKeyPressedImpl(keycode); }
    static bool isMouseButtonPressed(const int button) { return instance->isMouseButtonPressedImpl(button); }
    static std::pair<float, float> getMousePosition() { return instance->getMousePositionImpl(); }
    static float getMouseX() { return instance->getMouseXImpl(); }
    static float getMouseY() { return instance->getMouseYImpl(); }

protected:
    virtual bool isKeyPressedImpl(int keycode) = 0;
    virtual bool isMouseButtonPressedImpl(int button) = 0;
    virtual std::pair<float, float> getMousePositionImpl() = 0;
    virtual float getMouseXImpl() = 0;
    virtual float getMouseYImpl() = 0;

private:
    static Input* instance;
};

#endif //MEDIUM_INPUT_H
