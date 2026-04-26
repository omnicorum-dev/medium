//
// Created by Nico Russo on 4/17/26.
//
//#define DISABLE_TRACE

#ifdef RELEASE_MODE
#define DISABLE_TRACE
#define DISABLE_DEBUG
#endif

#include <graphite.h>
#include <mediumOpenGL.h>
#include <inputGLFW.h>
#include <base.h>
#include <random>

//using namespace omni;
using namespace Graphite;

#define WIDTH  (1920/3)
#define HEIGHT (1080/3)

#define LOG_LEVEL_CMD 6

namespace omni::basic {
    template<typename... Args>
    void _impl_log_cmd(const std::string& fmt, const char* file, const int line, const Args&... args) {
        std::cerr << file << ":" << line << ANSI_CYAN << "   [CMD] ";
        print(std::cerr, fmt, args...);
        std::cerr << ANSI_RESET << std::endl;

        logger.emplace_back(LOG_LEVEL_CMD, stringPrint(fmt, args...));

        if (!logPath.empty()) {
            std::ofstream os(logPath, std::ios_base::app);
            os << file << ":" << line << "   [CMD] ";
            print(os, fmt, args...);
            os << std::endl;
            os.close();
        }
    }

#define LOG_CMD(fmt, ...) _impl_log_cmd(fmt, __FILE__, __LINE__, ##__VA_ARGS__)
}

MediumOpenGL game;
Input* Input::instance = nullptr;
std::map<std::pair<int,int>, Input::EventCallback> Input::eventCallbacks;
Input::GlobalEventCallback Input::globalCallback;
Canvas& canvas = game.canvas;
Canvas zBuffer(WIDTH, HEIGHT);

std::filesystem::path assetRoot = game.getAssetRoot();
std::filesystem::path saveRoot  = game.getSaveRoot();

constexpr Color BACKGROUND = Colors::DarkGrey;
constexpr Color FOREGROUND = Colors::Green;

enum RenderType {
    TEX, COLOR, DEPTH, VERTEX, SINGLE
};

bool  logViewEnabled = false;
float logScroll = 0;
int visibleLines = 10;
bool diffuseLightingEnabled = false;
RenderType renderType = RenderType::VERTEX;
bool renderVertices = false;
bool renderSunVector = false;
Color vertexColor = Colors::Blue;
Color objectColor = Colors::Green;
Color sunVecColor = Colors::Yellow;
int vertexPointSize = 1;

std::string userInput;

void clearNew() {
    canvas.fill(0x18181818);
    zBuffer.fill(0x00000000);
}

Object3D sphere = loadOBJ(assetRoot / "uv_sphere.obj");
Canvas sphereTex(assetRoot / "sphere_uv.png");

Object3D teapot = loadOBJ(assetRoot / "cube.obj");
Canvas teapotTex(assetRoot / "teapot_6_tex.png");

Canvas uvTex(assetRoot / "uv_tex.jpg");

Camera camera = {
    {0, 0, -2},
    {0, 0, 0}
};

f32 average_fps = 0;

bool isValidNumber(const std::string& s)
{
    try {
        size_t pos;
        std::stod(s, &pos);
        return pos == s.size();
    }
    catch (...) {
        return false;
    }
}

bool commandFailed(const std::stringstream& ss, const std::string& command) {
    if (ss.fail()) {
        omni::LOG_ERROR("Invalid command '{}'", command);
        return true;
    }
    return false;
}

void commandFailed(const std::string& command) {
    omni::LOG_ERROR("Invalid command '{}'", command);
}

