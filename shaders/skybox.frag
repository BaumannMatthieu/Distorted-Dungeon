#version 330

in vec3 tx;

out vec4 color;

uniform samplerCube skybox;
 
void main(void) {
    color = texture(skybox, tx);
}