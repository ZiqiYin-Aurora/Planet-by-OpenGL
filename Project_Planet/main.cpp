//
//  main.cpp
//  Project_Planet
//
//  Created by Yin Celia on 2020/11/27.
//

// for Mac
//#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wdocumentation"
//
//#define GLEW_STATIC

// --------------------------------------------------------
// Project aim to simulate the Solar System
// Codes are based on TextBook program 4-4 and 6-2
// GROUP INFORMATION:
//        JIXUAN YANG 1809853U-I011-0093
//        ZIQI YIN 1809853D-I011-0160
// --------------------------------------------------------

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/SOIL2.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <stack>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "Utils.h"
#include "Sphere.h"
using namespace std;

#define numVAOs 1
#define numVBOs 4

//float cameraX, cameraY, cameraZ;
float torLocX, torLocY, torLocZ;

GLuint renderingProgram, renderingProgramCubeMap;

GLuint vao[numVAOs];
GLuint vbo[numVBOs];

// variable allocation for display
GLuint mvLoc, projLoc, nLoc;
GLuint globalAmbLoc, ambLoc, diffLoc, specLoc, posLoc, mambLoc, mdiffLoc, mspecLoc, mshiLoc;

int width, height;
float aspect;
int Lighting;

glm::mat4 pMat, vMat, mMat, mvMat, invTrMat, rMat;
glm::vec3 currentLightPos, transformed;

float lightPos[3];

stack<glm::mat4> mvStack;

GLuint skyTexture;
GLuint sunTexture, planetTexture, moonTexture, mercuryTexture, venusTexture, marsTexture, jupiterTexture, saturntexture, uranusTexture, neptuneTexture, plutoTexture;

Sphere mySphere = Sphere(48);

// for mouse control camera
bool firstMouse = true;
float lastX = 300;
float lastY = 300;

float yaw;
float pitch;

float fov = 45.0f;//View the proportion
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 25.0f);//location of camera
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);//direction from front(actually is the speed)
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);//direction form the top


//-------------Lighting
//light location
glm::vec3 lightLoc = glm::vec3(1.0f,0.0f, 0.0f);
// white light
float globalAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; //light position
float lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };   //RGBA model enviroment light
float lightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };   //RGBA model diffuse relection
float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };   //RGBA model mirror light

// silver material
float* matAmb = Utils::goldAmbient();
float* matDif = Utils::goldDiffuse();
float* matSpe = Utils::goldSpecular();
float matShi = Utils::goldShininess();

void installLights(glm::mat4 vMatrix) {
    transformed = glm::vec3(vMatrix * glm::vec4(currentLightPos, 1.0));
    lightPos[0] = transformed.x;
    lightPos[1] = transformed.y;
    lightPos[2] = transformed.z;

    // get the locations of the light and material fields in the shader
    globalAmbLoc = glGetUniformLocation(renderingProgram, "globalAmbient");
    ambLoc = glGetUniformLocation(renderingProgram, "light.ambient");
    diffLoc = glGetUniformLocation(renderingProgram, "light.diffuse");
    specLoc = glGetUniformLocation(renderingProgram, "light.specular");
    posLoc = glGetUniformLocation(renderingProgram, "light.position");
    mambLoc = glGetUniformLocation(renderingProgram, "material.ambient");
    mdiffLoc = glGetUniformLocation(renderingProgram, "material.diffuse");
    mspecLoc = glGetUniformLocation(renderingProgram, "material.specular");
    mshiLoc = glGetUniformLocation(renderingProgram, "material.shininess");

    //  set the uniform light and material values in the shader
    glProgramUniform4fv(renderingProgram, globalAmbLoc, 1, globalAmbient);
    glProgramUniform4fv(renderingProgram, ambLoc, 1, lightAmbient);
    glProgramUniform4fv(renderingProgram, diffLoc, 1, lightDiffuse);
    glProgramUniform4fv(renderingProgram, specLoc, 1, lightSpecular);
    glProgramUniform3fv(renderingProgram, posLoc, 1, lightPos);
    glProgramUniform4fv(renderingProgram, mambLoc, 1, matAmb);
    glProgramUniform4fv(renderingProgram, mdiffLoc, 1, matDif);
    glProgramUniform4fv(renderingProgram, mspecLoc, 1, matSpe);
    glProgramUniform1f(renderingProgram, mshiLoc, matShi);
}

