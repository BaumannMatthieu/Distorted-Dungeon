#version 400

 // We output the ex_Color variable to the next shader in the chain
in vec2 tx;
in float life_factor;


out vec4 color;

uniform sampler2D texSampler;
uniform vec4 color_particle;

void main(void) {
	vec4 color_center_fireball = vec4(1.f, 1.f, 0.f, 1.f);
	vec4 final_color = mix(color_particle, color_center_fireball, life_factor);
    color = final_color*texture(texSampler, tx);
}