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

MediumOpenGL game;
Input* Input::instance = new InputGLFW(&game);
Canvas& canvas = game.canvas;

#define WIDTH  800
#define HEIGHT 600

#define RATIO ((f32)WIDTH/HEIGHT)

#define POINT_SIZE 5

constexpr Color BACKGROUND = Colors::DarkGrey;
constexpr Color FOREGROUND = Colors::Green;

void clear() { canvas.fillStupid(0x18); }

Vec2<i32> worldToScreen(const Vec2<f32>& p)
{
    float s = std::min((float)WIDTH, (float)HEIGHT);

    float cx = WIDTH  * 0.5f;
    float cy = HEIGHT * 0.5f;

    return {
        (i32)(cx + p.x * s * 0.5f),
        (i32)(cy - p.y * s * 0.5f)
    };
}

Vec2<f32> project(const Vec3<f32>& p) {
    return {
        p.x/p.z,
        p.y/p.z
    };
}

Vec3<f32> translate(const Vec3<f32>& p, const Vec3<f32>& translation) {
    return {
        p.x + translation.x,
        p.y + translation.y,
        p.z + translation.z
    };
}

Vec3<f32> rotateXZ(const Vec3<f32>& p, const f32 angleRadians) {
    const f32 c = cosf(angleRadians);
    const f32 s = sinf(angleRadians);
    return {
        p.x*c - p.z*s,
        p.y,
        p.x*s + p.z*c,
    };
}

Vec3<f32> points[] = {
    { 0.25, 0.25,0.25},
    {-0.25, 0.25,0.25},
    { 0.25,-0.25,0.25},
    {-0.25,-0.25,0.25},

    { 0.25, 0.25,-0.25},
    {-0.25, 0.25,-0.25},
    { 0.25,-0.25,-0.25},
    {-0.25,-0.25,-0.25},
};

struct Face {
    std::vector<i32> pointIndices;
};

Face faces[] = {
    {{0, 1, 3, 2}},
    {{4, 5, 7, 6}},
    {{0, 4}},
    {{1, 5}},
    {{2, 6}},
    {{3, 7}},
};

Vec3<f32> trianglePoints[] = {
    {+0.5,-0.5, 0},
    {-0.5,-0.5, 0},
    {+0.0,+0.5, 0},
};

Face triangleFaces[] = {
    {{0, 1, 2}}
};

f32 dz = 0.5;
f32 angle = 0;

struct Point {
    Vec3<f32> position;
    Vec2<f32> uv;
};

struct Object {
    std::vector<Point> vertices;
    std::vector<std::vector<i32>> faces;
    Vec3<f32> position;
    f32 rotationXZ;
    Canvas& tex;

    void draw(Graphite::Canvas& c) const
    {
        struct Vtx
        {
            Vec2<i32> screen;
            Vec2<f32> uvOverZ;
            float invZ;
        };

        std::vector<Vtx> vtx(vertices.size());

        // -----------------------------
        // STEP 1: vertex transform
        // -----------------------------
        for (size_t i = 0; i < vertices.size(); i++)
        {
            Vec3<f32> v = vertices[i].position;

            v = rotateXZ(v, rotationXZ);
            v = translate(v, position);

            float invZ = 1.0f / v.z;

            Vec2<f32> proj = project(v);
            vtx[i].screen = worldToScreen(proj);

            vtx[i].uvOverZ = vertices[i].uv * invZ;
            vtx[i].invZ = invZ;
        }

        // -----------------------------
        // STEP 2: rasterize faces
        // -----------------------------
        for (const std::vector<i32>& face : faces)
        {
            if (face.size() < 3) continue;

            const auto& v0 = vtx[face[0]];

            for (size_t i = 1; i + 1 < face.size(); i++)
            {
                const auto& v1 = vtx[face[i]];
                const auto& v2 = vtx[face[i + 1]];

                c.fillTriangleUV(
                    v0.screen, v1.screen, v2.screen,
                    v0.uvOverZ, v1.uvOverZ, v2.uvOverZ,
                    v0.invZ, v1.invZ, v2.invZ,
                    tex
                );
            }
        }
    }

    void drawWireframe(Graphite::Canvas& c, Color color, int thickness) const {
        for (const std::vector<i32>& face : faces) {
            for (int i = 0; i < face.size(); i++) {
                Vec3<f32> p0 = vertices[face[i]].position;
                Vec3<f32> p1 = vertices[face[(i+1)%face.size()]].position;
                Vec2<i32> s0 = worldToScreen(project(translate(rotateXZ(p0, rotationXZ), position)));
                Vec2<i32> s1 = worldToScreen(project(translate(rotateXZ(p1, rotationXZ), position)));
                canvas.drawLine(s0, s1, color, thickness);
            }
        }
    }
};

Canvas tex("../testing/D.jpg");

