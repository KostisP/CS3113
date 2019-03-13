#include "ShaderProgram.h"



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
    
    Board(int x, int y, float rotation, int texture, float width, float height, float direction_x, float direction_y) :
    x(x), y(y), rotation(rotation), textureID(texture), width(width), height(height), direction_x(direction_x), direction_y(direction_y)
    {}
    
    void Draw(ShaderProgram &p) {
        float points[] = {-width, -height, width, -height, width, height, -width, -height, width, height, -width, height};
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, points);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    
};
