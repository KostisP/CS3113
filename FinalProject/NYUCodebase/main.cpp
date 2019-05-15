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
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <SDL_mixer.h>
using namespace std;


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6
float accumulator = 0.0f;

const float projectionX = 2.0f;
const float projectionY = 2.0f;


#define LEVEL_HEIGHT 46
#define LEVEL_WIDTH 46


float TILE_SIZE = 0.086956521f;

int spriteCountX = 16;
int spriteCountY = 16;

int mapWidth;
int mapHeight;

unsigned char** levelData = NULL;

std::vector<float> vertexxData;
std::vector<float> texxCoordData;


enum EntityType {Player, Enemy, Boss, playerBullet, enemyBullet};

Mix_Chunk *playerShoot;
Mix_Chunk *enemyShoot;
Mix_Chunk *hit;
Mix_Music *loop;


float animationTime = 0.0f;

float mapValue(float value, float srcMin, float srcMax, float dstMin, float dstMax) {
    float retVal = dstMin + ((value - srcMin)/(srcMax-srcMin) * (dstMax-dstMin));
    if(retVal < dstMin) {
        retVal = dstMin;
    }
    if(retVal > dstMax) {
        retVal = dstMax;
    }
    return retVal;
}


float easeInOut(float from, float to, float time) {
    float tVal;
    if(time > 0.5) {
        float oneMinusT = 1.0f-((0.5f-time)*-2.0f);
        tVal = 1.0f - ((oneMinusT * oneMinusT * oneMinusT * oneMinusT *
                        oneMinusT) * 0.5f);
    } else {
        time *= 2.0;
        tVal = (time*time*time*time*time)/2.0;
    }
    return (1.0f-tVal)*from + tVal*to;
}

float lerp(float from, float to, float t) {
    return (1.0-t)*from + t*to;
}

float easeOutElastic(float from, float to, float time) {
    float p = 0.3f;
    float s = p/4.0f;
    float diff = (to - from);
    return from + diff + (diff*pow(2.0f,-10.0f*time) * sin((time-s)*(2*4.14159265f)/p));
}

float xPosTitle = 0.0f;
float yPosTitle = 0.0f;

float xPosPlay = 0.0f;
float yPosPlay = 0.0f;


float xPosMove = 0.0f;
float yPosMove = 0.0f;

float xPosAttack = 0.0f;
float yPosAttack = 0.0f;


void worldToTileCoordinates(float worldX, float worldY, int &gridX, int &gridY) {
    gridX = (int)(worldX / TILE_SIZE);
    gridY = (int)(worldY / -TILE_SIZE);
}



class Entity {
public:
    
    GLuint textureID;
    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float width;
    float height;
    bool isStatic;
    EntityType entityType;
   
    bool bulletActive; //used only by bullet entites
    int health;
    int firedBulletIndex;
    
    

    bool collidedTop;
    bool collidedBottom;
    bool collidedLeft;
    bool collidedRight;
    
    
    
    Entity() {}
    
    Entity(GLuint textureID, float x, float y, float width, float height, float velocityx, float velocityy,
           float accx, float accy, float sizex, float sizey, EntityType type, int health) : entityType(type), textureID(textureID), width(width), height(height), health(health)   {
        
        position.x = x;
        position.y = y;
        size.x = sizex;
        size.y = sizey;
        velocity.x = velocityx;
        velocity.y = velocityy;
        acceleration.x = accx;
        acceleration.y = accy;
        bulletActive = false;
        firedBulletIndex = -1;
        
        
    }
    
    void checkCollisionsX() {
        int gridX;
        int gridY;
        worldToTileCoordinates(position.x+ 2.0f + (size.x /2), position.y-2.0f, gridX, gridY);
        collidedRight = !stepsOnFloor(gridX, gridY);
        if (collidedRight) {
            float penetration = fabs((-TILE_SIZE*gridX) + (position.x + 2.0 + size.x / 2));
            position.x -= penetration;
        }
        
        worldToTileCoordinates(position.x+ 2.0f - (size.x /2), position.y-2.0f, gridX, gridY);
        collidedLeft = !stepsOnFloor(gridX, gridY);
        if (collidedLeft) {
            float penetration = fabs(-(TILE_SIZE*gridX + TILE_SIZE) + (position.x + 2.0 - size.x / 2));
            position.x += penetration;;
        }
    }
    
