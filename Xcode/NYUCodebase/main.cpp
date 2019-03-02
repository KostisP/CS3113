#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;



GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
//        assert(false);
    }
    
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}




int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("HW1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 360);
    
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint bluePlaneTexture = LoadTexture(RESOURCE_FOLDER"planeBlue1.png");
    GLuint redPlaneTexture = LoadTexture(RESOURCE_FOLDER"planeBlue1.png");
    GLuint laserPlaneTexture = LoadTexture(RESOURCE_FOLDER"planeBlue1.png");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glClearColor(0.3f, 0.8f, 0.9f, 1.0f);

    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.8f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        
        
        // Drawing floor
        float vertices[] = {-1.777, -0.2, 1.777, -0.2, 1.777, 0.2, -1.777, -0.2, 1.777, 0.2, -1.777, 0.2};
        program.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        // float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        // glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        //  glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // glDisableVertexAttribArray(program.texCoordAttribute);
        glDisableVertexAttribArray(program.positionAttribute);
        
        
        
        /*
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.0f, 0.0f, 0.0f));
        
        program.SetModelMatrix(modelMatrix);
        
        // draw first object
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 0.0f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        
        // draw second object
        
         */
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}


