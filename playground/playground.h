#ifndef PLAYGROUND_H
#define PLAYGROUND_H

// Include GLEW
#include <GL/glew.h>

//some global variables for handling the vertex buffer
GLuint vertexbuffer;
GLuint VertexArrayID;
GLuint vertexbuffer_size;



//some global variables for handling the color buffer
GLuint colorbuffer;
GLuint uvbuffer;
GLuint normalbuffer;

//program ID of the shaders, required for handling the shaders with OpenGL
GLuint programID;


//Texture Variables
GLuint TextureID;
GLuint Texture;

//Different UV maps
GLuint uvmap1;
GLuint uvmap2;



int main( void ); //<<< main function, called at startup
void updateAnimationLoop(); //<<< updates the animation loop
bool initializeWindow(); //<<< initializes the window using GLFW and GLEW
bool initializeVertexbuffer(); //<<< initializes the vertex buffer array and binds it OpenGL
bool initializeColorbuffer();
bool initializeUVbuffer();
bool cleanupVertexbuffer(); //<<< frees all recources from the vertex buffer
bool closeWindow(); //<<< Closes the OpenGL window and terminates GLFW
void setCurrentTexture();

#endif
