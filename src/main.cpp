#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <vosk_api.h>

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

// SDL globals variables
int screen_width = 1280;
int screen_height = 720;
SDL_Window* window = nullptr;
SDL_GLContext gl_context = nullptr;
SDL_AudioDeviceID sdl_device;
SDL_AudioSpec sdl_audio_spec;
bool quit = false;

// Vosk global variables
VoskModel* vosk_model; 
VoskRecognizer* vosk_recognizer;
static const char* last_command = nullptr;

struct Signals
{
  bool nothing;
  bool red;
  bool green;
  bool blue;
  bool less;
  bool more;
} Signals;

struct Color 
{
  float r = 255.f;
  float g = 255.f;
  float b = 255.f;
  float val = 15.0;
} Color;

// Keyword dictionaries
const char* KEYWORDS = "[\"[unk]\", \"nothing\",\"red\", \"green\", \"blue\", \"more\", \"less\"]";


// Function declarations
void init();
void printSpec();
void voice_commands();
void input();
void audio_callback(void* userdata, Uint8* stream, int len);

void init()
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
  vosk_model = vosk_model_new("./thirdparty/vosk-model-en-us-0.22");
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
  vosk_recognizer_set_grm(vosk_recognizer, KEYWORDS);

  // SDL Audio
  SDL_Init(SDL_INIT_AUDIO);
  SDL_AudioSpec desired;
  SDL_zero(desired);
  desired.freq = 16000;
  desired.format = AUDIO_S16SYS;
  desired.channels = 1;
  desired.samples = 1024;
  desired.callback = audio_callback;
  sdl_device = SDL_OpenAudioDevice(NULL, SDL_TRUE, &desired, &sdl_audio_spec, 0);
  SDL_PauseAudioDevice(sdl_device, 0);

  // Print specification
  printSpec();
}

