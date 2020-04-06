#define R3D_IMPLEMENTATION
#include "r3d.h"

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

int main(int argc, char** argv)
{
    // Basic raylib window creation
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Example - Models Deffered Basic");
    SetTargetFPS(60);

    // Setup our camera
    Camera camera = {{0, 3.5, 3}, {0, 3, 0}, {0, 1, 0}, 90.0f, CAMERA_PERSPECTIVE};
    SetCameraMode(camera, CAMERA_FIRST_PERSON);
}