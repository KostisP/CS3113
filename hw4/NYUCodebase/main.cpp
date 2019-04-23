// move left and right with arrows
// jump with space
// collect coins



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


#define LEVEL_HEIGHT 50
#define LEVEL_WIDTH 50

#define FLOOR 43

float TILE_SIZE = 0.08;

int spriteCountX = 16;
int spriteCountY = 8;

int mapWidth;
int mapHeight;

unsigned char** levelData = NULL;

std::vector<float> vertexxData;
std::vector<float> texxCoordData;



enum EntityType {Player, Coin};




const int coinRunAnimation[] = {52, 53, 54, 55, 56};
const int coinNumFrames = 3;
int coinCurrentIndex = 0;
float animationElapsed = 0.0f;
float framesPerSecond = 1.0f;


void worldToTileCoordinates(float worldX, float worldY, int &gridX, int &gridY) {
    gridX = (int)(worldX / 0.08f);
    gridY = (int)(worldY / -0.08f);
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
    int coinsCollected;
    
    Entity() {}
    
    Entity(GLuint textureID, float x, float y, float width, float height, float velocityx, float velocityy,
           float accx, float accy, float sizex, float sizey, EntityType type) : entityType(type), textureID(textureID), width(width), height(height)   {
        
        position.x = x;
        position.y = y;
        size.x = sizex;
        size.y = sizey;
        velocity.x = velocityx;
        velocity.y = velocityy;
        acceleration.x = accx;
        acceleration.y = accy;
        
        if (entityType == Player) {
            coinsCollected = 0;
        }
        
        isStatic = (entityType == Coin);
    }
    
    
    
    void Update(float timestep) {
        if (entityType == Player) {
            int gridX;
            int gridY;
            worldToTileCoordinates(position.x, position.y - 0.5 * size.y, gridX, gridY);
            
            
            // LEFT RIGHT MOVEMENT
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_RIGHT]  && position.x + 0.5 * size.x   <= projectionX) {
                acceleration.x = 1;
            } else if (keys[SDL_SCANCODE_LEFT] && position.x -  0.5 *  size.x  >=
                       -projectionX) {
                acceleration.x = -1;
            } else {
                acceleration.x = 0;
            }
            
            
            // jump
            if (keys[SDL_SCANCODE_SPACE] && gridY == FLOOR) {
                velocity.y = 1.0;
            }
            
            
            // Determines if it has hit the floor
            if (gridY == FLOOR && !keys[SDL_SCANCODE_SPACE]) {
                velocity.y = 0;
            } else {
                velocity.y += timestep * acceleration.y;
            }
            
            position.x += velocity.x * acceleration.x * timestep;
            position.y += velocity.y * timestep;
            
        } else {
            //coin animation
            animationElapsed += timestep;
            if(animationElapsed > 1.0/framesPerSecond) {
                coinCurrentIndex++;
                animationElapsed = 0.0;
                if(coinCurrentIndex > coinNumFrames-1) {
                    coinCurrentIndex = 0;
                }
            }
        }
    };
    
    
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
            index = 99;
        } else if (entityType == Coin){
            index = coinRunAnimation[coinCurrentIndex];
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






struct GameState {
    Entity player;
    std::vector<Entity> coins;
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








void placeEntity(string type, float x, float y) {
    if (type == "Player") {
        state.player = Entity(worldSheet, x, y, 0.0625f, 0.0625f, 0.5f, 0.0f, 0.0f, -1.0f, 0.1f, 0.1f, Player);
    } else if (type == "Coin"){
        state.coins.push_back(Entity(worldSheet, x-projectionX, y + 0.2f, 0.15f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.1f, Coin));
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
    
    ifstream infile(RESOURCE_FOLDER"myMap.txt");
    string line;
    while (getline(infile, line)) {
        if(line == "[header]") {
            if(!readHeader(infile)) {
                return;
            }
        } else if(line == "[layer]") {
            readLayerData(infile);
        } else if(line == "[Object Layer 1]") {
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
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.0f, 0.0, 0.0f));
    program.SetModelMatrix(modelMatrix);
    
    int fontTexture = worldSheet;
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexxData.data());
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texxCoordData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6*50*50);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    
}

void displayCoins() {
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix,  glm::vec3(state.player.position.x-0.01, state.player.position.y + 0.1f, 0.0f));
    program.SetModelMatrix(modelMatrix);
    DrawText(program, font, std::to_string(state.player.coinsCollected), 0.1, 0.0f);
    
}



int main(int argc, char *argv[]) {
    Setup();
    readTileMap();
    genTileMap();
    
    float ticks = 0.0f;
    float elapsed = 0.0f;
    float lastFrameTicks = 0.0f;
    
    while (!done) {
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
        
        ProcessEvents();
        Render();
        
        SDL_GL_SwapWindow(displayWindow);
    }
    SDL_Quit();
    return 0;
}




void Setup() {
    
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("HW4", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 800, 800);
    
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-projectionX, projectionX, -projectionY, projectionY, -projectionY, projectionY);
    
    glUseProgram(program.programID);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    done = false;
    
    worldSheet = LoadTexture(RESOURCE_FOLDER"arne_sprites.png");
    font = LoadTexture(RESOURCE_FOLDER"font2.png");
    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.3f, 0.8f, 0.9f, 1.0f);
}




void ProcessEvents() {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}





void Update(float timestep) {
    state.player.Update(timestep);
    for (int i = 0; i <= state.coins.size()-1; i++) {
        state.coins[i].Update(timestep);
    }
    
    for (int i = 0; i <= state.coins.size(); i++) {
        if (state.player.CollidesWith(state.coins[i])) {
            state.player.coinsCollected++;
            state.coins[i].position.y = 200.0;
        }
    }
}




void Render() {
    
    renderTileMap();
    state.player.Render(program);
    for (int i = 0; i <= state.coins.size()-1; i++) {
        state.coins[i].Render(program);
    }
    
    displayCoins();
    
    glm::mat4 viewMatrixPlayer = glm::mat4(1.0f);
    viewMatrixPlayer = glm::translate(viewMatrixPlayer, glm::vec3(-state.player.position.x, -state.player.position.y, 0.0f));
    viewMatrixPlayer = glm::scale(viewMatrixPlayer, glm::vec3(1.4f, 1.35f, 0.0f));
    program.SetViewMatrix(viewMatrixPlayer);
    
}









