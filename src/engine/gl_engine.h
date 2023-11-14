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

        unsigned int gBuffer;
        unsigned int positionTexture, normalTexture, albedoTexture;

        Shader gBufferPipeline, SSAOPipeline, finalPipeline;

        std::vector<glm::vec3> rotationVectors;

        void RenderEngine::renderScene(std::vector<Model>& objs, Shader& shader, bool skipTextures);
};