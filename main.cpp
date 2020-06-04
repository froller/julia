#include <stdlib.h>
#include <stdio.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <SDL.h>

#define UNUSED(x) (void)x;

static int screenWidth = 1080;
static int screenHeight = 1080;

static glm::vec2 mousePos(0.f, 0.f);
static float sensitivity = 0.01f;

static glm::vec2 center(0.f, 0.f);
static float zoom = 1.f;

static glm::mat4 view(1.f);

void usage(int argc, char **argv);
GLuint loadShaders(const std::filesystem::path &vertexShaderFileName, const std::filesystem::path &fragmentShaderFileName);
GLuint loadShader(const GLuint shaderType, const std::filesystem::path &shaderFileName);
int render(const GLuint program);

int main(int argc, char **argv) {
    if (argc != 3)
    {
        usage(argc, argv);
        exit(1);
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing SDL");
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Couldn't initialize SDL: %s", SDL_GetError());
        exit(3);
    }

#if CMAKE_BUILD_TYPE == Debug
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#else
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
#endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Creating window");
    SDL_Window *window = SDL_CreateWindow("Julia", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL);
    if (!window)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Couldn't create window: %s", SDL_GetError());
        exit(3);
    }
    
    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    if (!ctx)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Couldn't create OpenGL context: %s", SDL_GetError());
        exit(3);
    }
    
    SDL_GL_MakeCurrent(window, ctx);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to init GLEW!");
        exit(3);
    }

    GLuint program = loadShaders(
        std::filesystem::path(argv[0]).remove_filename().append("../shaders/tri.glsl").lexically_normal(),
        std::filesystem::path(argv[0]).remove_filename().append("../shaders/julia.glsl").lexically_normal()
    );
    
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Running event loop");
    SDL_bool quit = SDL_FALSE;
    SDL_bool lmbDown = SDL_FALSE;
    SDL_bool ctrlDown = SDL_FALSE;
    SDL_bool altDown = SDL_FALSE;
    SDL_bool shiftDown = SDL_FALSE;
    SDL_Event event;
    while (!quit)
    {
        if (SDL_PollEvent(&event))
            switch (event.type)
            {
            case SDL_QUIT:
                SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_QUIT");
                quit = SDL_TRUE;
                break;
            case SDL_MOUSEMOTION:
                //SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_MOUSEMOTION: %+d, %+d (%d, %d)", event.motion.xrel, event.motion.yrel, event.motion.x, event.motion.y);
                if (lmbDown)
                {
                    mousePos.x = (float(event.motion.x) - screenWidth / 2) / screenWidth * 2;
                    mousePos.y = (float(event.motion.y) * -1 + screenHeight / 2) / screenWidth * 2;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                //SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_MOUSEBUTTONDOWN: %d", event.button.button);
                if (event.button.button == 1)
                {
                    lmbDown = SDL_TRUE;
                    mousePos.x = (float(event.motion.x) - screenWidth / 2) / screenWidth * 2;
                    mousePos.y = (float(event.motion.y) * -1 + screenHeight / 2) / screenWidth * 2;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                //SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_MOUSEBUTTONUP: %d", event.button.button);
                switch (event.button.button)
                {
                case 1:
                    lmbDown = SDL_FALSE;
                    break;
                case 2:
                    zoom = 1.0f;
                    center = glm::vec2(0, 0);
                    break;
                }
                break;
            case SDL_MOUSEWHEEL:
                //SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_MOUSEWHEEL: %+d, %+d", event.wheel.x, event.wheel.y);
                if (ctrlDown)
                {
                    if (sensitivity > 1.f)
                        sensitivity = 1.f;
                    else if (sensitivity < 0.001)
                        sensitivity = 0.001;
                    zoom = zoom * pow(1.1, event.wheel.y);
                }
                else if (altDown)
                    sensitivity = sensitivity + 0.001 * event.wheel.y;
                else if (shiftDown)
                    center += glm::vec2(float(event.wheel.y) / screenWidth * zoom * -10, 0);
                else
                    center += glm::vec2(float(event.wheel.x) / screenWidth * zoom * -10, float(event.wheel.y) / screenWidth * zoom * 10);

                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "Window size: %d, %d", event.window.data1, event.window.data2);
                    screenWidth = event.window.data1;
                    screenHeight = event.window.data2;
                }
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    if (event.key.state == SDL_PRESSED)
                    {
                        SDL_Event quitEvent;
                        quitEvent.type = SDL_QUIT;
                        SDL_PushEvent(&quitEvent);
                    }
                    break;
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                    ctrlDown = SDL_bool(event.key.state == SDL_PRESSED);
                    break;
                case SDLK_LALT:
                case SDLK_RALT:
                    altDown = SDL_bool(event.key.state == SDL_PRESSED);
                    break;
                case SDLK_LSHIFT:
                case SDLK_RSHIFT:
                    shiftDown = SDL_bool(event.key.state == SDL_PRESSED);
                    break;
                case SDLK_PRINTSCREEN:
                    if (event.key.state == SDL_PRESSED)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Making screenshots is yet to do");
                    break;
                }
                break;
            };
            
        render(program);
        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Sayonara");

    return 0;
}

