#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_Color;
layout(location = 2) in vec2 v_TexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	outColor = texture(texSampler, v_TexCoord) * vec4(v_Color, 1.0);
}
