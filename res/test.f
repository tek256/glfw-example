#version 330

uniform sampler2D tex;

in vec2 out_tc;

out vec4 out_c;

void main(){
    out_c = texture(tex, out_tc);
}
