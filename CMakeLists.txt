# Command: cmake -Dsdlpath={path to sdl}

cmake_minimum_required (VERSION 3.8)

project(chip-8-emulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure SDL (depends on Emscripten)
if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(USE_FLAGS "-s USE_SDL=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
else()
    #list(APPEND CMAKE_PREFIX_PATH C:\\vclib\\SDL2-2.26.2\\cmake)
    find_package(SDL2 REQUIRED)
endif()

# Compile ImGui as static library 
set(IMGUI_PATH "lib/imgui")
file(GLOB IMGUI_SOURCES "${IMGUI_PATH}/*.cpp") 
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_include_directories("ImGui" PUBLIC ${IMGUI_PATH})

# Add fmt
add_subdirectory(lib/fmt-10.1.0)

# Compile emulator
include_directories(
    src
    ${SDL2_INCLUDE_DIRS}
    "lib/ImGuiFileDialog"
    "lib/fmt-10.1.0/include"
)
add_executable(chip-8-emulator
    "src/main.cpp"
    "src/window.cpp"
    "src/chip8.cpp"
    "lib/ImGuiFileDialog/ImGuiFileDialog.cpp"
)
target_link_libraries(chip-8-emulator 
    PUBLIC 
        ImGui
        ${SDL2_LIBRARIES}
        
)

# Copy resources into build
set(RESOURCES_DIR "res")
file(COPY ${RESOURCES_DIR} DESTINATION ${CMAKE_BINARY_DIR})