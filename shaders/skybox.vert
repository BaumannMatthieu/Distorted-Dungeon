#version 330

uniform mat4 view, projection, model;

in vec3 in_position;

out vec3 tx;

void main(void) {
    gl_Position = projection*view*model*vec4(in_position, 1.0);
 	tx = in_position;
}