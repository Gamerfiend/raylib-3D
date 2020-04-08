/**********************************************************************************************
*
*   raylib-3D v0.1 - 3D extension library for raylib
*
*   DESCRIPTION:
*
*   An extension library for the amazing raylib library, that adds better support for 3D.
*
*   CONFIGURATION:
*
*   #define R3D_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define R3D_ASSIMP_SUPPORT
*       Defining this before the implementation adds support for Model/Material/Animation loading 
*       via the library assimp. NOTE: assimp is NOT included, is an external dependency that must
*       first install.
*
*   #define R3D_GLAD
*       Define if loading OpenGL
*
*   #define R3D_MALLOC()
*   #define R3D_CALLOC()
*   #define R3D_FREE()
*       You can define your own malloc/free implementation replacing stdlib.h malloc()/free() functions.
*
*   Use the following code to compile:
*   gcc -o $(NAME_PART).exe $(FILE_NAME) -lraylib -lgdi32 -lwinmm -std=c99
*
*   VERY THANKS TO:
*       - raysan5: creation of raylib
*       - the lumaio: creation of g-buffer implementation
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2020 Snowminx (github: @Gamerfiend)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#if !defined(R3D_H)
#define R3D_H

#if defined(__cplusplus)
    #define R3DDEF extern "C"
#else
    #define R3DDEF extern
#endif

#include <raylib.h>

// GBuffer implementation based on @TheLumaio
// GBuffer stores multiple render targets for a single render pass
typedef struct GBuffer {
    unsigned int id;
    int width;
    int height;
    Texture color;
    Texture normal;
    Texture position;
    Texture depth;
} GBuffer;

R3DDEF GBuffer LoadGBuffer(int width, int height);                // Loads a new GBuffer with given screen constraints
R3DDEF void UnloadGBuffer(GBuffer gbuffer);                       // Unload an existing GBuffer 
R3DDEF void BeginDefferedMode(GBuffer gbuffer);                   // Begin drawing in deffered mode (using GBuffer) NOTE: Should be called after BeginDrawing, before BeginMode3D
R3DDEF void EndDefferedMode();                                    // End drawing of deffered mode
R3DDEF void SetDefferedModeShaderTexture(Texture texture, int i); // Sets and binds a texture to active in GL context

#if defined(R3D_ASSIMP_SUPPORT)
    R3DDEF Model LoadModelAdvanced(const char* filename);         // Loads a model from ASSIMP (External Dependency)
#endif // R3D_ASSIMP_SUPPORT

#endif // R3D_H

// Raylib-3D Implementation
#if defined(R3D_IMPLEMENTATION)
#include <rlgl.h>

#if !defined(R3D_GLAD)
    //#define GLAD_IMPLEMENTATION
    #include "glad.h"
#endif

#pragma region GBUFFER
R3DDEF GBuffer LoadGBuffer(int width, int height)
{
    GBuffer gbuffer;
    gbuffer.id = 0;
    gbuffer.width = width;
    gbuffer.height = height;
    
    gbuffer.color.id = 0;
    gbuffer.color.width = width;
    gbuffer.color.height = height;
    gbuffer.color.format = UNCOMPRESSED_R8G8B8A8;
    gbuffer.color.mipmaps = 0;
    
    gbuffer.normal.id = 0;
    gbuffer.normal.width = width;
    gbuffer.normal.height = height;
    gbuffer.normal.format = UNCOMPRESSED_R8G8B8A8;
    gbuffer.normal.mipmaps = 0;
    
    gbuffer.position.id = 0;
    gbuffer.position.width = width;
    gbuffer.position.height = height;
    gbuffer.position.format = UNCOMPRESSED_R8G8B8A8;
    gbuffer.position.mipmaps = 0;

    glGenFramebuffers(1, &gbuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.id);
    
    glGenTextures(1, &gbuffer.position.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer.position.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MIN_FILTER, RL_FILTER_NEAREST);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gbuffer.position.id, 0);
    
    glGenTextures(1, &gbuffer.normal.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer.normal.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MIN_FILTER, RL_FILTER_NEAREST);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gbuffer.normal.id, 0);
    
    glGenTextures(1, &gbuffer.color.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer.color.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MIN_FILTER, RL_FILTER_NEAREST);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gbuffer.color.id, 0);
    
    unsigned int buffers[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    #if defined(GRAPHICS_API_OPENGL_ES2) // use extension where availible 
        glDrawBuffersEXT(3, buffers);
    #else //glDrawBuffers only availible on ES 3.0, GL 2, 3, 4
        glDrawBuffers(3, buffers);
    #endif
    
    
    
    glGenTextures(1, &gbuffer.depth.id);
    glBindTexture(GL_TEXTURE_2D, gbuffer.depth.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_WRAP_S, RL_WRAP_CLAMP);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_WRAP_T, RL_WRAP_CLAMP);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MIN_FILTER, RL_FILTER_NEAREST);
    rlTextureParameters(RL_TEXTURE, RL_TEXTURE_MAG_FILTER, RL_FILTER_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gbuffer.depth.id, 0);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        TraceLog(LOG_WARNING, "Framebuffer object could not be created...");
        
        switch (status)
        {
            case GL_FRAMEBUFFER_UNSUPPORTED: break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: TraceLog(LOG_WARNING, "Framebuffer incomplete attachment"); break;
            #if defined(GRAPHICS_API_OPENGL_ES2)
                case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: TraceLog(LOG_WARNING, "Framebuffer incomplete dimensions"); break;
            #endif
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: TraceLog(LOG_WARNING, "Framebuffer incomplete missing attachment"); break;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return gbuffer;
}

R3DDEF void UnloadGBuffer(GBuffer gbuffer)
{
    rlDeleteBuffers(gbuffer.id);
    rlDeleteTextures(gbuffer.color.id);
    rlDeleteTextures(gbuffer.normal.id);
    rlDeleteTextures(gbuffer.position.id);
}

R3DDEF void BeginDefferedMode(GBuffer gbuffer)
{
    rlglDraw();
    rlEnableRenderTexture(gbuffer.id);
    rlClearScreenBuffers();
    
    rlViewport(0, 0, gbuffer.width, gbuffer.height);
    
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();

    rlOrtho(0, gbuffer.width, gbuffer.height, 0, 0, 1);

    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();

    glDisable(GL_BLEND);
}

R3DDEF void EndDefferedMode()
{
    glEnable(GL_BLEND);
    rlglDraw();
    
    rlDisableRenderTexture();
    
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    
    rlMatrixMode(RL_PROJECTION);
    rlLoadIdentity();
    
    rlOrtho(0, GetScreenWidth(), GetScreenHeight(), 0, 0, 1);
    
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
} 

R3DDEF void SetDefferedModeShaderTexture(Texture texture, int i) 
{
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}
#pragma endregion

//#define R3D_ASSIMP_SUPPORT

#pragma region ASSIMP
#if defined(R3D_ASSIMP_SUPPORT)
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

R3DDEF Model LoadModelAdvanced(const char* filename)
{
    Model model = {0};

    const struct aiScene* aiModel = aiImportFile(filename, aiProcess_CalcTangentSpace | aiProcess_Triangulate);

    model.meshCount = aiModel->mNumMeshes;
    
    aiReleaseImport(aiModel);
    return model;
}
#pragma endregion

#endif // R3D_ASSIMP_SUPPORT
#endif // R3D_IMPLEMENTATION