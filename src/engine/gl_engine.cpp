#include "gl_engine.h"

#include <iostream>
#include <iterator>
#include <SDL.h>
#include <thread>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

void RenderEngine::init_resources() {
    planeBuffer = glutil::createPlane();
    planeTexture = glutil::loadTexture("../../resources/textures/wood.png");

    cubemap = EnviornmentCubemap("../../resources/textures/skybox/");

    positionTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RGBA, GL_RGBA16F);
    normalTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RGBA, GL_RGBA16F);
    albedoTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA8);

    glCreateFramebuffers(1, &gBuffer);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT0, positionTexture, 0);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT1, normalTexture, 0);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT2, albedoTexture, 0);

    GLint fbStatus = glCheckNamedFramebufferStatus(gBuffer, GL_FRAMEBUFFER);
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete." << "\n";
    }
}

void RenderEngine::render(std::vector<Model>& objs) {
    float currentFrame = static_cast<float>(SDL_GetTicks());
    animationTime = (currentFrame - startTime) / 1000.0f;

    checkFrustum(objs);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void RenderEngine::renderScene(std::vector<Model>& objs, Shader& shader, bool skipTextures) {
    drawModels(objs, shader, skipTextures & SKIP_TEXTURES);

    glm::mat4 planeModel = glm::mat4(1.0f);
    planeModel = glm::translate(planeModel, glm::vec3(0.0, -2.0, 0.0));

    if (!skipTextures) {
        glBindTextureUnit(0, planeTexture);
        shader.setInt("diffuseTexture", 0);
    }
    shader.setMat4("model", planeModel);
    glBindVertexArray(planeBuffer.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void RenderEngine::handleImGui() {
    ImGuiIO& io = ImGui::GetIO();

    if (ImGui::CollapsingHeader("Start Here")) {
    }
}
