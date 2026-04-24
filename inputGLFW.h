//
// Created by Nico Russo on 4/18/26.
//

#ifndef MEDIUM_INPUTGLFW_H
#define MEDIUM_INPUTGLFW_H

#include <input.h>
#include <base.h>

#include <GLFW/glfw3.h>
#include <eventTypes.h>

class InputGLFW : public Input {
public:
    InputGLFW(Medium* medium) : Input(medium) {
        instance = this;
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, glfwKeyCallback);
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
    }
protected:
    bool isKeyPressedImpl(const int keycode) override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        const int state = glfwGetKey(window, keycode);
        return state == MED_PRESS || state == MED_REPEAT;
    }

    bool isMouseButtonPressedImpl(const int button) override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        const int state = glfwGetMouseButton(window, button);
        return state == MED_PRESS;
    }

    std::pair<float, float> getMousePositionImpl() override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return {x, y};
    }

    float getMouseXImpl() override {
        return getMousePositionImpl().first;
    }

    float getMouseYImpl() override {
        return getMousePositionImpl().second;
    }

    void eventCallbackImpl(int eventObject, int eventType) override {
        Input::eventCallbackImpl(eventObject, eventType); // Dispatches to user callbacks
    }

    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto* self = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
        self->eventCallbackImpl(key, action);
    }

    static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        auto* self = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
        self->eventCallbackImpl(button, action);
    }
};

#endif //MEDIUM_INPUTGLFW_H
