//
// Created by Nico Russo on 4/18/26.
//

#ifndef MEDIUM_INPUT_H
#define MEDIUM_INPUT_H

#include <base.h>
#include <map>

#include <medium.h>

class Input {
public:
    Medium* medium;
    Input(Medium* medium) : medium(medium) {}

    using EventCallback = std::function<void()>;
    using GlobalEventCallback = std::function<void(int, int, int, double, double)>;

    virtual ~Input () = default;

    static bool isKeyPressed(const int keycode) { return instance->isKeyPressedImpl(keycode); }
    static bool isMouseButtonPressed(const int button) { return instance->isMouseButtonPressedImpl(button); }
    static std::pair<float, float> getMousePosition() { return instance->getMousePositionImpl(); }
    static float getMouseX() { return instance->getMouseXImpl(); }
    static float getMouseY() { return instance->getMouseYImpl(); }

    static void registerEventCallback(int eventObject, int eventType, const EventCallback &callback) {
        eventCallbacks[{eventObject, eventType}] = callback;
    }

    static void registerGlobalCallback(const GlobalEventCallback &callback) {
        globalCallback = callback;
    }

protected:
    virtual bool isKeyPressedImpl(int keycode) = 0;
    virtual bool isMouseButtonPressedImpl(int button) = 0;
    virtual std::pair<float, float> getMousePositionImpl() = 0;
    virtual float getMouseXImpl() = 0;
    virtual float getMouseYImpl() = 0;

    virtual void eventCallbackImpl(int eventObject, int eventType) {
        auto it = eventCallbacks.find({eventObject, eventType});
        if (it != eventCallbacks.end()) {
            it->second();
        }
    }

    virtual void globalCallbackImpl(const int object, const int action, const int mods, const double x, const double y) {
        globalCallback(object, action, mods, x, y);
    }

    static GlobalEventCallback globalCallback;

    static std::map<std::pair<int, int>, EventCallback> eventCallbacks;

public:
    static Input* instance;
};

#endif //MEDIUM_INPUT_H
