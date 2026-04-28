//
// Created by Nico Russo on 4/27/26.
//

#ifdef RELEASE_MODE
#define DISABLE_TRACE
#define DISABLE_DEBUG
#endif

#include <mediumOpenGL.h>
#include <inputGLFW.h>

#define WIDTH  (1920/3)
#define HEIGHT (1080/3)

MediumOpenGL game(WIDTH*2, HEIGHT*2, WIDTH, HEIGHT, "DA NEW CPP");
InputGLFW input;

void globalEventCallback(int key, int action, int mods, double x, double y) {

}

Graphite::Canvas background(WIDTH,HEIGHT);
Graphite::Canvas world1(WIDTH,HEIGHT);
Graphite::Canvas world2(WIDTH,HEIGHT);
Graphite::Canvas ui(WIDTH,HEIGHT);
Graphite::Canvas zBuffer(WIDTH,HEIGHT);

Shader shader1 = 0;
Shader shader2 = 0;
Shader shader3 = 0;
Shader shader4 = 0;

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
//Graphite::Object3D ground    = Graphite::loadOBJ(assetRoot / "test big.obj");
Graphite::Object3D car = Graphite::loadOBJ(assetRoot / "Car.obj");
Graphite::Object3D car2 = Graphite::loadOBJ(assetRoot / "Car2.obj");
Graphite::Object3D car7 = Graphite::loadOBJ(assetRoot / "Car7.obj");


Graphite::Canvas texture(assetRoot / "sphere_uv.png");
Graphite::Canvas ct1(assetRoot / "car.png");
Graphite::Canvas ct2(assetRoot / "car_blue.png");
Graphite::Canvas ct3(assetRoot / "car_gray.png");
Graphite::Canvas ct4(assetRoot / "car_red.png");
Graphite::Canvas ct5(assetRoot / "car_snow_blue.png");
Graphite::Canvas ct6(assetRoot / "car_snow_gray.png");
Graphite::Canvas ct7(assetRoot / "car_snow_red.png");
Graphite::Canvas ct8(assetRoot / "car_snowcovered.png");
Graphite::Canvas ct9(assetRoot / "car_snowcovered_blue.png");
Graphite::Canvas ct10(assetRoot / "car_snowcovered_gray.png");
Graphite::Canvas ct11(assetRoot / "car_snowcovered_red.png");

Graphite::Canvas c2t1(assetRoot / "car2.png");
Graphite::Canvas c2t2(assetRoot / "car2_black.png");
Graphite::Canvas c2t3(assetRoot / "car2_red.png");

Graphite::Canvas c7t1(assetRoot / "car7.png");
Graphite::Canvas c7t2(assetRoot / "car7_black.png");
Graphite::Canvas c7t3(assetRoot / "car7_brown.png");
Graphite::Canvas c7t4(assetRoot / "car7_green.png");
Graphite::Canvas c7t5(assetRoot / "car7_grey.png");
Graphite::Canvas c7t6(assetRoot / "car7_red.png");


std::vector<Graphite::Canvas*> car1Tex = {
    &ct1,
    &ct2,
    &ct3,
    &ct4,
    &ct5,
    &ct6,
    &ct7,
    &ct8,
    &ct9,
    &ct10,
    &ct11
};

std::vector<Graphite::Canvas*> car2Tex = {
    &c2t1,
    &c2t2,
    &c2t3,
};

std::vector<Graphite::Canvas*> car7Tex = {
    &c7t1,
    &c7t2,
    &c7t3,
    &c7t4,
    &c7t5,
    &c7t6,
};


float fps = 0;

