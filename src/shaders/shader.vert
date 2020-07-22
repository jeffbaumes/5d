#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 uv;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inXYZ;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    vec2 uv = ubo.uv;
    vec2 uvFloor = floor(uv);
    vec2 uvFrac = 1.0 - (uv - uvFloor);
    vec3 frac = vec3(uvFrac.x, 0.0, uvFrac.y);
    vec3 pos = vec3(1000000.0, inPosition.y, 0.0);
    if (inUV == uvFloor) {
        pos.x = inPosition.x * frac.x;
        pos.z = inPosition.z * frac.z;
    } else if (inUV == uvFloor + vec2(1.0, 0.0)) {
        pos.x = frac.x + inPosition.x * (1.0 - frac.x);
        pos.z = inPosition.z * frac.z;
    } else if (inUV == uvFloor + vec2(0.0, 1.0)) {
        pos.x = inPosition.x * frac.x;
        pos.z = frac.z + inPosition.z * (1.0 - frac.z);
    } else if (inUV == uvFloor + vec2(1.0, 1.0)) {
        pos.x = frac.x + inPosition.x * (1.0 - frac.x);
        pos.z = frac.z + inPosition.z * (1.0 - frac.z);
    }
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos + inXYZ, 1.0);
    fragColor = 0.5 * (inXYZ + 4.0) / 8.0 + 0.5 * vec3((inUV + 4.0)/ 8.0, 1.0) - 0.25 * inPosition;
    fragTexCoord = inUV;
}
