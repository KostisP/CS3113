
// Move with arrows, space to shoot


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
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


class SheetSprite {
public:
    
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
    
    SheetSprite();
    
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {}
    
    void draw(ShaderProgram &program) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        GLfloat texCoords[] = {
            u, v+height,
            u+width, v,
            u, v,
            u+width, v,
            u, v+height,
            u+width, v+height
        };
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size};

        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.positionAttribute);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    }
};



class Entity {
public:
    
    glm::vec3 position;
    glm::vec3 direction;
    float velocity;
    float size;
    float width;
    float height;
    SheetSprite* sprite;
    
    Entity() {}
    
    Entity(SheetSprite* sprite, float x, float y, float width, float height, float velocity, float size) :
    width(width), height(height), sprite(sprite), velocity(velocity), size(size) {

        position.x = x;
        position.x = x;
        position.y = y;
        direction.x = 1.0f;
        direction.y = 1.0f;
    }
    
    void draw(ShaderProgram program) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        program.SetModelMatrix(modelMatrix);
        sprite->draw(program);
    }
};



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



void DrawText(ShaderProgram &program, int fontTexture, std::string text, float size, float spacing) {
    float character_size = 1.0/16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
    for(int i=0; i < text.size(); i++) {
        int spriteIndex = (int)text[i];
        float texture_x = (float)(spriteIndex % 16) / 16.0f;
        float texture_y = (float)(spriteIndex / 16) / 16.0f;
        
        vertexData.insert(vertexData.end(), {
            ((size+spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (0.5f * size), -0.5f * size,
            ((size+spacing) * i) + (0.5f * size), 0.5f * size,
            ((size+spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x + character_size, texture_y + character_size,
            texture_x + character_size, texture_y,
            texture_x, texture_y + character_size,
        });
    }
    
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6*text.size());
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}


void titleScreenText(ShaderProgram &program, GLuint &font) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.70f, 1.0f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Space Invaders", 0.1, 0.01);
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.68f, -0.30f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Press S to play", 0.1, -0.001);
}


void endGameText(ShaderProgram &program, GLuint &font) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.55f, 1.0f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Game Ended", 0.1, 0.01);
}



// Globals
enum GameMode {MAIN_MENU, GAME, END_GAME};
GameMode mode;


struct GameState {
    Entity spaceShip;
    std::vector<Entity> invaders;
    Entity laser;

};
//GameState state;


ShaderProgram program;
const float projectionX = 1.0f;
const float projectionY = 1.777f;
const float largeX = 200.0f;
const float largeY = 200.0f;


GLuint spriteSheet;
GLuint font;

SDL_Event event;
bool done;
float lastFrameTicks;


void Setup(GameState& state);
void ProcessEvents();
void Update(GameState& state);
void Render(GameState& state);




int main(int argc, char *argv[]) {
    GameState state;
    Setup(state);
    
    
    // This should be in setup but for some reason SheetSprite object messes up when i place it there and entities become extremely //small
    SheetSprite spaceShipSprite(spriteSheet, 224.0f / 1024.0f, 832.0f / 1024.0f, 99.0f / 1024.0f, 75.0f / 1024.0f, 0.3f);
    SheetSprite invaderSprite(spriteSheet, 425.0f / 1024.0f, 468.0f / 1024.0f, 93.0f / 1024.0f, 84.0f / 1024.0f, 0.12f);
    SheetSprite laserSprite(spriteSheet, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f / 1024.0f, 0.2f);
    
    state.spaceShip = Entity(&spaceShipSprite, 0.0f, -1.5f, 99.0f, 75.0f, 1.0f, 0.3f);
    state.laser = Entity(&laserSprite, 0, 0, 9.0f, 54.0f, 5.0f, 0.2f);
    state.laser.position.y = largeY;
    for (float j = -0.8; j <= 0.8; j += 0.2f) {
        for (float i = 1.5; i >= 0.8; i -= 0.2f) {
            state.invaders.push_back(Entity(&invaderSprite, j, i, 93.0f, 84.0f, 0.04f, 0.12f));
        }
    }
    
    
    
    while (!done) {
        ProcessEvents();
        Update(state);
        Render(state);
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}




void Setup(GameState& state) {
   
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 380, 500, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-projectionX, projectionX, -projectionY, projectionY, -projectionY, projectionY);
    
    glUseProgram(program.programID);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    lastFrameTicks = 0.0f;
    mode = MAIN_MENU;
    done = false;
    
    spriteSheet = LoadTexture(RESOURCE_FOLDER"sheet.png");
    font = LoadTexture(RESOURCE_FOLDER"font2.png");
}




void ProcessEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}




