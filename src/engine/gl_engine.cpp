#include "gl_engine.h"

#include <iostream>
#include <iterator>
#include <SDL.h>
#include <thread>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <random>
#include <utils/math.h>

void RenderEngine::init_resources() {
    gBufferPipeline = Shader("deferred/gbuffer.vert", "deferred/gbuffer.frag");
    finalPipeline = Shader("default/defaultScreen.vert", "default/defaultScreen.frag");
    ssaoPipeline = ComputeShader("ssao/ssao.glsl");
    blurPipeline = ComputeShader("ssao/blur.glsl");
    gtaoPipeline = ComputeShader("gtao/gtao.comp");

    gtaoGraphicsPipeline = Shader("default/defaultScreen.vert", "gtao/gtao.frag");
    gtaoFrameTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RED, GL_R32F, nullptr, 5);
    gtaoFramebuffer = Framebuffer({ GL_COLOR_ATTACHMENT0 }, {gtaoFrameTexture});

    planeBuffer = glutil::createPlane();
    planeTexture = glutil::loadTexture("../resources/textures/wood.png");

    cubemap = EnviornmentCubemap("../resources/textures/skybox/");
    screenQuad.init();

    positionTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RGBA, GL_RGBA16F, nullptr, 5);
    normalTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RGBA, GL_RGBA16F, nullptr, 5);
    albedoTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_UNSIGNED_BYTE, GL_RGBA, GL_RGBA8, nullptr, 1);
    depthMap = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_DEPTH_STENCIL, GL_DEPTH32F_STENCIL8);

    glCreateFramebuffers(1, &gBuffer);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT0, positionTexture, 0);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT1, normalTexture, 0);
    glNamedFramebufferTexture(gBuffer, GL_COLOR_ATTACHMENT2, albedoTexture, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glNamedFramebufferDrawBuffers(gBuffer, 3, attachments);
    glNamedFramebufferTexture(gBuffer, GL_DEPTH_STENCIL_ATTACHMENT, depthMap, 0);

    GLint fbStatus = glCheckNamedFramebufferStatus(gBuffer, GL_FRAMEBUFFER);
    if (fbStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete." << "\n";
    }

    std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
    std::default_random_engine generator;
    
    for (unsigned int i = 0; i < 64; i++) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        float scale = i / 64.0;
        scale = lerp(0.1, 1.0f, scale * scale);
        sample *= scale;

        ssaoKernel.push_back(sample);
    }

    for (unsigned int i = 0; i < 16; i++) {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f
        );

        ssaoNoise.push_back(noise);
    }

    noiseTexture = glutil::createTexture(4, 4, GL_FLOAT, GL_RGBA, GL_RGBA16F, ssaoNoise.data(), 1);

    ssaoTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RGBA, GL_RGBA16F);
    glBindImageTexture(0, ssaoTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    blurTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RGBA, GL_RGBA16F);
    glBindImageTexture(1, blurTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    gtaoTexture = glutil::createTexture(WINDOW_WIDTH, WINDOW_HEIGHT, GL_FLOAT, GL_RGBA, GL_RGBA16F);
    glBindImageTexture(2, gtaoTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    historyAOTexture = glutil::createTexture(screenSize.x, screenSize.y, GL_FLOAT, GL_RGBA, GL_RGBA16F);
    glBindImageTexture(3, historyAOTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    historyDepthTexture = glutil::createTexture(screenSize.x, screenSize.y, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, GL_DEPTH_STENCIL, GL_DEPTH32F_STENCIL8);
    glBindImageTexture(4, historyDepthTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_FLOAT_32_UNSIGNED_INT_24_8_REV);
}

void RenderEngine::render(std::vector<Model>& objs) {
    float currentFrame = static_cast<float>(SDL_GetTicks());
    animationTime = (currentFrame - startTime) / 1000.0f;

    glm::mat4 proj = camera->getProjectionMatrix();
    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    checkFrustum(objs);

    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gBufferPipeline.use();
        gBufferPipeline.setMat4("proj", proj);
        gBufferPipeline.setMat4("view", view);
        gBufferPipeline.setMat4("model", model);
        renderScene(objs, gBufferPipeline, false);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glGenerateTextureMipmap(positionTexture);
    glGenerateTextureMipmap(normalTexture);

    // Do GTAO with graphics pipeline
    /*
    glBindFramebuffer(GL_FRAMEBUFFER, gtaoFramebuffer.id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        gtaoGraphicsPipeline.use();
        glBindTextureUnit(0, depthMap);

        gtaoGraphicsPipeline.setInt("depthTexture", 0);
        gtaoGraphicsPipeline.setVec2("reciprocalOfScreenSize", reciprocalOfScreenSize);
        gtaoGraphicsPipeline.setVec2("screenSize", screenSize);
        gtaoGraphicsPipeline.setMat4("inverseProj", glm::inverse(proj));

        screenQuad.draw();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    */

    // Do GTAO
    gtaoPipeline.use();
    glBindTextureUnit(0, positionTexture);
    glBindTextureUnit(1, normalTexture);

    gtaoPipeline.setVec2("reciprocalOfScreenSize", reciprocalOfScreenSize);
    gtaoPipeline.setVec2("screenSize", screenSize);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute(numWarps.x, numWarps.y, numWarps.z);


    // Do SSAO
    ssaoPipeline.use();
    glBindTextureUnit(0, positionTexture);
    glBindTextureUnit(1, normalTexture);
    glBindTextureUnit(2, noiseTexture);
    ssaoPipeline.setInt("gPosition", 0);
    ssaoPipeline.setInt("gNormal", 1);
    ssaoPipeline.setInt("texNoise", 2);
    for (int i = 0; i < 64; i++) {
        ssaoPipeline.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel.at(i));
    }
    ssaoPipeline.setMat4("projection", proj);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute(WINDOW_WIDTH / warpSize.x, WINDOW_HEIGHT / warpSize.y, warpSize.z);

    // Blur AO texture
    blurPipeline.use();
    glBindTextureUnit(0, gtaoTexture);
    blurPipeline.setInt("ssaoTexture", 0);
    blurPipeline.setVec2("texelSize", glm::vec2(WINDOW_WIDTH, WINDOW_HEIGHT));

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glDispatchCompute(WINDOW_WIDTH / warpSize.x, WINDOW_HEIGHT / warpSize.y, warpSize.z);

    // Show texture on screen
    finalPipeline.use();
    glBindTextureUnit(0, blurTexture);
    finalPipeline.setInt("blurTexture", 0);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    screenQuad.draw();
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
