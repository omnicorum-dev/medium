//
// Created by Nico Russo on 4/18/26.
//

#ifndef MEDIUM_INPUTGLFW_H
#define MEDIUM_INPUTGLFW_H

#include <input.h>
#include <base.h>

#include <GLFW/glfw3.h>

class InputGLFW : public Input {
public:
    InputGLFW(Medium* medium)
        : Input(medium) {}
protected:
    bool isKeyPressedImpl(const int keycode) override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        const int state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool isMouseButtonPressedImpl(const int button) override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        const int state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    std::pair<float, float> getMousePositionImpl() override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return {x, y};
    }

    float getMouseXImpl() override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        return getMousePositionImpl().first;
    }

    float getMouseYImpl() override {
        GLFWwindow* window = static_cast<GLFWwindow*>(medium->getNativeWindow());
        return getMousePositionImpl().second;
    }
};

#endif //MEDIUM_INPUTGLFW_H
