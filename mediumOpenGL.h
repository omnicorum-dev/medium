//
// Created by Nico Russo on 4/15/26.
//

#ifndef MEDIUMOPENGL_H
#define MEDIUMOPENGL_H

#include <filesystem>
#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <medium.h>
#include <base.h>
#include <graphite.h>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <limits.h>
#endif

#define Shader GLuint

class MediumOpenGL : public Medium {
private:
    GLFWwindow* window = nullptr;
    GLuint standardShader = {};
    GLuint defaultScreenShader = {};
    GLuint vao = {};

    GLuint fbo = 0;
    GLuint fbo_texture = 0;

    GLuint screenShader = 0;
    GLuint screenFragFBO = 0;
    GLuint screenFragTexture = 0;

    GLuint screen_texture = 0;

    int evenOddFrame = 0;

    bool pendingResize = false;
    int pendingFbW = 0, pendingFbH = 0;

    struct Viewport { int x, y, w, h; };

    Viewport viewport{};

    [[nodiscard]] Viewport computeLetterboxViewport(const int winW, const int winH) const {
        const float targetAspect = (float)GAME_WIDTH / (float)GAME_HEIGHT;
        const float windowAspect = (float)winW / (float)winH;

        int vpW, vpH;
        if (windowAspect > targetAspect) {
            // Window is wider than canvas → pillarbox
            vpH = winH;
            vpW = static_cast<int>(winH * targetAspect);
        } else {
            // Window is taller than canvas → letterbox
            vpW = winW;
            vpH = static_cast<int>(winW / targetAspect);
        }

        const int vpX = (winW - vpW) / 2;
        const int vpY = (winH - vpH) / 2;

        return { vpX, vpY, vpW, vpH };
    }

    /*
    static void framebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
        auto* self = static_cast<MediumOpenGL*>(glfwGetWindowUserPointer(window));
        auto [x, y, w, h] = self->computeLetterboxViewport(width, height);
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        self->viewport = self->computeLetterboxViewport(fbW, fbH);
        glViewport(x, y, w, h);
    }
    */

    static void framebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
        auto* self = static_cast<MediumOpenGL*>(glfwGetWindowUserPointer(window));
        // Don't touch GL here — just record that a resize happened
        self->pendingResize = true;
        self->pendingFbW = width;
        self->pendingFbH = height;
    }

    void resizeFBOs(int fbW, int fbH) {
        if (fbW <= 0 || fbH <= 0) return; // guard against minimized window

        glDeleteTextures(1, &screenFragTexture);
        glDeleteFramebuffers(1, &screenFragFBO);
        screenFragTexture = 0;
        screenFragFBO = 0;

        initScreenFBO(fbW, fbH);
        viewport = computeLetterboxViewport(fbW, fbH);
    }

    void initScreenFBO(int w, int h)
    {
        glGenFramebuffers(1, &screenFragFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, screenFragFBO);

        glGenTextures(1, &screenFragTexture);
        glBindTexture(GL_TEXTURE_2D, screenFragTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenFragTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "Screen FBO incomplete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    void initFBO(int w, int h) {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &fbo_texture);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "FBO incomplete!" << std::endl;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
        const float vertices[] = {
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
            static_cast<void *>(nullptr)
        );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(
            1, 2, GL_FLOAT, GL_FALSE,
            4 * sizeof(float),
            reinterpret_cast<void *>(2 * sizeof(float))
        );

        glBindVertexArray(0);

        return vao;
    }

    // In the private section, change upload_front_buffer to accept a pointer:
    void upload_buffer(const u32* pixels) const {
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0, 0,
            static_cast<int>(GAME_WIDTH),
            static_cast<int>(GAME_HEIGHT),
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels
        );
    }