    void checkCollisionsY() {
        int gridX;
        int gridY;
        worldToTileCoordinates(position.x + 2.0f, position.y-2.0f + (size.y /2), gridX, gridY);
        
        collidedTop = !stepsOnFloor(gridX, gridY);
        if (collidedTop) {
            float penetration = fabs(-(-TILE_SIZE*gridY - TILE_SIZE ) + (position.y - 2.0f + size.y /2));
            position.y -= penetration;;
        }
        
        worldToTileCoordinates(position.x + 2.0f, position.y-2.0f - (size.y / 2), gridX, gridY);
        collidedBottom = !stepsOnFloor(gridX, gridY);
        if (collidedBottom) {
            float penetration = fabs(-(-TILE_SIZE*gridY) + (position.y - 2.0f - size.y /2));
            position.y += penetration;;
        }
    }
    
    void Update(float timestep);
    

    
    bool stepsOnFloor(int X, int Y) {
        
        if (levelData[Y][X]  == 48) {
            return false;
        } else if (levelData[Y][X]  == 54) {
            return false;
        } else if (levelData[Y][X]  == 0) {
            return false;
        } else if (levelData[Y][X]  == 102) {
            return false;
        } else if (levelData[Y][X]  == 23) {
            return false;
        } else if (levelData[Y][X]  == 24) {
            return false;
        }
        return true;
    }
    
    
    
    bool CollidesWith(Entity &entity) {
        if (position.y -  0.5 * size.y < entity.position.y + 0.5 * entity.size.y && position.y + 0.5 * size.y > entity.position.y - 0.5 * size.y) {
            if (position.x + 0.5 * size.x * width / height > entity.position.x - 0.5 * entity.size.x * entity.width / entity.height
                && position.x - 0.5 * size.x * width / height < entity.position.x + 0.5 * entity.size.x * entity.width / entity.height) {
                return true;
            }
        }
        return false;
    }
    
    
  
    void Render(ShaderProgram& program) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        program.SetModelMatrix(modelMatrix);
        
        int index = 0;
        
        if (entityType == Player) {
            index = 132;
        } else if (entityType == Enemy){
            index = 149;
        } else if (entityType == Boss){
            index = 150;
        } else if (entityType == playerBullet) {
            index = 194;
        } else if (entityType == enemyBullet) {
            index = 211;
        }
        
        float u = (float)(((int)index) % spriteCountX) / (float) spriteCountX;
        float v = (float)(((int)index) / spriteCountX) / (float) spriteCountY;
        float spriteWidth = 1.0/(float)spriteCountX;
        float spriteHeight = 1.0/(float)spriteCountY;
        
        
        GLfloat texCoords[] = {
            u, v+spriteHeight,
            u+spriteWidth, v,
            u, v,
            u+spriteWidth, v,
            u, v+spriteHeight,
            u+spriteWidth, v+spriteHeight
        };
        
        
        float aspect = width / height;
        float vertices[] = {
            -0.5f * size.x * aspect, -0.5f * size.y,
            0.5f * size.x * aspect, 0.5f * size.y,
            -0.5f * size.x * aspect, 0.5f * size.y,
            0.5f * size.x * aspect, 0.5f * size.y,
            -0.5f * size.x * aspect, -0.5f * size.y ,
            0.5f * size.x * aspect, -0.5f * size.y};
        
     
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.positionAttribute);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
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





bool readHeader(std::ifstream &stream) {
    string line;
    mapWidth = - 1;
    mapHeight = - 1;
    while (getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "width") {
            mapWidth = atoi(value.c_str());
        } else if(key == "height"){
            mapHeight = atoi(value.c_str());
        }
    }
    if (mapWidth == - 1 || mapHeight == - 1) {
        return false;
    } else { // allocate our map data
        levelData = new unsigned char* [mapHeight];
        for (int i = 0; i < mapHeight; ++i) {
            levelData[i] = new unsigned char [mapWidth];
        }
        return true;
    }
}


