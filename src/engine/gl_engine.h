#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <vector>

#include "utils/compute.h"
#include "base_engine.h"

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
        ComputeShader ssaoPipeline;
        unsigned int ssaoTexture;

        ComputeShader blurPipeline;
        unsigned int blurTexture;

        std::vector<glm::vec3> ssaoKernel, ssaoNoise;
        unsigned int noiseTexture;

        void RenderEngine::renderScene(std::vector<Model>& objs, Shader& shader, bool skipTextures);
};