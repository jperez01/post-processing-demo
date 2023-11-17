#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, binding = 1) uniform image2D blurTexture;

uniform int radius = 2;
uniform vec2 texelSize;
uniform sampler2D ssaoTexture;

void main() {
	ivec2 texCoords = ivec2(gl_GlobalInvocationID.xy);

	float result = 0.0;
	for (int x = -radius; x < radius; x++) {
		for (int y = -radius; y < radius; y++) {
			vec2 offset = vec2(x + texCoords.x, y + texCoords.y) / texelSize;

			result += texture(ssaoTexture, offset).x;
		}
	}
	result /= (radius * radius * 4);

	imageStore(blurTexture, texCoords, vec4(result));
}