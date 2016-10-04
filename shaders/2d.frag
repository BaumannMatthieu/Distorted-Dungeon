#version 150

in vec4 ex_color;

out vec4 color;
 
void main(void) {
    color = vec4(ex_color);
}