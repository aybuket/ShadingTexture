#version 410

in  vec4 vPosition;
in  vec4 vColor;
in  vec3 vNormal;
in  vec2 vTexCoord;

out vec4 color;
out vec2 texCoord;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;
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
	
        vec3 L = normalize( LightPosition.xyz - pos ); //light direction
        vec3 V = normalize( -pos ); // viewer direction
        vec3 H = normalize( L + V ); // halfway vector

        // Transform vertex normal into camera coordinates
        vec3 N = normalize( ModelView * vec4(vNormal, 0.0) ).xyz;

        // Compute terms in the illumination equation
        vec4 ambient = AmbientProduct;

        float Kd = max( dot(L, N), 0.0 ); //set diffuse to 0 if light is behind the surface point
        vec4  diffuse = Kd*DiffuseProduct;

        float Ks = pow( max(dot(N, H), 0.0), Shininess );
        vec4  specular = Ks * SpecularProduct;
    
        //ignore also specular component if light is behind the surface point
        if( dot(L, N) < 0.0 ) {
            specular = vec4(0.0, 0.0, 0.0, 1.0);
        }

        color = ambient + diffuse + specular;
        color.a = 1.0;
    }
    gl_Position = Projection * ModelView * vPosition;

}
