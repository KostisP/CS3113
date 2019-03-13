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
#include <cstdlib>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;



class Board {
public:
    
    float x;
    float y;
    float rotation;
    int textureID;
    
    float width;
    float height;
    float velocity;
    float direction_x;
    float direction_y;
    
    Board(float x, float y, float rotation, int texture, float width, float height, float velocity, float direction_x, float direction_y) :
    x(x), y(y), rotation(rotation), textureID(texture), width(width), height(height), velocity(velocity), direction_x(direction_x), direction_y(direction_y)
    {}
    
    void Draw(ShaderProgram &p) {
        float points[] = {-width/2, -height/2, width/2, -height/2, width/2, height/2, -width/2, -height/2, width/2, height/2, -width/2, height/2};
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, 0.0f));
        p.SetModelMatrix(modelMatrix);
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, points);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    
};


















int main(int argc, char *argv[]) {
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    
    ShaderProgram program; // Used for textures
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    
    
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    
    float lastFrameTicks = 0.0f;
    
    //float angle = 90.0f * (3.1415926f / 180.0f);
    
    Board left(-1.747f, 0.0f, 0.0f, 0, 0.06f, 0.3f, 0.0f, 0.0f, 0.0f);
    Board right(1.747f, 0.0f, 0.0f, 0, 0.06f, 0.3f, 0.0f, 0.0f, 0.0f);
    Board ball(0.0f, 0.0f, 0.0f, 0, 0.08f, 0.08f, 0.0f, 1.0f, 0.5f);
    float pLeftX = 1.777f;
    float pRightX = 1.777f;
    float pLeftY = 1.0f;
    float pRightY = 1.0f;
    
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        
        glClear(GL_COLOR_BUFFER_BIT);
        modelMatrix = glm::mat4(1.0f);
        program.SetModelMatrix(modelMatrix);
        
        
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
      
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        
        
        if(event.type == SDL_KEYDOWN) {
            if(keys[SDL_SCANCODE_UP]) {
                right.y += elapsed * 1;
                
            } else if (keys[SDL_SCANCODE_DOWN]) {
                right.y -= elapsed * 1;
                
            }
            
            if(keys[SDL_SCANCODE_LEFT]) {
                left.y += elapsed * 1;
            } else if (keys[SDL_SCANCODE_RIGHT]) {
                left.y -= elapsed * 1;
            }
       }
        
        pLeftX = abs(left.x-ball.x) - (left.width + ball.width)/2;
        pRightX = abs(right.x-ball.x) - (right.width + ball.width)/2;
        
        pLeftY = abs(left.y-ball.y) - (left.height + ball.height)/2;
        pRightY = abs(right.y-ball.y) - (right.height + ball.height)/2;
        
        
        
        
        
        if ((right.y + (right.height/2)) >= 1.0f) {
            right.y = 1.0f - (right.height/2);
        }
        if ((right.y - (right.height/2)) <= -1.0f) {
            right.y = -1.0f + (right.height/2);
        }
        
        
        if ((left.y + (left.height/2)) >= 1.0f) {
            left.y = 1.0f - (left.height/2);
        }
        if ((left.y - (left.height/2)) <= -1.0f) {
            left.y = -1.0f + (left.height/2);
        }
        
        
        
        
        if (pRightX < 0 && pRightY < 0) {
            ball.direction_x = -1.0f;
        }
        if (pLeftX < 0 && pLeftY < 0) {
            ball.direction_x = 1.0f;
        }
        
        
        
        
        
        
        if ((ball.y + (ball.height/2)) > 1.0f) {
            ball.direction_y = ball.direction_y * -1.0f;
        }
        if ((ball.y - (ball.height/2)) < -1.0f) {
            ball.direction_y = ball.direction_y * -1.0f;
            
        }
        
        
        
        
        
        
        if ((ball.x + (ball.width/2)) >= 1.777f) {
            ball.x = 0.0f;
            ball.y = 0.0f;
            ball.direction_x = -1.0f;
            ball.direction_y = 0.2f;
        }
        if ((ball.x - (ball.width/2)) <= -1.777f) {
            ball.x = 0.0f;
            ball.y = 0.0f;
            ball.direction_x = 1.0f;
            ball.direction_y = 0.2f;
        }
        
  
        
        
        
   
        
        
        

        
        ball.x += ball.direction_x * elapsed * 1.0;
        ball.y += ball.direction_y * elapsed * 1.0;
        
        
        left.Draw(program);
        right.Draw(program);
        ball.Draw(program);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}











void Setup() {
    // setup SDL
    // setup OpenGL
    // Set our projection matrix
}

void ProcessEvents() {
    // our SDL event loop
    // check input events
}

void Update() {
    // move stuff and check for collisions
}

void Render() {
    // for all game elements
    // setup transforms, render sprites
}

void Cleanup() {
    // cleanup joysticks, textures, etc.
}
