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
        std::cout << "Unable to load image. Make sure the path is correct\n" << filePath;
        assert(false);
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
    
    ShaderProgram program; // Used for textures
    ShaderProgram programNonText; // Used for no textures
   
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    programNonText.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    GLuint redPlaneTexture = LoadTexture("/Users/kostispaschalakis/Desktop/NYU/Spring2019/CS 3113 Game Prog/Personal/CS3113/hw1/NYUCodebase/planeGreen1.png");
    
    GLuint greenPlaneTexture = LoadTexture("/Users/kostispaschalakis/Desktop/NYU/Spring2019/CS 3113 Game Prog/Personal/CS3113/hw1/NYUCodebase/planeRed1.png");
    GLuint laserTexture = LoadTexture("/Users/kostispaschalakis/Desktop/NYU/Spring2019/CS 3113 Game Prog/Personal/CS3113/hw1/NYUCodebase/laserRed03.png");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    glUseProgram(programNonText.programID);
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
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        
        program.SetModelMatrix(modelMatrix);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix);
        
        
        programNonText.SetModelMatrix(modelMatrix);
        programNonText.SetProjectionMatrix(projectionMatrix);
        programNonText.SetViewMatrix(viewMatrix);
        
        
        // Drawing floor
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.8f, 0.0f));
        programNonText.SetModelMatrix(modelMatrix);
        
        float floor[] = {-1.777, -0.2, 1.777, -0.2, 1.777, 0.2, -1.777, -0.2, 1.777, 0.2, -1.777, 0.2};
        programNonText.SetColor(0.2f, 0.8f, 0.4f, 1.0f);
        glVertexAttribPointer(programNonText.positionAttribute, 2, GL_FLOAT, false, 0, floor);
        glEnableVertexAttribArray(programNonText.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(programNonText.positionAttribute);
        
        
       
        
        // Green Plane
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 0.5f, 0.0f));
        program.SetModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, greenPlaneTexture);
        
        float bluePlane[] = {-0.2, -0.2, 0.2, -0.2, 0.2, 0.2, -0.2, -0.2, 0.2, 0.2, -0.2, 0.2};
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, bluePlane);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float bluePlanetexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, bluePlanetexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        
      
        // Red Plane
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(1.0f, 0.5f, 0.0f));
        float angle = 45.0f * (3.1415926f / 180.0f);
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        program.SetModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, redPlaneTexture);
        
        float redPlane[] = {-0.2, -0.2, 0.2, -0.2, 0.2, 0.2, -0.2, -0.2, 0.2, 0.2, -0.2, 0.2};
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, redPlane);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float redPlanetexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, redPlanetexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        
        
        
        
        //Laser
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, 0.5f, 0.0f));
        angle = 90.0f * (3.1415926f / 180.0f);
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.6f, 1.0f));
        program.SetModelMatrix(modelMatrix);
        
        glBindTexture(GL_TEXTURE_2D, laserTexture);
        
        float laser[] = {-0.2, -0.2, 0.2, -0.2, 0.2, 0.2, -0.2, -0.2, 0.2, 0.2, -0.2, 0.2};
        
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, laser);
        glEnableVertexAttribArray(program.positionAttribute);
        
        float lasertexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, lasertexCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}


