#version 330 core

// Complex math
#define cplxmul(a, b) vec2(a.x*b.x-a.y*b.y, a.x*b.y+a.y*b.x)
#define cplxcon(a)    vec2(a.x,-a.y)
#define cplxdiv(a, b) vec2(((a.x*b.x+a.y*b.y)/(b.x*b.x+b.y*b.y)),((a.y*b.x-a.x*b.y)/(b.x*b.x+b.y*b.y)))

// Rainbow generator BRYGCBMW
#define rainbow(x) vec3( 3.496 * (abs(x - 0.0) - abs(x - 0.143) - abs(x - 0.286) + abs(x - 0.429) + abs(x - 0.715) - abs(x - 0.858)) + 0.5, 3.496 * (abs(x - 0.143) - abs(x - 0.286) - abs(x - 0.572) + abs(x - 0.715) + abs(x - 0.858) - abs(x - 1.001)) + 0.5, 3.496 * (abs(x - 0.429) - abs(x - 0.572)) + 0.5 )
    

in vec2 UV;

uniform vec2 C;
uniform float sensitivity;
uniform int showColor;

vec2 mandelbrotTransform(const vec2 Z);
vec3 mandelbrotGetColor(const vec2 point);

void mandelbrotMain() {
    gl_FragColor = gl_FragColor + vec4(mandelbrotGetColor(UV * 2), 1.0) * 0.5;
}

vec2 mandelbrotTransform(const vec2 Z, const vec2 c) {
    return cplxmul(Z, Z) + c;
}

vec3 mandelbrotGetColor(const vec2 point) {
    float c = 0;
    vec2 z = vec2(0);
    while (c <= 1.001 && length(z) < 2.0) {
        z = mandelbrotTransform(z, point);
        c += sensitivity;
    }
    
    if (showColor == 0)
        return vec3(c);
    else
        return rainbow(c);
}
