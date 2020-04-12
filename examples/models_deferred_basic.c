#define R3D_IMPLEMENTATION
#include "../r3d.h"
#include <stdlib.h>

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

int main()
{
    // Basic raylib window creation
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Example - Models Deffered Basic");
    SetTargetFPS(60);

    // Setup our camera
    Camera camera = { { 0.2f, 0.4f, 0.2f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, 45.0f, CAMERA_PERSPECTIVE};
    SetCameraMode(camera, CAMERA_FIRST_PERSON);

    //Load shaders
    Shader gBufferShader = LoadShader("assets/shaders/gbuffer.vs", "assets/shaders/gbuffer.fs");
    gBufferShader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(gBufferShader, "modelMatrix");

    Image imMap = LoadImage("assets/textures/cubicmap.png");      // Load cubicmap image (RAM)
    Texture2D cubicmap = LoadTextureFromImage(imMap);       // Convert image to texture to display (VRAM)
    Mesh mesh = GenMeshCubicmap(imMap, (Vector3){ 1.0f, 1.0f, 1.0f });
    Model model = LoadModelFromMesh(mesh);

    // NOTE: By default each cube is mapped to one part of texture atlas
    Texture2D texture = LoadTexture("assets/textures/cubicmap_atlas.png");    // Load map texture
    model.materials[0].maps[MAP_DIFFUSE].texture = texture;             // Set map diffuse texture
    model.materials[0].maps[MAP_METALNESS].texture = GetTextureDefault();
    model.materials[0].maps[MAP_NORMAL].texture = GetTextureDefault();
    model.materials[0].shader = gBufferShader;

    // Get map image data to be used for collision detection
    Color *mapPixels = GetImageData(imMap);
    UnloadImage(imMap);             // Unload image from RAM

    Vector3 mapPosition = { -16.0f, 0.0f, -8.0f };  // Set model position
    Vector3 playerPosition = camera.position;       // Set player position

    GBuffer gBuffer = LoadGBuffer(SCREEN_WIDTH, SCREEN_HEIGHT);
    RenderTexture renderTarget = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        Vector3 oldCamPos = camera.position;    // Store old camera position

        UpdateCamera(&camera);      // Update camera

        // Check player collision (we simplify to 2D collision detection)
        Vector2 playerPos = { camera.position.x, camera.position.z };
        float playerRadius = 0.1f;  // Collision radius (player is modelled as a cilinder for collision)

        int playerCellX = (int)(playerPos.x - mapPosition.x + 0.5f);
        int playerCellY = (int)(playerPos.y - mapPosition.z + 0.5f);

        // Out-of-limits security check
        if (playerCellX < 0) playerCellX = 0;
        else if (playerCellX >= cubicmap.width) playerCellX = cubicmap.width - 1;

        if (playerCellY < 0) playerCellY = 0;
        else if (playerCellY >= cubicmap.height) playerCellY = cubicmap.height - 1;

        // Check map collisions using image data and player position
        // TODO: Improvement: Just check player surrounding cells for collision
        for (int y = 0; y < cubicmap.height; y++)
        {
            for (int x = 0; x < cubicmap.width; x++)
            {
                if ((mapPixels[y*cubicmap.width + x].r == 255) &&       // Collision: white pixel, only check R channel
                    (CheckCollisionCircleRec(playerPos, playerRadius,
                    (Rectangle){ mapPosition.x - 0.5f + x*1.0f, mapPosition.z - 0.5f + y*1.0f, 1.0f, 1.0f })))
                {
                    // Collision detected, reset camera position
                    camera.position = oldCamPos;
                }
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginDeferredMode(gBuffer);
                BeginMode3D(camera);

                    DrawModel(model, mapPosition, 1.0f, WHITE);                     // Draw maze map

                EndMode3D();
            EndDeferredMode();

            DrawTexturePro(gBuffer.color, (Rectangle){0, 0, gBuffer.width, -gBuffer.height}, (Rectangle){0, 0, gBuffer.width, gBuffer.height}, Vector2Zero(), 0.0f, WHITE);

            DrawTextureEx(cubicmap, (Vector2){ GetScreenWidth() - cubicmap.width*4 - 20, 20 }, 0.0f, 4.0f, WHITE);
            DrawRectangleLines(GetScreenWidth() - cubicmap.width*4 - 20, 20, cubicmap.width*4, cubicmap.height*4, GREEN);

            // Draw player position radar
            DrawRectangle(GetScreenWidth() - cubicmap.width*4 - 20 + playerCellX*4, 20 + playerCellY*4, 4, 4, RED);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    free(mapPixels);            // Unload color array

    UnloadTexture(cubicmap);    // Unload cubicmap texture
    UnloadTexture(texture);     // Unload map texture
    UnloadModel(model);         // Unload map model
    UnloadGBuffer(gBuffer);
    UnloadShader(gBufferShader);
    UnloadRenderTexture(renderTarget);

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
