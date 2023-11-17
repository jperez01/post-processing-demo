#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 view;
uniform mat4 model;
uniform mat4 proj;

void main() {
	vec4 convertedPos = view * model * vec4(aPos, 1.0);

	FragPos = convertedPos.xyz;
	TexCoords = aTexCoords;

	mat3 normalMatrix = mat3(transpose(inverse(view * model)));
	Normal = normalMatrix * aNormal;

	gl_Position = proj * convertedPos;
}