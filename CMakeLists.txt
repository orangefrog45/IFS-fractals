cmake_minimum_required(VERSION 3.8)

project(GAME_LAYER)

add_executable(GAME_LAYER
src/main.cpp
src/GameLayer.cpp
 "headers/GameLayer.h")

target_include_directories(GAME_LAYER PUBLIC
"headers"
"../ORNG-Core/headers"
"../ORNG-Core/extern/spdlog/include"
"../ORNG-Core/extern/assimp/include"
"../ORNG-Core/extern/assimp/build/include"
"../ORNG-Core/extern/glfw/include"
"../ORNG-Core/extern/glew-cmake/include"
"../ORNG-Core/extern/physx/physx/include"
"../ORNG-Core/extern"
"../ORNG-Core/extern/imgui"
"../ORNG-Core/headers"
"../ORNG-Core/extern/fastnoise2/include"
"../ORNG-Core/extern/yaml/include"
"../ORNG-Core/extern/plog/include"
"../ORNG-Core/extern/bitsery/include"
"../ORNG-Core/extern/fmod/api/core/inc"
)

target_link_libraries(GAME_LAYER PUBLIC 
ORNG_CORE
imgui
)

target_precompile_headers(GAME_LAYER REUSE_FROM ORNG_CORE)
target_compile_definitions(GAME_LAYER PUBLIC GLM_ENABLE_EXPERIMENTAL BASE_DIR="${CMAKE_CURRENT_LIST_DIR}")

#Get res folder with shaders etc
file(COPY ${CMAKE_CURRENT_LIST_DIR}/res DESTINATION "${CMAKE_BINARY_DIR}/IFS-fractals/")
file(COPY ${CMAKE_CURRENT_BINARY_DIR}/../ORNG-Core/res DESTINATION "${CMAKE_BINARY_DIR}/IFS-fractals/")

add_custom_command(TARGET GAME_LAYER POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:GAME_LAYER>)
foreach(core_binary IN LISTS ORNG_CORE_BINARIES)
    add_custom_command(TARGET GAME_LAYER POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${core_binary}
        $<TARGET_FILE_DIR:GAME_LAYER>)
endforeach()