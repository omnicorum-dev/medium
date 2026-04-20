//
// Created by Nico Russo on 4/17/26.
//
#define DISABLE_TRACE
#include <graphite.h>
#include <mediumOpenGL.h>
#include <inputGLFW.h>
#include <base.h>
#include <random>

using namespace omni;
using namespace Graphite;

#define WIDTH  320
#define HEIGHT 180

MediumOpenGL game;
Input* Input::instance = new InputGLFW(&game);
Canvas& canvas = game.canvas;
Canvas zBuffer(WIDTH, HEIGHT);

constexpr Color BACKGROUND = Colors::DarkGrey;
constexpr Color FOREGROUND = Colors::Green;

void clearNew() {
    canvas.fill(0x18181818);
    zBuffer.fill(0x00000000);
}

//Object3D cube = loadOBJ("../utah_teapot_6.obj");

Object3D sphere = loadOBJ("../uv_sphere.obj");
Canvas sphereTex("../sphere_uv.png");

Object3D teapot = loadOBJ("../teapot_6.obj");
Canvas teapotTex("../teapot_6_tex.png");

Camera camera = {
    {0, 0, -2},
    {0, 0, 0}
};

struct keyPress {
    int key{};
    bool pressed = false;

    bool check() {
        if (Input::isKeyPressed(key)) {
            if (!pressed) {
                pressed = true;
                return true;
            }
            pressed = true;
        } else {
            pressed = false;
        }
        return false;
    }
};

void gameUpdate(const f32 dt) {
    clearNew();

    constexpr float moveSpeed = 5.f;
    constexpr float rotSpeed = 3.f;

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

    camera.drawObject(sphere, canvas, &zBuffer);
    //camera.drawObjectWireframe(sphere, canvas, Colors::Orange, 1);

    const f32 fps = 1/dt;
    canvas.writeStringBaseline(stringPrint("FPS: {}", fps), 10, 26, 16, Colors::White);
}

int main(int argc, char **argv) {
    game.setWindowName("3D Testing");

    sphere.tex = &sphereTex;
    teapot.tex = &teapotTex;

    game.mediumInit(WIDTH*4, HEIGHT*4, WIDTH, HEIGHT);
    LOG_DEBUG("Game Resolution: {}x{}", game.GAME_WIDTH, game.GAME_HEIGHT);
    LOG_DEBUG("Screen Resolution: {}x{}", game.SCREEN_WIDTH, game.SCREEN_HEIGHT);
    game.mediumStartup();
    game.mediumRun(gameUpdate);
    game.mediumShutdown();

    return 0;
}