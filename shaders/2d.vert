#version 150

uniform mat4 model, view, projection;

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_position;
in vec4 in_color;

 // We output the ex_Color variable to the next shader in the chain
out vec4 ex_color;

void main(void) {
    // Since we are using flat lines, our input only had two points: x and y.
    // Set the Z coordinate to 0 and W coordinate to 1
    gl_Position = projection*view*model*vec4(in_position, 1.0);
 
    // Pass the color on to the fragment shader
    ex_color = in_color;
}