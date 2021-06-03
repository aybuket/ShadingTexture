#version 410
in  vec4 color;
out vec4 fcolor;

in  vec2 texCoord;
uniform int TextureFlag;
uniform int ShadowFlag;
uniform sampler2D tex;

void main() 
{
    if (ShadowFlag == 1)
    {
        fcolor = color;
    }
    else if (TextureFlag == 1)
    {
        //sample a texture color from texture object
        fcolor = texture(tex, texCoord );
    }else{
        fcolor = color;
    }
} 

