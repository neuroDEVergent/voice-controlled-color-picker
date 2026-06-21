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


static char instructions[4096] = 
  "step1\n"
  "step2\n"
  "step3\n";

// Vosk global variables
VoskModel* vosk_model; 
VoskRecognizer* vosk_recognizer;
static const char* last_command = nullptr;
float command_volume = 0.0f;
bool selected = false;

unsigned int spacing = 30;
typedef struct Bar
{
  ImVec2 pos;
  ImVec2 size;
  float value;
  float target_value;
} Bar;

Bar red_bar;
Bar blue_bar;
Bar green_bar;
Bar* selected_bar;


struct Color 
{
  float r;
  float g;
  float b;
  float val = 15.0;
} Color;

// Keyword dictionaries
const char* KEYWORDS = "[\"[unk]\", \"nothing\",\"red\", \"green\", \"blue\", \"more\", \"less\"]";


// Function declarations
void init();
void printSpec();
void voice_commands();
void rgb_tab();
void hsl_tab();
void hex_tab();
void ui();
void mainLoop();
void input();
void cleanUp();
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

  // Bars
  red_bar = {};
  red_bar.pos = ImVec2(435, 80);
  red_bar.size = ImVec2(30, 300);
  red_bar.value = 255.0f;
  red_bar.target_value = red_bar.value;

  blue_bar = {};
  blue_bar.pos = ImVec2(red_bar.pos.x + red_bar.size.x + spacing, red_bar.pos.y);
  blue_bar.size = ImVec2(30, 300);
  blue_bar.value = 150.0f;
  blue_bar.target_value = blue_bar.value;

  green_bar = {};
  green_bar.pos = ImVec2(blue_bar.pos.x + blue_bar.size.x + spacing, blue_bar.pos.y);
  green_bar.size = ImVec2(30, 300);
  green_bar.value = 50.0f;
  green_bar.target_value = green_bar.value;

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
    if (strstr(last_command, "nothing"))
    {
      selected = false;
    }
    if (strstr(last_command, "red") || strstr(last_command, "read"))
    {
      selected = true;
      selected_bar = &red_bar;
    }
    if (strstr(last_command, "green"))
    {
      selected = true;
      selected_bar = &green_bar;
    }
    if (strstr(last_command, "blue"))
    {
      selected = true;
      selected_bar = &blue_bar;
    }
    if (strstr(last_command, "less"))
    {
      selected_bar->target_value = selected_bar->value - command_volume * 100.0f;
      if (selected_bar->target_value <= 0.0) selected_bar->target_value = 0.0f;
    }
    if (strstr(last_command, "more"))
    {
      selected_bar->target_value = selected_bar->value + command_volume * 100.0f;
      if (selected_bar->target_value >= 255.0) selected_bar->target_value = 255.0;
    }

    last_command = NULL;
  }
}

void rgb_tab()
{
  ImDrawList* draw_list = ImGui::GetForegroundDrawList();

  ImVec2 pos(85, 85);
  ImVec2 size(300, 300);

  ImU32 color = IM_COL32((int)red_bar.value, (int)green_bar.value, (int)blue_bar.value, 255);

  draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), color);

  int spacing = 50;
  ImVec2 bar_pos(435, 85);
  ImVec2 bar_size(30, 300);

  // Red bar
  draw_list->AddRectFilled(
    ImVec2(red_bar.pos.x, red_bar.pos.y + red_bar.size.y - (red_bar.value / 255.0f) * red_bar.size.y),
    ImVec2(red_bar.pos.x + red_bar.size.x, red_bar.pos.y + red_bar.size.y),
    IM_COL32(255, 0, 0, 255)
  );

  // Green bar
  draw_list->AddRectFilled(
    ImVec2(green_bar.pos.x, green_bar.pos.y + green_bar.size.y - (green_bar.value / 255.0f) * green_bar.size.y),
    ImVec2(green_bar.pos.x + green_bar.size.x, green_bar.pos.y + green_bar.size.y),
    IM_COL32(0, 255, 0, 255)
  );

  // Blue bar
  draw_list->AddRectFilled(
    ImVec2(blue_bar.pos.x, blue_bar.pos.y + blue_bar.size.y - (blue_bar.value / 255.0f) * blue_bar.size.y),
    ImVec2(blue_bar.pos.x + blue_bar.size.x, blue_bar.pos.y + blue_bar.size.y),
    IM_COL32(0, 0, 255, 255)
  );

  // Borders
  draw_list->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32_WHITE);

  if (selected)
  {
    draw_list->AddRect(
      selected_bar->pos, 
      ImVec2(selected_bar->pos.x + selected_bar->size.x, selected_bar->pos.y + selected_bar->size.y), 
      IM_COL32_WHITE
    );

    float speed = 3.0;
    selected_bar->value += (selected_bar->target_value - selected_bar->value) * speed * ImGui::GetIO().DeltaTime;
  }

  // Instruction text box
  ImGui::SetWindowFontScale(1.3f);
  ImGui::SetCursorPos(ImVec2(650, 85)); // hardcoded x, y within the window
  ImGui::Text("---Instructions---");

  ImGui::SetCursorPos(ImVec2(650, 100));
  ImGui::Text("- Say: \'red\', \'green\' or \'blue\' to select the color channel");

  ImGui::SetCursorPos(ImVec2(650, 115));
  ImGui::Text("- Say: \'nothing\' to deselect the color channel");

  ImGui::SetCursorPos(ImVec2(650, 130));
  ImGui::Text("- Say: \'more\' to increase it's value");

  ImGui::SetCursorPos(ImVec2(650, 145));
  ImGui::Text("- Say: \'less\' to decrease it's value");

  ImGui::SetCursorPos(ImVec2(650, 160));
  ImGui::Text("- Yelling will significantly increase the amount of color!");
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

static float utterance_peak = 0.0f;

void audio_callback(void* userdata, Uint8* stream, int len)
{
  const int16_t* samples = (const int16_t*)stream;
  int sample_count = len / sizeof(int16_t);

  float sum_sq = 0.0f;
  for (int i = 0; i < sample_count; i++) {
    float s = samples[i] / 32768.0f;
    sum_sq += s * s;
  }

  float rms = sqrtf(sum_sq / sample_count);

  float gain = 1.0f;
  float exponent = 0.5f;
  float boosted = powf(rms * gain, exponent);
  boosted = fminf(boosted, 1.0f);

  if (boosted > utterance_peak)
    utterance_peak = boosted;

  if (vosk_recognizer_accept_waveform(vosk_recognizer, (const char*)stream, len))
  {
    last_command = vosk_recognizer_result(vosk_recognizer);
    command_volume = utterance_peak;   // commit the peak for this command
    utterance_peak = 0.0f;             // reset for the next utterance
  }
}
