#version 400

// in_Position was bound to attribute index 0 and in_Color was bound to attribute index 1
in vec3 in_position;
in vec3 in_normal;
in vec4 in_color;
in vec2 in_texcoord;

 // We output the ex_Color variable to the next shader in the chain
out vec4 c;
out vec2 tx;
out vec3 V;
out vec3 N;
out vec3 E;

uniform mat4 model, view, projection;
uniform mat4 inv_model = mat4(1.0);

void main(void) {
    gl_Position = projection*view*model*vec4(in_position, 1.0);
 
    c = in_color;
    tx = in_texcoord;

    V = vec3(model * vec4(in_position, 1.f));
    E = -vec3( view[3] ) * mat3( view );
    N = normalize(vec3(inv_model * vec4(in_normal, 0.f)));
}