bool readLayerData(std::ifstream &stream) {
    string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                istringstream lineStream(line);
                string tile;
                for(int x=0; x < mapWidth; x++) {
                    getline(lineStream, tile, ',');
                    unsigned char val = (unsigned char)atoi(tile.c_str());
                    if(val > 0) {
                        // be careful, the tiles in this format are indexed from 1 not 0
                        levelData[y][x] = val-1;
                    } else {
                        levelData[y][x] = 0;
                    }
                    
                }
            }
        }
       
    }
    
    return true;
}




// Globals
enum GameMode {MAIN_MENU, LEVEL1, LEVEL2, LEVEL3, WIN, GAMEOVER};
GameMode mode;

struct GameState {
    Entity player;
    Entity playerBullet;
    std::vector<Entity> enemies;
    std::vector<Entity> enemyBullets;
};

GameState state;

ShaderProgram program;

GLuint font;
GLuint worldSheet;

SDL_Event event;
bool done;

void Setup();
void ProcessEvents();
void Update(float timestep);
void Render();
void applyLighting();








void placeEntity(string type, float x, float y) {
    if (type == "Player") {
        state.player = Entity(worldSheet, x-projectionX, y+projectionY, 0.0625f, 0.0625f, 0.5f, 0.5f, 0.0f, -1.0f, 0.12f, 0.12f, Player, 200);
    } else if (type == "Enemy"){
        state.enemies.push_back(Entity(worldSheet, x-projectionX, y+projectionY, 0.15f, 0.2f, 0.25f, 0.0f, 1.0f, 0.0f, 0.12f, 0.12f, Enemy, 100));
    } else if (type == "EnemyV") {
        state.enemies.push_back(Entity(worldSheet, x-projectionX, y+projectionY, 0.15f, 0.2f, 0.00f, 0.25f, 0.0f, 1.0f, 0.12f, 0.12f, Enemy, 100));
    }else if (type == "Boss") {
        state.enemies.push_back(Entity(worldSheet, x-projectionX, y+projectionY, 0.15f, 0.2f, 0.25f, 0.0f, -1.0f, 0.0f, 0.18f, 0.18f, Boss, 200));
    }
}



bool readEntityData(std::ifstream &stream) {
    
    string line;
    string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        istringstream sStream(line);
        string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            istringstream lineStream(value);
            string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            float placeX = atoi(xPosition.c_str())*TILE_SIZE;
            float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
            placeEntity(type, placeX, placeY);
        }
    }
    return true;
    
}





void readTileMap() {
    
    ifstream infile;
switch(mode) {
    case MAIN_MENU: {
        infile = ifstream(RESOURCE_FOLDER"Level1.txt");
    break;
    } case LEVEL1: {
        infile = ifstream(RESOURCE_FOLDER"Level1.txt");
    break;
    } case LEVEL2: {
        infile = ifstream(RESOURCE_FOLDER"Level2.txt");
    break;
    
    } case LEVEL3: {
        infile = ifstream(RESOURCE_FOLDER"Level3.txt");
    break;
    }
}

    string line;
    while (getline(infile, line)) {
        if(line == "[header]") {
            if(!readHeader(infile)) {
                return;
            }
        } else if(line == "[layer]") {
            readLayerData(infile);
        } else if(line == "[ObjectLayer1]") {
            readEntityData(infile);
        }
    }
    
}



