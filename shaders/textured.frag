#version 330

in vec4 c;
in vec2 tx;
in vec3 V;
in vec3 N;
in vec3 C;

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

	vec3 ambiant, diffuse, specular;
	
	for(int i = 0; i < size_light; ++i) {
		vec3 L = normalize(light[i].position - V);
		vec3 E = normalize(C-V);
		vec3 R = normalize(reflect(-L, N));

		float diff = max(0.0, dot(L, N));
		float spec = pow(max(0.0, dot(R, E)), material.shininess); 
		float distance = length(light[i].position - V);

		float attenuation = 48.0/(light[i].constant + light[i].linear*distance + light[i].quadratic*(distance*distance));

		ambiant += light[i].ambiant * texture(texSampler, tx).rgb * attenuation;
		diffuse += light[i].diffuse * texture(texSampler, tx).rgb * attenuation;
		specular += light[i].specular * texture(texSampler, tx).rgb * attenuation;
	}

    color = vec4(ambiant + diffuse + specular, 1.0);
}