void setupVertices(void) {

    //Planets
    std::vector<int> ind = mySphere.getIndices();
    std::vector<glm::vec3> vert = mySphere.getVertices();
    std::vector<glm::vec2> tex = mySphere.getTexCoords();
    std::vector<glm::vec3> norm = mySphere.getNormals();
    std::vector<float> pvalues;
    std::vector<float> tvalues;
    std::vector<float> nvalues;

    int numIndices = mySphere.getNumIndices();
    for (int i = 0; i < numIndices; i++) {
        pvalues.push_back((vert[ind[i]]).x);
        pvalues.push_back((vert[ind[i]]).y);
        pvalues.push_back((vert[ind[i]]).z);
        tvalues.push_back((tex[ind[i]]).s);
        tvalues.push_back((tex[ind[i]]).t);
        nvalues.push_back((norm[ind[i]]).x);
        nvalues.push_back((norm[ind[i]]).y);
        nvalues.push_back((norm[ind[i]]).z);
    }
    glGenVertexArrays(1, vao);
    glBindVertexArray(vao[0]);
    glGenBuffers(numVBOs, vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //position
    glBufferData(GL_ARRAY_BUFFER, pvalues.size() * 4, &pvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); //texture
    glBufferData(GL_ARRAY_BUFFER, tvalues.size() * 4, &tvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]); //normal
    glBufferData(GL_ARRAY_BUFFER, nvalues.size() * 4, &nvalues[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * 4, &ind[0], GL_STATIC_DRAW);

}

void init(GLFWwindow* window) {

    renderingProgram = Utils::createShaderProgram("vertShader.glsl", "fragShader.glsl");

    glfwGetFramebufferSize(window, &width, &height);
    aspect = (float)width / (float)height;
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
  //  cameraX = 0.0f; cameraY = 3.5f; cameraZ = 40.0f;
    // for light
    torLocX = 0.0f; torLocY =0.0f; torLocZ = -40.0f;

    setupVertices();

    skyTexture = Utils::loadTexture("starsky.png");
    sunTexture = Utils::loadTexture("sun.jpg");
    planetTexture = Utils::loadTexture("earth.bmp");
    moonTexture = Utils::loadTexture("moon.bmp");
    mercuryTexture = Utils::loadTexture("mercury.bmp");
    venusTexture = Utils::loadTexture("venus.bmp");
    marsTexture = Utils::loadTexture("mars.bmp");
    jupiterTexture = Utils::loadTexture("jupiter.bmp");
    saturntexture = Utils::loadTexture("saturn.bmp");
    uranusTexture = Utils::loadTexture("uranus.bmp");
    neptuneTexture = Utils::loadTexture("neptune.bmp");
    plutoTexture = Utils::loadTexture("pluto.bmp");
}

void display(GLFWwindow* window, double currentTime) {

    float cameraSpeed = 0.5f;
    // push esc -> Quit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);//close window
    }
    // push 'W' -> Enlarge
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cameraPos += cameraSpeed * cameraFront;
    }
    // push 'S' -> Small
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    // push 'A' -> Left
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    // push 'D' -> Right
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    glClear(GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(renderingProgram);

    mvLoc = glGetUniformLocation(renderingProgram, "mv_matrix");
    projLoc = glGetUniformLocation(renderingProgram, "proj_matrix");
    nLoc = glGetUniformLocation(renderingProgram, "norm_matrix");

    //view matrix
   // vMat = glm::translate(glm::mat4(1.0f), glm::vec3(-cameraX, -cameraY, -cameraZ));
    vMat = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    //light position
    currentLightPos = glm::vec3(lightLoc.x, lightLoc.y, lightLoc.z);
    installLights(vMat);

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(pMat));
    mvStack.push(vMat);

    // ----------------------  Starsky
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime*0.15f, glm::vec3(0.0, 1.0, 0.0));
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(55, 55, 55)); // the size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    //mv matrix for normal vector is the inverse transpose of MV.
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //bind the vertices buffer to vertex attribute#0 in the vertex shader
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0); // 0 for position

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1); // 1 for texture coords

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, skyTexture);
    glUniform1i(glGetUniformLocation(renderingProgram, "samp"), 0); // need for Mac

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop();
    mvStack.pop(); //remove starsky from the stack

    //-----------------------  Sun

    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //sun position
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime, glm::vec3(0.0, -1.0, 0.0)); //sun rotation
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2); // 2 for vertex normal
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);


    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());


    mvStack.pop(); //remove the sun's axial rotation from the stack

    //-----------------------  Mercury

    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime+20)*2.5, 0.1f, cos((float)currentTime+20)*2.5));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime*0.5f, glm::vec3(0.0, -1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)); // the Mercury size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mercuryTexture);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop();
    mvStack.pop(); //remove Mercury's scale, rotation and position

    //-----------------------  Venus

    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime+14)*4.5, 0.3f, cos((float)currentTime+14)*4.5));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime*0.2f, glm::vec3(0.0, -1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(1.2f, 1.2f, 1.2f)); // the Venus size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, venusTexture);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); //remove
    mvStack.pop();


    //-----------------------  Planet-Earth


    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3( sin((float)currentTime + 3)*6.5, 0.4f,cos((float)currentTime + 3)*6.5));
    mvStack.push(mvStack.top());
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime*3, glm::vec3(0.0, -1.0, 0.3)); //planet rotation
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planetTexture);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); //remove Earth scale and rotation

    //-----------------------  Moon
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3( sin((float)currentTime+3)*2.5, 0.0f,cos((float)currentTime+3)*2.5));
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f)); //make the moon smaller
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, moonTexture);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); mvStack.pop(); //remove moon scale/rotation/position, earth position

    //-----------------------  Mars
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime+2)*9.0, 0.5f, cos((float)currentTime+2)*9.0));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime*2.8f, glm::vec3(0.0, -1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f)); //make the mars smaller
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, marsTexture);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); //remove
    mvStack.pop();

    //-----------------------  Jupiter
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime*0.52)*13.0, 0.85f, cos((float)currentTime*0.52)*13.0));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime*4, glm::vec3(0.0, -1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f)); // the Jupiter size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, jupiterTexture);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); //remove
    mvStack.pop();

    //-----------------------  Saturn
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime*0.43)*18.0, 1.25f, cos((float)currentTime*0.43)*18.0));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), (float)currentTime*5, glm::vec3(0.0, -1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(2.5f, 2.5f, 2.5f)); // the Saturn size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, saturntexture);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); //remove
    mvStack.pop();

    //-----------------------  Uranus
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime*0.37)*22, 1.75f, cos((float)currentTime*0.37)*22));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), -(float)currentTime*3.8f, glm::vec3(0.0, 1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(2.2f,2.2f, 2.2f)); // the Saturn size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, neptuneTexture);
    glUniform1i(glGetUniformLocation(renderingProgram, "samp"), 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); //remove
    mvStack.pop();

    //-----------------------  Neptune
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime*0.3)*25, 2.35f, cos((float)currentTime*0.3)*25));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), -(float)currentTime*4, glm::vec3(0.0, 1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(1.8f, 1.8f, 1.8f)); // the Saturn size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, uranusTexture);
    glUniform1i(glGetUniformLocation(renderingProgram, "samp"), 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop(); //remove
    mvStack.pop();

    //-----------------------  Pluto
    mvStack.push(mvStack.top());
    mvStack.top() *= glm::translate(glm::mat4(1.0f), glm::vec3(sin((float)currentTime*0.25)*27, 2.75f, cos((float)currentTime*0.25)*27));
    mvStack.push(mvStack.top());
    mvStack.top() *= rotate(glm::mat4(1.0f), -(float)currentTime*3, glm::vec3(0.0, 1.0, 0.0));  //rotation
    mvStack.top() *= scale(glm::mat4(1.0f), glm::vec3(.8f, .8f, .8f)); // the size
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
    mvStack.top() = glm::transpose(glm::inverse(mvStack.top()));
    glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, plutoTexture);
    glUniform1i(glGetUniformLocation(renderingProgram, "samp"), 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDrawArrays(GL_TRIANGLES, 0, mySphere.getNumIndices());

    mvStack.pop();
    mvStack.pop(); //remove Pluto

    mvStack.pop(); // remove sun
    mvStack.pop();  // the final pop is for the view matrix

}

void window_size_callback(GLFWwindow* win, int newWidth, int newHeight) {
    aspect = (float)newWidth / (float)newHeight;
    glViewport(0, 0, newWidth, newHeight);
    pMat = glm::perspective(1.0472f, aspect, 0.1f, 1000.0f);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) // This bool variable is initially set to true
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    pitch = pitch > 89.0f ? 89.0f : pitch;
    pitch = pitch < -89.0f ? -89.0f : pitch;

    glm::vec3 front;
    //The vector is calculated from the pitch and yaw angles, which are the values of the velocity in three dimensions
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)) - 1;
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (fov >= 1.0f && fov <= 45.0f) {
        fov -= yoffset;
    }

    fov = fov <= 1.0f ? 1.0f : fov;
    fov = fov >= 45.0f ? 45.0f : fov;
}


int main(void) {
    if (!glfwInit()) { exit(EXIT_FAILURE); }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1600, 1200, "Planet Project", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
    glfwSwapInterval(1);

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    init(window);

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
