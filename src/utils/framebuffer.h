//
// Created by jpabl on 11/29/2023.
//

#pragma once
#include <vector>

class Framebuffer {
public:
    Framebuffer();
    Framebuffer(std::vector<unsigned int> attachmentValues, std::vector<unsigned int> textureIds);

    unsigned int id = 0;
};
