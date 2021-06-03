//
//  main.cpp
//  graphics_bouncing_ball
//
//  Created by Aybüke Buket Akgül on 28.03.2021.
//  Copyright © 2021 Aybüke Buket Akgül. All rights reserved.
//

#include "Angel.h"
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
typedef vec4  point4;
typedef vec4 color4;

// Default values
GLenum drawing_mode = GL_TRIANGLES;

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection, ShadingMode;
float ambient_mode  = 1.0;
float diffuse_mode  = 1.0;
float specular_mode = 1.0;
color4 ambient_product, diffuse_product, specular_product;

// buffers
GLuint sphereBuffer, bunnyBuffer;

// vertex array, attributes, program
GLuint vao[3], vPosition, vNormal, vColor, vTexCoord, program;

const int NumTimesToSubdivide   = 5;
const int NumTrianglesOfSphere  = 4096;
const int sphereNumVertices     = 3 * NumTrianglesOfSphere;
float aspect;

point4 * bunnyVertices;
point4 * bunnyPoints;
color4 * bunnyNormals;
int bunnyVertexNum;
int bunnyPointNum;
vec2 * bunnyTexCoords;

point4 sphereVertices[sphereNumVertices];
point4 spherePoints[sphereNumVertices];
color4 sphereColors[sphereNumVertices];
vec3   sphereNormals[sphereNumVertices];
vec2   sphereTexCoords[sphereNumVertices];

// Texture objects and storage for texture image
GLuint textures[3];

GLubyte image1[512][256][3];
GLubyte image2[2048][1024][3];

int TextureWidth[2] = {512, 2048};
int TextureHeight[2] = {256, 1024};

// original size
float original_x = 512;
float original_y = 512;

// initial point
float a_init = -original_x/20; float a = -original_x/20;
float b = original_y/12; float ceeling = original_y/12; float ceeling_init = original_y/12;
float gravity = -0.98;

// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

bool textureFlag = false; //enable texture mapping
GLuint  TextureFlagLoc, ShadowFlagLoc; // texture flag uniform location

mat4 M;
color4 light_position(-2.0,1.0,1.0, 1.0 );
bool fixedLight = true;
bool shadingFlag = true;
bool shadowFlag = true;

// enums for menu selection
enum ObjectType { Sphere, Bunny };
enum DrawingMode { Wireframe, Shading, Texture };
enum Movement { Jump, Turn };
enum ComponentMode { Ambient, Diffuse, Specular };
enum Light { Fixed, Moving };
enum Shading { Gouraud, Phong };

color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
float  material_shininess = 100.0;

// define current object and related info to use globally
ObjectType currentObject = Sphere;
int currentCount = sphereNumVertices;
Movement movement_mode = Turn;

//----------------------------------------------------------------------

vec2 UVcoordinates(point4 a)
{
    float radius = sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
    
    float u = acos(a.y / radius) / (2*M_PI);
    float v = acos( a.z / (radius * sin(2 *M_PI * u)))/(2*M_PI);
    
    if ( a.x < 0.0)
    {
        u = 0.5+u;
        v = 1.0-v;
    }
    else if (a.x == 0)
    {
        v = a.z > 0 ? a.z : 1+a.z;
        std::cout << a << " " << vec2(u,v) << endl;
    }
    
    return vec2(u,v);
}

int sphereIndex = 0;
void
triangle( const point4& a, const point4& b, const point4& c )
{
    sphereTexCoords[sphereIndex] = UVcoordinates(a);
    sphereNormals[sphereIndex] = vec3(a[0], a[1], a[2]);
    sphereColors[sphereIndex] = color4(1.0,0.5,0.5,1.0);
    spherePoints[sphereIndex] = a;  sphereIndex++;
    
    sphereTexCoords[sphereIndex] = UVcoordinates(b);
    sphereNormals[sphereIndex] = vec3(b[0], b[1], b[2]);
    sphereColors[sphereIndex] = color4(1.0,0.5,0.5,1.0);
    spherePoints[sphereIndex] = b;  sphereIndex++;
    
    sphereTexCoords[sphereIndex] = UVcoordinates(c);
    sphereNormals[sphereIndex] = vec3(c[0], c[1], c[2]);
    sphereColors[sphereIndex] = color4(1.0,0.5,0.5,1.0);
    spherePoints[sphereIndex] = c;  sphereIndex++;
}
//----------------------------------------------------------------------
point4
unit( const point4& p )
{
    float len = p.x*p.x + p.y*p.y + p.z*p.z;
    point4 t;
    if ( len > DivideByZeroTolerance ) {
        t = p / sqrt(len);
        t.w = 1.0;
    }
    return t;
}

