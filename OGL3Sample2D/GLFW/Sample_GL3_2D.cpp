#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ao/ao.h>
#include <mpg123.h>
using namespace std;
#define BITS 8
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;
    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;
typedef struct COLOR{
    float r;
    float g;
    float b; 
}Color;
typedef struct mirrors {
  GLfloat vertex[18];
  GLfloat color[18];
} MIRRORS;
typedef struct objects {
  GLfloat vertex[18];
  GLfloat color[18];
} OBJECTS;
vector <OBJECTS > finalObjects;
vector <OBJECTS > fallingBlocks;
vector <MIRRORS > mirrors;
OBJECTS bucket1,bucket2,shooter1,shooter2, tempBlock,Laser;
MIRRORS tempMirror;
double last_update_time = glfwGetTime(), current_time;
int previousXcoor = 0;
int previousYcoor = 0;
float angle = 0;
double presentXcoor,presentYcoor;
int checkMousePressed=0;
int vis[11002], timeDiff = 0,laser_pos=0;
//int playerScore = 0;
//float angle=0;
typedef struct AllObjectsInfo {
    COLOR color;
    float x,y,x_original, y_original,anglePresent,height,width;VAO* object;int falling;    
}AllObjectsInfo;

int ShowScoreInSegments[100];

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;
AllObjectsInfo tempObject;
vector <AllObjectsInfo> objectBorder,objectMirror,objectLaser,objectScores,objectBricks,objectBucket,objectCannon;
float x_change = 0; //For the camera pan
float y_change = 0; //For the camera pan
float zoom_camera = 1;
int playerScore = 0;
int gameOver = 0;
int brickFallingSpeed = 1;
GLuint programID;
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open()){
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }
  GLint Result = GL_FALSE;
  int InfoLogLength;
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);
  return ProgramID;
}
static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window){
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL){
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors
    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );
    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL){
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i+=1) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }
    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}
void draw3DObject (struct VAO* vao){
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}
int blueBucket = 0, blueBucketDisplacement = 0,redBucketDisplacement = 0,displacement=0,redBucket=0,cannonFlag=0,cannonDisplacement=0;;
int bucketSpeed = 10;
int cannonSpeed = 5;
int altKey=0,controlKey=0,leftKey=0,rightKey=0;
double cannon_angle = 0;
//float x_change = 0, y_change = 0;
void check_pan(){
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-300.0f/zoom_camera<-300)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+300.0f/zoom_camera>300)
        y_change=300-300.0f/zoom_camera;
}
void mousescroll(GLFWwindow* window, double xoffset, double yoffset){
    if (yoffset==-1) { 
        zoom_camera /= 1.1;
    }
    else if(yoffset==1){
        zoom_camera *= 1.1;
    }
    if (zoom_camera<=1) {
        zoom_camera = 1;
    }
    if (zoom_camera>=4) {
        zoom_camera=4;
    }
    if(x_change-400.0f/zoom_camera<-400)
        x_change=-400+400.0f/zoom_camera;
    else if(x_change+400.0f/zoom_camera>400)
        x_change=400-400.0f/zoom_camera;
    if(y_change-400.0f/zoom_camera<-400)
        y_change=-300+300.0f/zoom_camera;
    else if(y_change+400.0f/zoom_camera>400)
        y_change=400-400.0f/zoom_camera;
    Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera + x_change), (float)(400.0f/zoom_camera + x_change), (float)(-400.0f/zoom_camera + y_change), (float)(400.0f/zoom_camera + y_change), 0.1f, 500.0f);
}