void handleCommand(const std::string& command) {
    std::stringstream ss(command);
    std::string word;
    while (ss >> word) {
        if (word == "help") {
            omni::LOG_INFO("render <tex/color/depth/vertex/single>:");
            omni::LOG_INFO("      set which render type is displayed");
            omni::LOG_INFO("               diffuse <on/off>: enable or disable diffuse lighting");
            omni::LOG_INFO("                 lines <number>: number of lines visible in console");
            omni::LOG_INFO("              obj color [color]: set color of objects in single mode");
            omni::LOG_INFO("                points <on/off>: render vertices of objects");
            omni::LOG_INFO("           points color [color]: set color of vertex points");
            omni::LOG_INFO("                sunvec <on/off>: render arrow showing sun direction");
            omni::LOG_INFO("           sunvec color [color]: set sun vector color");
            omni::LOG_INFO("                         colors: list available colors");
        }
        if (word == "colors") {
            omni::LOG_INFO("Red, Green, Blue, LightBlue, White, Black, Yellow, Pink");
            omni::LOG_INFO("Orange, Cyan, Purple, Tan, DarkGrey, Grey, LightGrey, Brown");
        }
        if (word == "clear") {
            logger.clear();
        }
        if (word == "render") {
            ss >> word;
            if (commandFailed(ss, word)) return;
            if (word == "tex") {
                renderType = RenderType::TEX;
            } else if (word == "color") {
                renderType = RenderType::COLOR;
            } else if (word == "depth") {
                renderType = RenderType::DEPTH;
            } else if (word == "vertex") {
                renderType = RenderType::VERTEX;
            } else if (word == "single") {
                renderType = RenderType::SINGLE;
            } else {
                commandFailed(command);
            }
        }
        if (word == "diffuse") {
            ss >> word;
            if (commandFailed(ss, word)) return;
            if (word == "on") {
                diffuseLightingEnabled = true;
            } else if (word == "off") {
                diffuseLightingEnabled = false;
            } else {
                commandFailed(command);
            }
        }
        if (word == "points") {
            ss >> word;
            if (commandFailed(ss, word)) return;
            if (word == "on") {
                renderVertices = true;
            } else if (word == "off") {
                renderVertices = false;
            } else if (word == "color") {
                ss >> word;
                if (commandFailed(ss, word)) return;
                vertexColor = stringToColor(word);
            } else if (word == "size") {
                ss >> word;
                if (ss.fail() || !isValidNumber(word)) {
                    omni::LOG_ERROR("Invalid command '{}'", command);
                    return;
                }
                vertexPointSize = std::stoi(word);
            }
            else {
                commandFailed(command);
            }
        }
        if (word == "obj") {
            ss >> word;
            if (commandFailed(ss, word)) return;
            if (word == "color") {
                ss >> word;
                if (commandFailed(ss, word)) return;
                objectColor = stringToColor(word);
            }
        }
        if (word == "sunvec") {
            ss >> word;
            if (commandFailed(ss, word)) return;
            if (word == "on") {
                renderSunVector = true;
            } else if (word == "off") {
                renderSunVector = false;
            } else if (word == "color") {
                ss >> word;
                if (commandFailed(ss, word)) return;
                sunVecColor = stringToColor(word);
            }
            else {
                commandFailed(command);
            }
        }
        if (word == "lines") {
            ss >> word;
            if (ss.fail() || !isValidNumber(word)) {
                omni::LOG_ERROR("Invalid command '{}'", command);
                return;
            }
            visibleLines = std::stoi(word);
        }
    }
}

void globalEventCallback(int key, int action, int mods, double x, double y) {
    if (logViewEnabled) {
        if (key == MED_MOUSE_SCROLL) {
            //omni::LOG_INFO("Scroll: ({}, {})", x, y);
            logScroll += y;
        }
        if (action != MED_RELEASE) {
            const char typed = keycodeToChar(key, mods);
            if (typed != '\0'
                && typed != '`'
                && typed != '-'
                && typed != '=')
                { userInput += typed; }
            if (key == MED_KEY_BACKSPACE) {
                if (!userInput.empty()) {
                    userInput.pop_back();
                }
            }
            if (key == MED_KEY_ENTER) {
                omni::LOG_CMD(userInput);
                handleCommand(userInput);
                userInput = "";
            }
        }
    }
}

void leftMouseClick() {
    omni::LOG_DEBUG("Left mouse");
}

