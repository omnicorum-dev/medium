//
// Created by Nico Russo on 4/15/26.
//

#ifndef MEDIUMOPENGL_H
#define MEDIUMOPENGL_H

#include <cstdint>
#include <cstring>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <medium.h>
#include <base.h>
#include <graphite.h>


class MediumOpenGL : public Medium {
private:
    GLFWwindow* window = nullptr;
    GLuint shader = {};
    GLuint vao = {};
    u32* frontBuffer;

    GLuint screen_texture = 0;


    void initBuffers(int w, int h) {
        frontBuffer = new std::uint32_t[w * h];
        backBuffer  = new std::uint32_t[w * h];

        std::memset(frontBuffer, 0, w * h * sizeof(std::uint32_t));
        std::memset(backBuffer,  0, w * h * sizeof(std::uint32_t));
    }

    void initTexture(int w, int h) {
        glGenTextures(1, &screen_texture);
        glBindTexture(GL_TEXTURE_2D, screen_texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            w,
            h,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }

    static GLuint createShader(const GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> errorLog(length);
        glGetShaderInfoLog(shader, length, &length, errorLog.data());
        std::cerr << "Shader compilation failed:\n" << errorLog.data() << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

    static GLuint createShaderProgram(const char* vertSrc, const char* fragSrc) {
        const GLuint vertShader = createShader(GL_VERTEX_SHADER, vertSrc);
        if (vertShader == 0) return 0;

        const GLuint fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);
        if (fragShader == 0) {
            glDeleteShader(vertShader);
            return 0;
        }

        const GLuint program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        GLint linked;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (!linked) {
            GLint length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            std::vector<char> errorLog(length);
            glGetProgramInfoLog(program, length, &length, errorLog.data());
            std::cerr << "Shader linking failed:\n" << errorLog.data() << std::endl;
            glDeleteShader(vertShader);
            glDeleteShader(fragShader);
            glDeleteProgram(program);
            return 0;
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return program;
    }

    static GLuint createFullscreenQuadVAO() {
        float vertices[] = {
            // pos      // uv
            -1.f, -1.f,  0.f, 1.f,
             1.f, -1.f,  1.f, 1.f,
            -1.f,  1.f,  0.f, 0.f,
             1.f,  1.f,  1.f, 0.f,
        };

        GLuint vao = 0, vbo = 0;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(
            0, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(float),
            (void*)0
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(float),
            (void*)(2 * sizeof(float))
        );

        glBindVertexArray(0);

        return vao;
    }

    inline void swap_buffers() {
        std::swap(frontBuffer, backBuffer);
    }

    void upload_front_buffer() {
        glBindTexture(GL_TEXTURE_2D, screen_texture);

        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0, 0,
            GAME_WIDTH,
            GAME_HEIGHT,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            frontBuffer
        );
    }

    void render_screen() {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, screen_texture);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

public:

    u32 mediumStartup() override {
        if (!glfwInit()) return -1;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowName.c_str(), nullptr, nullptr);
        if (!window) return -1;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            return -1;
        }

        initBuffers(SCREEN_WIDTH, SCREEN_HEIGHT);
        initTexture(GAME_WIDTH, GAME_HEIGHT);

        const char* vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aTexCoord;

            out vec2 TexCoord;

            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        const char* fragmentSrc = R"(
            #version 330 core
            in vec2 TexCoord;
            out vec4 FragColor;

            uniform sampler2D screenTexture;

            void main() {
                FragColor = texture(screenTexture, TexCoord);
            }
        )";

        shader = createShaderProgram(vertexSrc, fragmentSrc);
        vao = createFullscreenQuadVAO();

        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClearColor(0, 0, 0, 1);

        return 0;
    }

    void mediumRun(std::function<Graphite::Canvas&(f32)> gameUpdate) override {
        double last_time = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {

            double now = glfwGetTime();
            float dt = float(now - last_time);
            last_time = now;

            glfwPollEvents();

            // 1. UPDATE (writes to BACK buffer)
            double t0 = glfwGetTime();
            canvas.linkCanvas(backBuffer, GAME_WIDTH, GAME_HEIGHT);
            Graphite::Canvas& gameCanvas = gameUpdate(dt);
            double t10 = glfwGetTime();
            //consoleCanvas.blitCanvas(gameCanvas);

            // 2. SWAP (back becomes front)
            double t1 = glfwGetTime();
            swap_buffers();

            // 3. GPU upload (reads FRONT buffer)
            double t2 = glfwGetTime();
            upload_front_buffer();

            // 4. DRAW
            double t3 = glfwGetTime();
            render_screen();

            double t4 = glfwGetTime();
            glfwSwapBuffers(window);

            double t5 = glfwGetTime();
            LOG_TRACE("gameUpdate: {}s, blitCanvas: {}s, swap_buffers: {}s, upload_front_buffer: {}s, render_screen: {}s, glfwSwapBuffers: {}s", t10-t0, t1-t10, t2-t1, t3-t2, t4-t3, t5-t4);
        }
    }

    u32 mediumShutdown() override {
        delete[] frontBuffer;
        delete[] backBuffer;

        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }

};

#endif //MEDIUMOPENGL_H
