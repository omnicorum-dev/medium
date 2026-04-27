//
// Created by Nico Russo on 4/15/26.
//

#ifndef MEDIUMOPENGL_H
#define MEDIUMOPENGL_H

#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
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


class MediumOpenGL : public Medium {
private:
    GLFWwindow* window = nullptr;
    GLuint shader = {};
    GLuint vao = {};
    u32* frontBuffer = nullptr;

    GLuint screen_texture = 0;

    // -----------------------------------------------------------------------
    // Scene depth FBO
    //
    // A single R32F texture that accumulates the maximum invZ value seen so
    // far across all GPUCanvas::renderToScreen(checkDepth=true) calls in the
    // current frame.  Cleared to 0.0 at the start of each frame (before
    // gameUpdate is called).  GPUCanvas reads this texture to decide whether
    // a fragment is occluded, then writes the winning invZ back into it via
    // the depthMerge shader.
    // -----------------------------------------------------------------------
    GLuint sceneDepthTex_ = 0;   ///< R32F texture storing per-pixel max invZ.
    GLuint sceneDepthFBO_ = 0;   ///< FBO with sceneDepthTex_ as colour attachment.

    GLuint depthMergeShader_ = 0; ///< Writes max(layerInvZ, sceneInvZ) → sceneDepthFBO_.
    GLuint depthCheckShader_ = 0; ///< Draws colour only where layerInvZ >= sceneInvZ.

    struct Viewport { int x, y, w, h; };

    [[nodiscard]] Viewport computeLetterboxViewport(const int winW, const int winH) const {
        const float targetAspect = (float)GAME_WIDTH / (float)GAME_HEIGHT;
        const float windowAspect = (float)winW / (float)winH;

        int vpW, vpH;
        if (windowAspect > targetAspect) {
            vpH = winH;
            vpW = (int)(winH * targetAspect);
        } else {
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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    // -----------------------------------------------------------------------
    // Scene depth FBO init
    //
    // Creates the R32F scene depth texture and an FBO that renders into it.
    // Called once from mediumStartup() after the GL context is ready.
    // -----------------------------------------------------------------------
    void initSceneDepthFBO(int w, int h) {
        // R32F texture — each texel holds one float (the max invZ seen so far).
        glGenTextures(1, &sceneDepthTex_);
        glBindTexture(GL_TEXTURE_2D, sceneDepthTex_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0,
                     GL_RED, GL_FLOAT, nullptr);

        // FBO with the depth texture as its single colour attachment.
        glGenFramebuffers(1, &sceneDepthFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, sceneDepthFBO_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, sceneDepthTex_, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "initSceneDepthFBO: framebuffer incomplete\n";

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

    // -----------------------------------------------------------------------
    // Built-in depth shaders
    //
    // Both shaders share the same fullscreen-quad vertex shader.  Graphite
    // stores invZ as a raw float reinterpreted into a uint32 pixel, so on the
    // GPU we upload the depth canvas as RGBA/GL_UNSIGNED_BYTE and reconstruct
    // the float via uintBitsToFloat in the shader.
    // -----------------------------------------------------------------------
    void initDepthShaders() {
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

        // depthMerge: reads the layer's depth texture (uploaded as RGBA, with
        // the raw float bits packed into the four bytes) and the current scene
        // depth texture, then writes max(layerInvZ, sceneInvZ) into the FBO.
        const char* depthMergeFrag = R"(
            #version 330 core
            in vec2 TexCoord;
            out float FragDepth;

            uniform sampler2D layerDepthTex;  // RGBA upload of the Graphite depth canvas
            uniform sampler2D sceneDepthTex;  // R32F scene accumulator

            float unpackInvZ(sampler2D tex, vec2 uv) {
                vec4 raw = texture(tex, uv);
                uint b0 = uint(raw.r * 255.0 + 0.5);
                uint b1 = uint(raw.g * 255.0 + 0.5);
                uint b2 = uint(raw.b * 255.0 + 0.5);
                uint b3 = uint(raw.a * 255.0 + 0.5);
                uint packed = b0 | (b1 << 8u) | (b2 << 16u) | (b3 << 24u);
                return uintBitsToFloat(packed);
            }

            void main() {
                float layerInvZ = unpackInvZ(layerDepthTex, TexCoord);
                float sceneInvZ = texture(sceneDepthTex, TexCoord).r;
                FragDepth = max(layerInvZ, sceneInvZ);
            }
        )";

        // depthCheck: draws the layer's colour only where layerInvZ >= sceneInvZ.
        // >= means equal depth lets the later renderToScreen() call win.
        const char* depthCheckFrag = R"(
            #version 330 core
            in vec2 TexCoord;
            out vec4 FragColor;

            uniform sampler2D colorTex;       // layer colour
            uniform sampler2D layerDepthTex;  // layer depth (RGBA-packed floats)
            uniform sampler2D sceneDepthTex;  // accumulated scene depth (R32F)

            float unpackInvZ(sampler2D tex, vec2 uv) {
                vec4 raw = texture(tex, uv);
                uint b0 = uint(raw.r * 255.0 + 0.5);
                uint b1 = uint(raw.g * 255.0 + 0.5);
                uint b2 = uint(raw.b * 255.0 + 0.5);
                uint b3 = uint(raw.a * 255.0 + 0.5);
                uint packed = b0 | (b1 << 8u) | (b2 << 16u) | (b3 << 24u);
                return uintBitsToFloat(packed);
            }

            void main() {
                float layerInvZ = unpackInvZ(layerDepthTex, TexCoord);
                float sceneInvZ = texture(sceneDepthTex, TexCoord).r;

                // Discard if behind the current scene depth.
                // >= lets the later renderToScreen() call win at equal depth.
                if (layerInvZ < sceneInvZ) discard;

                FragColor = texture(colorTex, TexCoord);
            }
        )";

        depthMergeShader_ = createShaderProgram(vertSrc, depthMergeFrag);
        depthCheckShader_  = createShaderProgram(vertSrc, depthCheckFrag);
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
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(float), (void*)(2 * sizeof(float)));

        glBindVertexArray(0);

        return vao;
    }

    inline void swap_buffers() {
        std::swap(frontBuffer, backBuffer);
    }

    void upload_front_buffer() {
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        GAME_WIDTH, GAME_HEIGHT,
                        GL_RGBA, GL_UNSIGNED_BYTE, frontBuffer);
    }

