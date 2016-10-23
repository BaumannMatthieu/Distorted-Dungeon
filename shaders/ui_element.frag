#version 400

uniform sampler2D texSampler;
uniform bool triggered;

 // We output the ex_Color variable to the next shader in the chain
in vec2 tx;

out vec4 color;

void main(void) {
	vec4 c = vec4(texture(texSampler, tx).rgb, 1.0);
    if(triggered) {
    	c += vec4(0.1f, 0.1f, 0.1f, 1.0f);
    }
    color = c;
    //color = vec4(1.0f, 0.f, 0.f, 1.f);
}