glm::vec3 defaultCarPos = {-20, -3, 3};
float carDist = 8.f;

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

    constexpr float walkSpeed = 5.f;
    constexpr float sprintSpeed = 10.f;
    constexpr float turnSpeed = 2.f;


    float moveSpeed = 0.f;

    if (input.isKeyPressed(MED_KEY_LEFT_SHIFT)) { moveSpeed = sprintSpeed; } else { moveSpeed = walkSpeed; }

    // MOVE CAMERA
    if (input.isKeyPressed(MED_KEY_W)) {
        gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::FORWARD) * moveSpeed * dt;
        //car.position = car.position + gameState.camera.directionObj(Graphite::Dir3D::FORWARD) * moveSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_S)) {
        gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::BACKWARD) * moveSpeed * dt;
        //car.position = car.position + gameState.camera.directionObj(Graphite::Dir3D::BACKWARD) * moveSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_A)) {
        gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::LEFT) * moveSpeed * dt;
        //car.position = car.position + gameState.camera.directionObj(Graphite::Dir3D::LEFT) * moveSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_D)) {
        gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::RIGHT) * moveSpeed * dt;
        //car.position = car.position + gameState.camera.directionObj(Graphite::Dir3D::RIGHT) * moveSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_E)) {
        gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::UP) * moveSpeed * dt;
        //car.position = car.position + gameState.camera.directionObj(Graphite::Dir3D::UP) * moveSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_Q)) {
        gameState.camera.position = gameState.camera.position + gameState.camera.directionObj(Graphite::Dir3D::DOWN) * moveSpeed * dt;
        //car.position = car.position + gameState.camera.directionObj(Graphite::Dir3D::DOWN) * moveSpeed * dt;
    }

    if (input.isKeyPressed(MED_KEY_I)) {
        gameState.camera.rotation.x -= turnSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_K)) {
        gameState.camera.rotation.x += turnSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_J)) {
        gameState.camera.rotation.y -= turnSpeed * dt;
    }
    if (input.isKeyPressed(MED_KEY_L)) {
        gameState.camera.rotation.y += turnSpeed * dt;
    }

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


    // car :D
    car.position = defaultCarPos;

    car.rotation.y += 1.0f * dt;
    car2.rotation.y += 1.0f * dt;
    car7.rotation.y += 1.0f * dt;

    for (Graphite::Canvas* c : car1Tex) {
        car.tex = c;
        gameState.camera.drawObject(car, world1,
        {&zBuffer, true, true, gameState.sunAngle});
        car.position.x += carDist;
    }

    car2.position = defaultCarPos;
    car2.position.z += carDist;

    for (Graphite::Canvas* c : car2Tex) {
        car2.tex = c;
        gameState.camera.drawObject(car2, world1,
            {&zBuffer, true, true, gameState.sunAngle});
        car2.position.x += carDist;
    }

    car7.position = defaultCarPos;
    car7.position.z += 2 * carDist;

    for (Graphite::Canvas* c : car7Tex) {
        car7.tex = c;
        gameState.camera.drawObject(car7, world1,
            {&zBuffer, true, true, gameState.sunAngle});
        car7.position.x += carDist;
    }

    /*
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
        */


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
    //input.registerGlobalCallback(globalEventCallback);

    shader1 = MediumOpenGL::buildShader(assetRoot / "crtFrag.glsl");
    shader2 = MediumOpenGL::buildShader(assetRoot / "crtFrag2.glsl");
    shader3 = MediumOpenGL::buildShader(assetRoot / "crtFrag3.glsl");
    shader4 = MediumOpenGL::buildShader(assetRoot / "testFrag.glsl");

    background.fill({0xff909090});

    uvSphere.position  = {-2, 0, 2};
    icosphere.position = {2, 0, 2};
    cube.position      = {0, 0, 2};
    teapotLow.position = {0, 2, 2};
    teapotHi.position  = {0, -4, 2};

    car.position = {0, -9, 5};

    game.setScreenShader(shader4);

    uvSphere.tex = &texture;
    icosphere.tex = &texture;
    cube.tex = &texture;
    teapotLow.tex = &texture;
    teapotHi.tex = &texture;
}

int main(int argc, char** argv) {
    game.mediumStartup();
    input.initializeInput(&game);
    gameStart();

    game.mediumRun(gameUpdate);
    game.mediumShutdown();
    return 0;
}