void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods){
    if (action == GLFW_RELEASE) {
        switch (key) {
          case GLFW_KEY_UP:
              mousescroll(window,0,+1);
              //check_pan();
              break;
            case GLFW_KEY_DOWN:
              mousescroll(window,0,-1);
              //check_pan();
              break;
            case GLFW_KEY_A:
              cannonFlag = 0;
              break;
            case GLFW_KEY_D:
              cannonFlag = 0;
              break;
            case GLFW_KEY_S:
              if(objectCannon[0].anglePresent < 45)
                objectCannon[0].anglePresent+=15;
              else
                objectCannon[0].anglePresent = 60;
                break;
            case GLFW_KEY_F:
              if(objectCannon[0].anglePresent>-45)
                objectCannon[0].anglePresent-=10;
              else
                objectCannon[0].anglePresent = -60;
                break;
            case GLFW_KEY_SPACE:
              for(int i=0;i<objectLaser.size();i+=1){
                //string current = it->first;
                if(objectLaser[i].falling==0 && current_time - last_update_time > 0.5){
                  cout << "IN air zero for " << i << endl;
                  objectLaser[i].falling = 1;
                  objectLaser[i].anglePresent = objectCannon[0].anglePresent;
                  objectLaser[i].x = objectCannon[0].x;
                  objectLaser[i].y = objectCannon[0].y + cannonDisplacement ;
                  last_update_time = current_time;
                  //createRectangle("laser",shade,red,red,red,objectLaser[i].x,objectLaser[i].y,10,30);
                  break;
                }
              }
              cout << "Exited !" << endl;
              break;
            case GLFW_KEY_LEFT:
              leftKey = 0;
              blueBucket = redBucket = 0;
              break;
            case GLFW_KEY_RIGHT:
              rightKey = 0;
              blueBucket = redBucket = 0;
              break;

            case GLFW_KEY_LEFT_CONTROL:
              controlKey = 0;
              blueBucket = redBucket = 0;
            case GLFW_KEY_LEFT_ALT:
              altKey = 0;
              blueBucket = redBucket = 0;


            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_A:
              cannonFlag = 1;
              break;
            case GLFW_KEY_D:
              cannonFlag = -1;

            case GLFW_KEY_N:
              brickFallingSpeed +=1;
              if (brickFallingSpeed > 5) brickFallingSpeed = 5;
              break;
              
            case GLFW_KEY_M:
              brickFallingSpeed -= 1;
              if(brickFallingSpeed <= 0) brickFallingSpeed = 1;
              break;

            case GLFW_KEY_LEFT_CONTROL:
              controlKey = 1;
              break;

            case GLFW_KEY_LEFT_ALT:
              altKey = 1;
              break;
            case GLFW_KEY_LEFT:
              cout << "Left is pressed" << controlKey << endl;
              if(controlKey == 1){
                blueBucket = -1;  
              }
              else if(altKey == 1){
                redBucket = -1;  
              }
              else{
                cout << "Entered condition" << endl;
                x_change -= 10;
                check_pan();
                mousescroll(window,0,0);
              }
              leftKey = 1;
              break;


            case GLFW_KEY_RIGHT:
            cout << "Right is pressed" << controlKey << endl;
              if(controlKey == 1){
                blueBucket = 1;  
              }
              else if(altKey == 1){
                redBucket = 1;  
              }
              else{
                x_change += 10;
                check_pan();
                mousescroll(window,0,0);
              }
              rightKey = 1;
              break;  

            default:
                break;
        }
    }
}
void shootLaser(){
  for(int i=0;i<objectLaser.size();i+=1){
      //string current = it->first;
      if(objectLaser[i].falling==0 && current_time - last_update_time > 0.5){
        cout << "IN air zero for " << i << endl;
        objectLaser[i].falling = 1;
        objectLaser[i].anglePresent = objectCannon[0].anglePresent;
        objectLaser[i].x = objectCannon[0].x;
        objectLaser[i].y = objectCannon[0].y + cannonDisplacement ;
        //createRectangle("laser",shade,red,red,red,objectLaser[i].x,objectLaser[i].y,10,30);
        break;
      }
    }
  cout << "Exited !" << endl;
              
}

mpg123_handle *mh;
unsigned char *buffer;
size_t buffer_size;
size_t done;
int err;
int driver;
ao_device *dev;
ao_sample_format format;
int channels, encoding;
long rate;
void audio_init() {
    ao_initialize();
    driver = ao_default_driver_id();
    mpg123_init();
    mh = mpg123_new(NULL, &err);
    buffer_size = 2500;
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    mpg123_open(mh, "themeSong.mp3");
    mpg123_getformat(mh, &rate, &channels, &encoding);

    format.bits = mpg123_encsize(encoding) * BITS;
    format.rate = rate;
    format.channels = channels;
    format.byte_format = AO_FMT_NATIVE;
    format.matrix = 0;
    dev = ao_open_live(driver, &format, NULL);
}
void audio_play() {
    if (mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK)
        ao_play(dev, (char*) buffer, done);
    else mpg123_seek(mh, 0, SEEK_SET);
}
void audio_close() {
    free(buffer);
    ao_close(dev);
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    ao_shutdown();
}