public:

    [[nodiscard]] GLuint getDefaultScreenShader() const {
        return defaultScreenShader;
    }

    void setScreenShader(const GLuint shader_) override {
        screenShader = shader_;
    }

    static GLuint createCustomShader(const char* fragSrc) {
        const auto vertSrc = R"(
            #version 330 core
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aTexCoord;
            out vec2 TexCoord;
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";
        return createShaderProgram(vertSrc, fragSrc);
    }

    static GLuint buildShader(const std::filesystem::path& fragSrc) {
        std::ifstream ifs(fragSrc);
        std::stringstream ss;
        ss << ifs.rdbuf();

        std::string src = ss.str();
        return createCustomShader(src.c_str());
    }

    void renderCanvas(const Graphite::Canvas& externalCanvas, const GLuint customShader = 0,
                  int x = 0, int y = 0, int w = -1, int h = -1) override {

        if (w == -1) w = GAME_WIDTH;
        if (h == -1) h = GAME_HEIGHT;

        const auto GW = static_cast<int>(GAME_WIDTH);
        const auto GH = static_cast<int>(GAME_HEIGHT);

        upload_buffer(externalCanvas.getPixels());

        // Pass 1 : CANVAS -> CANVAS SHADER -> FBO_TEXTURE
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, GW, GH);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const GLuint canvasShader = (customShader == 0) ? standardShader : customShader;

        applyRect(canvasShader, 0, 0, GW, GH); // fullscreen in FBO
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Pass 2 : FBO_TEXTURE -> DEFAULT SHADER -> SCREEN_FBO_TEXTURE
        glBindFramebuffer(GL_FRAMEBUFFER, screenFragFBO);
        glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glUseProgram(standardShader);
        glUniform2f(glGetUniformLocation(standardShader, "u_scale"),  1.0f, 1.0f);
        glUniform2f(glGetUniformLocation(standardShader, "u_offset"), 0.0f, 0.0f);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

private:
    void present() override {
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        auto vp = computeLetterboxViewport(fbW, fbH);

        // -----------------------------
        // Back to default framebuffer
        // -----------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(vp.x, vp.y, vp.w, vp.h);

        glDisable(GL_BLEND); // usually safer for full-screen post FX
        glClear(GL_COLOR_BUFFER_BIT);

        // -----------------------------
        // FINAL POST-PROCESS SHADER (CRT)
        // -----------------------------
        glUseProgram(screenShader); // <-- THIS is your final screen shader

        // Bind the fully composed scene
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenFragTexture);

        // IMPORTANT: match sampler in CRT shader
        glUniform1i(glGetUniformLocation(screenShader, "screenTexture"), 0);

        // Optional uniforms if your CRT uses them
        glUniform2f(glGetUniformLocation(screenShader, "u_resolution"),
                    (float)vp.w, (float)vp.h);

        glUniform1f(glGetUniformLocation(screenShader, "u_time"),
                    (float)glfwGetTime());

        glUniform1f(glGetUniformLocation(screenShader, "u_frame"), static_cast<float>(evenOddFrame));

        // -----------------------------
        // DRAW FULLSCREEN QUAD
        // -----------------------------
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


        // -----------------------------
        // PRESENT TO SCREEN
        // -----------------------------
        glfwSwapBuffers(window);
    }
    void applyRect(const GLuint program, const int x, const int y, const int w, const int h) override {
        // convert game pixels to NDC, Y flipped (0,0 = top left)
        float scaleX = (float)w / GAME_WIDTH;
        float scaleY = (float)h / GAME_HEIGHT;

        float offsetX = ((float)x / GAME_WIDTH) * 2.0f - 1.0f + scaleX;
        float offsetY = 1.0f - ((float)y / GAME_HEIGHT) * 2.0f - scaleY;

        float time = glfwGetTime();

        double mouseX, mouseY;

        glfwGetCursorPos(window, &mouseX, &mouseY);

        glUseProgram(program);
        glUniform2f(glGetUniformLocation(program, "u_scale"),  scaleX, scaleY);
        glUniform2f(glGetUniformLocation(program, "u_offset"), offsetX, offsetY);
        glUniform2f(glGetUniformLocation(program, "u_resolution"), static_cast<float>(w), static_cast<float>(h));
        glUniform2f(glGetUniformLocation(program, "u_mouse"), static_cast<float>(mouseX), static_cast<float>(mouseY));
        glUniform1f(glGetUniformLocation(program, "u_time"),  time);
        glUniform1f(glGetUniformLocation(program, "u_deltaTime"),  deltaTime);
    }
