// Building on Windows using MinGW
// g++ models_loading_assimp.c -lraylib -lgdi32 -lwinmm -lassimp -lIrrXML -lzlibstatic

#define R3D_ASSIMP_SUPPORT
#define R3D_IMPLEMENTATION
#include "../r3d.h"
#include <stdlib.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

int main(void)
{

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Example - Loading Model With Assimp");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 4.0f, 5.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.type = CAMERA_PERSPECTIVE;

    SetCameraMode(camera, CAMERA_ORBITAL);  // Set an orbital camera mode

    SetTargetFPS(60);                       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    Model model = LoadModelAdvanced("assets/models/tilewall.glb");
    // Main game loop
    while (!WindowShouldClose())            // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(WHITE);

            BeginMode3D(camera);

                DrawGrid(10, 1.0f);        // Draw a grid
                DrawModel(model, Vector3Zero(), 1.0f, WHITE);

            EndMode3D();

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
    UnloadModelAdvanced(model);
    CloseWindow();              // Close window and OpenGL context
    return 0;
}