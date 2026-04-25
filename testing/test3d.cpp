//
// Created by Nico Russo on 4/17/26.
//
//#define DISABLE_TRACE
#include <graphite.h>
#include <mediumOpenGL.h>
#include <inputGLFW.h>
#include <base.h>
#include <random>

//using namespace omni;
using namespace Graphite;

#define WIDTH  (800)
#define HEIGHT (600)

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

constexpr Color BACKGROUND = Colors::DarkGrey;
constexpr Color FOREGROUND = Colors::Green;

bool  logViewEnabled = false;
float logScroll = 0;

int visibleLines = 10;

std::string userInput;

void clearNew() {
    canvas.fill(0x18181818);
    zBuffer.fill(0x00000000);
}

Object3D sphere = loadOBJ("../uv_sphere.obj");
Canvas sphereTex("../sphere_uv.png");

Object3D teapot = loadOBJ("../teapot_6.obj");
Canvas teapotTex("../teapot_6_tex.png");

Canvas uvTex("../uv_tex.jpg");

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

void handleCommand(const std::string& command) {
    std::stringstream ss(command);
    std::string word;
    while (ss >> word) {
        if (word == "set") {
            ss >> word;
            if (ss.fail()) {
                omni::LOG_ERROR("Invalid command '{}'", command);
                return;
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

void gameUpdate(const f32 dt) {
    clearNew();

    constexpr float moveSpeed = 5.f;
    constexpr float rotSpeed = 3.f;

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

    camera.drawObject(teapot, canvas, &zBuffer);
    camera.drawObject(sphere, canvas, &zBuffer);

    const f32 fps = 1/dt;
    average_fps = (average_fps + fps)/2;
    canvas.writeStringBaseline(stringPrint("FPS {}", average_fps), 10, 28, 16, Colors::White);

    if (logViewEnabled) {
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
        canvas.writeStringBaseline(userInput, 10, canvas.getHeight() - 30, 8, Colors::White);
    }
}

int main(int argc, char **argv) {
    game.setWindowName("3D Testing");

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
    Input::registerEventCallback(MED_KEY_MINUS, MED_PRESS, []()  {logScroll += 1;});
    Input::registerEventCallback(MED_KEY_MINUS, MED_REPEAT, []() {logScroll += 1;});
    Input::registerEventCallback(MED_KEY_EQUAL, MED_PRESS, []()  {logScroll -= 1;});
    Input::registerEventCallback(MED_KEY_EQUAL, MED_REPEAT, []() {logScroll -= 1;});

    Input::registerGlobalCallback(globalEventCallback);

    game.mediumRun(gameUpdate);
    game.mediumShutdown();

    delete Input::instance;

    return 0;
}