void
divide_triangle( const point4& a, const point4& b,
       const point4& c, int count )
{
    if ( count > 0 ) {
        point4 v1 = unit( a + b );
        point4 v2 = unit( a + c );
        point4 v3 = unit( b + c );
        divide_triangle(  a, v1, v2, count - 1 );
        divide_triangle(  c, v2, v3, count - 1 );
        divide_triangle(  b, v3, v1, count - 1 );
        divide_triangle( v1, v3, v2, count - 1 );
    }
    else
    {
        triangle( a, b, c );
    }
}

void
tetrahedron( int count )
{
    point4 v[4] = {
        vec4( 0.0, 0.0, 1.0, 1.0 ),
        vec4( 0.0, 0.942809, -0.333333, 1.0 ),
        vec4( -0.816497, -0.471405, -0.333333, 1.0 ),
        vec4( 0.816497, -0.471405, -0.333333, 1.0 )
};
    divide_triangle( v[0], v[1], v[2], count );
    divide_triangle( v[3], v[2], v[1], count );
    divide_triangle( v[0], v[3], v[1], count );
    divide_triangle( v[0], v[2], v[3], count );
}

//----------------------------------------------------------------------------
//initialize buffer for sphere object
void initialize_sphere_buffer(){
    glGenBuffers( 1, &sphereBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, sphereBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(spherePoints) + sizeof(sphereNormals) + sizeof(sphereColors) + sizeof(sphereTexCoords), NULL, GL_STATIC_DRAW );
    int offset = 0;
    glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(spherePoints), spherePoints );
    offset += sizeof(spherePoints);
    glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(sphereNormals), sphereNormals);
    offset += sizeof(sphereNormals);
    glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(sphereColors), sphereColors );
    offset += sizeof(sphereColors);
    glBufferSubData( GL_ARRAY_BUFFER, offset, sizeof(sphereTexCoords), sphereTexCoords );

    offset = 0;
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset) );
    
    offset += sizeof(spherePoints);
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset) );
    
    offset += sizeof(sphereNormals);
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset) );
    
    offset += sizeof(sphereColors);
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset) );
}

// initialize buffer for bunny object
void initialize_bunny_buffer()
{
    glGenBuffers( 1, &bunnyBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, bunnyBuffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(point4)*bunnyPointNum*2,
                 NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(point4)*bunnyPointNum, bunnyPoints );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(point4)*bunnyPointNum, sizeof(color4)*bunnyPointNum, bunnyNormals);
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)*bunnyPointNum) );
}

// initialize shader parameters
void initialize_shader_parameters(color4 material_ambient, color4 material_diffuse, color4 material_specular,float  material_shininess)
{
    // Initialize shader lighting parameters
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
    
    ambient_product = light_ambient * material_ambient;
    diffuse_product = light_diffuse * material_diffuse;
    specular_product = light_specular * material_specular;
    
    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, specular_product );
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position );
    glUniform1f( glGetUniformLocation(program, "Shininess"), material_shininess );
    glUniform1i( ShadingMode, true );
}

void init_shader()
{
    // Load shaders and use the resulting shader program
    if( shadingFlag == true)
    {
        program = InitShader( "vshader-gouraud.glsl", "fshader-gouraud.glsl" );
    }
    else
    {
        program = InitShader( "vshader-phong.glsl", "fshader-phong.glsl" );
    }
    glUseProgram( program );
    
    // Initialize the vertex position attribute from the vertex shader
    vPosition = glGetAttribLocation( program, "vPosition" );
    vNormal = glGetAttribLocation( program, "vNormal" );
    vColor = glGetAttribLocation( program, "vColor" );
    vTexCoord = glGetAttribLocation( program, "vTexCoord" );
    
    // Retrieve transformation uniform variable locations
    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );
    ShadingMode = glGetUniformLocation( program, "ShadingMode" );
    TextureFlagLoc = glGetUniformLocation( program, "TextureFlag" );
    ShadowFlagLoc = glGetUniformLocation( program, "ShadowFlag" );
    initialize_shader_parameters(material_ambient, material_diffuse, material_specular, material_shininess);
}

