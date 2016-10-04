#version 330

uniform mat4 view, projection;

in vec3 in_position;

out vec3 tx;

void main(void) {
    gl_Position = projection*view*vec4(in_position, 1.0);
 	tx = in_position;
}