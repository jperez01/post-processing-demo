#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SDL.h>
#include <vector>

#include "utils/types.h"
#include "utils/shader.h"
#include "utils/camera.h"
#include "utils/model.h"
#include "utils/common_primitives.h"

#include "ui/editor.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_stdlib.h"
#include "ImGuizmo.h"

enum DrawOptions {
    SKIP_TEXTURES = (1u << 0),
    SKIP_CULLING = (1u << 1)
};

class GLEngine {
public:
    virtual void init_resources();
    virtual void render(std::vector<Model>& objs) = 0;
    virtual void handleImGui() = 0;
    virtual void handleObjs(std::vector<Model>& objs) {}

    void loadModelData(Model& model);

    Camera* camera = nullptr;
    int WINDOW_WIDTH = 1920, WINDOW_HEIGHT = 1080;

protected:
    float shininess = 200.0f;

    float startTime = 0.0f;
    float animationTime = 0.0f;
    int chosenAnimation = 0;

    void drawModels(std::vector<Model>& models, Shader& shader, unsigned char drawOptions = 0);
    void drawPlane();
    void checkFrustum(std::vector<Model>& objs);
};