cmake_minimum_required(VERSION 3.0)

project(julia)

set(OpenGL_GL_PREFERENCE "GLVND")

if(WIN32)
    set(SDL2_LIBRARIES "SDL2.lib")
    set(GLEW_LIBRARIES "glew.lib")
    set(OPENGL_LIBRARIES "OpenGL32.lib")
    set(PNG_LIBRARIES "libpng16.lib")


    set(SDL2_LIBDIR "build/packages/sdl2.2.0.5/build/native/lib/$(PlatformName)/dynamic")
    set(GLEW_LIBDIR "build/packages/glew.1.9.0.1/build/native/lib/v110/$(PlatformName)/$(Configuration)/dynamic")
    set(PNG_LIBDIR  "build/packages/libpng.1.6.28.1/build/native/lib/$(PlatformName)/v140/dynamic/$(Configuration)")
    set(ZLIB_LIBDIR "build/packages/zlib.v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8/lib/native/v140/windesktop/msvcstl/dyn/rt-dyn/$(PlatformName)/$(Configuration)")


    set(SDL2_INCLUDE_DIRS "build/packages/sdl2.2.0.5/build/native/include")
    set(GLEW_INCLUDE_DIRS "build/packages/glew.1.9.0.1/build/native/include")
    set(GLM_INCLUDE_DIRS "build/packages/glm.0.9.9.800/build/native/include")
    set(PNG_INCLUDE_DIRS "build/packages/libpng.1.6.28.1/build/native/include")
    set(ZLIB_INCLUDE_DIRS "build/packages/zlib.v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8/build/native/include")

    set(PNG_FOUND TRUE)
    set(ZLIB_FOUND TRUE)
else()
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glm REQUIRED)
    find_package(PNG)
endif(WIN32)

if(${PNG_FOUND})
    add_definitions(${PNG_DEFINITIONS})
    add_compile_definitions("WITH_PNG")
    if(${ZLIB_FOUND})
        add_compile_definitions("WITH_ZLIB")
    endif(${ZLIB_FOUND})
endif(${PNG_FOUND})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_options(-Wno-write-strings)
endif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")

if (WIN32)
    add_executable(julia WIN32 main.cpp)
    set_property(TARGET julia PROPERTY VS_PACKAGE_REFERENCES "sdl2_2.0.5;glew_1.9.0.1;glm_0.9.9.800;libpng_1.6.28.1;zlib_v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8")
else()
    add_executable(julia main.cpp)
endif(WIN32)
target_compile_features(julia PUBLIC cxx_std_17)
target_include_directories(julia PUBLIC ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_directories(julia PUBLIC ${SDL2_LIBDIR} ${GLEW_LIBDIR} ${PNG_LIBDIR} ${ZLIB_LIBDIR})
target_link_libraries(julia PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})

install(TARGETS julia RUNTIME DESTINATION bin)