void genTileMap() {
    
   
    
    float spriteWidth = 1.0f/(float)spriteCountX;
    float spriteHeight = 1.0f/(float)spriteCountY;
    for(int y=0; y < LEVEL_HEIGHT; y++) {
        for(int x=0; x < LEVEL_WIDTH; x++) {
            
            if(levelData[y][x] != 0) {
                float u = (float)(((int)levelData[y][x]) % spriteCountX) / (float) spriteCountX;
                float v = (float)(((int)levelData[y][x]) / spriteCountX) / (float) spriteCountY;
                vertexxData.insert(vertexxData.end(), {
                    
                    TILE_SIZE * x, -TILE_SIZE * y,
                    TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    
                    TILE_SIZE * x, -TILE_SIZE * y,
                    (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                    (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
                });
                
                texxCoordData.insert(texxCoordData.end(), {
                    u, v,
                    u, v+ spriteHeight,
                    u+spriteWidth, v+(spriteHeight),
                    
                    u, v,
                    u+spriteWidth, v+(spriteHeight),
                    u+spriteWidth, v
                });
            }
        }
    }
    
}










void renderTileMap() {
 
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 2.0, 0.0f));
    program.SetModelMatrix(modelMatrix);
  
    int fontTexture = worldSheet;
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexxData.data());
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texxCoordData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, vertexxData.size()/2);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
}



void resetGame() {

   
    levelData = NULL;
    
    texxCoordData.clear();
    vertexxData.clear();
    
    state.enemies.clear();
    state.enemyBullets.clear();

    readTileMap();
    genTileMap();
    
    state.playerBullet = Entity(worldSheet, 200.0f, 20.0f, 0.0625f, 0.0625f, 1.0f, 1.0f, 0.0f, 0.0f, 0.10f, 0.10f, playerBullet, 0);
    for (int i = 0; i < state.enemies.size(); i++) {
        state.enemyBullets.push_back(Entity(worldSheet,200.0f, 200.0f, 0.0625f, 0.0625f, 0.5f, 0.5f, 0.0f, 0.0f, 0.12f, 0.12f, enemyBullet, 0));
    }
    animationTime = 0.0f;
}









int main(int argc, char *argv[]) {
    Setup();
    resetGame();
    
    
    float ticks = 0.0f;
    float elapsed = 0.0f;
    float lastFrameTicks = 0.0f;
    
    
    while (!done) {
        ProcessEvents();
        
        applyLighting();
        

        glClear(GL_COLOR_BUFFER_BIT);
        ticks = (float)SDL_GetTicks() / 1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
        }
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        
        
        Render();
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    Mix_FreeChunk(playerShoot);
    Mix_FreeChunk(enemyShoot);
    Mix_FreeChunk(hit);
    Mix_FreeMusic(loop);
    SDL_Quit();
    return 0;
}


void mainMenuRender(ShaderProgram &program, GLuint &font) {
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(xPosTitle, yPosTitle, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Dungeons", 0.2, 0.001);
   
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(xPosPlay, yPosPlay, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Press P to play", 0.1, -0.001);
   
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(xPosMove, yPosMove, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Move with Arrows", 0.1, -0.001);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(xPosAttack, yPosAttack, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Space to attack", 0.1, -0.001);
}

void gameOverRender(ShaderProgram &program, GLuint &font) {
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.75f, 1.0f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Game Over", 0.2, 0.001);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.68f, 0.6f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Press Q to Exit", 0.1, -0.001);
    
}

void winRender(ShaderProgram &program, GLuint &font) {
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.68f, 1.0f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "YOU WON", 0.2, 0.001);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.68f, 0.6f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Press Q to Exit", 0.1, -0.001);
    
}




void LevelTextRender(ShaderProgram &program, GLuint &font) {
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.35f, 1.8f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    string text = "";
    if (mode == LEVEL1) {
        text = "Level: 1";
    } else if (mode == LEVEL2) {
        text = "Level: 2";
    } else if (mode == LEVEL3) {
        text = "Level: 3";
    }
    DrawText(program, font, text, 0.1, 0.001);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(1.20f, 1.9f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "Q to Exit", 0.08, 0.0005);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.80f, 1.5f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, "HP: " + to_string(state.player.health), 0.08, 0.0005);
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.80f, 1.40f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    int bossHealth = 0;
    for (int i = 0; i <= state.enemies.size(); i++) {
        if (state.enemies[i].entityType == Boss) {
            bossHealth = state.enemies[i].health;
        }
    }
    DrawText(program, font, "Boss HP: " + to_string(bossHealth), 0.08, 0.0005);
    
    
    
}





