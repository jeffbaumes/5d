#version 450
#extension GL_ARB_separate_shader_objects : enable

highp vec2;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec2 uv;
    vec3 selectedCell;
    vec2 selectedCellUV;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inXYZ;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec2 texCord;
layout(location = 4) in vec3 face;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 fragPosition;


void main() {
    vec2 uv = ubo.uv;
    vec2 uvFloor = floor(uv);
    vec2 uvFrac = uv - uvFloor;
    vec3 frac = vec3(uvFrac.x, 0.0, uvFrac.y);
    vec3 pos = vec3(1000000.0, inPosition.y, 0.0);
    vec2 tex = texCord;
    float area = 0.0;
    if (inUV == uvFloor) {
        pos.x = inPosition.x - frac.x;
        pos.z = inPosition.z - frac.z;
        if (face.x == -1.0) {
            pos.x = 0.001;
        } else if (face.z == -1.0) {
            pos.z = 0.001;
        }
        area = (1.0 - frac.x) * (1.0 - frac.z);
    } else if (inUV == uvFloor + vec2(1.0, 0.0) && uv != uvFloor) {
        pos.x = inPosition.x - frac.x + 1.0;
        pos.z = inPosition.z - frac.z;
        if (face.x == 1.0) {
            pos.x = 0.999;
        } else if (face.z == -1.0) {
            pos.z = 0.001;
        }
        area = frac.x * (1.0 - frac.z);
    } else if (inUV == uvFloor + vec2(0.0, 1.0) && uv != uvFloor) {
        pos.x = inPosition.x - frac.x;
        pos.z = inPosition.z - frac.z + 1.0;
        if (face.x == -1.0) {
            pos.x = 0.001;
        } else if (face.z == 1.0) {
            pos.z = 0.999;
        }
        area = (1.0 - frac.x) * frac.z;
    } else if (inUV == uvFloor + vec2(1.0, 1.0) && uv != uvFloor) {
        pos.x = inPosition.x - frac.x + 1.0;
        pos.z = inPosition.z - frac.z + 1.0;
        if (face.x == 1.0) {
            pos.x = 0.999;
        } else if (face.z == 1.0) {
            pos.z = 0.999;
        }
        area = frac.x * frac.z;
    }
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos + inXYZ, 1.0);
    if (floor(inXYZ) == ubo.selectedCell && floor(inUV) == ubo.selectedCellUV) {
        fragColor = vec3(0.0, 1.0, 0.1);
    } else {
        fragColor = inXYZ / 8;
        // fragColor = 0.5 * (inXYZ + 4.0) / 8.0 + 0.5 * vec3((inUV + 4.0)/ 8.0, 1.0) - 0.25 * inPosition;
    }
    fragColor = vec3(area);
    fragTexCoord = tex;
    fragPosition = vec2(pos.x, pos.z);
}