void usage(int argc, char ** argv)
{
    UNUSED(argc)
    std::filesystem::path executable(argv[0]);
    fprintf(stderr, "Usage: %s a b\n\n\ta\treal component\n\tb\timaginary component\n\n", executable.filename().c_str());
}

GLuint loadShaders(const std::filesystem::path &vertexShaderFileName, const std::filesystem::path &fragmentShaderFileName)
{
    GLuint program = glCreateProgram();
    
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexShaderFileName);
    glAttachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentShaderFileName);
    glAttachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "Linking program");
    glLinkProgram(program);
    GLint result = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (!result)
    {
        GLint infoLogLen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);
        std::string infoLog;
        infoLog.reserve(infoLogLen);
        glGetProgramInfoLog(program, infoLogLen, nullptr, infoLog.data());
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Error linking program: %s", infoLog.c_str());
    }
    else
        SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "Program linked successfully");
  
    return program;
}

GLuint loadShader (const GLuint shaderType, const std::filesystem::path &shaderFileName )
{
    GLuint shader = glCreateShader(shaderType);

    SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "Loading shader source from %s", shaderFileName.c_str());
    std::ifstream fs(shaderFileName, std::ios::in);
    if (fs && fs.good())
    {
        std::string shaderText;
        std::stringstream ss;
        ss << fs.rdbuf();
        shaderText = ss.str();
        fs.close();

        SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "Compiling shader");
        char const *shaderSource = shaderText.c_str(); 
        glShaderSource(shader, 1, &shaderSource, nullptr);
        glCompileShader(shader);
        GLint result = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if (!result)
        {
            GLint infoLogLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);
            std::string infoLog;
            infoLog.reserve(infoLogLen);
            glGetShaderInfoLog(shader, infoLogLen, nullptr, infoLog.data());
            SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Error compiling shader: %s", infoLog.c_str());
        }
        else
            SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "Shader compiled successfully");
    }
    else
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Error loading shader");

    return shader;
}


int render (const GLuint program)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);
   
    glm::mat4 view = glm::lookAt(
        glm::vec3(center.x, center.y, 1.0f),    // положение камеры
        glm::vec3(center.x, center.y, 0.0f),    // положение "прицела"
        glm::vec3(0, 1, 0)                      // ориентация 0, 1, 0 - головой вверх
    );
    GLuint viewId = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]);

    glm::mat4 scale = glm::diagonal4x4(glm::vec4(zoom, zoom, 1.f, 1.f));

    GLuint scaleId = glGetUniformLocation(program, "scale");
    glUniformMatrix4fv(scaleId, 1, GL_FALSE, &scale[0][0]);
    
    GLuint CId = glGetUniformLocation(program, "C");
    glUniform2fv(CId, 1, &mousePos[0]);

    GLuint sensitivityId = glGetUniformLocation(program, "sensitivity");
    glUniform1f(sensitivityId, sensitivity);
    
    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDeleteVertexArrays(1, &vertexArray);

    return 0;
}
