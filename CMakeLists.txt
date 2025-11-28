cmake_minimum_required(VERSION 3.10)
project(ShiftingMaze VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLUT
find_package(GLUT REQUIRED)

# Add executable
add_executable(ShiftingMaze
    src/main.cpp
)

# Include directories
target_include_directories(ShiftingMaze PRIVATE
    ${OPENGL_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIRS}
    src
)

# Link libraries
target_link_libraries(ShiftingMaze
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARY}
)

# Windows specific
if(WIN32)
    target_link_libraries(ShiftingMaze
        opengl32
        glu32
        freeglut
    )
endif()

# Set output directory
set_target_properties(ShiftingMaze PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
