/*
  Compilation on Linux
  g++ -std=c++17 ./src/* -o prog -I ./include/ -I ./thirdparty/glm-master/ -lSDL2 -ldl
*/

// Third Party Libraries
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.h>

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// Personal libraries
#include <Shader.hpp>

// #################### vvv Globals vvv ####################
// Globals are prefixed with 'g'

// Screen dimensions
int gScreenWidth = 1280;
int gScreenHeight = 720;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

// Main loop flag
bool gQuit = false; // If this is true then the program terminates

// shader
// The following stores the unique id for the graphics pipeline
// program object that will be used for our OpenGL draw calls.
GLuint gGraphicsPipelineShaderProgram = 0;


// OpenGL Objects
// Vertex Array Object (VAO)
// Vertex array objects encapsulate all of the items needed to render an object
// For example, we may have multiple vertex buffer objects (VBO) related to rendering one object.
// The VAO allows us to setup the OpenGL state to render that object using the correct
// layout and correct buffers with one call after being setup
GLuint VAO = 0;
//Vertex Buffer Object (VBO)
// Vertex Buffer Objects store information relating to vertices (e.g. positions, normals, textures)
// VBOs are our mechanisim for arranging geometry on the GPU.
GLuint VBO = 0;
// Index Buffer Object (IBO)
// This is used to store the array of indices that we want
// to draw from, when we do indexed drawing.
GLuint EBO = 0;

// #################### ^^^ Globals ^^^ ####################



// #################### vvv Error handling routines vvv ####################
static void GLClearAllErrors()
{
  while(glGetError() != GL_NO_ERROR)
  {
    
  }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line)
{
  while (GLenum error = glGetError())
  {
    std::cout << "OpenGL Error:" << error
	      << "\tLine: " << line
	      << "\tfunction: " << function << std::endl;
    return true;
  }

  return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);
// #################### ^^^ Error handling routines ^^^ ####################

/*
  Helper function to get OpenGL Version Information
*/
void GetOpenGLVersionInfo()
{
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

/*
  Create the graphics pipeline

  @return void
*/
void CreateGraphicsPipeline()
{
	Shader ourShader("./shaders/vert.glsl","./shaders/frag.glsl");
	gGraphicsPipelineShaderProgram = ourShader.ID;
}

/*
  @return void
*/
void VertexSpecification()
{
  // Geometry Data
  const std::vector<GLfloat> vertices
  {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f, // top left
  };

  const std::vector<GLint> indicies
  {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
  };



  // Generate VAO, VBO and EBO
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, 
               vertices.size() * sizeof(GLfloat), 
               vertices.data(), 
               GL_STATIC_DRAW
               );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indicies.size() * sizeof(GLint),
               indicies.data(),
               GL_STATIC_DRAW
               );

  // Position attribute
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(GLfloat),
                        (GLvoid*)0
                       );
  
  // Color attribute
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(GLfloat),
                        (GLvoid*)(sizeof(GLfloat)*3)
                        );

  // texture coord attribute
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2,
                        2,
                        GL_FLOAT,
                        GL_FALSE,
                        8 * sizeof(GLfloat),
                        (GLvoid*)(sizeof(GLfloat)*6));

  // Disable any attributes we opened in our Vertex Attribute Array,
  // as we do not want to leave them open
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

/*
  Initialization of the graphics application. Typically this will involve setting up a window
  and the OpenGL Context (with the appropriate version)
  
  @return void
*/
void InitializeProgram()
{
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cout << "SDL2 could not initialize video subsystem" << std::endl;
    exit(1);
  }
  // Setup the OpenGL Context
  // Use OpenGL 4.1 core or greater
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  
  // We want to request a double buffer for smooth updating
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 2);

  // Create an application window using OpenGL that supports SDL
  gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL Window", 0, 0, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);

  // Check if Window did not create
  if (gGraphicsApplicationWindow == nullptr)
  {
    std::cout << "SDL Window was not able to be created" << std::endl;
    exit(1);
  }

  // Create an OpenGL Graphics Context
  gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);

  if (gOpenGLContext == nullptr)
  {
    std::cout << "OpenGL context could not be created" << std::endl;
    exit(1);
  }

  // Initialize the Glad Library
  if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
  {
    std::cout << "glad was not initialized" << std::endl;
    exit(1);
  }

  GetOpenGLVersionInfo();
}

/*
  // Function called in the main application loop to handle user input
  
  @return void
*/
void Input()
{
  // Event handler that handles various events in SDL
  // that are related to input and output
  SDL_Event e;
  // Handle events on queue
  while(SDL_PollEvent(&e) != 0)
  {
    // If user posts an event to quit
    // An example is hitting the "x" in the corner of the window
    if (e.type == SDL_QUIT)
    {
      std::cout << "Goodbye!" << std::endl;
      gQuit = true;
    }
  }
}

/*
  PreDraw
  Typically we will use this for setting some sort of 'state'
  Note: some of the calls may take place at different stages (post-processing) of the pipeline
  @return void
*/
void PreDraw()
{
  // Disable depth test and face culling.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Initialize clear color
  // This is the background of the screen
  glViewport(0, 0, gScreenWidth, gScreenHeight);
  glClearColor(0, 0, 0, 1.f);

  // Clear color buffer and depth buffer
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
 
  // Use our shader
  glUseProgram(gGraphicsPipelineShaderProgram);
}

/*
  Draw
  The render function gets called once per loop
  Typically this includes glDraw related calls, and the relevant setup of buffers for those calls

  @return void
*/
void Draw()
{
  // Enable our attributes
  glBindVertexArray(VAO);

  // Render data
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  // Stop using our current graphics pipeline
  // Note: This is not necessary if we only have one graphics pipeline.
  glUseProgram(0);
  
}

/*
  Main Application Loop
  This is an infinite loop
  
  @return void
*/
void MainLoop()
{
  // While application is running
  while (!gQuit)
  {
    // Handle input 
    Input();
    // Setup anything that needs to take place before draw calls 

    PreDraw();
    // Draw calls in OpenGL

    Draw();
    // Update screen of our specified window
    SDL_GL_SwapWindow(gGraphicsApplicationWindow);
  }
}

void CleanUp()
{
  SDL_DestroyWindow(gGraphicsApplicationWindow);
  SDL_Quit();
}

/*
  The entry point into a program
  @return program status
*/
int main( int argc, char* args[] )
{

  // 1. Setup the graphics program
  InitializeProgram();

  // 2. Setup the geometry
  VertexSpecification();

  // 3. Create our graphics pipeline
  CreateGraphicsPipeline();

  // 4. Call the main application loop
  MainLoop();

  // 5. Call the cleanup funcion when our program terminates
  CleanUp();

  return 0;
  
}
