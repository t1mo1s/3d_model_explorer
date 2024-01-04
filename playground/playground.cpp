#pragma comment(lib, "winmm.lib")
#include "playground.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cassert>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include "parse_stl.h"

#include <vector>
#include <memory>
#include <random>
#include<windows.h>

//include time
#include <time.h>

#include "GameObject.h"
#include "MainObject.h"


std::vector< std::shared_ptr<GameObject> > gameObjects;
float applicationStartTimeStamp; //time stamp of application start
glm::vec3 startPos = glm::vec3(-55, 20, 100);
glm::vec3 endPosScene1 = glm::vec3(-0.3, 6.7, -8.8);
int switchedScene = 0;

bool keyD = false;
bool keyA = false;


// global variables to handle the MVP matrix
double lastX = width / 2.0, lastY = height / 2.0;
bool firstMouse = true;
float yaw = 118.7f;
float pitch = 3.5f;


int main(void)
{
    cameraPos = startPos;
    //Initialize window
    bool windowInitialized = initializeWindow();
    if (!windowInitialized) return -1;

    glEnable(GL_DEPTH_TEST);

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("../playground/VertexShaderScene1.vertexshader", "../playground/FragmentShaderScene1.fragmentshader");

    Sleep(200);
    applicationStartTimeStamp = (float)glfwGetTime();
	std::shared_ptr<GameObject> lightingDemoObj = std::make_shared<MainObject>(programID, "../stlFiles/Dragon 2.5_stl.stl", width / height);
	gameObjects.push_back(lightingDemoObj);

    cameraFront = glm::vec3(0.0f, 0.0f, 0.0f); // Anfangsrichtung, in die die Kamera schaut
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // "Oben" Vektor der Welt
    createVPTransformation();

   curr_x = startPos.x;
   curr_y = startPos.y;
   curr_z = startPos.z;

    //start animation loop until escape key is pressed
    do {

        // if a key is pressed , set the corresponding bool to true
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            keyA = true;
        }
        else {
            keyA = false;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            keyD = true;
        }
        else {
            keyD = false;
        }

        updateAnimationLoop(); 

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0 && curr_time < 35.0f);


    //Cleanup and close window
    cleanupVertexbuffer();
    glDeleteProgram(programID);
    closeWindow();

    return 0;
}

void parseStl(std::vector< glm::vec3 >& vertices,
    std::vector< glm::vec3 >& normals,
    std::string stl_file_name)
{
    stl::stl_data info = stl::parse_stl(stl_file_name);
    std::vector<stl::triangle> triangles = info.triangles;
    for (int i = 0; i < info.triangles.size(); i++) {
        stl::triangle t = info.triangles.at(i);
        glm::vec3 triangleNormal = glm::vec3(t.normal.x,
            t.normal.y,
            t.normal.z);
        //add vertex and normal for point 1:
        vertices.push_back(glm::vec3(t.v1.x, t.v1.y, t.v1.z));
        normals.push_back(triangleNormal);
        //add vertex and normal for point 2:
        vertices.push_back(glm::vec3(t.v2.x, t.v2.y, t.v2.z));
        normals.push_back(triangleNormal);
        //add vertex and normal for point 3:
        vertices.push_back(glm::vec3(t.v3.x, t.v3.y, t.v3.z));
        normals.push_back(triangleNormal);
    }
}




void updateAnimationLoop()
{
    // print yaw and pitch
    std::cout << "yaw: " << yaw << " pitch: " << pitch << std::endl;

    // print camera Front
    std::cout << "cameraFront: " << cameraFront.x << " " << cameraFront.y << " " << cameraFront.z << std::endl;
    // print camera position
    std::cout << "cameraPos: " << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << std::endl;

    float cameraSpeed = 0.3f; // passen Sie die Geschwindigkeit nach Bedarf an

    // Berechne den horizontalen Frontvektor ohne Höhenänderung
    glm::vec3 cameraFrontHorizontal = cameraFront;
    cameraFrontHorizontal.y = 0;
    cameraFrontHorizontal = glm::normalize(cameraFrontHorizontal);

    // Berechne den Rechtsvektor, der senkrecht zur Blickrichtung und zum Up-Vektor steht
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFrontHorizontal, cameraUp));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFrontHorizontal;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFrontHorizontal;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos += cameraRight * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos -= cameraRight * cameraSpeed;


    // set variable time to current time in miliseconds
	curr_time = (float)glfwGetTime() - applicationStartTimeStamp;

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);
    createVPTransformation();
    // Send our transformation to the currently bound shader, 
    // in the "MVP" uniform
    glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projection[0][0]);
    glUniform3f(cameraPosID, cameraPos.x, cameraPos.y, cameraPos.z);

    for (int i = 0; i < gameObjects.size(); i++)
    {
        gameObjects.at(i)->Update();
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = ypos- lastY; // Umgekehrt, da y-Koordinaten von unten nach oben gehen
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;
    // print yaw and pitch
    std::cout << "yaw: " << yaw << " pitch: " << pitch << std::endl;

    // Sicherstellen, dass der Bildschirm nicht überkippt
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
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
    width = 1920;
	height = 1080;
    window = glfwCreateWindow(width, height, "Demo: Cube", NULL, NULL);
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


    // Setze den Maus-Cursor-Modus
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Registriere den Maus-Callback
    glfwSetCursorPosCallback(window, mouse_callback);
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
    return true;
}

bool createVPTransformation() {
    
    GLuint viewIDNew = glGetUniformLocation(programID, "view");
    viewID = viewIDNew;

    GLuint projectionIDNew = glGetUniformLocation(programID, "projection");
    projectionID = projectionIDNew;

	GLuint cameraPosIDNew = glGetUniformLocation(cameraPosID, "cameraPos");
	cameraPosID = cameraPosIDNew;

    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 500.0f);
    // Camera matrix
    
    glm::mat4 View = glm::lookAt(
        cameraPos,
        cameraPos - cameraFront,
        cameraUp
    );

    
    view = View;

	projection = Projection;

    return true;
}

bool cleanupVertexbuffer()
{
    // Cleanup VBO
    glDeleteVertexArrays(1, &VertexArrayID);
    return true;
}

bool cleanupColorbuffer()
{
    // Cleanup VBO
    glDeleteBuffers(1, &colorbuffer);
    return true;
}

bool closeWindow()
{
    glfwTerminate();
    return true;
}


