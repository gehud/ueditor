#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;

uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_UV;

void main() {
	v_UV = a_UV;
	gl_Position = u_Projection * u_View * u_Model * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

uniform sampler2D u_Texture;
uniform vec4 u_Color = vec4(1.0);

in vec2 v_UV;

void main() {
	gl_FragColor = texture(u_Texture, v_UV) * u_Color;
}