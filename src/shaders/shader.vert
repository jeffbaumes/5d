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
    vec4 entityLocationUV[10];
    vec4 entityRotation[10];
    vec4 entityLocationXYZ[10];
} ubo;

layout(location = 0) in uint inPos;
layout(location = 1) in ivec3 inXYZ;
layout(location = 2) in ivec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec2 fragPosition;


mat4 rotationX( in float angle ) {
	return mat4(	1.0,		0,			0,			0,
			 		0, 	cos(angle),	-sin(angle),		0,
					0, 	sin(angle),	 cos(angle),		0,
					0, 			0,			  0, 		1);
}

mat4 rotationY( in float angle ) {
	return mat4(	cos(angle),		0,		sin(angle),	0,
			 				0,		1.0,			 0,	0,
					-sin(angle),	0,		cos(angle),	0,
							0, 		0,				0,	1);
}

mat4 rotationZ( in float angle ) {
	return mat4(	cos(angle),		-sin(angle),	0,	0,
			 		sin(angle),		cos(angle),		0,	0,
							0,				0,		1,	0,
							0,				0,		0,	1);
}

vec2 materialTexCoord(int material, int face, ivec3 inPosition) {
    float a2 = 0.0001;
    int TEX_WIDTH = 3;
    vec2 materialTexCoord = vec2(
        ((material - 1) % TEX_WIDTH) / float(TEX_WIDTH) + a2,
        ((material - 1) / TEX_WIDTH) / float(TEX_WIDTH) + a2
    );
    float a = 1.0 / float(TEX_WIDTH) - 2.0 * a2;
    vec2 texCoord;
    if (abs(face) == 1) {
        texCoord.x = materialTexCoord.x + inPosition.y * a;
        texCoord.y = materialTexCoord.y + inPosition.z * a;
    } else if (abs(face) == 2) {
        texCoord.x = materialTexCoord.x + inPosition.x * a;
        texCoord.y = materialTexCoord.y + inPosition.z * a;
    } else if (abs(face) == 3) {
        texCoord.x = materialTexCoord.x + inPosition.x * a;
        texCoord.y = materialTexCoord.y + inPosition.y * a;
    }
    return texCoord;
}

void main() {
    int MY_MAX_INT = 32765;
    ivec3 inPosition = ivec3((inPos / 4) % 2, (inPos / 2) % 2, inPos % 2);
    int face = int((inPos / 8) % 8) - 3;
    int material = int(inPos / 64);
    vec3 calcXYZ = inXYZ;
    vec2 calcUV = inUV;
    vec3 rotation = vec3(0);
    if (inXYZ.y == MY_MAX_INT) {
        calcXYZ = ubo.entityLocationXYZ[inXYZ.x].xyz;
        calcUV = ubo.entityLocationUV[inXYZ.x].xy;
        rotation = ubo.entityRotation[inXYZ.x].xyz;
    }

    vec2 texCoord = materialTexCoord(material, face, inPosition);

    vec3 inShow;
    vec2 inHide;
    vec3 eyeShow;
    vec2 eyeHide;
    vec3 loc;
    if (ubo.uvView < 0.5f) {
        inShow = calcXYZ;
        inHide = calcUV;
        eyeShow = ubo.xyz;
        eyeHide = ubo.uv - 0.5f;
        float a = ubo.uvView * 2.0;
        loc = mix(inShow, floor(eyeShow), a);
    } else {
        inShow = vec3(calcUV.x, calcXYZ.y, calcUV.y);
        inHide = calcXYZ.xz;
        eyeShow = vec3(ubo.uv.x, ubo.xyz.y, ubo.uv.y);
        eyeHide = ubo.xyz.xz - 0.5f;
        float a = (ubo.uvView - 0.5f) * 2.0;
        loc = mix(floor(eyeShow), inShow, a);
    }
    loc.y = calcXYZ.y;

    vec3 pos = vec3(1000000.0, inPosition.y, 0.0);
    float area = 0.0;

    if (inXYZ.y == MY_MAX_INT) {
        inHide = round(inHide - 0.5) + 0.5;
        if (abs((inHide.x - 0.5) - eyeHide.x) < 0.5 && abs((inHide.y - 0.5) - eyeHide.y) < 0.5) {
            pos = inPosition;
            area = 1.0;
        }
    } else {
        vec2 eyeHideFloor = floor(eyeHide);
        vec2 eyeHideFrac = eyeHide - eyeHideFloor;
        vec3 frac = vec3(eyeHideFrac.x, 0.0, eyeHideFrac.y);
        if (inHide == eyeHideFloor) {
            pos.x = inPosition.x - frac.x;
            pos.z = inPosition.z - frac.z;
            if (face == -1) {
                pos.x = 0.001;
            } else if (face == -3) {
                pos.z = 0.001;
            }
            area = (1.0 - frac.x) * (1.0 - frac.z);
        } else if (inHide == eyeHideFloor + vec2(1.0, 0.0) && eyeHide != eyeHideFloor) {
            pos.x = inPosition.x - frac.x + 1.0;
            pos.z = inPosition.z - frac.z;
            if (face == 1) {
                pos.x = 0.999;
            } else if (face == -3) {
                pos.z = 0.001;
            }
            area = frac.x * (1.0 - frac.z);
        } else if (inHide == eyeHideFloor + vec2(0.0, 1.0) && eyeHide != eyeHideFloor) {
            pos.x = inPosition.x - frac.x;
            pos.z = inPosition.z - frac.z + 1.0;
            if (face == -1) {
                pos.x = 0.001;
            } else if (face == 3) {
                pos.z = 0.999;
            }
            area = (1.0 - frac.x) * frac.z;
        } else if (inHide == eyeHideFloor + vec2(1.0, 1.0) && eyeHide != eyeHideFloor) {
            pos.x = inPosition.x - frac.x + 1.0;
            pos.z = inPosition.z - frac.z + 1.0;
            if (face == 1) {
                pos.x = 0.999;
            } else if (face == 3) {
                pos.z = 0.999;
            }
            area = frac.x * frac.z;
        }
    }

    pos = (vec4(pos - vec3(0.5), 1.0) * rotationX(rotation.x) * rotationY(rotation.y) * rotationZ(rotation.z)).xyz + vec3(0.5);
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(pos + loc, 1.0);

    fragColor = vec3(area);
    fragTexCoord = texCoord;
    if (inXYZ.y == MY_MAX_INT) {
        fragPosition = vec2(0);
    } else {
        fragPosition = vec2(pos.x, pos.z);
    }
}