void printSpec()
{
  printf("Program initialized successfully\n\n");

  printf("---OpenGL---\n");
  printf("  Vendor: %s\n", glGetString(GL_VENDOR));
  printf("  Renderer: %s\n", glGetString(GL_RENDERER));
  printf("  Version: %s\n", glGetString(GL_VERSION));
  printf("  Shading Language: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
  printf("\n");

  printf("---Audio Device---\n");
  printf("  Frequency: %d\n", sdl_audio_spec.freq);
  printf("  Channels: %d\n", sdl_audio_spec.channels);
  printf("  Samples: %d\n", sdl_audio_spec.samples);
  printf("  Format: 0x%x\n", sdl_audio_spec.format);
  printf("\n");

  printf("---Dear ImGui---\n");
  printf("  Version: %s\n", IMGUI_VERSION);
  printf("\n");

  printf("---Vosk---\n");
  printf("  Model: %s\n", "vosk-model-en-us-0.22");
  printf("\n");
 
}

void voice_commands()
{
  if (last_command)
  {
//    ImGui::Text("Last command: %s", last_command);
    
    if (strstr(last_command, "nothing"))
    {
      Signals = {};
      Signals.nothing = true;
    }
    if (strstr(last_command, "red") || strstr(last_command, "read"))
    {
      Signals = {};
      Signals.red = true;
    }
    if (strstr(last_command, "green"))
    {
      Signals = {};
      Signals.green = true;
    }
    if (strstr(last_command, "blue"))
    {
      Signals = {};
      Signals.blue = true;
    }
    if (strstr(last_command, "less"))
    {
      Signals.less = true;
    }
    if (strstr(last_command, "more"))
    {
      Signals.more = true;
    }

    last_command = NULL;
  }

  else
  {
    Signals.less = false;
    Signals.more = false;
  }
}

void rgb_tab()
{
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();

  ImVec2 pos(85, 85);
  ImVec2 size(300, 300);

  ImU32 color = IM_COL32((int)Color.r, (int)Color.g, (int)Color.b, 255);

  draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), color);

  ImVec2 bar_pos(435, 85);
  ImVec2 bar_size(30, 300);
  int spacing = 50;

  // Red bar
  draw_list->AddRectFilled(
    ImVec2(bar_pos.x, bar_pos.y + bar_size.y - (Color.r / 255.0f) * bar_size.y),
    ImVec2(bar_pos.x + bar_size.x, bar_pos.y + bar_size.y),
    IM_COL32(255, 0, 0, 255)
  );

  // Green bar
  draw_list->AddRectFilled(
    ImVec2(bar_pos.x + spacing, bar_pos.y + bar_size.y - (Color.g / 255.0f) * bar_size.y),
    ImVec2(bar_pos.x + bar_size.x + spacing, bar_pos.y + bar_size.y),
    IM_COL32(0, 255, 0, 255)
  );

  // Blue bar
  draw_list->AddRectFilled(
    ImVec2(bar_pos.x + 2.0 * spacing, bar_pos.y + bar_size.y - (Color.b / 255.0f) * bar_size.y),
    ImVec2(bar_pos.x + bar_size.x + 2.0 * spacing, bar_pos.y + bar_size.y),
    IM_COL32(0, 0, 255, 255)
  );

  // Borders
  draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32_WHITE);

  if (Signals.red) 
  {
    draw_list->AddRect(bar_pos, ImVec2(bar_pos.x + bar_size.x, bar_pos.y + bar_size.y), IM_COL32_WHITE);

    if (Signals.less) Color.r -= Color.val;
    if (Signals.more) Color.r += Color.val;

  }
  if (Signals.green) 
  {
    draw_list->AddRect(ImVec2(bar_pos.x + spacing, bar_pos.y), ImVec2(bar_pos.x + bar_size.x + spacing, bar_pos.y + bar_size.y), IM_COL32_WHITE);
    if (Signals.less) Color.g -= Color.val;
    if (Signals.more) Color.g += Color.val;
  }
  if (Signals.blue)
  {
    draw_list->AddRect(ImVec2(bar_pos.x + 2 * spacing, bar_pos.y), ImVec2(bar_pos.x + bar_size.x + 2* spacing, bar_pos.y + bar_size.y), IM_COL32_WHITE);

    if (Signals.less) Color.b -= Color.val;
    if (Signals.more) Color.b += Color.val;
  }
}

void hsl_tab()
{
  ImGui::Text("TODO\nNo time to create :(\nMight add in the future.\n");
}

void hex_tab()
{
  ImGui::Text("TODO\nNo time to create :(\nMight add in the future.\n");
}

void ui()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();

  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

  ImGuiWindowFlags flags = 
    ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoResize |
    ImGuiWindowFlags_NoTitleBar;

  ImGui::Begin("Main", nullptr, flags);

  static unsigned int tab = 0;
  if (ImGui::Button("RGB")) tab = 0;
  ImGui::SameLine();
  if (ImGui::Button("HSL")) tab = 1;
  ImGui::SameLine();
  if (ImGui::Button("HEX")) tab = 2;
  ImGui::Separator();

  if      (tab == 0) rgb_tab();
  else if (tab == 1) hsl_tab();
  else if (tab == 2) hex_tab();

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void mainLoop()
{
  while (!quit)
  {
    SDL_Delay(1);

    input();

    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0, 0, 0, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    voice_commands();

    ui();

    SDL_GL_SwapWindow(window);
  }
 
}

void input()
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
  SDL_PauseAudioDevice(sdl_device, 1);
  SDL_CloseAudioDevice(sdl_device);
  vosk_recognizer_free(vosk_recognizer);
  vosk_model_free(vosk_model);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int main( int argc, char* args[] )
{
  init();

  mainLoop();

  cleanUp();

  return 0;
}

void audio_callback(void* userdata, Uint8* stream, int len)
{
  if (vosk_recognizer_accept_waveform(vosk_recognizer, (const char*)stream, len))
  {
    last_command = vosk_recognizer_result(vosk_recognizer);
  }
}
