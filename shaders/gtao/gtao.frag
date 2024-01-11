#version 430 core

out float aoTerm;

in vec2 TexCoords;

#define PI 3.1415926535897932384626433832795
#define PI_HALF 1.5707963267948966192313216916398

uniform sampler2D depthTexture;

uniform vec2 screenSize;
uniform vec2 reciprocalOfScreenSize;

uniform int sliceCount = 8;
uniform int sliceStepCount = 4;

uniform mat4 inverseProj;

uniform float inputEffectRadius = 0.5;
uniform float inputFalloffRange = 0.615;
uniform float inputRadiusMultiplier = 1.457;
uniform float sampleDistributionPower = 1.0;
uniform float thinOccluderCompensation = 1.0;
uniform float samplingMipOffset = 3.3;
uniform float mipLevels = 5;

float GTAOFastAcos(float x)
{
    float res = -0.156583 * abs(x) + PI_HALF;
    res *= sqrt(1.0 - abs(x));
    return x >= 0 ? res : PI - res;
}

vec3 getViewSpacePositionFromDepth(vec2 texCoords) {
    float depth = texture(depthTexture, texCoords).x;
    depth = depth * 2.0 - 1.0;

    vec3 screenSpaceVector = vec3(texCoords * 2.0 - 1.0, depth);

    vec4 projVector = inverseProj * vec4(screenSpaceVector, 1.0);
    vec3 viewSpaceVector = projVector.xyz / projVector.w;

    return viewSpaceVector;
}

vec2 getViewSpacePositionFromScreenSpace(vec2 texCoords) {
    vec3 screenSpaceVector = vec3(texCoords * 2.0 - 1.0, 1.0);
    vec4 projVector = inverseProj * vec4(screenSpaceVector, 1.0);

    return projVector.xy / projVector.w;
}

vec3 getViewSpaceNormalFromDepth(vec3 initialPos, vec2 texCoords, vec2 delta) {
    vec3 posUp = getViewSpacePositionFromDepth(texCoords + vec2(0, delta.y));
    vec3 posDown = getViewSpacePositionFromDepth(texCoords + vec2(0, -delta.y));
    vec3 posLeft = getViewSpacePositionFromDepth(texCoords + vec2(delta.x, 0));
    vec3 posRight = getViewSpacePositionFromDepth(texCoords + vec2(-delta.x, 0));
}

void main() {
    vec2 texCoords = TexCoords;

    vec3 viewSpacePos = getViewSpacePositionFromDepth(texCoords);
    vec3 viewVector = normalize(-viewSpacePos);

    const float effectRadius = inputEffectRadius * inputRadiusMultiplier;
    const float falloffRange = inputFalloffRange * effectRadius;
    const float falloffFrom = effectRadius * (1.0 - inputFalloffRange);
    const float falloffMultiplier = -1.0 / falloffRange;
    const float falloffAdd = falloffFrom / falloffRange + 1.0;

    vec3 otherViewSpacePos = getViewSpacePositionFromDepth(texCoords + reciprocalOfScreenSize);
    vec3 diffInViewSpace = otherViewSpacePos - viewSpacePos;
    float screenSpaceRadius = effectRadius / diffInViewSpace.x;

    float visibility = 0.0;
    visibility += clamp((10 - screenSpaceRadius) / 100 * 0.5, 0.0, 1.0);

    const float minS = 1.3 / screenSpaceRadius;

    for (int slice = 0; slice < sliceCount; slice++) {
        float phi = (PI / sliceCount) * slice;
        vec2 angleVector = vec2(cos(phi), sin(phi));
        vec2 omega = vec2(angleVector.x, -angleVector.y) * screenSpaceRadius;

        // Project normal onto slice plane
        vec3 directionVector = vec3(angleVector.x, angleVector.y, 0);
        vec3 orthoDirectionVector = directionVector - (dot(directionVector, viewVector) * viewVector);
        vec3 axis = normalize(cross(directionVector, viewVector));
        vec3 projectedNormal = viewSpaceNormal - axis * dot(viewSpaceNormal, axis);

        float projectedNormalLength = length(projectedNormal);
        float cosNormal = dot(projectedNormal, viewVector) / projectedNormalLength;
        float clampedCos = clamp(cosNormal, 0.0, 1.0);

        float signOfNormal = sign(dot(orthoDirectionVector, projectedNormal));
        float n = signOfNormal * GTAOFastAcos(clampedCos);

        float minHorizon0 = cos(n + PI_HALF), minHorizon1 = cos(n - PI_HALF);
        float horizon0 = minHorizon0, horizon1 = minHorizon1;

        for (float step = 0.0; step < sliceStepCount; step++) {
            float stepBaseNoise = (slice + step * sliceStepCount) * 0.6180339887498948482;
            float stepNoise = frac(noiseSample + stepBaseNoise);

            float s = (step + stepNoise) / sliceStepCount;
            s = pow(s, sampleDistributionPower);
            s += minS;

            vec2 sampleOffset = s * omega;
            float sampleOffsetLength = length(sampleOffset);

            float mipLevel = clamp(log2(sampleOffsetLength) - samplingMipOffset, 0, mipLevels);
            sampleOffset  = round(sampleOffset) * reciprocalOfScreenSize;

            vec3 samplePos0 = getViewSpacePositionFromDepth(texCoords + sampleOffset);
            vec3 samplePos1 = getViewSpacePositionFromDepth(texCoords - sampleOffset);

            vec3 sampleDelta0 = samplePos0 - viewSpacePos, sampleDelta1 = samplePos1 - viewSpacePos;

            vec3 sampleHorizonVec0 = normalize(sampleDelta0), sampleHorizonVec1 = normalize(sampleDelta1);

            float falloffBase0 = length(vec3(sampleDelta0.x, sampleDelta0.y, sampleDelta0.z * (1 + thinOccluderCompensation)));
            float falloffBase1 = length(vec3(sampleDelta1.x, sampleDelta1.y, sampleDelta1.z * (1 + thinOccluderCompensation)));

            float weight0 = clamp(falloffBase0 * falloffMultiplier + falloffAdd, 0.0, 1.0);
            float weight1 = clamp(falloffBase1 * falloffMultiplier + falloffAdd, 0.0, 1.0);

            float sampleHorizonCos0 = dot(sampleHorizonVec0, viewVector), sampleHorizonCos1 = dot(sampleHorizonVec1, viewVector);

            sampleHorizonCos0 = lerp(minHorizon0, sampleHorizonCos0, weight0);
            sampleHorizonCos1 = lerp(minHorizon1, sampleHorizonCos1, weight1);

            horizon0 = max(horizon0, sampleHorizonCos0);
            horizon1 = max(horizon1, sampleHorizonCos1);
        }

        float finalHorizon0 = -GTAOFastAcos(horizon1), finalHorizon1 = GTAOFastAcos(horizon0);

        float integratedArc0 = 0.25 * (clampedCos + 2 * finalHorizon0 * sin(n) - cos(2 * finalHorizon0 - n));
        float integratedArc1 = 0.25 * (clampedCos + 2 * finalHorizon1 * sin(n) - cos(2 * finalHorizon1 - n));

        float localVisibility = projectedNormalLength * (integratedArc0 + integratedArc1);
        visibility += localVisibility;
    }
    visibility /= sliceCount;
    visibility = max(0.03, visibility);

    aoTerm = visibility;
}