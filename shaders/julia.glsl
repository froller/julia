#version 330 core

// Complex math
#define cplxmul(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define cplxcon(a)    vec2(a.x,-a.y)
#define cplxdiv(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))

// Rainbow generator
#define rainbow(x, o) float(2 * (abs(x - o) - abs(x - 0.2 - o) - abs(x - 0.4 - o) + abs(x - 0.6 - o)))

in vec2 UV;

uniform vec2 C;
uniform float sensitivity;

vec2 transform(const vec2 Z);
vec3 getColor(const vec2 point);

void main() {
    gl_FragColor = vec4(getColor(UV * 2), 1.0);
}

vec2 transform(const vec2 Z) {
    return cplxmul(Z, Z) + C * 2;
}

vec3 getColor(const vec2 point) {
    float c = 0;
    vec2 z = point + vec2(0, 0);
    while (c <= 1.0 && length(z) < 2.0) {
        z = transform(z);
        c += sensitivity;
    }
    return vec3(rainbow(c, 0.0), rainbow(c, 0.2), rainbow(c, 0.4));
}
