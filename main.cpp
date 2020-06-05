#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef __APPLE__
# include <OpenGL/gl3.h>
# include <OpenGL/gl.h>
# include <SDL2/SDL.h>
#else
# include <GL/gl.h>
# include <SDL.h>
#endif

#ifndef __APPLE__
# define GLM_ENABLE_EXPERIMENTAL
# include <GL/glew.h>
#endif


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_operation.hpp>


#ifdef WITH_PNG
#include <png.h>
#ifdef WITH_ZLIB
#include <zlib.h>
#endif // WITH_ZLIB
#endif // WITH_PNG

#define UNUSED(x) (void)x;

static int screenWidth = 1080;
static int screenHeight = 1080;

static glm::vec2 mousePos(0.f, 0.f);
static float sensitivity = 0.01f;

static glm::vec2 center(0.f, 0.f);
static float zoom = 1.f;

static GLboolean showColor;

static glm::mat4 view(1.f);

static const float scrollSpeed = 50;


void usage(int argc, char **argv);
GLuint loadShaders(const std::filesystem::path &vertexShaderFileName, const std::filesystem::path &fragmentShaderFileName);
GLuint loadShader(const GLuint shaderType, const std::filesystem::path &shaderFileName);
int render(const GLuint program);
int makeScreenShot();

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
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
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
  
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "Failed to init GLEW!");
        exit(3);
    }
#endif
  
#ifdef __APPLE__
  GLuint program = loadShaders(
                               std::filesystem::path(argv[0]).remove_filename().append("../Resources/tri.glsl").lexically_normal(),
                               std::filesystem::path(argv[0]).remove_filename().append("../Resources/julia.glsl").lexically_normal()
                               );
#else
  GLuint program = loadShaders(
                               std::filesystem::path(argv[0]).remove_filename().append("../shaders/tri.glsl").lexically_normal(),
                               std::filesystem::path(argv[0]).remove_filename().append("../shaders/julia.glsl").lexically_normal()
                               );
#endif


    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Running event loop");
    SDL_bool quit = SDL_FALSE;
    SDL_bool lmbDown = SDL_FALSE;
    SDL_bool ctrlDown = SDL_FALSE;
    SDL_bool altDown = SDL_FALSE;
    SDL_bool shiftDown = SDL_FALSE;
    SDL_Event event;
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_QUIT");
                quit = SDL_TRUE;
                break;
            case SDL_MOUSEMOTION:
                //SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_MOUSEMOTION: %+d, %+d (%d, %d)", event.motion.xrel, event.motion.yrel, event.motion.x, event.motion.y);
                if (lmbDown && zoom == 1.f)
                {
                    mousePos.x = (float(event.motion.x) - screenWidth / 2) / screenWidth * 2;
                    mousePos.y = (float(event.motion.y) * -1 + screenHeight / 2) / screenWidth * 2;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                //SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_MOUSEBUTTONDOWN: %d", event.button.button);
                if (event.button.button == 1 && zoom == 1.f)
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
                }
                break;
            case SDL_MOUSEWHEEL:
                //SDL_LogDebug(SDL_LOG_CATEGORY_TEST, "SDL_MOUSEWHEEL: %+d, %+d", event.wheel.x, event.wheel.y);
                if (ctrlDown)
                    zoom = zoom * pow(1.1, event.wheel.y);
                else if (altDown)
                    sensitivity = sensitivity + 0.001 * event.wheel.y;
                else if (shiftDown)
                    center += glm::vec2(float(event.wheel.y) / screenWidth / zoom * scrollSpeed * -1, 0);
                else
                    center += glm::vec2(float(event.wheel.x) / screenWidth / zoom * scrollSpeed * -1, float(event.wheel.y) / screenWidth / zoom * scrollSpeed);

                if (zoom < 1.f)
                    zoom = 1.f;
                
                if (sensitivity > 1.f)
                    sensitivity = 1.f;
                else if (sensitivity < 0.001)
                    sensitivity = 0.001;
                
                if (center.x < -1.f)
                    center.x = -1.f;
                else if (center.x > 1.f)
                    center.x = 1.f;
                
                if (center.y < -1.f)
                    center.y = -1.f;
                else if (center.y > 1.f)
                    center.y = 1.f;
                
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
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    SDL_Event quitEvent;
                    quitEvent.type = SDL_QUIT;
                    SDL_PushEvent(&quitEvent);
                    break;
                case SDLK_PRINTSCREEN:
                    makeScreenShot();
                    break;
                case SDLK_s:
                    if (SDL_GetModState() & (KMOD_CTRL | KMOD_LCTRL | KMOD_RCTRL))
                        makeScreenShot();
                    break;
                case SDLK_SPACE:
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Viewport reset");
                    zoom = 1.0f;
                    center = glm::vec2(0, 0);
                    break;
                case SDLK_c:
                    if (showColor = !showColor)
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Color mode");
                    else
                        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Grayscale mode");
                    break;
                case SDLK_p:
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Params:\n\tC = %f%+fi\n\tCenter: %f%+fi\n\tZoom: %f\n\tSensitivity: %f",
                                mousePos.x, mousePos.y,
                                center.x, center.y,
                                zoom,
                                sensitivity
                    );
                }
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
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
                }
                break;
            };
        }
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
   
