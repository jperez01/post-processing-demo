#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image2D ssaoTexture;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform int kernelSize = 64;
uniform float radius = 0.5;
uniform float bias = 0.025;

uniform vec3 samples[64];
uniform mat4 projection;

const vec2 noiseScale = vec2(1920 / 4.0, 1080 / 4.0);

void main() {
	ivec2 texCoords = ivec2(gl_GlobalInvocationID.xy);

	vec2 convertedTexCoords = vec2(float(texCoords.x) / 1920.0, float(texCoords.y) / 1080.0);

	vec3 fragPos = texture(gPosition, convertedTexCoords).xyz;
	vec3 normal = normalize(texture(gNormal, convertedTexCoords).xyz);
	vec3 randomVec = normalize(texture(texNoise, convertedTexCoords * noiseScale).xyz);

	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0f;
	for (int i = 0; i < kernelSize; i++) {
		vec3 samplePos = TBN * samples[i];
		samplePos = fragPos + samplePos * radius;

		vec4 offset = vec4(samplePos, 1.0);
		offset = projection * offset;
		offset.xyz /= offset.w;
		offset.xyz = offset.xyz * 0.5 + 0.5;

		float sampleDepth = texture(gPosition, offset.xy).z;

		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
	}
	
	occlusion = 1.0 - (occlusion / kernelSize);
	imageStore(ssaoTexture, texCoords, vec4(occlusion));
}