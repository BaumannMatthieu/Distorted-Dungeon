#version 400

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_position;
in vec2 in_texcoord;

 // We output the ex_Color variable to the next shader in the chain
out vec2 tx;

uniform mat4 model, view, projection;

void main(void) {
	//vec3 position = vec3(position_screen.x, position_screen.y, 0.f) + vec3((in_position.x + 0.5f)*size.x, (in_position.z + 0.5f)*size.y, 0.f);

    gl_Position = vec4(in_position.x, in_position.y, 0.f, 1.f);

    tx = in_texcoord;
}