void renderLogView() {
    int y = 53;
    canvas.fillRect(5, 40, canvas.getWidth() - 10, 12 * visibleLines + 4, Colors::Black);
    int count = (int)logger.size();

    // maximum amount you can scroll upward
    int maxScroll = std::max(0, count - visibleLines);

    // clamp global scroll value
    if (logScroll < 0) logScroll = 0;
    if (logScroll > maxScroll) logScroll = maxScroll;

    // start index:
    // scroll 0 = newest logs
    // scroll max = oldest logs
    int start = count - visibleLines - logScroll;
    if (start < 0) start = 0;

    int end = std::min(count, start + visibleLines);

    for (int i = start; i < end; ++i) {
        const LogEvent& logEvent = logger[i];
        std::string msg;

        switch (logEvent.level) {
            case LOG_LEVEL_INFO:
                msg = stringPrint(" [INFO] {}", logEvent.message);
                canvas.writeStringBaseline(msg, 10, y, 8, Colors::Green);
                break;

            case LOG_LEVEL_DEBUG:
                msg = stringPrint("[DEBUG] {}", logEvent.message);
                canvas.writeStringBaseline(msg, 10, y, 8, Colors::LightBlue);
                break;

            case LOG_LEVEL_WARN:
                msg = stringPrint(" [WARN] {}", logEvent.message);
                canvas.writeStringBaseline(msg, 10, y, 8, Colors::Yellow);
                break;

            case LOG_LEVEL_ERROR:
                msg = stringPrint("[ERROR] {}", logEvent.message);
                canvas.writeStringBaseline(msg, 10, y, 8, Colors::Red);
                break;

            case LOG_LEVEL_FATAL:
                msg = stringPrint("[FATAL] {}", logEvent.message);
                canvas.writeStringBaseline(msg, 10, y, 8, Colors::Red);
                break;

            case LOG_LEVEL_TRACE:
                msg = stringPrint("[TRACE] {}", logEvent.message);
                canvas.writeStringBaseline(msg, 10, y, 8, Colors::LightGrey);
                break;

            case LOG_LEVEL_CMD:
                msg = stringPrint("  [CMD] {}", logEvent.message);
                canvas.writeStringBaseline(msg, 10, y, 8, Colors::Pink);
                break;
            default:
                LOG_ERROR("INVALID COMMAND TYPE");
        }

        y += 12;
    }


    canvas.fillRect(5, canvas.getHeight() - (18+25), canvas.getWidth() - 10, 18, Colors::Black);
    canvas.writeStringBaseline(userInput + "_", 10, canvas.getHeight() - 30, 8, Colors::White);
}

fvec3 sun = glm::normalize(fvec3{-1, -1, -1});