void Setup() {
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Final Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 736, 736, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 736, 736);
    
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-projectionX, projectionX, -projectionY, projectionY, -projectionY, projectionY);
    
    glUseProgram(program.programID);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    done = false;
    
    worldSheet = LoadTexture(RESOURCE_FOLDER"Dungeon.png");
    font = LoadTexture(RESOURCE_FOLDER"font2.png");
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //glClearColor(0.1019f, 0.06666f, 0.08627f, 1.0f);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
    
    
    playerShoot = Mix_LoadWAV(RESOURCE_FOLDER"playerShoot.wav");
    enemyShoot = Mix_LoadWAV(RESOURCE_FOLDER"enemyShoot.wav");
    hit = Mix_LoadWAV(RESOURCE_FOLDER"hit.wav");
    loop = Mix_LoadMUS(RESOURCE_FOLDER"loop.wav");
    
    Mix_PlayMusic(loop, -1);
    Mix_VolumeMusic(50);
    
    mode = MAIN_MENU;
    
}

bool shouldRemoveEnemy(Entity& enemy) {
    return !(enemy.health > 0);
}


void ProcessEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}





void Update(float timestep) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    
    switch(mode) {
        case MAIN_MENU: {
        
        if (keys[SDL_SCANCODE_P]) {
            mode = LEVEL1;
        }
            
             animationTime = animationTime + timestep;
            
            float animationValueTitle = mapValue(animationTime,  0.0f, 3.0f, 0.0f, 1.0f);
            float animationValuePlay = mapValue(animationTime,  0.0f, 2.0f, 0.0f, 1.0f);
            float animationValueMove = mapValue(animationTime,  0.0f, 2.0f, 0.0f, 1.0f);
            float animationValueAttack = mapValue(animationTime,  0.0f, 2.0f, 0.0f, 1.0f);
            
            
             xPosTitle = easeOutElastic(5.0f, -0.68f, animationValueTitle);
             yPosTitle = easeOutElastic(5.0f, 1.00f, animationValueTitle);
            
            xPosPlay = easeInOut(-5.0f, -0.68f, animationValuePlay);
            yPosPlay = easeInOut(0.60f, 0.60f, animationValuePlay);
            
            xPosMove = easeInOut(-0.75f, -0.75f, animationValueMove);
            yPosMove = easeInOut(-5.0f, 0.20f, animationValueMove);
            
            xPosAttack = lerp(-0.68f, -0.68f, animationValueAttack);
            yPosAttack = lerp(-5.0f, 0.10f, animationValueAttack);
        
            
        break;
        }
    case LEVEL1:
        renderTileMap();
        state.player.Update(timestep);
        state.playerBullet.Update(timestep);
            
        
        for (int i = 0; i <= state.enemies.size()-1; i++) {
             state.enemies[i].Update(timestep);
             state.enemyBullets[i].Update(timestep);
        }
        
        state.enemies.erase(std::remove_if(state.enemies.begin(), state.enemies.end(), shouldRemoveEnemy), state.enemies.end());
           
        if (state.enemies.size() == 0) {
            mode = LEVEL2;
            resetGame();
        }
        if (state.player.health <= 0) {
            mode = GAMEOVER;
        }
        if (keys[SDL_SCANCODE_Q]) {
            mode = MAIN_MENU;
            resetGame();
        }
            
        break;
    case LEVEL2:
            renderTileMap();
            state.player.Update(timestep);
            state.playerBullet.Update(timestep);
            
            
            for (int i = 0; i <= state.enemies.size()-1; i++) {
                state.enemies[i].Update(timestep);
                state.enemyBullets[i].Update(timestep);
            }
            
            state.enemies.erase(std::remove_if(state.enemies.begin(), state.enemies.end(), shouldRemoveEnemy), state.enemies.end());
            
            if (state.enemies.size() == 0) {
                mode = LEVEL3;
                resetGame();
            }
            if (state.player.health <= 0) {
                mode = GAMEOVER;
            }
            
            if (keys[SDL_SCANCODE_Q]) {
                mode = MAIN_MENU;
                resetGame();
            }
            
            
        
        
        break;
        
    case LEVEL3:
            renderTileMap();
            state.player.Update(timestep);
            state.playerBullet.Update(timestep);
            
            
            for (int i = 0; i <= state.enemies.size()-1; i++) {
                state.enemies[i].Update(timestep);
                state.enemyBullets[i].Update(timestep);
            }
            
            state.enemies.erase(std::remove_if(state.enemies.begin(), state.enemies.end(), shouldRemoveEnemy), state.enemies.end());
            
            if (state.enemies.size() == 0) {
                mode = WIN;
            }
            if (state.player.health <= 0) {
                mode = GAMEOVER;
            }
            if (keys[SDL_SCANCODE_Q]) {
                resetGame();
                mode = MAIN_MENU;
            }
        
        break;
        
    case WIN:
            
        
        winRender(program, font);
            if (keys[SDL_SCANCODE_Q]) {
                mode = MAIN_MENU;
                resetGame();
                
            }
        
        break;
        
    case GAMEOVER:
        
        gameOverRender(program, font);
            if (keys[SDL_SCANCODE_Q]) {
                mode = MAIN_MENU;
                resetGame();
            }
        break;
    }
    

}



