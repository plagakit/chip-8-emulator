# chip-8-emulator

[Check out the emulator online here!](https://plagakit.github.io/chip8/chip-8-emulator.html)

### Building The Project

This project is built using CMake. Make sure CMake is installed on your computer (or that CMake can do `find_package(SDL2 REQUIRED)` without errors).

#### Desktop Application (Linux)

Clone the repo, then on your machine do:

```bash
mkdir bin
cmake ..
make
```

#### Emscripten

Wrangling Emscripten, CMake, and SDL2 was a terrible challenge, one that I barely managed to do and that I have no reproducible steps for. Good luck!
