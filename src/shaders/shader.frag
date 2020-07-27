#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec2 fragPosition;

layout(location = 0) out vec4 outColor;

void main() {
    // outColor = vec4(fragColor, 1.0);
    if (fragPosition.x < 0.0 || fragPosition.x > 1.0 || fragPosition.y < 0.0 || fragPosition.y > 1.0) {
        discard;
    }
    outColor = texture(texSampler, fragTexCoord);
    // outColor = vec4(fragTexCoord, 0.0, 1.0);
}
