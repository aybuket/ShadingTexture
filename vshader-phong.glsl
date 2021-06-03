#version 410

in  vec4 vPosition;
in  vec4 vColor;
in  vec3 vNormal;
in  vec2 vTexCoord;

out vec4 color;
out vec2 texCoord;

// output values that will be interpretated per-fragment
out vec3 fN;
out vec3 fV;
out vec3 fL;

uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform mat4 Projection;
uniform int ShadowFlag;
uniform int TextureFlag;

void main()
{
    if (ShadowFlag == 1)
    {
        color = vec4(0.0, 0.0, 0.0, 0.1);
    }
    else if (TextureFlag == 1)
    {
        color       = vColor;
        texCoord    = vTexCoord;
    }
    else
    {
        // Transform vertex position into camera (eye) coordinates
        vec3 pos = (ModelView * vPosition).xyz;
    
        fN = (ModelView * vec4(vNormal, 0.0)).xyz; // normal direction in camera coordinates

        fV = -pos; //viewer direction in camera coordinates

        fL = LightPosition.xyz; // light direction

        if( LightPosition.w != 0.0 ) {
            fL = LightPosition.xyz - pos;  //directional light source
        }
    }
    gl_Position = Projection * ModelView * vPosition;
}
