//
// Created by Nico Russo on 4/27/26.
//

#include <mediumOpenGLBroken.h>
#include <inputGLFW.h>

#define WIDTH  800
#define HEIGHT 600


Input* Input::instance = nullptr;
std::map<std::pair<int,int>, Input::EventCallback> Input::eventCallbacks;
Input::GlobalEventCallback Input::globalCallback;

MediumOpenGL game;

std::filesystem::path assetRoot = game.getAssetRoot();
std::filesystem::path saveRoot  = game.getSaveRoot();

std::unique_ptr<GPUCanvas> background;
std::unique_ptr<GPUCanvas> objects;
std::unique_ptr<GPUCanvas> objects2;
std::unique_ptr<GPUCanvas> foreground;

Graphite::Object3D sphere = Graphite::loadOBJ(assetRoot / "icosphere.obj");
Graphite::Object3D sphere2 = Graphite::loadOBJ(assetRoot / "icosphere.obj");

Graphite::Camera camera = {
    {0, 0, -3},
    {0, 0, 0}
};

float elapsedTime = 0.0f;

void globalEventCallback(int key, int action, int mods, double x, double y) {
    //LOG_DEBUG("global callback!");
}

void gameUpdate(float dt) {
    // --- Base layer: background, drawn opaque ---
    background->canvas.fill(Graphite::Colors::DarkGrey);
    background->canvas.fillCircle(100, 100, 50, Graphite::Colors::Red);
    background->uploadToGPU();
    background->renderToScreen(false);  // opaque — clears whatever was beneath

    // --- Middle layer: game world with a wobble effect ---
    objects->canvas.fill(Graphite::Colors::Invisible);
    // ... draw world stuff ...
    objects->canvas.fillCircle(100, 100, 50, Graphite::Colors::Red);
    camera.drawObjectSingleColor(sphere,  objects->canvas,  Graphite::Colors::Red,  {.zBuffer = &objects->depthCanvas, .cullBackface = true});
    camera.drawObjectSingleColor(sphere2, objects2->canvas, Graphite::Colors::Blue, {.zBuffer = &objects2->depthCanvas, .cullBackface = true});

    objects->uploadToGPU();
    objects2->uploadToGPU();
    //objects.applyShader(wobbleShader);
    //glUniform1f(glGetUniformLocation(wobbleShader, "time"), elapsedTime);

    // alpha-blended on top of background
    objects2->renderToScreen(true, true);
    objects->renderToScreen(true, true);

    // --- Top layer: UI, no shader ---
    foreground->canvas.fill(Graphite::Colors::Invisible);
    std::string text = "Time: " + std::to_string(elapsedTime);
    foreground->canvas.writeStringBaseline(text, {10, 20}, 16, Graphite::Colors::White);
    foreground->uploadToGPU();
    foreground->renderToScreen();  // alpha-blended on top of everything

    sphere2.position.z -= 0.5f * dt;

    elapsedTime += dt;
}

int main() {
    LOG_DEBUG("Hello, World!");

    sphere2.position = {1, 0, 2};

    game.setWindowName("GPU System Testing");
    game.mediumInit(WIDTH, HEIGHT, WIDTH, HEIGHT);
    game.mediumStartup();
    game.useMainCanvas = false;

    Input::instance = new InputGLFW(&game);
    Input::registerGlobalCallback(globalEventCallback);

    background = std::make_unique<GPUCanvas>(&game, WIDTH, HEIGHT, true);
    objects    = std::make_unique<GPUCanvas>(&game, WIDTH, HEIGHT, true);
    objects2   = std::make_unique<GPUCanvas>(&game, WIDTH, HEIGHT, true);
    foreground = std::make_unique<GPUCanvas>(&game, WIDTH, HEIGHT, true);

    game.mediumRun(gameUpdate);

    game.mediumShutdown();

    delete Input::instance;

    return 0;
}