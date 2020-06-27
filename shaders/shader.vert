#version 450
#extension GL_ARB_separate_shader_objects : enable
// #extension GL_EXT_debug_printf : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) flat out vec3 outColor;
layout(location = 1) flat out vec3 outNormal;
layout(location = 2) flat out vec3 outLightVec;
layout(location = 3) flat out float outZ;
layout(location = 4) 	  out vec2 outTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    outColor    = inColor;
	outNormal   = (ubo.view * ubo.model * vec4(inNormal, 1.0)).xyz;
    outLightVec = (ubo.view * ubo.model * vec4(vec3(0.0, 3.0, 1.0), 1.0)).xyz;
    outTexCoord = inTexCoord;

	outZ = max(dot(
		normalize(inNormal),
		normalize(vec3(0.0, 0.0, 1.0))
	), 0.0);
}