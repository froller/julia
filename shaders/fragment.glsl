#version 330 core

void juliaMain();
void mandelbrotMain();

uniform int fractal;

void main() {
    if (fractal == 0)
        juliaMain();
    else
        mandelbrotMain();
}

