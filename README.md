# Voice Controlled Color Picker
This is an attempt at my own challenge, to make worst possible, the most impractical color picker I can imagine.

## Features
- Will add soon

## Libraries
External libraries which are included in this project:
- **SDL2**: Handles input, windows and OpenGL context 
- **GLAD**: Loading OpenGL functions
- **GLM**: For various mathematical operations
- **imGUI**: For UI elements
- **Whisper**: For translating voice inputs into strings

## Compile on Linux with
```bash
g++ -std=c++17 ./src/* -o prog -I ./include/ -I ./thirdparty/glm-master/ -lSDL2 -ldl
```
