#include "playground.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/texture.hpp>
#include <common/objloader.hpp>

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint LightID;
int currentTextureID;

glm::mat4 mvp;
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;


int main(void)
{
    //Initialize window
    bool windowInitialized = initializeWindow();
    if (!windowInitialized) return -1;



    //Initialize vertex buffer
    bool vertexbufferInitialized = initializeVertexbuffer();
    if (!vertexbufferInitialized) return -1;

	////Initialize uv buffer
 //   bool uvInitialized = initializeUVbuffer();
 //   if (!uvInitialized) return -1;

    ////Initialize color buffer
    //bool colorbufferInitialized = initializeColorbuffer();
    //if (!colorbufferInitialized) return -1;

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");


    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    currentTextureID = 1;

    //start animation loop until escape key is pressed
    do {



        updateAnimationLoop();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);


    //Cleanup and close window
    cleanupVertexbuffer();
    glDeleteProgram(programID);
    closeWindow();

    return 0;
}

void updateAnimationLoop()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);


    setCurrentTexture();
    computeMatricesFromInputs();
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();
    glm::mat4 ModelMatrix = glm::mat4(1.0f);
    mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;


    // Send our transformation to the currently bound shader, in the "MVP" uniform
  // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

    glm::vec3 lightPos = glm::vec3(4, 4, 4);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);





    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    glUniform1i(TextureID, 0);


    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        2,  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );


    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        2,                                // attribute
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );




    //// 2nd attribute buffer : colors
    //glEnableVertexAttribArray(1);
    //glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    //glVertexAttribPointer(
    //    1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
    //    3,                                // size
    //    GL_FLOAT,                         // type
    //    GL_FALSE,                         // normalized?
    //    0,                                // stride
    //    (void*)0                          // array buffer offset
    //);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, vertices.size()); // 3 indices starting at 0 -> 1 triangle

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

bool initializeWindow()
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return false;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Tutorial 04 - Red triangle", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return false;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    return true;
}

bool initializeUVbuffer() {
    //// Load the texture using any two methods
    ////GLuint Texture = loadBMP_custom("uvtemplate.bmp");
	//Texture = loadDDS("uvmap.DDS");

    // Get a handle for our "myTextureSampler" uniform
    TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals; // Won't be used at the moment.
    bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

    

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    return true;
}


//bool initializeVertexbuffer()
//{
//    glGenVertexArrays(1, &VertexArrayID);
//    glBindVertexArray(VertexArrayID);
//
//
//    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
//    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
//
//    // Or, for an ortho camera :
//    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
//
//    // Camera matrix
//    glm::mat4 View= glm::lookAt(
//        glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
//        glm::vec3(0, 0, 0), // and looks at the origin
//        glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
//    );
//
//
//
//    // Model matrix : an identity matrix (model will be at the origin)
//    glm::mat4 Model = glm::mat4(1.0f);
//    // Our ModelViewProjection : multiplication of our 3 matrices
//    mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
//
//
//  // Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
//  // A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
//    static const GLfloat g_vertex_buffer_data[] = {
//        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
//        -1.0f,-1.0f, 1.0f,
//        -1.0f, 1.0f, 1.0f, // triangle 1 : end
//        1.0f, 1.0f,-1.0f, // triangle 2 : begin
//        -1.0f,-1.0f,-1.0f,
//        -1.0f, 1.0f,-1.0f, // triangle 2 : end
//        1.0f,-1.0f, 1.0f,
//        -1.0f,-1.0f,-1.0f,
//        1.0f,-1.0f,-1.0f,
//        1.0f, 1.0f,-1.0f,
//        1.0f,-1.0f,-1.0f,
//        -1.0f,-1.0f,-1.0f,
//        -1.0f,-1.0f,-1.0f,
//        -1.0f, 1.0f, 1.0f,
//        -1.0f, 1.0f,-1.0f,
//        1.0f,-1.0f, 1.0f,
//        -1.0f,-1.0f, 1.0f,
//        -1.0f,-1.0f,-1.0f,
//        -1.0f, 1.0f, 1.0f,
//        -1.0f,-1.0f, 1.0f,
//        1.0f,-1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f,-1.0f,-1.0f,
//        1.0f, 1.0f,-1.0f,
//        1.0f,-1.0f,-1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f,-1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f,-1.0f,
//        -1.0f, 1.0f,-1.0f,
//        1.0f, 1.0f, 1.0f,
//        -1.0f, 1.0f,-1.0f,
//        -1.0f, 1.0f, 1.0f,
//        1.0f, 1.0f, 1.0f,
//        -1.0f, 1.0f, 1.0f,
//        1.0f,-1.0f, 1.0f
//    };
//
//
//
//
//    glGenBuffers(1, &vertexbuffer);
//    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
//
//    vertexbuffer_size = sizeof(g_vertex_buffer_data);
//
//    return true;
//}

bool initializeVertexbuffer()
{
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    // Load the texture using any two methods
    //GLuint Texture = loadBMP_custom("uvtemplate.bmp");
    uvmap1 = loadDDS("uvmap.DDS");
    uvmap2 = loadDDS("uvmap2.DDS");
    
    // Get a handle for our "myTextureSampler" uniform
    TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file .
    bool res = loadOBJ("cube.obj", vertices, uvs, normals);



    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);


    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);


    return true;
}



bool initializeColorbuffer()
{
    static const GLfloat g_color_buffer_data[] = {
    0.583f,  0.771f,  0.014f,
    0.609f,  0.115f,  0.436f,
    0.327f,  0.483f,  0.844f,
    0.822f,  0.569f,  0.201f,
    0.435f,  0.602f,  0.223f,
    0.310f,  0.747f,  0.185f,
    0.597f,  0.770f,  0.761f,
    0.559f,  0.436f,  0.730f,
    0.359f,  0.583f,  0.152f,
    0.483f,  0.596f,  0.789f,
    0.559f,  0.861f,  0.639f,
    0.195f,  0.548f,  0.859f,
    0.014f,  0.184f,  0.576f,
    0.771f,  0.328f,  0.970f,
    0.406f,  0.615f,  0.116f,
    0.676f,  0.977f,  0.133f,
    0.971f,  0.572f,  0.833f,
    0.140f,  0.616f,  0.489f,
    0.997f,  0.513f,  0.064f,
    0.945f,  0.719f,  0.592f,
    0.543f,  0.021f,  0.978f,
    0.279f,  0.317f,  0.505f,
    0.167f,  0.620f,  0.077f,
    0.347f,  0.857f,  0.137f,
    0.055f,  0.953f,  0.042f,
    0.714f,  0.505f,  0.345f,
    0.783f,  0.290f,  0.734f,
    0.722f,  0.645f,  0.174f,
    0.302f,  0.455f,  0.848f,
    0.225f,  0.587f,  0.040f,
    0.517f,  0.713f,  0.338f,
    0.053f,  0.959f,  0.120f,
    0.393f,  0.621f,  0.362f,
    0.673f,  0.211f,  0.457f,
    0.820f,  0.883f,  0.371f,
    0.982f,  0.099f,  0.879f
    };


    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    return true;
}


void setCurrentTexture() {
    int tmpIndex = getIndexFromInput();
    if (tmpIndex != 0) {
        currentTextureID = tmpIndex;
    }


    if (currentTextureID == 1){
        Texture = uvmap1;
    }
    if (currentTextureID == 2) {
        Texture = uvmap2;
    }
}



bool cleanupVertexbuffer()
{
    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);
    return true;
}

bool closeWindow()
{
    glfwTerminate();
    return true;
}