void read_bunny();
void read_texture(string str, int imageIndex);

void init_texture()
{
    read_texture("basketball.ppm", 0);
    read_texture("earth.ppm", 1);
    
    // Initialize texture objects
    glGenTextures( 3, textures );

    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[0], TextureHeight[0], 0,
          GL_RGB, GL_UNSIGNED_BYTE, image1 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //try here different alternatives
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //try here different alternatives

    glBindTexture( GL_TEXTURE_2D, textures[1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureWidth[1], TextureHeight[1], 0,
          GL_RGB, GL_UNSIGNED_BYTE, image2 );
    glGenerateMipmap(GL_TEXTURE_2D); // try also activating mipmaps for the second texture object
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        
    glBindTexture( GL_TEXTURE_1D, textures[2] );
    glTexImage1D( GL_TEXTURE_1D, 0, GL_RGB, TextureWidth[0], 0,
          GL_RGB, GL_UNSIGNED_BYTE, image1 );
    glGenerateMipmap(GL_TEXTURE_1D); // try also activating mipmaps for the second texture object
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    
    glBindTexture( GL_TEXTURE_2D, textures[0] ); //set current texture
}

void
init( void )
{
    // Subdivide a tetrahedron into a sphere
    tetrahedron( NumTimesToSubdivide );
    read_bunny();

    M = identity(); // shadow projection matrix initially an identity matrix
    M[3][2] = -1.0/light_position[2];
    M[3][3] = 0.0;
    
    init_shader();
    init_texture();
    
    // Create a vertex array object
    glGenVertexArrays( 2, vao );
    
    // Sphere
    glBindVertexArray( vao[0] );
    initialize_sphere_buffer();
    
    // Bunny
    glBindVertexArray( vao[1] );
    initialize_bunny_buffer();
    
    // Draw sphere first
    glBindVertexArray( vao[0] );
    
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_CULL_FACE);
    
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window
    
    const vec3 viewer_pos( 0.0, 0.0, 3.0 );
    mat4  model_view  = identity();
    /*
    if (movement_mode == Jump){
        //  Generate the model-view matrix, move both horizontal and vertical
        const vec3 displacement( 0.0+a, 0.0+b, 0.0 );
        if (currentObject == Bunny)
        {
            model_view = Translate(-viewer_pos)*(Translate( displacement ) * Scale(0.75, 0.75, 0.75) *
                                                 RotateX(90) *
                                                 RotateY(180) *
                                                 RotateZ(-50));  // Scale(), Translate(), RotateX(), RotateY(), RotateZ(): user-defined functions in mat.h
        }
        else {
            model_view = Translate(-viewer_pos)*(Translate( displacement ) * Scale(4, 4, 4));
        }
    }*/
    float scaling = 0.5;

    if (movement_mode == Turn)
    {
        if (currentObject == Bunny)
        {
            scaling = 0.025;
        }
        
        model_view = ( Translate( -viewer_pos ) * Scale(scaling, scaling, scaling) *
                 RotateX( Theta[Xaxis] ) *
                 RotateY( Theta[Yaxis] ) *
                 RotateZ( Theta[Zaxis] ) );
        
        if (!fixedLight)
        {
            light_position =  model_view * light_position;
            
        }
    }
    
    if (shadowFlag)
    {
        mat4 model_view2;
    
        if (fixedLight)
        {
            model_view2 = Translate(light_position[0], light_position[1],light_position[2])
        * Translate (-viewer_pos) *M * Translate(-light_position[0], -light_position[1], -light_position[2]);
        }
        else
        {
            model_view2 = model_view *Scale(0.5,0.5,0.5) * M;
        }
    
        glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view2);
        glUniform1i( ShadowFlagLoc, true );
        glUniform3f( vColor, 0.1, 0.1, 0.1 );
        // render shadow polygon
        glDrawArrays(GL_TRIANGLES, 0, currentCount);
    }
    
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, light_position );
    glUniform1f( glGetUniformLocation(program, "AmbientMode"), ambient_mode );
    glUniform1f( glGetUniformLocation(program, "DiffuseMode"), diffuse_mode );
    glUniform1f( glGetUniformLocation(program, "SpecularMode"), specular_mode );
    glUniform1i( ShadingMode, true );
    glUniform1i( TextureFlagLoc, textureFlag );
    glUniform1i( ShadowFlagLoc, false );
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view );

    glDrawArrays( drawing_mode, 0, currentCount );
    
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void reset();
void print_help();
bool stop = false;

