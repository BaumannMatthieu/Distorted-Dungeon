#version 400

uniform sampler2D texSampler;

 // We output the ex_Color variable to the next shader in the chain
in vec2 tx;

out vec4 color;

void main(void) {
    color = vec4(1.f, 1.f, 1.f, texture(texSampler, tx).r);
    //color = vec4(1.0f, 0.f, 0.f, 1.f);
}