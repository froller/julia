cmake_minimum_required(VERSION 3.30)

project(julia)

set(OpenGL_GL_PREFERENCE "GLVND")

if(WIN32)
    set(SDL2_LIBRARIES "SDL2.lib")
    set(GLEW_LIBRARIES "glew32.lib")
    set(OPENGL_LIBRARIES "OpenGL32.lib")
    set(PNG_LIBRARIES "libpng.lib")
	set(ZLIB_LIBRARIES "zlib.lib")


    set(SDL2_LIBDIR "build/packages/sdl2.2.0.5/build/native/lib/$(PlatformName)/dynamic")
    set(GLEW_LIBDIR "build/packages/GLEW-dynamic.2.2.0/build/native/lib/$(PlatformName)/$(Configuration)")
    set(PNG_LIBDIR  "build/packages/libpng_static.1.6.39.2/build/native/lib/$(PlatformName)/v143/$(Configuration)/MultiThreadedDLL")
    set(ZLIB_LIBDIR "build/packages/zlib_static.1.2.11.9/build/native/lib/$(PlatformName)/v142/$(Configuration)/MultiThreadedDLL")

    set(SDL2_INCLUDE_DIRS "build/packages/sdl2.2.0.5/build/native/include")
    set(GLEW_INCLUDE_DIRS "build/packages/GLEW-dynamic.2.2.0/build/native/include")
    set(PNG_INCLUDE_DIRS  "build/packages/libpng_static.1.6.39.2/build/native/include")
    set(ZLIB_INCLUDE_DIRS "build/packages/zlib_static.1.2.11.9/build/native/include")
    set(GLM_INCLUDE_DIRS  "build/packages/glm.1.0.1/build/native/include")

    set(PNG_FOUND TRUE)
    set(ZLIB_FOUND TRUE)
else()
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glm REQUIRED)
    find_package(PNG)
endif()

if(${PNG_FOUND})
    add_definitions(${PNG_DEFINITIONS})
    add_compile_definitions("WITH_PNG")
    if(${ZLIB_FOUND})
        add_compile_definitions("WITH_ZLIB")
    endif()
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    add_compile_options(-Wno-write-strings)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(-g -ggdb)
endif()

if (WIN32)
    add_executable(julia WIN32 main.cpp)
    set_property(TARGET julia PROPERTY VS_PACKAGE_REFERENCES "sdl2.2.0.5;GLEW-dynamic_2.2.0;libpng_static.1.6.39.2;zlib_static.1.2.11.9;glm.1.0.1")
else()
    add_executable(julia main.cpp)
endif()
target_compile_features(julia PUBLIC cxx_std_17)
target_include_directories(julia PUBLIC ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_directories(julia PUBLIC ${SDL2_LIBDIR} ${GLEW_LIBDIR} ${PNG_LIBDIR} ${ZLIB_LIBDIR})
target_link_libraries(julia PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})

install(TARGETS julia RUNTIME DESTINATION bin)