// Checks for colision between two Entities
bool checkCollision(Entity &one, Entity &two) {
    if (one.position.y -  0.5 * one.size < two.position.y + 0.5 * two.size && one.position.y + 0.5 * one.size > two.position.y - 0.5 * two.size) {
        if (one.position.x + 0.5 * one.size * one.width / one.height > two.position.x - 0.5 * two.size * two.width / two.height
            && one.position.x - 0.5 * one.size * one.width / one.height < two.position.x + 0.5 * two.size * two.width / two.height) {
            return true;
        }
    }
    return false;
}





bool shouldRemoveInvader(Entity& invader) {
    return !(invader.position.x < projectionX);
}




void Update(GameState& state) {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    switch (mode) {
        case MAIN_MENU: {
            if (keys[SDL_SCANCODE_S]) {
                mode = GAME;
            }
            break;
        }
        case GAME: {
            
            // SpaceShip Movement
            if (keys[SDL_SCANCODE_RIGHT] && state.spaceShip.position.x +  0.5 * state.spaceShip.size * state.spaceShip.width / state.spaceShip.height <= projectionX) {
                state.spaceShip.position.x += state.spaceShip.velocity * elapsed;
            }
            if (keys[SDL_SCANCODE_LEFT] && state.spaceShip.position.x - 0.5 * state.spaceShip.size * state.spaceShip.width / state.spaceShip.height >= -projectionX) {
                state.spaceShip.position.x -= state.spaceShip.velocity * elapsed;
                
            }
            if (keys[SDL_SCANCODE_SPACE] && state.laser.position.y > projectionY) {
                state.laser.position.x = state.spaceShip.position.x;
                state.laser.position.y = state.spaceShip.position.y + 0.2f;
            };
            
            
            
            // Move laser if it exists
            if (state.laser.position.y <= projectionY) { state.laser.position.y += state.laser.velocity * elapsed; }
            // Reactivate laser if goes off screen
            if (state.laser.position.y >= projectionY) { state.laser.position.y = 200.0f; }
        
            
        
            bool hitRightWall = false;
            bool hitLeftWall = false;
            for (Entity &invader : state.invaders) {
                
                // Right/Left invader movement set flags when colission with walls
                invader.position.x += invader.velocity * invader.direction.x * elapsed;
                if (invader.position.x + 0.5 * invader.size * invader.width / invader.height >= projectionX) {
                    hitRightWall = true;
                } else if (invader.position.x - 0.5 * invader.size * invader.width / invader.height <= -projectionX) {
                    hitLeftWall = true;
                }
            
                
                // Checking collisions
                if (checkCollision(invader, state.spaceShip)) {
                        mode = END_GAME;
                }
                if (state.laser.position.y <= projectionY && checkCollision(state.laser, invader)) {
                    invader.position.x = largeX;
                    state.laser.position.y = largeY;
                }
               
            }
            
            // Move invaders down if they have colided with a wall and reverse direction of movement
            for (Entity &invader : state.invaders) {
                if (hitRightWall) {
                    invader.direction.x = -1.0f;
                    invader.position.y -= 0.08f;
                } else if (hitLeftWall) {
                    invader.direction.x = 1.0f;
                    invader.position.y -= 0.08f;
                }
            }
            
            
            // Remove dead invaders
            state.invaders.erase(std::remove_if(state.invaders.begin(), state.invaders.end(), shouldRemoveInvader), state.invaders.end());
            
            
            // End game if all invaders are dead
            if (state.invaders.size() == 0) {
                mode = END_GAME;
            }
            break;
        }
        
    }
}

void Render(GameState& state) {
    glClear(GL_COLOR_BUFFER_BIT);
    switch(mode) {
        case MAIN_MENU:
            titleScreenText(program, font);
            break;
        case GAME:
            state.spaceShip.draw(program);
            if (state.laser.position.y <= projectionY) {state.laser.draw(program);}
            for (Entity& invader : state.invaders) {
                invader.draw(program);
            }
            break;
        case END_GAME:
            endGameText(program, font);
            break;
    }
}








