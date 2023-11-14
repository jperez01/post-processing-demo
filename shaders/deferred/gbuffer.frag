#version 430 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D albedoTexture;

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);

	gAlbedo = texture(albedoTexture, TexCoords);
}