public:

    void setWindowName(const std::string &_windowName) override {
        windowName = _windowName;
        glfwSetWindowTitle(window, windowName.c_str());
    }

    u32 mediumStartup() override {
        if (!glfwInit()) return -1;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowName.c_str(), nullptr, nullptr);
        if (!window) return -1;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

        // Register before any GL calls that depend on window size
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            return -1;
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const char* vertexSrc = R"(
            #version 330 core
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aTexCoord;

            out vec2 TexCoord;

            uniform vec2 u_offset; // in NDC
            uniform vec2 u_scale;  // in NDC

            void main() {
                vec2 pos = aPos * u_scale + u_offset;
                gl_Position = vec4(pos, 0.0, 1.0);
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

        standardShader = createShaderProgram(vertexSrc, fragmentSrc);
        defaultScreenShader = standardShader;
        screenShader = standardShader;
        vao = createFullscreenQuadVAO();

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        //initBuffers(GAME_WIDTH, GAME_HEIGHT);
        initTexture(GAME_WIDTH, GAME_HEIGHT);
        initFBO(GAME_WIDTH, GAME_HEIGHT);
        initScreenFBO(fbW, fbH);
        viewport = computeLetterboxViewport(fbW, fbH);
        glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
        glClearColor(0, 0, 0, 1);

        return 0;
    }

    void mediumRun(std::function<void(f32)> gameUpdate) override {
        double last_time = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {
            double now = glfwGetTime();
            deltaTime = float(now - last_time);
            last_time = now;

            glfwPollEvents();

            // Recompute letterbox only if needed
            /*
            int fbW, fbH;
            glfwGetFramebufferSize(window, &fbW, &fbH);
            */
            if (pendingResize) {
                pendingResize = false;
                resizeFBOs(pendingFbW, pendingFbH);
            }

            // Use stored viewport from callback
            glViewport(viewport.x, viewport.y, viewport.w, viewport.h);

            glClear(GL_COLOR_BUFFER_BIT);

            //canvas.linkCanvas(backBuffer, GAME_WIDTH, GAME_HEIGHT);
            gameUpdate(deltaTime);

            evenOddFrame = (evenOddFrame == 0) ? 1 : 0;

            //swap_buffers();
            present();
        }
    }

    u32 mediumShutdown() override {
        //delete[] frontBuffer;
        //delete[] backBuffer;

        glfwDestroyWindow(window);
        glfwTerminate();
        return 0;
    }

    static std::filesystem::path getExecutablePath() {
#if defined(_WIN32)
        char buffer[1024];
        DWORD len = GetModuleFileName(nullptr, buffer, 1024);
        if (len == 0) throw std::runtime_error("GetModuleFileName failed");
        return std::filesystem::path(buffer);
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size); // get required size
        std::string buffer(size, '\0');

        if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
            throw std::runtime_error("NSGetExecutablePath failed");
        }

        return std::filesystem::canonical(buffer);
#elif defined(__linux__)
        char buffer[1024];
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer));
        if (len == -1) throw std::runtime_error("readlink failed");

        return std::filesystem::path(std::string(buffer, len));
#else
#error Unsupported platform
#endif
    }

    static std::filesystem::path getExecutableDir() {
        return getExecutablePath().parent_path();
    }

    std::filesystem::path getAssetRoot() override {
        return getExecutableDir() / "assets";
    }

    std::filesystem::path getSaveRoot() override {
        return getExecutableDir() / "saves";
    }

    [[nodiscard]] void* getNativeWindow() const override {
        return window;
    }

    void cursorHide() override {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }

    void cursorShow() override {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    void cursorCapture() override {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    MediumOpenGL(const int consoleWidth, const int consoleHeight, const int gameWidth, const int gameHeight, const std::string& str = "Medium OpenGL") :
        Medium(consoleWidth, consoleHeight, gameWidth, gameHeight, str) {};
};

#endif //MEDIUMOPENGL_H
