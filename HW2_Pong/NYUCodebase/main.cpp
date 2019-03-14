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


// Note: using W ans S for left paddle, up and down for right paddle



class Rectangle {
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
    
    Rectangle(float x, float y, float rotation, int texture, float width, float height, float velocity, float direction_x, float direction_y) :
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




void Setup();
void ProcessEvents();
void Update();
void Render();


void pointWon(); // Resets paddles and ball, ball will move towards whoever won the point


// Globals
ShaderProgram program;
float lastFrameTicks;


const float ballSpeedBeforeFirstToutch = 0.8f;
const float ballSpeedFinal = 1.5f;
const float reflectionAngle = 0.3f;
const float paddleSpeed = 1.0f;


// X and Y for projection matrix
const float projectionX = 1.777;
const float projectionY = 1.0;


Rectangle left(-1.747f, 0.0f, 0.0f, 0, 0.06f, 0.3f, paddleSpeed, 0.0f, 0.0f);
Rectangle right(1.747f, 0.0f, 0.0f, 0, 0.06f, 0.3f, paddleSpeed, 0.0f, 0.0f);
Rectangle ball(0.0f, 0.0f, 0.0f, 0, 0.08f, 0.08f, ballSpeedBeforeFirstToutch, 1.0f, reflectionAngle);


SDL_Event event;
bool done;




int main(int argc, char *argv[]) {
    Setup();
    while (!done) {
        ProcessEvents();
        Update();
        Render();
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}




void Setup() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-projectionX, projectionX, -projectionY, projectionY, -projectionY, projectionY);
    
    glUseProgram(program.programID);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    lastFrameTicks = 0.0f;
    done = false;
}




void ProcessEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
        
        
        
        
        
    }
}




void Update() {
    float ticks = (float)SDL_GetTicks()/1000.0f;
    float elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    // Input Movement
    if(keys[SDL_SCANCODE_UP]) {
        right.y += elapsed * right.velocity;
            
    } else if (keys[SDL_SCANCODE_DOWN]) {
        right.y -= elapsed * right.velocity;
    }
        
    if(keys[SDL_SCANCODE_W]) {
        left.y += elapsed * left.velocity;
    } else if (keys[SDL_SCANCODE_S]) {
        left.y -= elapsed * left.velocity;
    }

    
    
    // Calculates Distances for collision
    float pLeftX = abs(left.x-ball.x) - (left.width + ball.width)/2;
    float pRightX = abs(right.x-ball.x) - (right.width + ball.width)/2;
    float pLeftY = abs(left.y-ball.y) - (left.height + ball.height)/2;
    float pRightY = abs(right.y-ball.y) - (right.height + ball.height)/2;
    
    
    
    // Determines if right paddle is on screen boarder
    if ((right.y + (right.height/2)) >= projectionY) {
        right.y = projectionY - (right.height/2);
    }
    if ((right.y - (right.height/2)) <= -projectionY) {
        right.y = -projectionY + (right.height/2);
    }
    
    
    
    // Determines if left paddle is on screen boarder
    if ((left.y + (left.height/2)) >= projectionY) {
        left.y = projectionY - (left.height/2);
    }
    if ((left.y - (left.height/2)) <= -projectionY) {
        left.y = -projectionY + (left.height/2);
    }
    
    
    
    // Collision with right paddle
    if (pRightX < 0 && pRightY < 0) {
        ball.direction_x *= -1.0f;
        ball.velocity = ballSpeedFinal;
    }
    
    // Collision with left paddle
    if (pLeftX < 0 && pLeftY < 0) {
        ball.direction_x *= -1.0f;
        ball.velocity = ballSpeedFinal;
    }
    
    
    
    
    if ((ball.y + (ball.height/2)) > projectionY) {
        ball.direction_y *= -1.0f;
    }
    if ((ball.y - (ball.height/2)) < -projectionY) {
        ball.direction_y *= -1.0f;
    }
    
    
    
    // Left Player Scores
    if ((ball.x + (ball.width/2)) >= projectionX) {
        pointWon();
    }
    // Right Player Scores
    if ((ball.x - (ball.width/2)) <= -projectionX) {
        pointWon();
    }
    
    ball.x += ball.direction_x * elapsed * ball.velocity;
    ball.y += ball.direction_y * elapsed * ball.velocity;
}




void pointWon() {
    ball.x = 0.0f;
    ball.y = 0.0f;
    right.y = 0.0f;
    left.y = 0.0f;
    ball.direction_y = reflectionAngle;
    ball.direction_x *= -1.0f;
    ball.velocity = ballSpeedBeforeFirstToutch;
}





void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    program.SetModelMatrix(modelMatrix);
    
    left.Draw(program);
    right.Draw(program);
    ball.Draw(program);
}








