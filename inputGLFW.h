//
// Created by Nico Russo on 4/18/26.
//

#ifndef MEDIUM_INPUTGLFW_H
#define MEDIUM_INPUTGLFW_H

#include <input.h>
#include <GLFW/glfw3.h>

class InputGLFW : public Input {
public:

    void initializeInput(Medium* medium_) override {
        medium = medium_;
        auto* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, glfwKeyCallback);
        glfwSetMouseButtonCallback(window, glfwMouseButtonCallback);
        glfwSetScrollCallback(window, glfwScrollCallback);
    }

    bool isKeyPressed(int keycode) override {
        auto* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        const int state = glfwGetKey(window, keycode);
        return state == MED_PRESS || state == MED_REPEAT;
    }

    bool isMouseButtonPressed(int button) override {
        auto* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        const int state = glfwGetMouseButton(window, button);
        return state == MED_PRESS;
    }

    std::pair<float, float> getMousePosition() override {
        auto* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return { x, y };
    }

    float getMouseX() override { return getMousePosition().first; }
    float getMouseY() override { return getMousePosition().second; }

private:
    static void glfwScrollCallback(GLFWwindow* window, const double x_offset, const double y_offset) {
        auto* self = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
        self->globalCallbackImpl(MED_MOUSE_SCROLL, 0, 0, x_offset, y_offset);
    }

    static void glfwKeyCallback(GLFWwindow* window, const int key, int scancode, const int action, const int mods) {
        auto* self = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
        self->eventCallbackImpl(key, action);
        self->globalCallbackImpl(key, action, mods, 0, 0);
    }

    static void glfwMouseButtonCallback(GLFWwindow* window, const int button, const int action, const int mods) {
        auto* self = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
        self->eventCallbackImpl(button, action);
        self->globalCallbackImpl(button, action, mods, 0, 0);
    }
};

#endif //MEDIUM_INPUTGLFW_H
