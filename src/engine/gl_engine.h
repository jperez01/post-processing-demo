#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <vector>

#include "utils/compute.h"
#include "base_engine.h"
#include "utils/framebuffer.h"

class RenderEngine : public GLEngine {
public:
    void init_resources();
    void render(std::vector<Model>& objs);
    void handleImGui();

private:
    ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;

    AllocatedBuffer planeBuffer;
    unsigned int planeTexture;

    EnviornmentCubemap cubemap;
    ScreenQuad screenQuad;

    unsigned int gBuffer;
    unsigned int positionTexture, normalTexture, albedoTexture, depthMap;

    Shader gBufferPipeline, finalPipeline;

    glm::vec3 warpSize = glm::vec3(8.0f, 8.0f, 1.0f);
    glm::ivec3 numWarps = glm::ivec3(screenSize.x / warpSize.x, screenSize.y / warpSize.y, 1.0);

    Shader gtaoGraphicsPipeline;
    Framebuffer gtaoFramebuffer;
    unsigned int gtaoFrameTexture;

    ComputeShader ssaoPipeline;
    unsigned int ssaoTexture;

    ComputeShader gtaoPipeline;
    unsigned int gtaoTexture;

    ComputeShader temporalFilterPipeline;
    ComputeShader historyPipeline;
    unsigned int historyAOTexture, historyDepthTexture;

    ComputeShader blurPipeline;
    unsigned int blurTexture;

    std::vector<glm::vec3> ssaoKernel, ssaoNoise;
    unsigned int noiseTexture;

    void renderScene(std::vector<Model>& objs, Shader& shader, bool skipTextures);
};
