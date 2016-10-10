#version 400

 // We output the ex_Color variable to the next shader in the chain
in vec2 tx;
in float life_factor;

out vec4 color;

uniform sampler2D texSampler;

void main(void) {
    color = glm::vec4(1.0f, life_factor/2.f, 0.f, life_factor)*texture(texSampler, tx);
}