void Render() {
   
    switch(mode) {
        case MAIN_MENU:
            mainMenuRender(program, font);
            break;
        case LEVEL1:
            renderTileMap();
            LevelTextRender(program, font);
            state.player.Render(program);
            state.playerBullet.Render(program);
            for (int i = 0; i <= state.enemies.size()-1; i++) {
                state.enemies[i].Render(program);
                state.enemyBullets[i].Render(program);
            }
            
            
            break;
        case LEVEL2:
            renderTileMap();
            LevelTextRender(program, font);
            state.player.Render(program);
            state.playerBullet.Render(program);
            for (int i = 0; i <= state.enemies.size()-1; i++) {
                state.enemies[i].Render(program);
                state.enemyBullets[i].Render(program);
            }
            
            break;
        
        case LEVEL3:
            renderTileMap();
            LevelTextRender(program, font);
            state.player.Render(program);
            state.playerBullet.Render(program);
            for (int i = 0; i <= state.enemies.size()-1; i++) {
                state.enemies[i].Render(program);
                state.enemyBullets[i].Render(program);
            }
            
            break;
            
        case WIN:
            
            break;
        
        case GAMEOVER:
            
            break;
    }
    
}





void Entity::Update(float timestep) {
    
    if (entityType == Player) {
        
        
        checkCollisionsX();
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_RIGHT] ) {
            acceleration.x = 1;
        } else if (keys[SDL_SCANCODE_LEFT] ) {
            acceleration.x = -1;
        } else {
            acceleration.x = 0;
            
        }
        
        
        checkCollisionsY();
        
        
        if (keys[SDL_SCANCODE_UP]) {
            acceleration.y = 1;
        } else if (keys[SDL_SCANCODE_DOWN]) {
            acceleration.y = -1;
        } else {
            acceleration.y = 0;
        }
        
        
        for (int i = 0; i <  state.enemies.size(); i ++) {
            float distance = sqrt(pow(position.x - state.enemies[i].position.x, 2)+pow(position.y - state.enemies[i].position.y, 2));
            
            if (distance <= 0.5 && keys[SDL_SCANCODE_SPACE] && !state.playerBullet.bulletActive) {
                Mix_PlayChannel( -1, playerShoot, 0);
                state.playerBullet.bulletActive = true;
                state.playerBullet.position.x = position.x;
                state.playerBullet.position.y = position.y;
                state.playerBullet.acceleration.x = state.enemies[i].position.x - position.x;
                state.playerBullet.acceleration.y = state.enemies[i].position.y - position.y;
            }
            
            if (CollidesWith(state.enemies[i])) {
                mode = GAMEOVER;
            }
        }
        
      
    
        
        position.x += velocity.x * acceleration.x * timestep;
        position.y += velocity.y * acceleration.y * timestep;
        
    } else if (entityType == Enemy || entityType == Boss) {
        
        
        int gridX;
        int gridY;
        worldToTileCoordinates(position.x+ 2.0f + (size.x /2), position.y-2.0f, gridX, gridY);
        collidedRight = !stepsOnFloor(gridX, gridY);
        if (collidedRight) {
            acceleration.x = -1;
        }
        
        worldToTileCoordinates(position.x+ 2.0f - (size.x /2), position.y-2.0f, gridX, gridY);
        collidedLeft = !stepsOnFloor(gridX, gridY);
        if (collidedLeft) {
            acceleration.x = 1;
        }
        
        int bulletIndex = -1; // find an unused bullet
        for (int i = 0; i <  state.enemyBullets.size(); i ++) {
            if (!state.enemyBullets[i].bulletActive) {
                bulletIndex = i;
                break;
            }
        }
        
        
    
        worldToTileCoordinates(position.x + 2.0f, position.y-2.0f + (size.y /2), gridX, gridY);
        collidedTop = !stepsOnFloor(gridX, gridY);
        if (collidedTop) {
            acceleration.y = -1;
        }
        
        worldToTileCoordinates(position.x + 2.0f, position.y-2.0f - (size.y / 2), gridX, gridY);
        collidedBottom = !stepsOnFloor(gridX, gridY);
        if (collidedBottom) {
            acceleration.y = 1;
        }
        
        
        
        
        float distance = sqrt(pow(position.x - state.player.position.x, 2)+pow(position.y - state.player.position.y, 2));
        
        if (distance <= 0.7 && firedBulletIndex == -1) {
            Mix_PlayChannel( -1, enemyShoot, 0);
            bulletActive = true;
            firedBulletIndex = bulletIndex;
            state.enemyBullets[bulletIndex].bulletActive = true;
            state.enemyBullets[bulletIndex].position.x = position.x;
            state.enemyBullets[bulletIndex].position.y = position.y;
            state.enemyBullets[bulletIndex].acceleration.x = state.player.position.x - position.x;
            state.enemyBullets[bulletIndex].acceleration.y = state.player.position.y - position.y;
        }
        
        
        position.x += velocity.x * acceleration.x * timestep;
        position.y += velocity.y * acceleration.y * timestep;
        
        
    } else if (entityType == playerBullet){
        
        
        if (bulletActive) {
            int gridX;
            int gridY;
            worldToTileCoordinates(position.x+ 2.0f + (size.x /2), position.y-2.0f, gridX, gridY);
            collidedRight = !stepsOnFloor(gridX, gridY);
            worldToTileCoordinates(position.x+ 2.0f - (size.x /2), position.y-2.0f, gridX, gridY);
            collidedLeft = !stepsOnFloor(gridX, gridY);
            worldToTileCoordinates(position.x + 2.0f, position.y-2.0f + (size.y /2), gridX, gridY);
            collidedTop = !stepsOnFloor(gridX, gridY);
            worldToTileCoordinates(position.x + 2.0f, position.y-2.0f - (size.y / 2), gridX, gridY);
            collidedBottom = !stepsOnFloor(gridX, gridY);
            
            if (collidedRight || collidedLeft || collidedTop || collidedBottom) {
                bulletActive = false;
                position.x = 200.0f;
                position.y = 200.0f;
                acceleration.x = 0.0f;
                acceleration.y = 0.0f;
            }
            
            for (int i = 0; i < state.enemies.size(); i++) {
                if (CollidesWith(state.enemies[i])) {
                    bulletActive = false;
                    position.x = 200.0f;
                    position.y = 200.0f;
                    acceleration.x = 0.0f;
                    acceleration.y = 0.0f;
                    state.enemies[i].health -= 50;
                    Mix_PlayChannel( -1, hit, 0);
                }
            }
            
            position.x += velocity.x * acceleration.x * timestep;
            position.y += velocity.y * acceleration.y * timestep;
            
        }
    } else if (entityType == enemyBullet) {
        
        
        if (bulletActive) {
            int gridX;
            int gridY;
            worldToTileCoordinates(position.x+ 2.0f + (size.x /2), position.y-2.0f, gridX, gridY);
            collidedRight = !stepsOnFloor(gridX, gridY);
            worldToTileCoordinates(position.x+ 2.0f - (size.x /2), position.y-2.0f, gridX, gridY);
            collidedLeft = !stepsOnFloor(gridX, gridY);
            worldToTileCoordinates(position.x + 2.0f, position.y-2.0f + (size.y /2), gridX, gridY);
            collidedTop = !stepsOnFloor(gridX, gridY);
            worldToTileCoordinates(position.x + 2.0f, position.y-2.0f - (size.y / 2), gridX, gridY);
            collidedBottom = !stepsOnFloor(gridX, gridY);
            
            bool collidedPlayer = CollidesWith(state.player);
            
            if (collidedRight || collidedLeft || collidedTop || collidedBottom || collidedPlayer) {
             bulletActive = false;
             position.x = 200.0f;
             position.y = 200.0f;
             acceleration.x = 0.0f;
             acceleration.y = 0.0f;
                int myIndex = -1;
                for (int i = 0; i < state.enemyBullets.size(); i++) {
                    if (this == &state.enemyBullets[i]) {
                        myIndex = i;
                    }
                }
                for (int i = 0; i < state.enemies.size(); i++) {
                    if (state.enemies[i].firedBulletIndex == myIndex) {
                        state.enemies[i].bulletActive = false;
                        state.enemies[i].firedBulletIndex = -1;
                    }
                }
                
                if (collidedPlayer) {
                    state.player.health -= 50;
                    Mix_PlayChannel( -1, hit, 0);
                }
                
             }
            
        }
       
        position.x += velocity.x * acceleration.x * timestep;
        position.y += velocity.y * acceleration.y * timestep;
        
    }
}



