//
// Created by Nico Russo on 4/15/26.
//

#ifndef MEDIUMOPENGL_H
#define MEDIUMOPENGL_H

#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include <sstream>
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


class MediumOpenGL : public Medium {
public:
    bool useMainCanvas = true;
private:
    GLFWwindow* window = nullptr;
    GLuint shader = {};
    GLuint vao = {};
    u32* frontBuffer = nullptr;

    GLuint screen_texture = 0;

    struct Viewport { int x, y, w, h; };

    [[nodiscard]] Viewport computeLetterboxViewport(const int winW, const int winH) const {
        const float targetAspect = (float)GAME_WIDTH / (float)GAME_HEIGHT;
        const float windowAspect = (float)winW / (float)winH;

        int vpW, vpH;
        if (windowAspect > targetAspect) {
            // Window is wider than canvas → pillarbox
            vpH = winH;
            vpW = (int)(winH * targetAspect);
        } else {
            // Window is taller than canvas → letterbox
            vpW = winW;
            vpH = (int)(winW / targetAspect);
        }

        const int vpX = (winW - vpW) / 2;
        const int vpY = (winH - vpH) / 2;

        return { vpX, vpY, vpW, vpH };
    }

    static void framebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
        const MediumOpenGL* self = static_cast<MediumOpenGL*>(glfwGetWindowUserPointer(window));
        auto [x, y, w, h] = self->computeLetterboxViewport(width, height);
        glViewport(x, y, w, h);
    }

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

    GLuint createShader(const GLenum type, const char* src) {
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

    GLuint createShaderProgram(const char* vertSrc, const char* fragSrc) {
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

    GLuint createFullscreenQuadVAO() {
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

    // -----------------------------------------------------------------------
    // Accessors for GPUCanvas
    // -----------------------------------------------------------------------

    /// @brief Returns the fullscreen-quad VAO shared by all GPUCanvas instances.
    [[nodiscard]] GLuint getQuadVAO() const { return vao; }

    /// @brief Returns the default passthrough shader program.
    [[nodiscard]] GLuint getDefaultShader() const { return shader; }

    /**
     * @brief Compile and link a shader program from a fragment shader file.
     *
     * The vertex shader is always the built-in fullscreen-quad passthrough.
     * Your fragment shader receives:
     *   - in vec2 TexCoord          (0,0) top-left → (1,1) bottom-right
     *   - uniform sampler2D screenTexture   bound to unit 0
     *
     * Add any additional uniforms you need and set them after applyShader().
     * Returns 0 on failure (errors logged to stderr).
     * Call this once after mediumStartup() — GL must be initialized first.
     * The returned handle is owned by the caller; call glDeleteProgram() when done.
     *
     * @param fragPath  Path to the fragment shader source file.
     * @return Linked GL program ID, or 0 on failure.
     */
    [[nodiscard]] GLuint loadShaderProgram(const std::filesystem::path& fragPath) {
        std::ifstream f(fragPath);
        if (!f) {
            std::cerr << "loadShaderProgram: could not open " << fragPath << "\n";
            return 0;
        }
        std::ostringstream ss;
        ss << f.rdbuf();
        const std::string fragSrc = ss.str();
        if (fragSrc.empty()) return 0;

        const char* vertSrc = R"(
            #version 330 core
            layout(location = 0) in vec2 aPos;
            layout(location = 1) in vec2 aTexCoord;
            out vec2 TexCoord;
            void main() {
                gl_Position = vec4(aPos, 0.0, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        return createShaderProgram(vertSrc, fragSrc.c_str());
    }

    // -----------------------------------------------------------------------


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

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        auto [x, y, w, h] = computeLetterboxViewport(fbW, fbH);
        glViewport(x, y, w, h);
        glClearColor(0, 0, 0, 1);

        return 0;
    }

    void mediumRun(std::function<void(f32)> gameUpdate) override {
        double last_time = glfwGetTime();

        while (!glfwWindowShouldClose(window)) {

            double now = glfwGetTime();
            float dt = float(now - last_time);
            last_time = now;

            glfwPollEvents();

            // Recompute letterbox every frame — handles resize automatically
            int fbW, fbH;
            glfwGetFramebufferSize(window, &fbW, &fbH);
            auto vp = computeLetterboxViewport(fbW, fbH);
            glViewport(vp.x, vp.y, vp.w, vp.h);

            canvas.linkCanvas(backBuffer, GAME_WIDTH, GAME_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT);
            gameUpdate(dt);

            swap_buffers();
            if (useMainCanvas) {
                upload_front_buffer();
                render_screen();
            }

            glfwSwapBuffers(window);
        }
    }

    u32 mediumShutdown() override {
        delete[] frontBuffer;
        delete[] backBuffer;

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

};

// =============================================================================
//  GPUCanvas
//
//  A Graphite::Canvas paired with a GL texture, allowing CPU-side pixel drawing
//  to be uploaded to the GPU, optionally processed with a custom shader, and
//  composited onto the screen in any order.
//
//  Typical usage inside mediumRun's gameUpdate callback:
//
//      // --- Construction (once, outside the loop) ---
//      GPUCanvas background(&medium, GAME_WIDTH, GAME_HEIGHT);
//      GPUCanvas ui(&medium, GAME_WIDTH, GAME_HEIGHT);
//
//      // --- Each frame ---
//      background.canvas.fill(Colors::Black);
//      // ... draw background ...
//      background.uploadToGPU();
//      background.renderToScreen();          // opaque base layer
//
//      ui.canvas.fill(Colors::Invisible);
//      // ... draw UI elements ...
//      ui.uploadToGPU();
//      ui.renderToScreen();                  // alpha-blended on top (default)
//
//      // --- With a custom shader ---
//      ui.applyShader(myWobbleShader);       // binds the program; set uniforms after
//      glUniform1f(glGetUniformLocation(myWobbleShader, "time"), t);
//      ui.renderToScreen();
//
//  Memory / GL object ownership:
//      - If constructed with (medium, w, h): GPUCanvas allocates its own pixel
//        buffer via Canvas(w, h) and frees it on destruction.
//      - If constructed with (medium): the canvas starts empty; call
//        canvas.linkCanvas(ptr, w, h) to point it at an external buffer.
//        The external buffer is NOT freed on destruction.
//      - The GL texture is always owned by GPUCanvas and deleted in the destructor.
// =============================================================================

class GPUCanvas {
public:

    /// The CPU-side pixel surface. Draw into this each frame before calling uploadToGPU().
    Graphite::Canvas canvas;

    // -------------------------------------------------------------------------
    // Construction / destruction
    // -------------------------------------------------------------------------

    /**
     * @brief Construct a GPUCanvas that owns its own pixel buffer.
     *
     * Allocates a Canvas of the given dimensions and creates the backing GL texture.
     *
     * @param medium Pointer to the active MediumOpenGL (must outlive this object).
     * @param width  Canvas width in pixels.
     * @param height Canvas height in pixels.
     */
    GPUCanvas(MediumOpenGL* medium, uint32_t width, uint32_t height)
        : canvas(width, height)
        , medium_(medium)
        , width_(width)
        , height_(height)
    {
        initTexture();
    }

    /**
     * @brief Construct a GPUCanvas without an owned pixel buffer.
     *
     * The internal canvas starts empty. Call canvas.linkCanvas(ptr, w, h) before
     * the first uploadToGPU() to point it at an external pixel buffer.
     *
     * @param medium Pointer to the active MediumOpenGL (must outlive this object).
     */
    explicit GPUCanvas(MediumOpenGL* medium)
        : canvas()
        , medium_(medium)
        , width_(0)
        , height_(0)
    {
        // Texture is created lazily on the first uploadToGPU() call once we
        // know the canvas dimensions.
    }

    ~GPUCanvas() {
        if (texture_ != 0)
            glDeleteTextures(1, &texture_);
    }

    // Non-copyable; movable if needed, but not implemented here for simplicity.
    GPUCanvas(const GPUCanvas&)            = delete;
    GPUCanvas& operator=(const GPUCanvas&) = delete;

    // -------------------------------------------------------------------------
    // Core API
    // -------------------------------------------------------------------------

    /**
     * @brief Upload the current canvas pixels to the GPU texture.
     *
     * Must be called after all CPU-side drawing is finished for this frame and
     * before renderToScreen(). Safe to call every frame; uses glTexSubImage2D
     * so the texture object is not reallocated.
     *
     * If the canvas was linked to an external buffer after construction with the
     * no-size constructor, the texture is created on the first call here.
     */
    void uploadToGPU() {
        const uint32_t w = canvas.getWidth();
        const uint32_t h = canvas.getHeight();

        if (w == 0 || h == 0) return;

        // Lazy texture creation for the linked-buffer path.
        if (texture_ == 0 || w != width_ || h != height_) {
            if (texture_ != 0)
                glDeleteTextures(1, &texture_);
            width_  = w;
            height_ = h;
            initTexture();
        }

        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            0, 0,
            static_cast<GLsizei>(w),
            static_cast<GLsizei>(h),
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            canvas.getPixels()   // protected member — GPUCanvas lives in the same translation unit
        );
    }

    /**
     * @brief Bind a custom shader program for the upcoming renderToScreen() call.
     *
     * The shader must follow the same vertex contract as the built-in passthrough:
     *   - layout(location = 0): vec2 position   (fullscreen quad, already bound)
     *   - layout(location = 1): vec2 texCoord
     *   - uniform sampler2D screenTexture        (bound to unit 0)
     *
     * After calling applyShader() you may set additional uniforms before calling
     * renderToScreen(). The default shader is restored automatically on the next
     * renderToScreen() call unless applyShader() is called again first.
     *
     * @param shaderProgram A compiled and linked GL shader program ID.
     */
    void applyShader(const GLuint shaderProgram) {
        pendingShader_ = shaderProgram;
    }

    /**
     * @brief Draw this canvas's texture onto the screen.
     *
     * Uses the shader set by the last applyShader() call, or the medium's default
     * passthrough shader if applyShader() has not been called this frame.
     * After rendering, the pending shader is reset to the default so the next call
     * is clean without any extra bookkeeping.
     *
     * @param alphaBlend If true (default), enables GL_BLEND so transparent pixels
     *                   in this canvas reveal the layers rendered beneath it.
     *                   Pass false to overwrite whatever is already on screen.
     */
    void renderToScreen(const bool alphaBlend = true) {
        if (texture_ == 0) return;

        if (alphaBlend) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }

        const GLuint prog = (pendingShader_ != 0) ? pendingShader_ : medium_->getDefaultShader();
        glUseProgram(prog);
        glBindVertexArray(medium_->getQuadVAO());
        glBindTexture(GL_TEXTURE_2D, texture_);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Reset for next frame — caller doesn't need to manually clear this.
        pendingShader_ = 0;

        // Leave blend state enabled; MediumOpenGL's own render_screen() disables
        // it implicitly by calling glClear at the start of the next frame.
    }

private:

    MediumOpenGL* medium_       = nullptr;
    GLuint        texture_      = 0;
    GLuint        pendingShader_= 0;
    uint32_t      width_        = 0;
    uint32_t      height_       = 0;

    void initTexture() {
        glGenTextures(1, &texture_);
        glBindTexture(GL_TEXTURE_2D, texture_);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            static_cast<GLsizei>(width_),
            static_cast<GLsizei>(height_),
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
};

#endif //MEDIUMOPENGL_H