void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
        case '1':
            glBindTexture( GL_TEXTURE_2D, textures[0] );
            break;
        case '2':
            glBindTexture( GL_TEXTURE_2D, textures[1] );
            break;
        case '3':
            glBindTexture( GL_TEXTURE_1D, textures[2] );
            break;
        case 'I':
        case 'i':
            reset(); // return to the inital point
            break;
        case 'H':
        case 'h':
            print_help(); // print info to console
            break;
        case 'Q':
        case 'q':
            exit( EXIT_SUCCESS ); // quit
            break;
        case 's':
        case 'S':
            stop = !stop; //stop&start animation
            break;
        case 'd':
        case 'D':
            if (drawing_mode == GL_LINES)
            {
                drawing_mode = GL_TRIANGLES;
            }
            else
            {
                drawing_mode = GL_LINES;
            }
            break;
    }
}

// print helper information to console.
void print_help()
{
    std::cout << "------------------------------" << std::endl;
    std::cout << "Right click to choose objects:" << std::endl;
    std::cout << "-- Sphere, Bunny" << std::endl;
    std::cout << "Right click to choose mode:" << std::endl;
    std::cout << "--Wireframe, Shading, Texture" << std::endl;
    std::cout << "Right click to change movement mode:" << std::endl;
    std::cout << "--Turn, Jump" << std::endl;
    std::cout << "Right click to choose shading type:" << std::endl;
    std::cout << "--Gouraud, Phong" << std::endl;
    std::cout << "Right click to change light mode:" << std::endl;
    std::cout << "--Fixed, Moving" << std::endl;
    std::cout << "Right click to turn on/off:" << std::endl;
    std::cout << "--Ambient, Diffuse, Specular" << std::endl;
    std::cout << "------------------------------" << std::endl;
    std::cout << "Q/q : Quit" << std::endl;
    std::cout << "1/2/3 : Change texture" << std::endl;
    std::cout << "S/s : Stop/start animation" << std::endl;
    std::cout << "H/h : Helper" << std::endl;
    std::cout << "I/i : Initialize" << std::endl;
    std::cout << "D/d : Change drawing mode" << std::endl;
}

// return to the initial point
void reset(){
    a = a_init;
    b = ceeling_init;
    ceeling = ceeling_init;
    return;
}

//----------------------------------------------------------------------------
// Read .off file to create bunny
void read_bunny()
{
    string line;
    string filename ="bunny.off";
    ifstream myfile (filename);
    if (!myfile.is_open()) {
        std::cerr << "Could not open the file - '"
        << filename << "'" << endl;
    }
    
    myfile >> line;
    if ( line == "OFF" )
    {
        int edge_count;
        myfile >> bunnyVertexNum;
        myfile >> bunnyPointNum;
        myfile >> edge_count;
        
        // second part of the file contains indices of an triangle, so makes 3 points.
        bunnyPointNum*=3;
        bunnyVertices = new point4[bunnyVertexNum];
        bunnyPoints = new point4[bunnyPointNum];
        bunnyNormals = new point4[bunnyPointNum];
        
        // (x,y,z) coordinates of vertices.
        for ( int v = 0; v < bunnyVertexNum ; v++ )
        {
            float x; float y; float z;
            myfile >> x;
            myfile >> y;
            myfile >> z;
            bunnyVertices[v] = point4(x, y, z, 1.0);
        }
        
        // indices of triangles
        for ( int t = 0; t < bunnyPointNum ; t+=3 )
        {
            int n; int x; int y; int z;
            myfile >> n;
            myfile >> x;
            myfile >> y;
            myfile >> z;
            bunnyPoints[t] = bunnyVertices[x];
            bunnyPoints[t+1] = bunnyVertices[y];
            bunnyPoints[t+2] = bunnyVertices[z];

            vec4 u = bunnyPoints[t+1] - bunnyPoints[t];
            vec4 v = bunnyPoints[t+2] - bunnyPoints[t];

            vec3 normal = normalize( cross(u, v) );
            
            bunnyNormals[t] = normal;
            bunnyNormals[t+1] = normal;
            bunnyNormals[t+2] = normal;
        }
        myfile.close();
    }
}
//----------------------------------------------------------------------------
void read_texture(string filename, int imageIndex)
{
    int k, n, m;
    int red, green, blue;
    ifstream myfile (filename);
    if (!myfile.is_open()) {
        std::cerr << "Could not open the file - '"
        << filename << "'" << endl;
    }
    string line;
    myfile >> line;
    if(line[0]!='P'|| line[1] != '3'){
        std::cerr << filename << " is not a PPM file!\n" << endl;
        exit(0);
    }
    std::cout << filename << " is a PPM file!\n" << endl;
    myfile >> line;
    while(line[0] == '#')
    {
        myfile >> line;
        std::cout << line << endl;
    }
    n = stoi( line );
    myfile >> m;
    myfile >> k;
    
    std::cout << n << " rows " << m << " columns, max value:  " << k << endl;

    int i,j;
    for(j=0; j<m; j++)
    {
        for(i=0; i<n; i++)
        {
            myfile >> red >> green >> blue;
            if (imageIndex == 0)
            {
                image1[i][j][0]=red;
                image1[i][j][1]=green;
                image1[i][j][2]=blue;
            }else{
                image2[i][j][0]=red;
                image2[i][j][1]=green;
                image2[i][j][2]=blue;
            }
        
        }
    }
}