void keyboardChar (GLFWwindow* window, unsigned int key){
  switch (key) {
    case 'Q':
    case 'q':
            quit(window);
            break;
    default:
      break;
  }
}
void mouseButton (GLFWwindow* window, int button, int action, int mods){
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
            {
                checkMousePressed = 0;
            }
            else if (action == GLFW_PRESS) 
            {
              shootLaser();
                checkMousePressed = 1;
            }
            break;
        default:
            break;
    }
}
void reshapeWindow (GLFWwindow* window, int width, int height){
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    GLfloat fov = 90.0f;
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
    Matrices.projection = glm::ortho(-400.0f/zoom_camera, 400.0f/zoom_camera, -400.0f/zoom_camera, 400.0f/zoom_camera, 0.1f, 500.0f);
}
VAO  *rectangle;
VAO *triangle,*sh1,*sh2,*laser;
void createTriangle (){
  static const GLfloat vertex_buffer_data [] = {
    0, 1,0, // vertex 0
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
  };
  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 0
    0,1,0, // color 1
    0,0,1, // color 2
  };
  triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void initObjects(){
  GLfloat vertex_buffer_data_bucket1 [] = {
    125,-400,0, // vertex 1
    175,-400,0, // vertex 2
    200, -350,0, // vertex 3

    200, -350,0, // vertex 3
    100, -350,0, // vertex 4
    125,-400,0  // vertex 1
  };
  GLfloat color_buffer_data_bucket1 [] = {
    0,1,0, // color 1
    0,1,0, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,1,0, // color 4
    0,1,0  // color 1
  };
  for(int i=0;i<18;i++){bucket1.vertex[i]=vertex_buffer_data_bucket1[i];vis[(int)(vertex_buffer_data_bucket1[0])+400]=1;}
  for(int i=0;i<18;i++){bucket1.color[i]=color_buffer_data_bucket1[i];}
    GLfloat vertex_buffer_data_bucket2 [] = {
    -175,-400,0, // vertex 1
    -125,-400,0, // vertex 2
    -100, -350,0, // vertex 3

    -100, -350,0, // vertex 3
    -200, -350,0, // vertex 4
    -175,-400,0  // vertex 1
  };
  GLfloat color_buffer_data_bucket2 [] = {
    1,0,0, // color 1
    1,0,0, // color 2
    1,0,0, // color 3

    1,0,0, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  for(int i=0;i<18;i++){bucket2.vertex[i]=vertex_buffer_data_bucket2[i];vis[(int)(vertex_buffer_data_bucket1[0])+400]=1;}
  for(int i=0;i<18;i++){bucket2.color[i]=color_buffer_data_bucket2[i];}
   GLfloat vertex_buffer_data_shooter1 [] = {
    -350,15,0, // vertex 1
    -400,15,0, // vertex 2
    -400, -15,0, // vertex 3

    -400, -15,0, // vertex 3
    -350, -15,0, // vertex 4
    -350,15,0  // vertex 1
  };
  GLfloat color_buffer_data_shooter1 [] = {
    0,0,0, // color 1
    0.3,0.3,0.3, // color 2
    0.3,0.3,0.3, // color 3

    0.3,0.3,0.3, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };
  for(int i=0;i<18;i++){shooter1.vertex[i]=vertex_buffer_data_shooter1[i];}
  for(int i=0;i<18;i++){shooter1.color[i]=color_buffer_data_shooter1[i];}
  sh1=create3DObject(GL_TRIANGLES,6,shooter1.vertex,shooter1.color,GL_FILL);
  GLfloat vertex_buffer_data_shooter2 [] = {
    -325,10,0, // vertex 1
    -350,10,0, // vertex 2
    -350, -10,0, // vertex 3

    -350, -10,0, // vertex 3
    -325, -10,0, // vertex 4
    -325,10,0  // vertex 1
  };
  GLfloat color_buffer_data_shooter2 [] = {
    0,0,0, // color 1
    0.5,0.5,0.5, // color 2
    0.5,0.5,0.5, // color 3

    0.5,0.5,0.5, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };
  for(int i=0;i<18;i++){shooter2.vertex[i]=vertex_buffer_data_shooter2[i];}
  for(int i=0;i<18;i++){shooter2.color[i]=color_buffer_data_shooter2[i];}
  sh2=create3DObject(GL_TRIANGLES,6,shooter2.vertex,shooter2.color,GL_FILL);
  GLfloat vertex_buffer_data_laser [] = {
    400,10,0, // vertex 1
    -325,10,0, // vertex 2
    -325, -10,0, // vertex 3

    -325, -10,0, // vertex 3
    400, -10,0, // vertex 4
    400,10,0  // vertex 1
  };

  GLfloat color_buffer_data_L_color [] = {
    1,0,0, // color 1
    1,0.5,0.5, // color 2
    1,0.5,0.5, // color 3

    1,0.5,0.5, // color 3
    1,0,0, // color 4
    1,0,0  // color 1
  };
  laser=create3DObject(GL_TRIANGLES,6,vertex_buffer_data_laser,color_buffer_data_L_color,GL_FILL);
  finalObjects.push_back(bucket1);
  finalObjects.push_back(bucket2);
  finalObjects.push_back(shooter1);
  finalObjects.push_back(shooter2);
}
void drawAllObjects(){
  for(int i=0;i<finalObjects.size() - 2;i++){
    Matrices.model = glm::mat4(1.0f);
    draw3DObject(create3DObject(GL_TRIANGLES, 6, finalObjects[i].vertex, finalObjects[i].color, GL_FILL));
  }
  for(int i=0;i<fallingBlocks.size();i++){
    draw3DObject(create3DObject(GL_TRIANGLES,6,fallingBlocks[i].vertex, fallingBlocks[i].color,GL_FILL));
  }
  for(int i=0;i<mirrors.size();i++){
    draw3DObject(create3DObject(GL_LINES,6,mirrors[i].vertex, mirrors[i].color,GL_FILL));
  }
  return ;
}

void createRectangle (string assignedShape, COLOR colorA, COLOR colorB, COLOR colorC, COLOR colorD, float x, float y, float height, float width){
    float w=width/2,h=height/2;
    GLfloat vertex_buffer_data [] = {
        -w,-h,0, // vertex 1
        -w,h,0, // vertex 2
        w,h,0, // vertex 3

        w,h,0, // vertex 3
        w,-h,0, // vertex 4
        -w,-h,0  // vertex 1
    };
    COLOR first=colorA,second=colorB,third=colorC,fourth=colorD;
    GLfloat color_buffer_data [] = {
        first.r,first.g,first.b, // color 1
        second.r,second.g,second.b, // color 2
        third.r,third.g,third.b, // color 3

        third.r,third.g,third.b, // color 4
        fourth.r,fourth.g,fourth.b, // color 5
        first.r,first.g,first.b // color 6
    };
    rectangle=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    AllObjectsInfo newObject = {};
    newObject.color = colorA;
    newObject.object = rectangle;
    newObject.x=newObject.x_original=x;
    newObject.y=newObject.y_original=y;
    newObject.height=height;
    newObject.width=width;
    newObject.falling=0;
    newObject.anglePresent = 0;
    if(assignedShape=="cannonBig" || assignedShape == "cannonSmall") objectCannon.push_back(newObject);
    if(assignedShape=="bucketBlue" || assignedShape == "bucketRed") objectBucket.push_back(newObject);
    if(assignedShape=="brick") objectBricks.push_back(newObject);
    if(assignedShape=="scores") objectScores.push_back(newObject);
    if(assignedShape=="0") objectLaser[0]=newObject;
    if(assignedShape=="1") objectLaser[1]=newObject;
    if(assignedShape=="2") objectLaser[2]=newObject;
    if(assignedShape=="3") objectLaser[3]=newObject;
    if(assignedShape=="4") objectLaser[4]=newObject;
    if(assignedShape=="5") objectLaser[5]=newObject;
    if(assignedShape=="6") objectLaser[6]=newObject;
    if(assignedShape=="7") objectLaser[7]=newObject;
    if(assignedShape=="8") objectLaser[8]=newObject;
    if(assignedShape=="9") objectLaser[9]=newObject;
    if(assignedShape=="mirror"){  newObject.anglePresent=45;objectMirror.push_back(newObject);}
    if(assignedShape=="borderLine") objectBorder.push_back(newObject);
}
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
int validLaser(AllObjectsInfo tempLaser){
  if(tempLaser.x > 250 || tempLaser.x < -400 || tempLaser.y > 375 || tempLaser.y < -375)return 1;
  return 0;
}
void showScore(int a){
  for(int i=0;i<10;i++) ShowScoreInSegments[i] = 0;
  if(a == 0){for(int i=0;i<7;i++){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[6] = 0;}
  if(a == 1){ShowScoreInSegments[1] = 1;ShowScoreInSegments[2] = 1;}
  if(a == 2){ShowScoreInSegments[0] = 1;ShowScoreInSegments[1] = 1;ShowScoreInSegments[3] = 1;ShowScoreInSegments[4] = 1;ShowScoreInSegments[6] = 1;}
  if(a == 3){for(int i=0;i<7;i++){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[4] = 0;ShowScoreInSegments[5] = 0;}
  if(a == 4){ShowScoreInSegments[1] = ShowScoreInSegments[2] = ShowScoreInSegments[5] = ShowScoreInSegments[6] = 1;}
  if(a == 5){for(int i=0;i<7;i++){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[1] = 0;ShowScoreInSegments[4] = 0;}
  if(a == 6){for(int i=0;i<7;i++){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[1] = 0;}
  if(a == 7){ShowScoreInSegments[0] = ShowScoreInSegments[1] = ShowScoreInSegments[2] = 1;}
  if(a == 8){for(int i=0;i<7;i++){ShowScoreInSegments[i] = 1;}}
  if(a == 9){for(int i=0;i<7;i++){ShowScoreInSegments[i] = 1;}ShowScoreInSegments[4] = 0;}
  return ;
}
int collideBrick(AllObjectsInfo tempBrick){
  int fl=0;
  for(int i=0;i<objectLaser.size();i+=1){
    if( objectLaser[i].falling == 1){
      float distance_1=abs(objectLaser[i].x - tempBrick.x),distance_2=abs(objectLaser[i].y - tempBrick.y),widthBrick=0.5*(tempBrick.width + objectLaser[i].width),heightBrick=0.5*(tempBrick.height + objectLaser[i].height);
      //cout << distance_1 << " " << distance_2 << " " << widthBrick << " " << heightBrick << endl;
      if(distance_1 < widthBrick + 5.0 && distance_2 < heightBrick + 5.0){
        objectLaser[i].falling = 0;
        if(tempBrick.color.r == 1 || tempBrick.color.b == 1){playerScore -= 1;}
        else{playerScore+=1;}
        if(playerScore <= 0) playerScore = 0;
        fl=1;
      }
    }
  }
  return fl;
}
int collideMirror(AllObjectsInfo * tempLaser){
  for(int i=0;i<objectMirror.size();i+=1){
    float xCoor1 = tempLaser->x + tempLaser->width*0.5*cos((tempLaser->anglePresent/180.0f)*M_PI);
    float yCoor1 = tempLaser->y + tempLaser->width*0.5*sin((tempLaser->anglePresent/180.0f)*M_PI);
    float distance_1 = (xCoor1 - objectMirror[i].x)/cos((objectMirror[i].anglePresent/180.0f)*M_PI);
    float distance_2 = (yCoor1 - objectMirror[i].y)/sin((objectMirror[i].anglePresent/180.0f)*M_PI);
    //cout << (xCoor1 - objectMirror[i].x) << " " << (yCoor1 - objectMirror[i].y) << endl;
    if(distance_1 >= -objectMirror[i].width*0.5f - 2 && distance_1 <= objectMirror[i].width*0.5f + 2 && distance_2 >= -objectMirror[i].width*0.5f - 2 && distance_2 <= objectMirror[i].width*0.5 + 2 && abs(distance_1-distance_2) <= 5.0f){
      tempLaser->anglePresent = 2*objectMirror[i].anglePresent - tempLaser->anglePresent;
      cout << "Hello"<< endl;
      return 1;
    }
  }
  return 0;
}
void draw (){
  if(blueBucketDisplacement + blueBucket*bucketSpeed + objectBucket[0].x < 230 && blueBucketDisplacement + blueBucket*bucketSpeed + objectBucket[0].x> -350) blueBucketDisplacement += blueBucket*bucketSpeed;
  if(redBucketDisplacement + redBucket*bucketSpeed + objectBucket[1].x < 230 && redBucketDisplacement + redBucket*bucketSpeed + objectBucket[1].x> -350) redBucketDisplacement += redBucket*bucketSpeed;
  if(cannonDisplacement + cannonFlag*cannonSpeed + objectCannon[0].y < 350 && cannonDisplacement + cannonFlag*cannonSpeed + objectCannon[0].y > -350) cannonDisplacement += cannonFlag*cannonSpeed;
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram (programID);
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  glm::vec3 target (0, 0, 0);
  glm::vec3 up (0, 1, 0);
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
  glm::mat4 VP = Matrices.projection * Matrices.view;
  for(int i=0;i<objectBricks.size();i+=1){
        if(objectBricks[i].falling == 1){
          if(objectBricks[i].y - brickFallingSpeed > -375){
            objectBricks[i].y -= brickFallingSpeed;
          }
          else{
            COLOR colorBrickFalling = objectBricks[i].color;
            if(objectBricks[i].x  <= objectBucket[0].x + objectBucket[0].width + blueBucketDisplacement && 
                  objectBricks[i].x  >= objectBucket[0].x - objectBucket[0].width + blueBucketDisplacement && 
                  colorBrickFalling.b == 1){
              playerScore++;
              //cout << "score changed : " << playerScore << "Color blue entered" << endl;
            }
            if(objectBricks[i].x  <= objectBucket[1].x + objectBucket[1].width + redBucketDisplacement && 
                    objectBricks[i].x  >= objectBucket[1].x - objectBucket[1].width + redBucketDisplacement && 
                    colorBrickFalling.r == 1){
              playerScore++;
              //cout << "score changed : " << playerScore << "Color red entered" << endl;
            }
            if(colorBrickFalling.r != 0 && colorBrickFalling.b != 0){
             //cout << "Black just fell on " << objectBricks[i].x << endl; 
             //cout << objectBucket[1].x + objectBucket[1].width + redBucketDisplacement << " : " << objectBucket[0].x + objectBucket[0].width + blueBucketDisplacement << endl;;
             if(objectBricks[i].x <= objectBucket[0].x + objectBucket[0].width + blueBucketDisplacement && objectBricks[i].x >= objectBucket[0].x - objectBucket[0].width + blueBucketDisplacement){playerScore--;}
             else if(objectBricks[i].x <= objectBucket[1].x + objectBucket[1].width + redBucketDisplacement && objectBricks[i].x >= objectBucket[1].x - objectBucket[1].width + redBucketDisplacement){playerScore--;}
             if(playerScore < 0) playerScore = 0;

            }
            /*if(
              (
                (objectBricks[i].x  <= objectBucket[1].x + objectBucket[1].width + redBucketDisplacement && objectBricks[i].x  >= objectBucket[1].x - objectBucket[1].width + redBucketDisplacement) || 
                (objectBricks[i].x  <= objectBucket[0].x + objectBucket[0].width + blueBucketDisplacement && objectBricks[0].x >= objectBucket[0].x - objectBucket[0].width + blueBucketDisplacement )
                ) 
                  && colorBrickFalling.r != 0 && colorBrickFalling.b != 0){
              playerScore--;
              if(playerScore < 0) playerScore = 0;
              cout << "score changed : " << playerScore << "Color black entered" << endl;
            }*/
            objectBricks[i].y = 320;
            objectBricks[i].falling = 0;
          }
          if(collideBrick(objectBricks[i])==1){
            objectBricks[i].x=objectBricks[i].x_original;
            objectBricks[i].y=objectBricks[i].y_original;
            objectBricks[i].falling = 0;
          }
          glm::mat4 MVP;  // MVP = Projection * View * Model
          Matrices.model = glm::mat4(1.0f);
          glm::mat4 ObjectTransform;
          glm::mat4 translateObject = glm::translate (glm::vec3(objectBricks[i].x, objectBricks[i].y, 0.0f)); // glTranslatef
          ObjectTransform=translateObject;
          Matrices.model *= ObjectTransform;
          MVP = VP * Matrices.model; // MVP = p * V * M
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(objectBricks[i].object);
        }
  }
  showScore(playerScore%10);
  for(int i=0;i<objectMirror.size();i+=1){
      glm::mat4 MVP;  // MVP = Projection * View * Model
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 ObjectTransform;
      glm::mat4 translateObject = glm::translate (glm::vec3(objectMirror[i].x, objectMirror[i].y, 0.0f)); // glTranslatef
      glm::mat4 rotateObject = glm::rotate ((float)(objectMirror[i].anglePresent/**M_PI/180.0f*/), glm::vec3(0,0,1)); // glTranslatef
      ObjectTransform=translateObject;
      Matrices.model *= ( ObjectTransform* rotateObject );
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(objectMirror[i].object);
  }
  for(int i=0;i<objectBorder.size();i+=1){
      glm::mat4 MVP;  // MVP = Projection * View * Model
      Matrices.model = glm::mat4(1.0f);
      glm::mat4 ObjectTransform;
      glm::mat4 translateObject = glm::translate (glm::vec3(objectBorder[i].x, objectBorder[i].y, 0.0f)); // glTranslatef
      glm::mat4 rotateObject = glm::rotate ((float)(objectMirror[i].anglePresent*M_PI/180.0f), glm::vec3(0,0,1)); // glTranslatef
      ObjectTransform=translateObject;
      Matrices.model *= ( ObjectTransform);
      MVP = VP * Matrices.model; // MVP = p * V * M
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(objectBorder[i].object); 
  }
  for(int i=0;i<objectLaser.size();i+=1){
        if(objectLaser[i].falling==1){
          collideMirror(&objectLaser[i]);
          objectLaser[i].x +=  4.5f*cos(objectLaser[i].anglePresent*M_PI/180.0f);
          objectLaser[i].y +=  4.5f*sin(objectLaser[i].anglePresent*M_PI/180.0f);
          if(validLaser(objectLaser[i])){
            objectLaser[i].falling=0;
          }
          else{
            glm::mat4 MVP;  // MVP = Projection * View * Model
            Matrices.model = glm::mat4(1.0f);
            glm::mat4 ObjectTransform;
            glm::mat4 translateObject = glm::translate (glm::vec3(objectLaser[i].x, objectLaser[i].y, 0.0f)); // glTranslatef
            glm::mat4 rotateObject = glm::rotate((float)(objectLaser[i].anglePresent*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
            ObjectTransform=translateObject*rotateObject;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            draw3DObject(objectLaser[i].object);
          }
        }
  }
  for(int i=0;i<objectBucket.size();i+=1){
    glm::mat4 MVP;
    if(i == 0) displacement = blueBucketDisplacement;
    else displacement = redBucketDisplacement;
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 ObjectTransform;
    glm::mat4 translateObject = glm::translate (glm::vec3(objectBucket[i].x + displacement, objectBucket[i].y, 0.0f)); // glTranslatef
    ObjectTransform=translateObject;
    Matrices.model *= ObjectTransform;
    MVP = VP * Matrices.model; // MVP = p * V * M    
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(objectBucket[i].object);
  }
  for(int i=0;i<objectCannon.size();i+=1){
        glm::mat4 MVP;  // MVP = Projection * View * Model
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 ObjectTransform;
        //if(i == 0) cout << objectCannon[i].anglePresent*M_PI/180.0f << endl;
        glm::mat4 translateObject = glm::translate (glm::vec3(objectCannon[i].x , objectCannon[i].y + cannonDisplacement, 0.0f)); // glTranslatef
        glm::mat4 rotateObject = glm::rotate((float)(objectCannon[i].anglePresent*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
        ObjectTransform=translateObject*rotateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M        
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(objectCannon[i].object);
  }
  //cout << "Score is : "  << playerScore << endl;
  for(int i=0;i<objectScores.size()/2;i++){
   // cout << "Yes, Entered" << endl;
    if(ShowScoreInSegments[i] == 1){
        //cout << "Drawing " << i << "th object" << endl;
        glm::mat4 MVP;  // MVP = Projection * View * Model
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 ObjectTransform;
        glm::mat4 translateObject = glm::translate (glm::vec3(objectScores[i].x , objectScores[i].y, 0.0f)); // glTranslatef
        ObjectTransform=translateObject;
        Matrices.model *= ObjectTransform;
        MVP = VP * Matrices.model; // MVP = p * V * M        
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        draw3DObject(objectScores[i].object);
    }
  }
  if((playerScore/10)%10 != 0){
    showScore((playerScore/10)%10);
    for(int i=0;i<objectScores.size();i++){
      //cout << "Yes, Entered" << endl;
      if(ShowScoreInSegments[i] == 1){
         // cout << "Drawing " << i << "th object" << endl;
          glm::mat4 MVP;  // MVP = Projection * View * Model
          Matrices.model = glm::mat4(1.0f);
          glm::mat4 ObjectTransform;
          glm::mat4 translateObject = glm::translate (glm::vec3(objectScores[i+7].x , objectScores[i+7].y, 0.0f)); // glTranslatef
          ObjectTransform=translateObject;
          Matrices.model *= ObjectTransform;
          MVP = VP * Matrices.model; // MVP = p * V * M        
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(objectScores[i+7].object);
      }
  } 
  }
  Matrices.model = glm::mat4(1.0f);
  float increments = 1;
}
GLFWwindow* initGLFW (int width, int height){
    GLFWwindow* window; // window desciptor/handle
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);
    if (!window) {
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);
    glfwSetWindowCloseCallback(window, quit);
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling
    glfwSetMouseButtonCallback(window, mouseButton); // mouse button clicks
    glfwSetScrollCallback(window, mousescroll); // mouse scroll
    return window;
}
void createLine(){
  return ;
}
void createAllObjects(){
    COLOR grey = {168.0/255.0,168.0/255.0,168.0/255.0};
    COLOR shade = {0.3,0.3,0};
    COLOR green = {0,1,0};
    COLOR gold = {218.0/255.0,165.0/255.0,32.0/255.0};
    COLOR red = {1,0,0};
    COLOR black = {30/255.0,30/255.0,21/255.0};
    COLOR blue = {0,0,1};
    COLOR white = {255/255.0,255/255.0,255/255.0};
  createRectangle("0",black,black,shade,black,objectLaser[0].x,objectLaser[0].y,10,30);
  createRectangle("1",black,black,shade,black,objectLaser[1].x,objectLaser[1].y,10,30);
  createRectangle("2",black,black,shade,black,objectLaser[2].x,objectLaser[2].y,10,30);
  createRectangle("3",black,black,shade,black,objectLaser[3].x,objectLaser[3].y,10,30);
  createRectangle("4",black,black,shade,black,objectLaser[4].x,objectLaser[4].y,10,30);
  createRectangle("5",black,black,shade,black,objectLaser[0].x,objectLaser[0].y,10,30);
  createRectangle("6",black,black,shade,black,objectLaser[1].x,objectLaser[1].y,10,30);
  createRectangle("7",black,black,shade,black,objectLaser[2].x,objectLaser[2].y,10,30);
  createRectangle("8",black,black,shade,black,objectLaser[3].x,objectLaser[3].y,10,30);
  createRectangle("9",black,black,shade,black,objectLaser[4].x,objectLaser[4].y,10,30);
  createRectangle("cannonSmall",grey,gold,grey,gold,-350,0,12,40);
  createRectangle("cannonBig",grey,gold,grey,gold,-380,0,35,60);
  createRectangle("bucketBlue",blue,blue,blue,blue,-300,-380,40,60);
  createRectangle("bucketRed",red,red,red,red,220,-380,40,60);
  createRectangle("mirror",black,black,black,black,-150,200,2,60);
  createRectangle("mirror",black,black,black,black,-150,-80,2,60);
  createRectangle("mirror",black,black,black,black,200,200,2,60);
  createRectangle("mirror",black,black,black,black,200,-80,2,60);

  createRectangle("brick",red,red,red,red,-250,375,20,20);
  createRectangle("brick",red,red,red,red,-200,375,20,20);
  createRectangle("brick",red,red,red,red,-100,375,20,20);
  createRectangle("brick",red,red,red,red,0,375,20,20);
  createRectangle("brick",red,red,red,red,100,375,20,20);
  createRectangle("brick",red,red,red,red,200,375,20,20);
  createRectangle("brick",red,red,red,red,210,375,20,20);
  createRectangle("brick",red,red,red,red,210,375,20,20);
  createRectangle("brick",red,red,red,red,-250,375,20,20);
  createRectangle("brick",red,red,red,red,-250,375,20,20);
  createRectangle("brick",red,red,red,red,210,375,20,20);
  createRectangle("brick",red,red,red,red,-50,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,-250,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,210,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,-50,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,-210,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,-200,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,-100,375,20,20);
  createRectangle("borderLine", black,black,black,black,250,0,800,5); 
  createRectangle("brick",blue,blue,blue,blue,0,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,100,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,200,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,210,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,210,375,20,20);
  createRectangle("brick",blue,blue,blue,blue,-250,375,20,20);
  createRectangle("brick",black,black,black,black,-250,375,20,20);
  createRectangle("brick",black,black,black,black,210,375,20,20);
  createRectangle("brick",black,black,black,black,-50,375,20,20);
  createRectangle("brick",black,black,black,black,-250,375,20,20);
  createRectangle("brick",black,black,black,black,-200,375,20,20);
  createRectangle("brick",black,black,black,black,-100,375,20,20); 
  createRectangle("scores",black,black,black,black,375,30,5,30);

  createRectangle("scores",black,black,black,black,390,15,30,5);
  createRectangle("scores",black,black,black,black,390,-15,30,5);
  createRectangle("scores",black,black,black,black,375,-30,5,30);
  createRectangle("scores",black,black,black,black,360,-15,30,5);
  createRectangle("scores",black,black,black,black,360,15,30,5);
  createRectangle("scores",black,black,black,black,375,0,5,30);


  createRectangle("scores",black,black,black,black,315,30,5,30);
  createRectangle("scores",black,black,black,black,330,15,30,5);
  createRectangle("scores",black,black,black,black,330,-15,30,5);
  createRectangle("scores",black,black,black,black,315,-30,5,30);
  createRectangle("scores",black,black,black,black,300,-15,30,5);
  createRectangle("scores",black,black,black,black,300,15,30,5);
  createRectangle("scores",black,black,black,black,315,0,5,30);
  
  return ;
}
void initGL (GLFWwindow* window, int width, int height){
  for(int i=0;i<10;i+=1)objectLaser.push_back(tempObject);
    createAllObjects();
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
  reshapeWindow (window, width, height);
  glClearColor (0.3,0.3,0.3,0.4f); // R, G, B, A
  glClearDepth (1.0f);
  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);
    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}
void fallBlocks(){
  int x = rand()%4 + 1,fl=0,x1,x2,colorBlock = rand()%3;
  vector <int > d;
  for(int i=0;i<fallingBlocks.size();i++){
    fl=0;
    for(int j=1;j<18;j+=3){
      fallingBlocks[i].vertex[j]-=25;
      if(fallingBlocks[i].vertex[j] <= -380) fl=1;
    }
    if(fl == 1){
      d.push_back(i);
    }
    /*else{
      glm::translate(glm::vec3(0.0f, -25, 0.0f));
    }*/
  }
  for(int i=0;i<d.size();i++){fallingBlocks.erase(fallingBlocks.begin() + d[i]);}
  timeDiff++;
  if(timeDiff%1 == 0){
    timeDiff = 0;
    if(colorBlock == 0){x=1;}
    for(int i=0;i<x;i++){
      GLint colr[18];
      x1=rand()%800 - 400;
      cout << "X1 is : " << x1 << "Loop size is : " << x << endl; 
      GLint vect [] = {
        x1,400,0, // vertex 1
        x1-10,400,0, // vertex 2
        x1-10, 375,0, // vertex 3

        x1-10, 375,0, // vertex 3
        x1, 375,0, // vertex 4
        x1,400,0  // vertex 1
      };
      if(colorBlock == 0){GLint colr [] = {
          0,0,0, // vertex 1
          0,0,0, // vertex 2
          0,0,0, // vertex 3

          0,0,0, // vertex 3
          0,0,0, // vertex 4
          0,0,0  // vertex 1
        };}
      else if(colorBlock == 1){GLint colr [] = {
          0,1,0, // vertex 1
          0,1,0, // vertex 2
          0,1,0, // vertex 3

          0,1,0, // vertex 3
          0,1,0, // vertex 4
          0,1,0  // vertex 1
        };}
      else if(colorBlock == 2){GLint colr [] = {
          1,0,0, // vertex 1
          1,0,0, // vertex 2
          1,0,0, // vertex 3

          1,0,0, // vertex 3
          1,0,0, // vertex 4
          1,0,0  // vertex 1
        };}
      for(int j=0;j<18;j++){tempBlock.vertex[j]=vect[j];tempBlock.color[j]=colr[j];}
      fallingBlocks.push_back(tempBlock);
    }
  }
}
void drawMirror(){
  int x=rand()%3 + 1,r=rand()%400;
  for(int i=0;i<x;i++){
      int x1=rand()%50,y1=rand()%50,a1=rand()%6 - 3;
      GLint vect [] = {
        x1,y1,0,
        x1 + r*cos(a1),y1 + r*sin(a1),0
      };
      GLint colr [] = {
        0.3,0.3,1, // vertex 1
        0.3,0.3,1 // vertex 2
      };
      for(int j=0;j<6;j++){tempMirror.vertex[j]=vect[j];tempMirror.color[j]=colr[j];}
      mirrors.push_back(tempMirror);
  }
}
void bringDownBricks(){
  for(int i=0;i<objectBricks.size();i+=1){
      int xa=rand()%objectBricks.size();
      if(objectBricks[xa].falling==0){
        objectBricks[xa].falling = 1;
        break;
      }
  }
}

int main (int argc, char** argv){
  srand(time(NULL));
  int x=0;
  vector <int> d;
  int width = 800;
  int height = 800;
  GLFWwindow* window = initGLFW(width, height);
  initGL (window, width, height);
  //audio_init();
    while (!glfwWindowShouldClose(window)) {
      //audio_play();
      if(checkMousePressed){
            glfwGetCursorPos(window, &presentXcoor, &presentYcoor);
            if(previousYcoor != presentYcoor || previousXcoor != presentXcoor){
                if(previousXcoor != presentXcoor)
                    previousXcoor = presentXcoor;
                if(previousYcoor != presentYcoor)
                    previousYcoor = presentYcoor;
                if(presentXcoor == 0)
                    angle = 90;
                else{
                    angle = atan(abs(presentYcoor-300) / abs(presentXcoor)); 
                    angle *= 180/M_PI;
                    if(presentYcoor > 400)
                        angle = -1*angle;
                    if(presentXcoor < 0 && presentYcoor < 400) angle = 90;
                    if(presentXcoor <=0 && presentYcoor >= 400) angle = -90;
                    objectCannon[0].anglePresent = angle;
                    if(objectCannon[0].anglePresent <= -60) objectCannon[0].anglePresent = -60;
                    if(objectCannon[0].anglePresent >= 60) objectCannon[0].anglePresent = 60;
                    //cout << "Angle is changed to : " << objectCannon[0].anglePresent   << endl;
                }

                // moving the buckets
                cout << "Show " << presentXcoor - 400 << " " << objectBucket[1].x + objectBucket[1].width << " " << objectBucket[1].x - objectBucket[1].width << " " << redBucketDisplacement <<  endl;
                if(presentXcoor - 400 <= objectBucket[0].x + objectBucket[0].width + blueBucketDisplacement && presentXcoor - 400 >= objectBucket[0].x - objectBucket[0].width + blueBucketDisplacement && presentYcoor >= 675){
                  cout << "Entered !! " << endl;
                  objectBucket[0].x = presentXcoor - 400;
                  if(objectBucket[0].x <= -335)objectBucket[0].x = -335;
                  if(objectBucket[0].x >= 220)objectBucket[0].x = 220; 
                }
                else if(presentXcoor - 400 <= objectBucket[1].x + objectBucket[1].width + redBucketDisplacement && presentXcoor - 400 >= objectBucket[1].x + redBucketDisplacement - objectBucket[1].width && presentYcoor >= 675){
                  cout << "Entered !! " << endl;
                  objectBucket[1].x = presentXcoor - 400;
                  if(objectBucket[1].x <= -350)objectBucket[1].x = -335;
                  if(objectBucket[1].x >= 220)objectBucket[1].x = 220; 
                }
            }
        }
        draw();
        glfwSwapBuffers(window);
        glfwPollEvents();
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) > 0.75) { // atleast 0.5s elapsed since last frame
            bringDownBricks();
            last_update_time = current_time;
        }
        cout << x_change << endl;
    }
    glfwTerminate();
}
