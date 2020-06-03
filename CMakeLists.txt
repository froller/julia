cmake_minimum_required(VERSION 3.0)

project(julia)

set(OpenGL_GL_PREFERENCE "GLVND")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

#message(STATUS ${glm_FOUND})
#message(STATUS ${glm_DIR})
#message(STATUS ${GLEW_LIBDIR})

add_executable(julia main.cpp)
target_compile_features(julia PUBLIC cxx_std_17)
target_include_directories(julia PUBLIC ${SDL2_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS})
target_link_directories(julia PUBLIC ${SDL2_LIBDIR})
target_link_libraries(julia PUBLIC ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES})

install(TARGETS julia RUNTIME DESTINATION bin)