void applyLighting() {
    
   
    if (mode == LEVEL1 || mode == LEVEL2 || mode == LEVEL3) {
        GLint lightPositionsUniform = glGetUniformLocation(program.programID, "lightPositions");
        GLfloat lightPositions[8];
        
        lightPositions[0] = state.player.position.x;
        lightPositions[1] = state.player.position.y;
        
        lightPositions[2] = 1.5f;
        lightPositions[3] = 1.9f;
        
        lightPositions[4] = -1.40f;
        lightPositions[5] = 1.45f;
        
        lightPositions[6] = 0.0f;
        lightPositions[7] = 1.8f;
        
        glUniform2fv(lightPositionsUniform, 8, lightPositions);
        
    } else if (mode == MAIN_MENU) {
        
        GLint lightPositionsUniform = glGetUniformLocation(program.programID, "lightPositions");
        GLfloat lightPositions[8];
        
        lightPositions[0] = 0.0f;
        lightPositions[1] = 1.0f;
        
        lightPositions[2] = 0.0f;
        lightPositions[3] = 0.8f;
        
        lightPositions[4] = 0.0f;
        lightPositions[5] = 0.25;
        
        lightPositions[6] = 0.0f;
        lightPositions[7] = 0.2f;
        
        glUniform2fv(lightPositionsUniform, 8, lightPositions);
        
    } else {
       GLint lightPositionsUniform = glGetUniformLocation(program.programID, "lightPositions");
        GLfloat lightPositions[8];
        lightPositions[0] = 0.40f;
        lightPositions[1] = 1.0f;
        
        lightPositions[2] = -0.0f;
        lightPositions[3] = 0.6;
        
        lightPositions[4] = -0.40f;
        lightPositions[5] = 1.0f;
        
        lightPositions[6] = -0.40f;
        lightPositions[7] = 1.0f;
        
        glUniform2fv(lightPositionsUniform, 8, lightPositions);
        
    }
    
}
