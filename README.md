# Voice Controlled Color Picker
This is an attempt at my own challenge, to make worst possible, the most impractical color picker I can imagine.

## Features
- Will add soon

## Libraries
External libraries which are included in this project:
- **SDL2**: Handles input, windows and OpenGL context 
- **GLAD**: Loading OpenGL functions
- **GLM**: For various mathematical operations
- **Dear imGUI**: For UI elements
- **Vosk**: For translating voice inputs into strings

## Compile on Linux with
```bash
g++ -std=c++17 ./src/* \
  ./thirdparty/imgui/*.cpp \
  ./thirdparty/imgui/backends/*.cpp \
  -o prog \
  -I./include/ \
  -I./thirdparty/glm-master/ \
  -I./thirdparty/imgui/ \
  -I./thirdparty/imgui/backends \
  -I/usr/include/SDL2 \
  -lSDL2 \
  -ldl \
  -lvosk
```