/*
    glm::mat4 view = glm::lookAt(
        glm::vec3(center.x, center.y, 1.0f),    // положение камеры
        glm::vec3(center.x, center.y, 0.0f),    // положение "прицела"
        glm::vec3(0, 1, 0)                      // ориентация 0, 1, 0 - головой вверх
    );
*/
    glm::mat4 view = glm::ortho(center.x - 1.f, center.x + 1.f, center.y - 1.f, center.y + 1.f);
    
    GLuint viewId = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewId, 1, GL_FALSE, &view[0][0]);

    glm::mat4 scale = glm::diagonal4x4(glm::vec4(zoom, zoom, 1.f, 1.f));

    GLuint scaleId = glGetUniformLocation(program, "scale");
    glUniformMatrix4fv(scaleId, 1, GL_FALSE, &scale[0][0]);
    
    GLuint CId = glGetUniformLocation(program, "C");
    glUniform2fv(CId, 1, &mousePos[0]);

    GLuint sensitivityId = glGetUniformLocation(program, "sensitivity");
    glUniform1f(sensitivityId, sensitivity);

    GLuint showColorId = glGetUniformLocation(program, "showColor");
    glUniform1i(showColorId, showColor);
    
    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDeleteVertexArrays(1, &vertexArray);

    return 0;
}

int makeScreenShot()
{
#ifdef WITH_PNG
    std::filesystem::path path = "julia.png"; // FIXME Приделать генератор имен

    int bpp = 0;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &bpp);
    
    int channelDepth = 8;
    int pixelSize = bpp / channelDepth;
    
    FILE *pngFile = fopen(path.c_str(), "wb");
    if (!pngFile) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", strerror(errno));
        return -1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "png_create_write_struct() failed");
        fclose(pngFile);
        return -1;
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "png_create_info_struct() failed");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(pngFile);
        return -1;
    }

// Set error handler
    if (setjmp (png_jmpbuf (png_ptr))) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error writing PNG");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(pngFile);
        return -1;
    }
    
// Set image attributes
    png_set_IHDR (png_ptr,
                  info_ptr,
                  screenWidth,
                  screenHeight,
                  channelDepth,
                  pixelSize == 4 ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
// Set image time    
    png_timep imageTime = new png_time_struct;
    png_convert_from_time_t(imageTime, time(nullptr));
    png_set_tIME(png_ptr,
                 info_ptr,
                 imageTime);
    delete imageTime;
    
// Set image text
    png_text text_ptr[6];
    text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[0].key = "Created with";
    text_ptr[0].text = "julia";
    text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[1].key = "Tranform";
    text_ptr[1].text = "Z = Z^2 + C";
    text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[2].key = "C";
    text_ptr[2].text_length = snprintf(nullptr, 0, "%f%+fi", mousePos.x, mousePos.y) + 1;
    text_ptr[2].text = static_cast<char *>(malloc(text_ptr[2].text_length));
    snprintf(text_ptr[2].text, text_ptr[2].text_length, "%f%+fi", mousePos.x, mousePos.y);
    text_ptr[3].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[3].key = "Center";
    text_ptr[3].text_length = snprintf(nullptr, 0, "%f%+fi", center.x, center.y) + 1;
    text_ptr[3].text = static_cast<char *>(malloc(text_ptr[3].text_length));
    snprintf(text_ptr[3].text, text_ptr[3].text_length, "%f%+fi", center.x, center.y);
    text_ptr[4].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[4].key = "Zoom";
    text_ptr[4].text_length = snprintf(nullptr, 0, "%f", zoom) + 1;
    text_ptr[4].text = static_cast<char *>(malloc(text_ptr[4].text_length));
    snprintf(text_ptr[4].text, text_ptr[4].text_length, "%f", zoom);
    text_ptr[5].compression = PNG_TEXT_COMPRESSION_NONE;
    text_ptr[5].key = "Sensitivity";
    text_ptr[5].text_length = snprintf(nullptr, 0, "%f", sensitivity) + 1;
    text_ptr[5].text = static_cast<char *>(malloc(text_ptr[5].text_length));
    snprintf(text_ptr[5].text, text_ptr[5].text_length, "%f", sensitivity);
    png_set_text(png_ptr, info_ptr, text_ptr, 6);
    for (int i = 2; i < 6; ++i)
        free(text_ptr[i].text);

// Make screenshot
    GLubyte *texture = new GLubyte[screenWidth * screenHeight * pixelSize];
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, texture);

    
// Initialize row pointers
    GLubyte **row_pointers = static_cast<GLubyte **>(malloc(screenHeight * sizeof(GLubyte *)));
    for (size_t y = 0; y < screenHeight; ++y)
        row_pointers[y] = &texture[y * screenWidth * pixelSize];
    
// Save data to file
    png_init_io (png_ptr, pngFile);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    free(row_pointers);
    delete texture;   
    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(pngFile);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Screenshots saved to %s", path.c_str());
#else // WITH_PNG
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "PNG not supported. Screenshots unavailable.");
#endif // WITH_PNG
    return 0;
}
