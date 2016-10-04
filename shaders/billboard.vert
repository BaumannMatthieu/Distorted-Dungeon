#version 400

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_position;
in vec4 in_color;
in vec2 in_texcoord;

 // We output the ex_Color variable to the next shader in the chain
out vec2 tx;

uniform mat4 model, view, projection;
uniform mat3 inv_model = mat3(1.0);

uniform vec2 size;
uniform vec3 position;

void main(void) {
    vec3 camera_up = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 camera_right = vec3(view[0][1], view[1][1], view[2][1]);

    gl_Position = projection*view*model*(vec4(position, 1.0) + vec4(camera_up*size.y*in_position.z + camera_right*size.x*in_position.x, 1.0));

    tx = in_texcoord;
}