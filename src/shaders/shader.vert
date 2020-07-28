#version 450
#extension GL_ARB_separate_shader_objects : enable

highp vec2;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 selectedCell;
    vec3 xyz;
    vec2 uv;
    vec2 selectedCellUV;
    float uvView;
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
    vec3 inShow;
    vec2 inHide;
    vec3 eyeShow;
    vec2 eyeHide;
    if (ubo.uvView < 0.5f) {
        inShow = inXYZ;
        inHide = inUV;
        eyeShow = ubo.xyz;
        eyeHide = ubo.uv;
    } else {
        inShow = vec3(inUV.x, inXYZ.y, inUV.y);
        inHide = inXYZ.xz;
        eyeShow = vec3(ubo.uv.x, ubo.xyz.y, ubo.uv.y);
        eyeHide = ubo.xyz.xz;
    }
    vec2 eyeHideFloor = floor(eyeHide);
    vec2 eyeHideFrac = eyeHide - eyeHideFloor;
    vec3 frac = vec3(eyeHideFrac.x, 0.0, eyeHideFrac.y);
    vec3 pos = vec3(1000000.0, inPosition.y, 0.0);
    vec2 tex = texCord;
    float area = 0.0;
    if (inHide == eyeHideFloor) {
        pos.x = inPosition.x - frac.x;
        pos.z = inPosition.z - frac.z;
        if (face.x == -1.0) {
            pos.x = 0.001;
        } else if (face.z == -1.0) {
            pos.z = 0.001;
        }
        area = (1.0 - frac.x) * (1.0 - frac.z);
    } else if (inHide == eyeHideFloor + vec2(1.0, 0.0) && eyeHide != eyeHideFloor) {
        pos.x = inPosition.x - frac.x + 1.0;
        pos.z = inPosition.z - frac.z;
        if (face.x == 1.0) {
            pos.x = 0.999;
        } else if (face.z == -1.0) {
            pos.z = 0.001;
        }
        area = frac.x * (1.0 - frac.z);
    } else if (inHide == eyeHideFloor + vec2(0.0, 1.0) && eyeHide != eyeHideFloor) {
        pos.x = inPosition.x - frac.x;
        pos.z = inPosition.z - frac.z + 1.0;
        if (face.x == -1.0) {
            pos.x = 0.001;
        } else if (face.z == 1.0) {
            pos.z = 0.999;
        }
        area = (1.0 - frac.x) * frac.z;
    } else if (inHide == eyeHideFloor + vec2(1.0, 1.0) && eyeHide != eyeHideFloor) {
        pos.x = inPosition.x - frac.x + 1.0;
        pos.z = inPosition.z - frac.z + 1.0;
        if (face.x == 1.0) {
            pos.x = 0.999;
        } else if (face.z == 1.0) {
            pos.z = 0.999;
        }
        area = frac.x * frac.z;
    }
    vec3 loc;
    if (ubo.uvView < 0.5f) {
        float a = ubo.uvView * 2.0;
        loc = mix(inShow, floor(eyeShow), a);
    } else {
        float a = (ubo.uvView - 0.5f) * 2.0;
        loc = mix(floor(eyeShow), inShow, a);
    }
    loc.y = inXYZ.y;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos + loc, 1.0);
    fragColor = vec3(area);
    fragTexCoord = tex;
    fragPosition = vec2(pos.x, pos.z);
}