    void render_screen() {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, screen_texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

public:

    bool useMainCanvas = true; ///< Set false to skip the built-in canvas render pass.

    // -----------------------------------------------------------------------
    // Accessors for GPUCanvas
    // -----------------------------------------------------------------------

    [[nodiscard]] GLuint getQuadVAO()          const { return vao; }
    [[nodiscard]] GLuint getDefaultShader()    const { return shader; }
    [[nodiscard]] GLuint getDepthMergeShader() const { return depthMergeShader_; }
    [[nodiscard]] GLuint getDepthCheckShader() const { return depthCheckShader_; }
    [[nodiscard]] GLuint getSceneDepthTex()    const { return sceneDepthTex_; }
    [[nodiscard]] GLuint getSceneDepthFBO()    const { return sceneDepthFBO_; }

    // -----------------------------------------------------------------------

    /**
     * @brief Compile and link a shader program from a fragment shader file.
     *
     * The vertex shader is always the built-in fullscreen-quad passthrough.
     * Your fragment shader receives:
     *   - in vec2 TexCoord
     *   - uniform sampler2D screenTexture   (bound to unit 0)
     *
     * Returns 0 on failure (errors logged to stderr).
     * Call once after mediumStartup() — GL must be initialized first.
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

        window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
                                  windowName.c_str(), nullptr, nullptr);
        if (!window) return -1;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

        initBuffers(SCREEN_WIDTH, SCREEN_HEIGHT);
        initTexture(GAME_WIDTH, GAME_HEIGHT);
        initSceneDepthFBO(GAME_WIDTH, GAME_HEIGHT);
        initDepthShaders();

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

            int fbW, fbH;
            glfwGetFramebufferSize(window, &fbW, &fbH);
            auto vp = computeLetterboxViewport(fbW, fbH);
            glViewport(vp.x, vp.y, vp.w, vp.h);

            // Clear the scene depth accumulator at the start of every frame so
            // GPUCanvas depth tests always start from a clean slate.
            glBindFramebuffer(GL_FRAMEBUFFER, sceneDepthFBO_);
            glClearColor(0.f, 0.f, 0.f, 0.f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Restore the default clear colour after touching the FBO.
            glClearColor(0.f, 0.f, 0.f, 1.f);

            canvas.linkCanvas(backBuffer, GAME_WIDTH, GAME_HEIGHT);
            gameUpdate(dt);

            if (useMainCanvas) {
                swap_buffers();
                upload_front_buffer();
                render_screen();
            }

            glfwSwapBuffers(window);
        }
    }

    u32 mediumShutdown() override {
        delete[] frontBuffer;
        delete[] backBuffer;

        glDeleteTextures(1, &sceneDepthTex_);
        glDeleteFramebuffers(1, &sceneDepthFBO_);
        glDeleteProgram(depthMergeShader_);
        glDeleteProgram(depthCheckShader_);

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
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        if (_NSGetExecutablePath(buffer.data(), &size) != 0)
            throw std::runtime_error("NSGetExecutablePath failed");
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

    std::filesystem::path getAssetRoot() override { return getExecutableDir() / "assets"; }
    std::filesystem::path getSaveRoot()  override { return getExecutableDir() / "saves"; }

    [[nodiscard]] void* getNativeWindow() const override { return window; }

    void cursorHide()    override { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);   }
    void cursorShow()    override { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);   }
    void cursorCapture() override { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
};


// =============================================================================
//  GPUCanvas
//
//  A Graphite::Canvas (colour) paired with an optional Graphite::Canvas (depth),
//  each backed by a GL texture.  Supports layered compositing with per-pixel
//  depth testing against a shared scene depth accumulator owned by MediumOpenGL.
//
//  Depth canvas pixel format:
//      Graphite stores invZ (1/z) as a raw float reinterpreted into a uint32
//      pixel — the same format used by the 3D draw pipeline's z-buffer Canvas.
//      GPUCanvas uploads this verbatim as GL_RGBA/GL_UNSIGNED_BYTE and the
//      built-in depth shaders unpack the float on the GPU.
//
//  Typical usage:
//
//      // Construction — once, after mediumStartup()
//      GPUCanvas world(&medium, GAME_WIDTH, GAME_HEIGHT);         // colour + depth
//      GPUCanvas ui   (&medium, GAME_WIDTH, GAME_HEIGHT, false);  // colour only
//
//      // Each frame
//      world.clear(Colors::Invisible);   // fill colour with Invisible, zero depth
//      // ... draw into world.canvas, passing &world.depthCanvas to 3D calls ...
//      world.uploadToGPU();
//      world.renderToScreen(true, true); // alpha-blend + depth test
//
//      ui.canvas.fill(Colors::Invisible);
//      // ... draw HUD ...
//      ui.uploadToGPU();
//      ui.renderToScreen();              // no depth test — always on top
//
// =============================================================================

class GPUCanvas {
public:

    /// CPU-side colour surface. Draw into this each frame before uploadToGPU().
    Graphite::Canvas canvas;

    /// CPU-side depth surface (invZ values stored as raw float bits in uint32
    /// pixels). Pass &depthCanvas to Graphite 3D draw calls as the z-buffer.
    /// Empty (zero size) if this GPUCanvas was constructed without depth support.
    Graphite::Canvas depthCanvas;

    // -------------------------------------------------------------------------
    // Construction / destruction
    // -------------------------------------------------------------------------

    /**
     * @brief Construct a GPUCanvas that owns its own colour and (optionally) depth buffers.
     *
     * @param medium     Pointer to the active MediumOpenGL (must outlive this object).
     * @param width      Canvas width in pixels.
     * @param height     Canvas height in pixels.
     * @param withDepth  If true (default), allocates a depth canvas and depth texture.
     */
    GPUCanvas(MediumOpenGL* medium, uint32_t width, uint32_t height, bool withDepth = true)
        : canvas(width, height)
        , depthCanvas(withDepth ? Graphite::Canvas(width, height) : Graphite::Canvas())
        , medium_(medium)
        , width_(width)
        , height_(height)
        , hasDepth_(withDepth)
    {
        initColorTexture();
        if (hasDepth_) initDepthTexture();
        initTempTarget();
    }

    /**
     * @brief Construct a GPUCanvas without any owned pixel buffer.
     *
     * Both canvas and depthCanvas start empty.  Call canvas.linkCanvas() and
     * (optionally) linkDepthCanvas() before the first uploadToGPU().
     *
     * @param medium  Pointer to the active MediumOpenGL (must outlive this object).
     */
    explicit GPUCanvas(MediumOpenGL* medium)
        : canvas()
        , depthCanvas()
        , medium_(medium)
        , width_(0)
        , height_(0)
        , hasDepth_(false)
    {
        // Textures are created lazily on the first uploadToGPU() call.
        initTempTarget();
    }

    ~GPUCanvas() {
        if (colorTex_ != 0) glDeleteTextures(1, &colorTex_);
        if (depthTex_ != 0) glDeleteTextures(1, &depthTex_);
    }

    GPUCanvas(const GPUCanvas&)            = delete;
    GPUCanvas& operator=(const GPUCanvas&) = delete;

    // -------------------------------------------------------------------------
    // Linking external buffers
    // -------------------------------------------------------------------------

    /**
     * @brief Point the colour canvas at an externally-owned pixel buffer.
     *
     * The buffer is NOT freed on destruction.  The GPU texture is reallocated
     * automatically on the next uploadToGPU() if dimensions have changed.
     */
    void linkColorCanvas(uint32_t* ptr, uint32_t width, uint32_t height) {
        canvas.linkCanvas(ptr, width, height);
    }

    /**
     * @brief Point the depth canvas at an externally-owned pixel buffer.
     *
     * The buffer must contain raw float invZ values stored as uint32 pixels
     * (the same format Graphite's z-buffer Canvas uses).  NOT freed on destruction.
     */
    void linkDepthCanvas(uint32_t* ptr, uint32_t width, uint32_t height) {
        depthCanvas.linkCanvas(ptr, width, height);
        hasDepth_ = true;
    }

    // -------------------------------------------------------------------------
    // Convenience
    // -------------------------------------------------------------------------

    /**
     * @brief Fill the colour canvas with @p color and zero the depth canvas.
     *
     * Equivalent to canvas.fill(color) + memset(depthCanvas, 0).
     * invZ == 0.0f means "no geometry here", so zeroing is the correct clear.
     *
     * @param color  Colour to fill with (defaults to Colors::Invisible).
     */
    void clear(Graphite::Color color = Graphite::Colors::Invisible) {
        canvas.fill(color);
        if (hasDepth_ && depthCanvas.getWidth() > 0)
            std::memset(depthCanvas.getPixels(), 0,
                        depthCanvas.getWidth() * depthCanvas.getHeight() * sizeof(uint32_t));
    }

    // -------------------------------------------------------------------------
    // Core API
    // -------------------------------------------------------------------------

    /**
     * @brief Upload colour (and depth, if present) to the GPU.
     *
     * Call after all CPU-side drawing and before renderToScreen().
     * Handles lazy texture creation and reallocation on dimension change.
     */
    void uploadToGPU() {
        const uint32_t w = canvas.getWidth();
        const uint32_t h = canvas.getHeight();
        if (w == 0 || h == 0) return;

        // Reallocate if dimensions changed (also covers lazy-init path).
        if (colorTex_ == 0 || w != width_ || h != height_) {
            if (colorTex_ != 0) glDeleteTextures(1, &colorTex_);
            if (depthTex_ != 0) { glDeleteTextures(1, &depthTex_); depthTex_ = 0; }
            width_  = w;
            height_ = h;
            initColorTexture();
            if (hasDepth_ && depthCanvas.getWidth() == w && depthCanvas.getHeight() == h)
                initDepthTexture();
        }

        // Upload colour.
        glBindTexture(GL_TEXTURE_2D, colorTex_);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        static_cast<GLsizei>(w), static_cast<GLsizei>(h),
                        GL_RGBA, GL_UNSIGNED_BYTE, canvas.getPixels());

        // Upload depth if present and matching dimensions.
        if (hasDepth_ && depthCanvas.getWidth() == w && depthCanvas.getHeight() == h) {
            if (depthTex_ == 0) initDepthTexture(); // lazy-create for linkDepthCanvas path
            glBindTexture(GL_TEXTURE_2D, depthTex_);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                            static_cast<GLsizei>(w), static_cast<GLsizei>(h),
                            GL_RGBA, GL_UNSIGNED_BYTE, depthCanvas.getPixels());
        }
    }

    /**
     * @brief Bind a custom fragment shader for the next renderToScreen() call.
     *
     * Ignored when checkDepthBuffer is true — the built-in depthCheck shader
     * is always used in that path.  Resets to 0 after each renderToScreen().
     */
    void applyShader(GLuint shaderProgram) {
        if (colorTex_ == 0 || shaderProgram == 0) return;

        const GLuint quadVAO = medium_->getQuadVAO();

        // Render into temp texture
        glBindFramebuffer(GL_FRAMEBUFFER, tempFBO_);
        glViewport(0, 0, width_, height_);
        glDisable(GL_BLEND);

        glUseProgram(shaderProgram);
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTex_);
        glUniform1i(glGetUniformLocation(shaderProgram, "screenTexture"), 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Swap textures
        std::swap(colorTex_, tempTex_);
    }

    /**
     * @brief Composite this canvas onto the screen.
     *
     * Without depth (checkDepthBuffer == false):
     *   Draws in call order using standard alpha-blending.  This is the same
     *   behaviour as before depth support was added.
     *
     * With depth (checkDepthBuffer == true, depth canvas present):
     *   Pass 1 — depthMerge: renders max(layerInvZ, sceneInvZ) into the scene
     *            depth FBO so the accumulator always holds the nearest surface.
     *   Pass 2 — depthCheck: draws to the screen, discarding fragments where
     *            layerInvZ < sceneInvZ.  Equal depth lets the later call win.
     *
     * If checkDepthBuffer is true but no depth canvas is present, falls back
     * to the no-depth path with a warning logged to stderr.
     *
     * @param alphaBlend       Enable GL_BLEND on the colour draw (default true).
     * @param checkDepthBuffer Enable per-pixel depth testing  (default false).
     */
    void renderToScreen(bool alphaBlend = true, bool checkDepthBuffer = false) {
        if (colorTex_ == 0) return;

        if (checkDepthBuffer && hasDepth_ && depthTex_ != 0) {
            renderWithDepth(alphaBlend);
        } else {
            if (checkDepthBuffer)
                std::cerr << "GPUCanvas::renderToScreen: checkDepthBuffer requested "
                             "but no depth canvas is present — falling back.\n";
            renderNoDepth(alphaBlend);
        }

        //pendingShader_ = 0;
    }

