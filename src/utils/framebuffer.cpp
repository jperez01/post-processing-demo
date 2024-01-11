//
// Created by jpabl on 11/29/2023.
//

#include "framebuffer.h"

#include <iostream>
#include <glad/glad.h>

Framebuffer::Framebuffer() = default;

Framebuffer::Framebuffer(std::vector<unsigned> attachmentValues, std::vector<unsigned> textureIds) {
    glCreateFramebuffers(1, &id);

    for (int i = 0; i < attachmentValues.size(); i++) {
        glNamedFramebufferTexture(id, attachmentValues.at(i), textureIds.at(i), 0);
    }

    glNamedFramebufferDrawBuffers(id, attachmentValues.size(), attachmentValues.data());

    GLint framebufferStatus = glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer not complete." << "\n";
    }
}
