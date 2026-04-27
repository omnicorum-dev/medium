//
// Created by Nico Russo on 4/27/26.
//

#ifdef RELEASE_MODE
#define DISABLE_TRACE
#define DISABLE_DEBUG
#endif

#include <mediumOpenGL.h>
#include <inputGLFW.h>

#define WIDTH  800
#define HEIGHT 600

MediumOpenGL game;
Input* Input::instance = nullptr;
std::map<std::pair<int,int>, Input::EventCallback> Input::eventCallbacks;
Input::GlobalEventCallback Input::globalCallback;

void globalEventCallback(int key, int action, int mods, double x, double y) {

}

Graphite::Canvas background(WIDTH,HEIGHT);
Graphite::Canvas world1(WIDTH,HEIGHT);
Graphite::Canvas world2(WIDTH,HEIGHT);
Graphite::Canvas ui(WIDTH,HEIGHT);
Graphite::Canvas zBuffer(WIDTH,HEIGHT);

Shader shader1 = 0;
Shader shader2 = 0;

std::filesystem::path assetRoot = game.getAssetRoot();
std::filesystem::path saveRoot  = game.getSaveRoot();

struct GameState {
    bool yes = true;
    Graphite::Camera camera = {
        {0, 0, -2},
        {0, 0, 0, }
    };
    glm::fvec3 sunAngle = glm::normalize(glm::fvec3{-1, -1, -1});
};

GameState gameState;

Graphite::Object3D uvSphere  = Graphite::loadOBJ(assetRoot / "uv_sphere.obj");
Graphite::Object3D icosphere = Graphite::loadOBJ(assetRoot / "icosphere.obj");
Graphite::Object3D cube      = Graphite::loadOBJ(assetRoot / "cube.obj");
Graphite::Object3D teapotLow = Graphite::loadOBJ(assetRoot / "utah_teapot_2.obj");
Graphite::Object3D teapotHi  = Graphite::loadOBJ(assetRoot / "utah_teapot_6.obj");

Graphite::Canvas texture(assetRoot / "sphere_uv.png");

float fps = 0;

void gameUpdate(const float dt) {
    // CLEAR UI
    world1.clear();
    world2.clear();
    ui.clear();
    zBuffer.clear();

    // ROTATE SPHERES
    uvSphere.rotation.y  += 1.0f * dt;
    icosphere.rotation.y -= 1.0f * dt;
    cube.rotation.x -= 1.0f * dt;

    constexpr float moveSpeed = 5.f;
    constexpr float turnSpeed = 2.f;

    // MOVE CAMERA
    if (Input::isKeyPressed(MED_KEY_W)) {gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::FORWARD) * moveSpeed * dt;}
    if (Input::isKeyPressed(MED_KEY_S)) {gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::BACKWARD) * moveSpeed * dt;}
    if (Input::isKeyPressed(MED_KEY_A)) {gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::LEFT) * moveSpeed * dt;}
    if (Input::isKeyPressed(MED_KEY_D)) {gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::RIGHT) * moveSpeed * dt;}
    if (Input::isKeyPressed(MED_KEY_E)) {gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::UP) * moveSpeed * dt;}
    if (Input::isKeyPressed(MED_KEY_Q)) {gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::DOWN) * moveSpeed * dt;}

    if (Input::isKeyPressed(MED_KEY_I)) { gameState.camera.rotation.x -= turnSpeed * dt; }
    if (Input::isKeyPressed(MED_KEY_K)) { gameState.camera.rotation.x += turnSpeed * dt; }
    if (Input::isKeyPressed(MED_KEY_J)) { gameState.camera.rotation.y -= turnSpeed * dt; }
    if (Input::isKeyPressed(MED_KEY_L)) { gameState.camera.rotation.y += turnSpeed * dt; }

    // DRAW OBJECTS

    /*
    gameState.camera.drawObjectSingleColor(uvSphere, world2, Graphite::Colors::Blue,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObjectSingleColor(icosphere, world2, Graphite::Colors::Blue,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObjectSingleColor(teapotHi, world2, Graphite::Colors::Blue,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObjectSingleColor(teapotLow, world2, Graphite::Colors::Blue,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObjectSingleColor(cube, world1, Graphite::Colors::Blue,
        {&zBuffer, true, true, gameState.sunAngle});
    */

    gameState.camera.drawObject(uvSphere, world1,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObject(icosphere, world1,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObject(cube, world2,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObject(teapotHi, world2,
        {&zBuffer, true, true, gameState.sunAngle});
    gameState.camera.drawObject(teapotLow, world2,
        {&zBuffer, true, true, gameState.sunAngle});


    // DRAW UI
    const float current_fps = 1.0f / dt;
    fps += (current_fps - fps) * 0.1f;  // alpha = 0.1 → smooth, ~10 frame lag
    const std::string s = "fps: " + std::to_string(static_cast<int>(fps));
    ui.writeOmniStringBaseline(s, 10, 40, 4);

    // RENDER EVERYTHING
    game.renderCanvas(background);
    game.renderCanvas(world2);
    game.renderCanvas(world1);
    game.renderCanvas(ui);
}

void gameStart() {
    Input::instance = new InputGLFW(&game);
    Input::registerGlobalCallback(globalEventCallback);

    shader1 = MediumOpenGL::buildShader(assetRoot / "crtFrag.glsl");
    shader2 = MediumOpenGL::buildShader(assetRoot / "crtFrag2.glsl");

    background.fill({0xff909090});

    uvSphere.position  = {-2, 0, 2};
    icosphere.position = {2, 0, 2};
    cube.position      = {0, 0, 2};
    teapotLow.position = {0, 2, 2};
    teapotHi.position  = {0, -4, 2};

    uvSphere.tex = &texture;
    icosphere.tex = &texture;
    cube.tex = &texture;
    teapotLow.tex = &texture;
    teapotHi.tex = &texture;
}

int main(int argc, char** argv) {
    game.setWindowName("NEW.CPP");
    game.mediumInit(WIDTH*2,HEIGHT*2,WIDTH,HEIGHT);
    game.mediumStartup();

    gameStart();

    game.mediumRun(gameUpdate);
    game.mediumShutdown();
    delete Input::instance;
    return 0;
}