private:

    MediumOpenGL* medium_        = nullptr;
    GLuint        colorTex_      = 0;
    GLuint        depthTex_      = 0;
    //GLuint        pendingShader_ = 0;
    GLuint        tempTex_       = 0;
    GLuint        tempFBO_       = 0;
    uint32_t      width_         = 0;
    uint32_t      height_        = 0;
    bool          hasDepth_      = false;

    // -----------------------------------------------------------------------
    // Texture helpers
    // -----------------------------------------------------------------------

    void initColorTexture() {
        glGenTextures(1, &colorTex_);
        glBindTexture(GL_TEXTURE_2D, colorTex_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     static_cast<GLsizei>(width_), static_cast<GLsizei>(height_),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    void initDepthTexture() {
        glGenTextures(1, &depthTex_);
        glBindTexture(GL_TEXTURE_2D, depthTex_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Same RGBA8 format as colour — raw uint32 bytes uploaded as four bytes.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     static_cast<GLsizei>(width_), static_cast<GLsizei>(height_),
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    void initTempTarget() {
        glGenTextures(1, &tempTex_);
        glBindTexture(GL_TEXTURE_2D, tempTex_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glGenFramebuffers(1, &tempFBO_);
        glBindFramebuffer(GL_FRAMEBUFFER, tempFBO_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, tempTex_, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // -----------------------------------------------------------------------
    // Render paths
    // -----------------------------------------------------------------------

    void renderNoDepth(bool alphaBlend) {
        if (alphaBlend) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }

        /*
        const GLuint prog = (pendingShader_ != 0)
                            ? pendingShader_
                            : medium_->getDefaultShader();
        */

        const GLuint prog = medium_->getDefaultShader();

        glUseProgram(prog);
        glBindVertexArray(medium_->getQuadVAO());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTex_);
        glUniform1i(glGetUniformLocation(prog, "screenTexture"), 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void renderWithDepth(bool alphaBlend) {
        const GLuint quadVAO       = medium_->getQuadVAO();
        const GLuint mergeShader   = medium_->getDepthMergeShader();
        const GLuint checkShader   = medium_->getDepthCheckShader();
        const GLuint sceneDepthTex = medium_->getSceneDepthTex();
        const GLuint sceneDepthFBO = medium_->getSceneDepthFBO();

        // ------------------------------------------------------------------
        // Pass 1 — depth merge
        //
        // Render into the scene depth FBO, writing max(layerInvZ, sceneInvZ).
        // Viewport must match the depth texture dimensions exactly.
        // ------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, sceneDepthFBO);
        glDisable(GL_BLEND);
        glViewport(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));

        glUseProgram(mergeShader);
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthTex_);
        glUniform1i(glGetUniformLocation(mergeShader, "layerDepthTex"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
        glUniform1i(glGetUniformLocation(mergeShader, "sceneDepthTex"), 1);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // ------------------------------------------------------------------
        // Pass 2 — depth-tested colour draw
        //
        // Restore the screen framebuffer, then draw colour with discard for
        // fragments that lost the depth test.
        // ------------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (alphaBlend) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        } else {
            glDisable(GL_BLEND);
        }

        glUseProgram(checkShader);
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorTex_);
        glUniform1i(glGetUniformLocation(checkShader, "colorTex"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthTex_);
        glUniform1i(glGetUniformLocation(checkShader, "layerDepthTex"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, sceneDepthTex);
        glUniform1i(glGetUniformLocation(checkShader, "sceneDepthTex"), 2);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Restore texture unit 0 as the active unit (convention).
        glActiveTexture(GL_TEXTURE0);
    }
};


#endif //MEDIUMOPENGL_H