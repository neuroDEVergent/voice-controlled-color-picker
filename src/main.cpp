// Third Party Libraries
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <vosk_api.h>

// Personal
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

// Screen dimensions
int screen_width = 1280;
int screen_height = 720;
SDL_Window* window = nullptr;
SDL_GLContext gl_context = nullptr;

// Main loop flag
bool quit = false; // If this is true then the program terminates

VoskModel* vosk_model; 
VoskRecognizer* vosk_recognizer;
static const char* last_command = nullptr;

void audioCallback(void* userdata, Uint8* stream, int len);
void Input();

void initializeProgram()
{
  // SDL Video
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("SDL could not initialize video subsystem\n");
    exit(1);
  }

  // OpenGL Context
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 2);

  // SDL Window 
  window = SDL_CreateWindow("Cursed Color Picker", 0, 0, screen_width, screen_height, SDL_WINDOW_OPENGL);
  if (window == nullptr)
  {
    printf("SDL Window was not able to be created\n");
    exit(1);
  }

  // OpenGL Context
  gl_context = SDL_GL_CreateContext(window);
  if (gl_context == nullptr)
  {
    printf("OpenGL context could not be created\n");
    exit(1);
  }

  // Glad
  if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
  {
    printf("glad was not initialized\n");
    exit(1);
  }
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Dear imGUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Vosk
  vosk_model = vosk_model_new("/home/p/Projects/voice-controlled-color-picker/thirdparty/vosk-model-en-us-0.22");
  if (!vosk_model)
  {
    printf("Failed to load Vosk Model\n");
    exit(1);
  }
  vosk_recognizer = vosk_recognizer_new(vosk_model, 16000.0f);
  if (!vosk_recognizer)
  {
    printf("Failed to load Vosk Recognizer\n");
    exit(1);
  }
  vosk_recognizer_set_grm(vosk_recognizer, "[\"red\",\"green\",\"blue\",\"yellow\",\"dark\",\"light\"]");

  // SDL Audio
  SDL_Init(SDL_INIT_AUDIO);
  SDL_AudioSpec desired;
  SDL_zero(desired);
  desired.freq = 16000;
  desired.format = AUDIO_S16SYS;
  desired.channels = 1;
  desired.samples = 1024;
  desired.callback = audioCallback;
  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, SDL_TRUE, &desired, NULL, 0);
  SDL_PauseAudioDevice(dev, 0);

  // Get the info
  printf("Program initialized successfully\n");
  printf("Vendor: %s\n", glGetString(GL_VENDOR));
  printf("Renderer: %s\n", glGetString(GL_RENDERER));
  printf("Version: %s\n", glGetString(GL_VERSION));
  printf("Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
}

void mainLoop()
{
  while (!quit)
  {
    SDL_Delay(100);

    Input();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0, 0, 0, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Begin("Hello");
    ImGui::Text("Hello from dear imgui");

    if (last_command)
    {
      ImGui::Text("Last command: %s", last_command);

      if (strstr(last_command, "red")) ImGui::Text("RED");
      if (strstr(last_command, "green")) ImGui::Text("GREEn");
      if (strstr(last_command, "blue")) ImGui::Text("BLUE");
    }

    static float value = 0.5f;
    ImGui::SliderFloat("Value", &value, 0.0f, 1.0f);

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
  }
 
}

void Input()
{
  SDL_Event e;
  while(SDL_PollEvent(&e) != 0)
  {
    ImGui_ImplSDL2_ProcessEvent(&e);
    if (e.type == SDL_QUIT)
    {
      printf("Goodbye!\n");
      quit = true;
    }
  }
}

void cleanUp()
{
  vosk_recognizer_free(vosk_recognizer);
  vosk_model_free(vosk_model);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int main( int argc, char* args[] )
{
  initializeProgram();

  mainLoop();

  cleanUp();

  return 0;
}

void audioCallback(void* userdata, Uint8* stream, int len)
{
  if (vosk_recognizer_accept_waveform(vosk_recognizer, (const char*)stream, len))
  {
    last_command= vosk_recognizer_result(vosk_recognizer);

    printf("FINAL: %s\n", last_command);
  }
}
