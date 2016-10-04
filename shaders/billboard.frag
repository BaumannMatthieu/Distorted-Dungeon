#version 400

uniform sampler2D texSampler;

 // We output the ex_Color variable to the next shader in the chain
in vec2 tx;

out vec4 color;

void main(void) {
    color = vec4(texture(texSampler, tx).rgb, 1.0);
}