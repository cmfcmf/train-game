#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;


layout(location = 0) flat in vec3 inColor;
layout(location = 1) flat in vec3 inNormal;
layout(location = 2) flat in vec3 inLightVec;
layout(location = 3) flat in float inZ;
layout(location = 4) 	  in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);

	// outColor = vec4(inColor, 1.0);

	// outColor = vec4(max(dot(L, N), 0.0) * inColor, 1.0);

	// outColor = vec4(1.0, 1.0 * min(1.0, (inZ - 27) / 20), 0.0, 1.0);

	float ratio = 1.0;

	outColor = vec4(
		texture(texSampler, inTexCoord).rgb * ratio
			+ (inColor * 0.25 + inColor * 0.75 * inZ) * (1.0 - ratio),
		1.0);
}