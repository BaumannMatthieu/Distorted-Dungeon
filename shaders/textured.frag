#version 330

in vec4 c;
in vec2 tx;
in vec3 V;
in vec3 N;
in vec3 E;

out vec4 color;

uniform sampler2D texSampler;

struct Material {
	float shininess;	
};

struct PointLight {
	vec3 position;
	
	vec3 ambiant;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

uniform Material material;

#define MAX_NUM_LIGHTS 50
uniform PointLight light[MAX_NUM_LIGHTS];

uniform int num_lights;
 
void main(void) {
	int size_light = min(MAX_NUM_LIGHTS, num_lights);

	for(int i = 0; i < num_lights; ++i) {
		vec3 LightVect = normalize(light[i].position - V);
		vec3 EyeVect = normalize(E-V);
		vec3 NormalVect = normalize(N);

		vec3 HalfVector = normalize(LightVect + EyeVect);

	    vec4 Ca = texture(texSampler, tx);
	    vec4 Cd = max(0.0, dot(LightVect, NormalVect)) * texture(texSampler, tx);
	    vec4 Cs = pow(max(0.0, dot(NormalVect, HalfVector)), 4.0f*material.shininess) * texture(texSampler, tx);

	    float distance = length(light[i].position - V);
	    float attenuation = 48.f/(light[i].constant + light[i].linear*distance + light[i].quadratic*(distance*distance));

	    color += (Ca + Cs + Cd) * attenuation;
	}
}
