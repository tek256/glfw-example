#version 330
layout(location = 0) in vec4 in_vert;

uniform mat4 proj;
uniform mat4 view;
//uniform mat4 model;
uniform mat4 models[16];

out vec2 out_tc;

void main(){
	mat4 model = models[gl_InstanceID];
	out_tc = in_vert.zw;
	gl_Position = proj * view * model * vec4(in_vert.xy, 0.f, 1.f);
}
