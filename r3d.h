/**********************************************************************************************
*
*   raylib-3D v0.2 - 3D extension library for raylib
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
*       Defining this before R3D_IMPLEMENTATION adds support for Model/Material/Animation loading 
*       via the library assimp. 
*       NOTE: assimp is NOT included, is an external dependency that must
*       first install. By using assimp, you will have to compile project with g++ or a compliant C++
*       compiler.
*
*   #define R3D_SKELETAL_ANIMATION_SUPPORT
*       Defining this before R3D_IMPLEMENTATION adds support for 3D skeletal animation, following standard
*       set by GLTF, COLLADE, FBX and more. The implementation follows suit by similar indie engines with 
*       open source code. Largely follows http://www.ogldev.org/www/tutorial38/tutorial38.html
*
*   #define R3D_GLAD
*       Define this flag if you wish to include your own GLAD OpenGL profile.
*       NOTE: Currently this flag is unsupported
*
*   #define R3D_CUSTOM_ALLOCATORS
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

#ifndef R3D_CUSTOM_ALLOCATORS
#include <stdlib.h>
#ifndef R3D_MALLOC
#define R3D_MALLOC malloc
#endif
#ifndef R3D_CALLOC
#define R3D_CALLOC calloc
#endif
#ifndef R3D_FREE
#define R3D_FREE free
#endif
#endif

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
R3DDEF void BeginDeferredMode(GBuffer gbuffer);                   // Begin drawing in Deferred mode (using GBuffer) NOTE: Should be called after BeginDrawing, before BeginMode3D
R3DDEF void EndDeferredMode();                                    // End drawing of Deferred mode
R3DDEF void SetDeferredModeShaderTexture(Texture texture, int i); // Sets and binds a texture to active in GL context

#if defined(R3D_SKELETAL_ANIMATION_SUPPORT)
typedef struct SkeletalBone {
    unsigned int id;       // id correlates directly to Raylib's BoneInfo id
    Matrix offsetMatrix;   // transforms local spaced vertices into bone space (skeleton)
    Matrix finalTransform; // Combination of the inverse matrix, offset matrix and global transform
} SkeletalBone;

typedef struct SkeletalAnimationChannel {
    char *name;            // name of this bone... NOTE: Could this changed to the id?
    Transform *transforms; // each transform represents this bone's position, rotation, and scale 
    unsigned int transformsAmount;
} SkeletalAnimationChannel;

typedef struct SkeletalAnimation {
    float duration;                      // length in ticks
    float ticksPerSecond;                // e.g 100 duration (ticks) at 25 ticks per second would give us a ~4 second animation
    unsigned int channelsAmount;
    SkeletalAnimationChannel *channels;  // these are the bones (skeleton) for an animation.. TODO: Channels should be replaced with a HashMap<channelName, channel> for 0(1) lookup.. must find c lib for such
} SkeletalAnimation;

typedef struct Skeleton {
    unsigned int bonesCount;
    SkeletalBone *bones;
} Skeleton;

typedef struct AnimatedModel {
    Model model;
    Skeleton skeleton;
    SkeletalAnimation *animations;
    unsigned int animationsCount;
} AnimatedModel;

R3DDEF AnimatedModel LoadAnimatedModel(const char *filename); // Load from file, uses raylib for (LoadModel)
#endif   

#if defined(R3D_ASSIMP_SUPPORT)
R3DDEF Model LoadModelAdvanced(const char *filename); // Loads a model from ASSIMP (External Dependency)
R3DDEF void UnloadModelAdvanced(Model model);         // Unload a model.. currently same as UnloadModel
#if defined(R3D_SKELETAL_ANIMATION_SUPPORT)
R3DDEF AnimatedModel LoadAnimatedModelAdvanced(const char *filename); // Load from file
#endif
#endif                                                // R3D_ASSIMP_SUPPORT

#endif // R3D_H

#if defined(R3D_IMPLEMENTATION)
#include <rlgl.h>

#if !defined(R3D_GLAD)
//#define GLAD_IMPLEMENTATION
#include "includes/glad.h"
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

//TODO: Query for extensions, these must be checked to ensure the user platform supports them.. ES2 may support glDrawBuffersEXT().. WebGL may support through the ANGLE web extensions
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
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        TraceLog(LOG_WARNING, "Framebuffer object could not be created...");

        switch (status)
        {
        case GL_FRAMEBUFFER_UNSUPPORTED:
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            TraceLog(LOG_WARNING, "Framebuffer incomplete attachment");
            break;
#if defined(GRAPHICS_API_OPENGL_ES2)
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            TraceLog(LOG_WARNING, "Framebuffer incomplete dimensions");
            break;
#endif
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            TraceLog(LOG_WARNING, "Framebuffer incomplete missing attachment");
            break;
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

R3DDEF void BeginDeferredMode(GBuffer gbuffer)
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

R3DDEF void EndDeferredMode()
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

R3DDEF void SetDeferredModeShaderTexture(Texture texture, int i)
{
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}
#pragma endregion

#pragma region ASSIMP
#if defined(R3D_ASSIMP_SUPPORT)
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <assimp/postprocess.h>
#include <assimp/color4.h>
#include <stdio.h>
#include "includes/stb_image.h"

static void setTextureFromAssimpMaterial(const struct aiScene *aiModel, Model *model, unsigned int materialIndex, enum aiTextureType textureType, MaterialMapType mapType)
{
    char buffer[512] = "unnamed";
    struct aiString path;

    // TODO: Support mutliple color for different types..
    // aiColor4D color = (aiColor4D){0.f, 0.f, 0.f, 0.f};
    // if (aiGetMaterialColor(aiModel->mMaterials[materialIndex], AI_MATKEY_COLOR_DIFFUSE, &color) == aiReturn_SUCCESS)
    // {
    //     model->materials[materialIndex].maps[MAP_ALBEDO].color = (Color){(unsigned char)(color.r * 255), (unsigned char)(color.g * 255), (unsigned char)(color.b * 255), (unsigned char)(color.a * 255)};
    // }

    unsigned int textureIndex = 0;

    if (aiGetMaterialTexture(aiModel->mMaterials[materialIndex], textureType, textureIndex, &path, NULL, NULL, NULL, NULL, NULL, NULL) == aiReturn_SUCCESS)
    {
        strncpy(buffer, path.data, sizeof(buffer));
        const char embedded[] = {"*"};

        // Embedded texture
        if (TextIsEqual(TextSubtext(buffer, 0, 1), embedded))
        {
            const char *cindex = TextSubtext(buffer, 1, TextLength(buffer));
            unsigned int index = atoi(cindex);
            struct aiTexture *embeddedTexture = aiModel->mTextures[index];

            unsigned char *imageData = NULL;
            int width;
            int height;
            Image rImage = {0};

            // Texture is compressed.. (jpg)
            if (embeddedTexture->mHeight == 0)
            {
                imageData = stbi_load_from_memory((const unsigned char *)embeddedTexture->pcData, embeddedTexture->mWidth, &width, &height, NULL, 4);
            }
            else
            {
                imageData = stbi_load_from_memory((const unsigned char *)embeddedTexture->pcData, embeddedTexture->mWidth * embeddedTexture->mHeight, &width, &height, NULL, 4);
            }

            rImage.data = imageData;
            rImage.width = width;
            rImage.height = height;
            rImage.format = UNCOMPRESSED_R8G8B8A8;
            rImage.mipmaps = 1;

            model->materials[materialIndex].maps[mapType].texture = LoadTextureFromImage(rImage);
            UnloadImage(rImage);
        }
        else
        {
            model->materials[materialIndex].maps[mapType].texture = LoadTexture(buffer);
        }
    }
}

R3DDEF Model LoadModelAdvanced(const char *filename)
{
    Model model = {0};
    const struct aiScene *aiModel = aiImportFile(filename, aiProcess_Triangulate);
    //TODO Error handling for when a model isn't loaded successfully
    if (!aiModel)
    {
        TraceLog(LOG_WARNING, "LoadModelAdvanced: Unable able to load model %s", filename);
        return model;
    }

    model.transform = MatrixIdentity();

    // Load Materials
    model.materialCount = aiModel->mNumMaterials;
    model.meshMaterial = (int *)R3D_CALLOC(model.meshCount, sizeof(int));
    model.materials = (Material *)R3D_CALLOC(model.materialCount, sizeof(Material));

    for (int i = 0; i < model.materialCount; i++)
    {
        model.materials[i] = LoadMaterialDefault();

        // TODO: Support Base Color texture type? It doesn't seem to be used, even in PBR material flows like GLTF
        // unsigned int baseColorAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_BASE_COLOR);
        if (aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_DIFFUSE) > 0) {
            setTextureFromAssimpMaterial(aiModel, &model, i, aiTextureType_DIFFUSE, MAP_ALBEDO);
        }

        // TODO: Support PBR normals? It doesn't seem to be used, even in PBR material like GLTF
        // unsigned int normalPBRAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_NORMAL_CAMERA);
        if (aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_NORMALS) > 0) {
            setTextureFromAssimpMaterial(aiModel, &model, i, aiTextureType_NORMALS, MAP_NORMAL);
        }

        // Must support both metalness and specular.. as metal is for PBR.. specular matches diffuse flow
        // aiTextureType_AMBIENT; Not currently supported, this is the result of the ambient lighting equation.. metalness
        // unsigned int metalAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_METALNESS);
        // unsigned int specularAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_SPECULAR);
        if (aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_AMBIENT) > 0) {
            setTextureFromAssimpMaterial(aiModel, &model, i, aiTextureType_AMBIENT, MAP_METALNESS);
        }

        // Must support both as the models could use the old flow.. or the current flow
        // unsigned int ambientOccAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_AMBIENT_OCCLUSION);
        // unsigned int ambientOccOldAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_LIGHTMAP);

        // Unique texture slots
        // aiTextureType_SHININESS also roughness
        if (aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_SHININESS) > 0) {
            setTextureFromAssimpMaterial(aiModel, &model, i, aiTextureType_SHININESS, MAP_ROUGHNESS);
        }
        
        // unsigned int roughnessAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_DIFFUSE_ROUGHNESS);
        // unsigned int emissiveAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_EMISSIVE);
        // unsigned int heightAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_HEIGHT);
        // unsigned int opacityAmount = aiGetMaterialTextureCount(aiModel->mMaterials[i], aiTextureType_OPACITY);

    }

    //Load Meshes for Model
    model.meshCount = aiModel->mNumMeshes;
    model.meshes = (Mesh*)R3D_CALLOC(model.meshCount, sizeof(Mesh));
    for (int i = 0; i < model.meshCount; i++)
    {
        struct aiMesh *importMesh = aiModel->mMeshes[i];

        model.meshes[i].vertexCount = importMesh->mNumVertices;
        // Assimp stores vertices in Vector3 (XYZ) Raylib stores vertices in float array, where every three is one vertex (XYZ)
        model.meshes[i].vertices = (float *)R3D_MALLOC((sizeof(float) * model.meshes[i].vertexCount) * 3);
        unsigned int vectorCounter = 0;
        for (int j = 0; j < model.meshes[i].vertexCount * 3; j += 3)
        {
            model.meshes[i].vertices[j] = importMesh->mVertices[vectorCounter].x;
            model.meshes[i].vertices[j + 1] = importMesh->mVertices[vectorCounter].y;
            model.meshes[i].vertices[j + 2] = importMesh->mVertices[vectorCounter].z;
            vectorCounter++;
        }

        // Assimp stores texCoords in Vector3 (XYZ), Raylib uses (UV) float array
        if (importMesh->mTextureCoords[0])
        {
            model.meshes[i].texcoords = (float *)R3D_MALLOC((sizeof(float) * model.meshes[i].vertexCount) * 2);
            unsigned int texCoord = 0;
            for (int j = 0; j < model.meshes[i].vertexCount * 2; j += 2)
            {
                model.meshes[i].texcoords[j] = importMesh->mTextureCoords[0][texCoord].x;
                model.meshes[i].texcoords[j + 1] = importMesh->mTextureCoords[0][texCoord].y;
                texCoord++;
            }
        }

        // Raylib supports two layers of textureCoords
        if (importMesh->mTextureCoords[1])
        {
            model.meshes[i].texcoords2 = (float *)R3D_MALLOC((sizeof(float) * model.meshes[i].vertexCount) * 2);
            unsigned int texCoord = 0;
            for (int j = 0; j < model.meshes[i].vertexCount * 2; j += 2)
            {
                model.meshes[i].texcoords2[j] = importMesh->mTextureCoords[1][texCoord].x;
                model.meshes[i].texcoords2[j + 1] = importMesh->mTextureCoords[1][texCoord].y;
                texCoord++;
            }
        }

        model.meshes[i].normals = (float *)R3D_MALLOC((sizeof(float) * model.meshes[i].vertexCount) * 3);
        unsigned int normalCounter = 0;
        for (int j = 0; j < model.meshes[i].vertexCount * 3; j += 3)
        {
            model.meshes[i].normals[j] = importMesh->mNormals[normalCounter].x;
            model.meshes[i].normals[j + 1] = importMesh->mNormals[normalCounter].y;
            model.meshes[i].normals[j + 2] = importMesh->mNormals[normalCounter].z;
            normalCounter++;
        }

        unsigned int indiceTotal = 0;
        for (unsigned int j = 0; j < importMesh->mNumFaces; j++)
        {
            indiceTotal += importMesh->mFaces[j].mNumIndices;
        }

        model.meshes[i].indices = (unsigned short *)R3D_MALLOC(sizeof(unsigned short) * indiceTotal);
        unsigned int indexCounter = 0;
        for (unsigned int j = 0; j < importMesh->mNumFaces; j++)
        {
            for (unsigned int k = 0; k < importMesh->mFaces[j].mNumIndices; k++)
            {
                model.meshes[i].indices[indexCounter] = importMesh->mFaces[j].mIndices[k];
                indexCounter++;
            }
        }

        model.meshes[i].triangleCount = importMesh->mNumFaces;

        if (importMesh->mTangents)
        {
            model.meshes[i].tangents = (float *)R3D_MALLOC((sizeof(float) * model.meshes[i].vertexCount) * 4);
            unsigned int tangentCounter = 0;
            for (int j = 0; j < model.meshes[i].vertexCount * 4; j += 4)
            {
                model.meshes[i].tangents[j] = importMesh->mTangents[tangentCounter].x;
                model.meshes[i].tangents[j + 1] = importMesh->mTangents[tangentCounter].y;
                model.meshes[i].tangents[j + 2] = importMesh->mTangents[tangentCounter].z;
                model.meshes[i].tangents[j + 3] = 0;
                tangentCounter++;
            }
        }

        if (importMesh->mColors[0])
        {
            model.meshes[i].colors = (unsigned char *)R3D_MALLOC((sizeof(unsigned char) * model.meshes[i].vertexCount) * 4);
            unsigned int colorCounter = 0;
            for (int j = 0; j < model.meshes[i].vertexCount * 4; j += 4)
            {
                model.meshes[i].colors[j] = importMesh->mColors[0][colorCounter].r;
                model.meshes[i].colors[j + 1] = importMesh->mColors[0][colorCounter].g;
                model.meshes[i].colors[j + 2] = importMesh->mColors[0][colorCounter].b;
                model.meshes[i].colors[j + 3] = importMesh->mColors[0][colorCounter].a;
                colorCounter++;
            }
        }

        // Mark this material index as active, so we can coorelate it to a material
        if (importMesh->mMaterialIndex >= 0)
        {
            model.meshMaterial[i] = importMesh->mMaterialIndex;
        }

        model.meshes[i].vboId = (unsigned int *)R3D_CALLOC(7, sizeof(unsigned int));
    }

    for (int i = 0; i < model.meshCount; i++)
    {
        rlLoadMesh(&model.meshes[i], false);
    }

    aiReleaseImport(aiModel);
    return model;
}

R3DDEF void UnloadModelAdvanced(Model model)
{
    UnloadModel(model);
}
#endif // R3D_ASSIMP_SUPPORT
#pragma endregion

#pragma region SKELETAL
#if defined(R3D_SKELETAL_ANIMATION_SUPPORT)
#endif // R3D_SKELETAL_ANIMATION_SUPPORT
#pragma endregion 

#endif // R3D_IMPLEMENTATION