Object quad = {
    {
        {{-0.5, -0.5, 0.25}, {0, 0}},
        {{+0.5, -0.5, 0.25}, {1, 0}},
        {{-0.5, +0.5, -0.25}, {0, 1}},
        {{+0.5, +0.5, -0.25}, {1, 1}}
    },

    {
        {0, 2, 3, 1}
    },

{0, 0, 1.5},

0,

    tex
};

Object triangle = {
    {
        {{+0.5,-0.5, 0}, {0, 0}},
        {{-0.5,-0.5, 0}, {0, 1}},
        {{+0.0,+0.5, 0}, {1, 0}}
    },

    {{0, 1, 2}},

    {0, 0, 1.5},

    0,

    tex
};

float rotSpeed = 0;
void gameUpdate(const f32 dt) {
    clear();

    //Vec2<i32> tp1 = {50, 100 + static_cast<int>((std::sin(dz)+1) * 100)};
    //canvas.fillTriangle(tp1.x, tp1.y, 150, 50, 250, 300, 0xff0000ff, 0xff00ff00, 0xffff0000);

    //canvas.drawPoint(WIDTH/2, HEIGHT/2, 2, Colors::White);

    quad.rotationXZ += rotSpeed * M_PI * dt;;

    float speed = 10.f;
    float rotAccel = 1.f;

    if (Input::isKeyPressed(MED_KEY_W)) { quad.position.z += speed * dt; }
    if (Input::isKeyPressed(MED_KEY_S)) { quad.position.z -= speed * dt; }
    if (Input::isKeyPressed(MED_KEY_A)) { quad.position.x -= speed * dt; }
    if (Input::isKeyPressed(MED_KEY_D)) { quad.position.x += speed * dt; }
    if (Input::isKeyPressed(MED_KEY_Q)) { quad.position.y -= speed * dt; }
    if (Input::isKeyPressed(MED_KEY_E)) { quad.position.y += speed * dt; }
    if (Input::isMouseButtonPressed(MED_MOUSE_BUTTON_LEFT)) { rotSpeed += rotAccel * dt; }
    if (Input::isMouseButtonPressed(MED_MOUSE_BUTTON_RIGHT)) { rotSpeed -= rotAccel * dt; }

    quad.draw(canvas);
    quad.drawWireframe(canvas, Colors::Green, 2);

    const f32 fps = 1/dt;
    canvas.writeStringBaseline(stringPrint("FPS: {}", fps), 10, 26, 16, Colors::White);

    std::pair<f32, f32> mousePosition = Input::getMousePosition();
    canvas.writeStringBaseline(stringPrint("Mouse Pos: ({}, {})", mousePosition.first, mousePosition.second), 10, 52, 16, Colors::White);







    std::string w = Input::isKeyPressed(MED_KEY_W) ? "w" : " ";
    std::string a = Input::isKeyPressed(MED_KEY_A) ? "a" : " ";
    std::string s = Input::isKeyPressed(MED_KEY_S) ? "s" : " ";
    std::string d = Input::isKeyPressed(MED_KEY_D) ? "d" : " ";
    std::string e = Input::isKeyPressed(MED_KEY_E) ? "e" : " ";
    std::string q = Input::isKeyPressed(MED_KEY_Q) ? "q" : " ";
    std::string up = Input::isKeyPressed(MED_KEY_UP) ? "u" : " ";
    std::string down = Input::isKeyPressed(MED_KEY_DOWN) ? "d" : " ";
    std::string left = Input::isKeyPressed(MED_KEY_LEFT) ? "l" : " ";
    std::string right = Input::isKeyPressed(MED_KEY_RIGHT) ? "r" : " ";
    std::string mouseL = Input::isMouseButtonPressed(MED_MOUSE_BUTTON_LEFT) ? "mouseL " : "       ";
    std::string mouseR = Input::isMouseButtonPressed(MED_MOUSE_BUTTON_RIGHT) ? "mouseR" : "      ";

    canvas.writeStringBaseline(stringPrint(
        "{}{}{}{}{}{}", w, a, s, d, q, e
        ), 10, 78, 16, Colors::White);
    canvas.writeStringBaseline(stringPrint(
        "{}{}{}{}", up, down, left, right
        ), 10, 78+26, 16, Colors::White);
    canvas.writeStringBaseline(stringPrint(
        "{}{}", mouseL, mouseR
        ), 10, 78+26+26, 16, Colors::White);

}

int main(int argc, char **argv) {
    //game.setWindowName("3D Testing");

    //tex.fillTriangle(0, 0, tex.getWidth(), 0, 0, tex.getHeight(), 0xff0000ff, 0xff00ff00, 0xffff0000);
    //tex.fillTriangle(tex.getWidth(), tex.getHeight(), tex.getWidth(), 0, 0, tex.getHeight(), 0xffffffff, 0xff00ff00, 0xffff0000);

    game.mediumInit(WIDTH*2, HEIGHT*2, WIDTH, HEIGHT);
    LOG_DEBUG("Game Resolution: {}x{}", game.GAME_WIDTH, game.GAME_HEIGHT);

    game.mediumStartup();

    game.mediumRun(gameUpdate);

    game.mediumShutdown();

    return 0;
}