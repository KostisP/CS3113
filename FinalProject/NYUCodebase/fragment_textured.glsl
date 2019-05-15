/*
uniform sampler2D diffuse;
varying vec2 texCoordVar;

void main() {
    gl_FragColor = texture2D(diffuse, texCoordVar);
}
*/

uniform sampler2D diffuse;
uniform vec2 lightPositions[8];
varying vec2 texCoordVar;
varying vec2 varPosition;
float attenuate(float dist, float a, float b) {
    return 1.0 / (1.0 + a*dist + b*dist*dist);
}
void main()
{
    float brightness = 0.0;
    
    for(int i=0; i < 8; i++) {
        brightness += attenuate(distance(lightPositions[i], varPosition), 2.0, 8.0);
    }
    

    
    vec4 textureColor = texture2D(diffuse, texCoordVar);
    gl_FragColor = textureColor * brightness;
    gl_FragColor.a = textureColor.a;
}