//----------------------------------------------------------------------------
void object_popup_handling(int item);
void mode_popup_handling(int item);
void movement_popup_handling(int item);
void component_popup_handling(int item);
void light_popup_handling(int item);
void shading_popup_handling(int item);
void shinning_popup_handling(int item);
void shadow_popup_handling(int item);

void popup_menu(){
    // Create a menu for object type
    int objectTypeMenu = glutCreateMenu(object_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("Sphere", Sphere);
    glutAddMenuEntry("Bunny", Bunny);
    
    // Create a menu for drawing mode
    int drawingModeMenu = glutCreateMenu(mode_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("Wireframe", Wireframe);
    glutAddMenuEntry("Shading", Shading);
    glutAddMenuEntry("Texture", Texture);
    
    // Create a menu for movement mode
    int movementModeMenu = glutCreateMenu(movement_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("Jump", Jump);
    glutAddMenuEntry("Turn", Turn);
    
    // Create a menu for component mode
    int componentModeMenu = glutCreateMenu(component_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("Turn on/off Ambient", Ambient);
    glutAddMenuEntry("Turn on/off Diffuse", Diffuse);
    glutAddMenuEntry("Turn on/off Specular", Specular);
    
    // Create a menu for light mode
    int shadingModeMenu = glutCreateMenu(shading_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("Gouraud", Gouraud);
    glutAddMenuEntry("Phong", Phong);
    
    // Create a menu for light mode
    int lightModeMenu = glutCreateMenu(light_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("Fixed", Fixed);
    glutAddMenuEntry("Moving", Moving);

    // Create a menu for light mode
    int shinningModeMenu = glutCreateMenu(shinning_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("Initial", 0);
    glutAddMenuEntry("Blue Plastic", 1);
    glutAddMenuEntry("Silver", 2);
    glutAddMenuEntry("Pearl", 3);
    glutAddMenuEntry("Green Rubber", 4);

    // Create a menu for shadow mode
    int shadowModeMenu = glutCreateMenu(shadow_popup_handling);
    
    // Add menu items
    glutAddMenuEntry("On", 0);
    glutAddMenuEntry("Off", 1);

    
    glutCreateMenu(NULL);
    glutAddSubMenu("Object Type", objectTypeMenu);
    glutAddSubMenu("Drawing Mode", drawingModeMenu);
    //glutAddSubMenu("Movement Mode", movementModeMenu);
    glutAddSubMenu("Component Mode", componentModeMenu);
    glutAddSubMenu("ShadingMode", shadingModeMenu);
    glutAddSubMenu("Light Mode", lightModeMenu);
    glutAddSubMenu("Material Mode", shinningModeMenu);
    glutAddSubMenu("Shadow", shadowModeMenu);
    
    // Associate a mouse button with menu
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
}

// Object selection handling. Sets the current object, bind the array and initializes the buffer of chosen object
// Also sets the position to the initial position.
void object_popup_handling(int item){
    switch (item) {
        case Sphere:
            currentCount = sphereNumVertices;
            currentObject = Sphere;
            glBindVertexArray( vao[0] );
            initialize_sphere_buffer();
            reset();
            break;
        case Bunny:
            currentCount = bunnyPointNum;
            currentObject = Bunny;
            glBindVertexArray( vao[1] );
            initialize_bunny_buffer();
            reset();
            break;
        default:
            reset();
            break;
    }
    glutPostRedisplay();
    return;
}

// Drawing mode handling, sets the drawing_mode to corresponding macro
void mode_popup_handling(int item){
    switch (item) {
        case Wireframe:
            drawing_mode = GL_LINES;
            textureFlag = false;
            break;
        case Shading:
            drawing_mode = GL_TRIANGLES;
            textureFlag = false;
            break;
        case Texture:
            drawing_mode = GL_TRIANGLES;
            textureFlag = true;
        default:
            break;
    }
    glUniform1i(TextureFlagLoc, textureFlag);
    glutPostRedisplay();
    return;
}

// Movement mode handling, sets the movement_mode to corresponding macro
void movement_popup_handling(int item){
    switch (item) {
        case Jump:
            movement_mode = Jump;
            reset();
            break;
        case Turn:
            movement_mode = Turn;
            break;
        default:
            break;
    }
    glutPostRedisplay();
    return;
}

// Component mode handling, sets the componet_mode to corresponding macro
void component_popup_handling(int item){
    switch (item) {
        case Ambient:
        {
            ambient_mode *= -1;
            color4 send_ambient = ambient_product;
            if (ambient_mode < 0.0)
            {
                send_ambient = vec4(0.0, 0.0, 0.0, 0.0);
            }
            glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1, send_ambient );
            break;
        }
        case Diffuse:
        {
            diffuse_mode *= -1;
            color4 send_diffuse = diffuse_product;
            if (diffuse_mode < 0.0)
            {
                send_diffuse = vec4(0.0, 0.0, 0.0, 0.0);
            }
            glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1, send_diffuse );
            break;
        }
        case Specular:
        {
            specular_mode *= -1;
            color4 send_specular = specular_product;
            if (specular_mode < 0.0)
            {
                send_specular = vec4(0.0, 0.0, 0.0, 0.0);
            }
            glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1, send_specular );
            break;
        }
        default:
            break;
    }
    
    glutPostRedisplay();
    return;
}

// Shading mode handling, sets the light_mode to corresponding macro
void shading_popup_handling(int item){
        switch (item) {
        case Gouraud:
                shadingFlag = true;
            glUniform1i( ShadingMode, shadingFlag);
            init_shader();
            break;
        case Phong:
                shadingFlag = false;
            glUniform1i( ShadingMode, shadingFlag);
            init_shader();
            break;
        default:
            break;
    }
    glutPostRedisplay();
    return;
}


// Light mode handling, sets the light_mode to corresponding macro
void light_popup_handling(int item){
        switch (item) {
        case Fixed:
            fixedLight = true;
            break;
        case Moving:
            fixedLight = false;
            break;
        default:
            break;
    }
    glutPostRedisplay();
    return;
}

// Light mode handling, sets the light_mode to corresponding macro
void shinning_popup_handling(int item){
        switch (item) {
        case 0:
            {
                material_ambient = color4( 1.0, 0.0, 1.0, 1.0 );
                material_diffuse = color4( 1.0, 0.8, 0.0, 1.0 );
                material_specular = color4( 1.0, 0.8, 0.0, 1.0 );
                material_shininess = 50.0;
                initialize_shader_parameters(material_ambient, material_diffuse, material_specular, material_shininess);
                break;
            }
        case 1:
            {
                material_ambient = color4( 0.0, 0.1, 0.06, 1.0 );
                material_diffuse = color4( 0.0, 0.50980392, 0.50980392, 1.0 );
                material_specular = color4( 0.50196078, 0.50196078, 0.50196078, 1.0 );
                material_shininess = 25.0;
                initialize_shader_parameters(material_ambient, material_diffuse, material_specular, material_shininess);
                break;
            }
        case 2:
            {
                material_ambient = color4( 0.19225, 0.19225, 0.19225, 1.0 );
                material_diffuse = color4( 0.50754, 0.50754, 0.50754, 1.0 );
                material_specular = color4( 0.508273, 0.508273, 0.508273, 1.0 );
                material_shininess = 40.0;
                initialize_shader_parameters(material_ambient, material_diffuse, material_specular, material_shininess);
                break;
            }
        case 3:
            {
                material_ambient = color4( 0.25, 0.20725, 0.20725, 1.0 );
                material_diffuse = color4( 1.0, 0.829, 0.829, 1.0 );
                material_specular = color4( 0.296648, 0.296648, 0.296648, 1.0 );
                material_shininess = 8.8;
                initialize_shader_parameters(material_ambient, material_diffuse, material_specular, material_shininess);
                break;
            }
        case 4:
            {
                material_ambient = color4( 0.0, 0.05, 0.0, 1.0 );
                material_diffuse = color4( 0.4, 0.5, 0.4, 1.0 );
                material_specular = color4( 0.04, 0.7, 0.04, 1.0 );
                material_shininess = 7.8125;
                initialize_shader_parameters(material_ambient, material_diffuse, material_specular, material_shininess);
                break;
            }
        default:
            break;
    }
    glutPostRedisplay();
    return;
}


// Light mode handling, sets the light_mode to corresponding macro
void shadow_popup_handling(int item){
        switch (item) {
        case 0:
            shadowFlag = true;
            break;
        case 1:
            shadowFlag = false;
            break;
        default:
            break;
    }
    glutPostRedisplay();
    return;
}


//---------------------------------------------------------------------
//
// reshape
//

void reshape( int w, int h )
{
    
    glViewport( 0, 0, w, h );
    aspect = GLfloat(w)/h;
    // Set projection matrix
    mat4  projection;
    projection = Perspective(45.0, aspect, 0.5, 6.0);
    
    if (movement_mode == Jump)
    {
        projection = Ortho(-51.2, 51.2, 0.0, 48.0, -10.0, 10.0);
    }
    
    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );
}


//----------------------------------------------------------------------------
float up = -1.0;
float diff = 5;
void
idle( void )
{
    if (movement_mode == Jump)
    {
        glUniformMatrix4fv( Projection, 1, GL_TRUE, Ortho(-51.2, 51.2, 0.0, 48.0, -10.0, 10.0) );
        // flag for stopping/starting animation
        if (!stop)
        {
            if (ceeling > 0)
            {
                // horizontal movement
                a += 0.50;
            
                // randomly chosen proportion to increase/decrease speed.
                float portion =ceeling/5;
                if (b < portion)
                {
                    b+=5*up;
                    diff=5;
                }
                else if (b < 2*portion)
                {
                    b+=4*up;
                }
                else if (b < 3*portion)
                {
                    b+=3*up;
                }
                else if (b < 4*portion)
                {
                    b+=2*up;
                }
                else
                {
                    b+=1*up;
                    diff = 3;
                }
            }
            else
            {
                b=0;
            }
        
            // Edges control
            if (a > -a_init || a < a_init)
            {
                a = -a;
            }
        
            // top ceeling cpntrol, change direction
            if (b > ceeling)
            {
                up *= -1.0;
            }
        
            // ground control, change direction and lower ceeling
            if(b <= 0)
            {
                ceeling = ceeling-diff;
                up *= -1.0;
            }
        }
    }
    
    if (movement_mode == Turn)
    {
        if (!stop)
        {
            glUniformMatrix4fv( Projection, 1, GL_TRUE, Perspective(45.0, aspect, 0.5, 6.0));
            Theta[Axis] += 3.0;

            if ( Theta[Axis] > 360.0 ) {
                Theta[Axis] -= 360.0;
            }
        }
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
    switch( button ) {
        case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
        case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
        case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
    }
    }
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE | GLUT_3_2_CORE_PROFILE );
    glutInitWindowSize( original_x, original_y );
    
    glutCreateWindow( "Bouncing Ball" );
    popup_menu();
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glewExperimental = GL_TRUE;
    glewInit();
    
    init();
    
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutIdleFunc( idle );
    glutMouseFunc( mouse );
    glutReshapeFunc( reshape );
    glutMainLoop();
    return 0;
}

