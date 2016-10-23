#version 400

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_position;
in vec2 in_texcoord;

 // We output the ex_Color variable to the next shader in the chain
out vec2 tx;

uniform mat4 model, view, projection;

uniform vec2 size;
uniform vec2 position_screen;

void main(void) {
	vec3 position = vec3(position_screen.x, position_screen.y, 0.f) + vec3((in_position.x + 0.5f)*size.x, (in_position.z + 0.5f)*size.y, 0.f);

    gl_Position = vec4(position_screen.x + in_position.x*size.x, position_screen.y + in_position.z*size.y, 0.f, 1.f);
    //gl_Position = projection * view * model * vec4(in_position, 1.0f);
    

    //vec3 camera_up = vec3(view[0][0], view[1][0], view[2][0]);
    //vec3 camera_right = vec3(view[0][1], view[1][1], view[2][1]);

    //gl_Position = projection*view*model*vec4(camera_up*size.y*in_position.z + camera_right*size.x*in_position.x, 1.0);

    tx = in_texcoord;
}