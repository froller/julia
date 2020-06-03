#version 330 core

in vec2 UV;

//const vec2 C = vec2(0.1, 0.63);
uniform vec2 C;
uniform float sensitivity;

vec2 juliaTransform(const vec2 Z);
vec3 getColor(const vec2 point);

void main() {
    gl_FragColor = vec4(getColor(UV * 2), 1.0);
}

vec2 juliaTransform(const vec2 Z) {
    return vec2(Z.x * Z.x - Z.y * Z.y, Z.x * Z.y + Z.y * Z.x) + C * 2;
}

vec3 getColor(const vec2 point) {
    float c = 0;
    vec2 z = point + vec2(0, 0);
    while (c < 1.0 && length(z) < 1000.0) {
        z = juliaTransform(z);
        c += sensitivity;
    }
    return vec3(c);
}