void gameUpdate(const f32 dt) {
    clearNew();

    // =========== MOVEMENTS ===========
    constexpr float moveSpeed = 5.f;
    constexpr float rotSpeed = 2.f;
    constexpr float objRotSpeed = 0.5f;

    sphere.rotation.x += objRotSpeed * dt;
    sphere.rotation.y += objRotSpeed * dt;
    teapot.rotation.x += objRotSpeed * dt;
    teapot.rotation.y += objRotSpeed * dt;

    sun = rotateY(sun, rotSpeed * dt);

    if (!logViewEnabled) {
        if (Input::isKeyPressed(MED_KEY_W)) {camera.position = camera.position + camera.directionObj(Dir3D::FORWARD) * moveSpeed * dt;}
        if (Input::isKeyPressed(MED_KEY_S)) {camera.position = camera.position + camera.directionObj(Dir3D::BACKWARD) * moveSpeed * dt;}
        if (Input::isKeyPressed(MED_KEY_A)) {camera.position = camera.position + camera.directionObj(Dir3D::LEFT) * moveSpeed * dt;}
        if (Input::isKeyPressed(MED_KEY_D)) {camera.position = camera.position + camera.directionObj(Dir3D::RIGHT) * moveSpeed * dt;}
        if (Input::isKeyPressed(MED_KEY_E)) {camera.position = camera.position + camera.directionObj(Dir3D::UP) * moveSpeed * dt;}
        if (Input::isKeyPressed(MED_KEY_Q)) {camera.position = camera.position + camera.directionObj(Dir3D::DOWN) * moveSpeed * dt;}

        if (Input::isKeyPressed(MED_KEY_I)) { camera.rotation.x -= rotSpeed * dt; }
        if (Input::isKeyPressed(MED_KEY_K)) { camera.rotation.x += rotSpeed * dt; }
        if (Input::isKeyPressed(MED_KEY_J)) { camera.rotation.y -= rotSpeed * dt; }
        if (Input::isKeyPressed(MED_KEY_L)) { camera.rotation.y += rotSpeed * dt; }
    }

    // =========== MAIN RENDERING ===========
    switch (renderType) {
        case TEX:
            camera.drawObjectTexture(teapot, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            camera.drawObjectTexture(sphere, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            break;
        case COLOR:
            camera.drawObjectColor(teapot, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            camera.drawObjectColor(sphere, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            break;
        case DEPTH:
            camera.drawObjectDepth(teapot, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            camera.drawObjectDepth(sphere, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            break;
        case VERTEX:
            camera.drawObjectVertexColor(teapot, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            camera.drawObjectVertexColor(sphere, canvas, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            break;
        case SINGLE:
            camera.drawObjectSingleColor(teapot, canvas, objectColor, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
            camera.drawObjectSingleColor(sphere, canvas, objectColor, {.zBuffer = &zBuffer, .diffuse = diffuseLightingEnabled, .sunVector = sun});
    }

    if (renderVertices) {
        camera.drawObjectVertices(sphere, canvas, vertexColor, vertexPointSize);
        camera.drawObjectVertices(teapot, canvas, vertexColor, vertexPointSize);
    }

    // =========== PURE SPHERE EXPERIMENT ===========
    constexpr fvec3 spherePosition = {0, 2, 6};
    const vec2 positionScreen = camera.transformPoint(spherePosition);

    if (positionScreen != vec2(TRANSFORM_OUT_OF_CAMERA_BOUNDS)) {
        float sphereRadius = 1;
        const vec2 positionPixel = normalizedToScreen(positionScreen, canvas.getWidth(), canvas.getHeight());
        const float projectedRadius = camera.worldRadiusToPixels(spherePosition, sphereRadius, canvas);

        canvas.drawCircle(positionPixel, projectedRadius, Colors::Red);
    }

    // =========== RENDER SUN VECTOR ===========
    if (renderSunVector) {
        const vec2 sunDirCam = camera.transformDirection(sun) * 20.0f;
        const vec2 center = {canvas.getWidth() / 2, canvas.getHeight() / 2};
        canvas.drawArrow(center, center - sunDirCam, sunVecColor, 1, 5);
    }

    // =========== FPS COUNTER ===========
    const f32 fps = 1/dt;
    average_fps = (average_fps + fps)/2;
    canvas.writeStringBaseline(stringPrint("FPS {}", average_fps), 10, 28, 16, Colors::White);

    // =========== CONSOLE VIEW ===========
    if (logViewEnabled) {
        renderLogView();
    }
}

int main(int argc, char **argv) {
    game.setWindowName("3D Testing");

    LOG_DEBUG("Assets Root: {}", assetRoot.string());
    LOG_DEBUG("Save Root:   {}", saveRoot.string());

    teapot.tex = &uvTex;
    sphere.tex = &uvTex;

    uvTex.writeOmniStringBaseline("let's fucking go!!", 10, 200, 7);
    uvTex.writeOmniStringBaseline("yeah i can't lie this is sick as hell", 10, 275, 3);

    sphere.position = {0, 0, 10};

    game.mediumInit(WIDTH*2, HEIGHT*2, WIDTH, HEIGHT);
    LOG_DEBUG("Game Resolution: {}x{}", game.GAME_WIDTH, game.GAME_HEIGHT);
    LOG_DEBUG("Screen Resolution: {}x{}", game.SCREEN_WIDTH, game.SCREEN_HEIGHT);
    LOG_ERROR("Example Error");
    LOG_FATAL("Example Fatal Error");
    LOG_INFO("Example Info");
    LOG_TRACE("Example Trace");
    LOG_WARN("Example Warn");

    game.mediumStartup();

    Input::instance = new InputGLFW(&game);
    Input::registerEventCallback(MED_MOUSE_BUTTON_LEFT, MED_PRESS, leftMouseClick);
    Input::registerEventCallback(MED_KEY_GRAVE_ACCENT, MED_PRESS, []() { logViewEnabled = !logViewEnabled; logScroll = 0; });

    Input::registerGlobalCallback(globalEventCallback);

    game.mediumRun(gameUpdate);
    game.mediumShutdown();

    delete Input::instance;

    return 0;
}