# Voice Controlled Color Picker
This is an attempt at my own challenge, to make worst possible, the most impractical color picker I can imagine.

![Demo)(./demo.gif)

## Features
- Only RGB mode for now
- Supports voice commands for selecting the color channel
- Supports voice commands for volume based color value increments

## Libraries
External libraries which are included in this project:
- **SDL2**: Handles input, windows and OpenGL context 
- **GLAD**: Loading OpenGL functions
- **Dear imGUI**: For UI elements
- **Vosk**: For voice recognition

## Compile on Linux with
```bash
g++ -std=c++17 ./src/* \
  ./thirdparty/imgui/*.cpp \
  ./thirdparty/imgui/backends/*.cpp \
  -o prog \
  -I./include/ \
  -I./thirdparty/imgui/ \
  -I./thirdparty/imgui/backends \
  -I/usr/include/SDL2 \
  -lSDL2 \
  -ldl \
  -lvosk
```
## Vosk
it's required to have another directory called "thirdparty", with vosk-model-en-us-0.22 downloaded and extracted from:  https://alphacephei.com/vosk/models 
