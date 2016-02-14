#include <iostream>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <stdio.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
using namespace std;

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
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

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
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

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
    // Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
    // Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

 float triangle_rot_dir = 1;
 float rectangle_rot_dir = 1;
 bool triangle_rot_status = true;
 bool rectangle_rot_status = true;

float Player_Z =0, Player_X=0, Player_Y=0.0, Player_jump=0;

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;


float Block_flag[20][20]; // for multiplying by (block_move+0.7)
float Block_flag1[20][20]; // for insert into if condition

float Block_dis_flag[20][20];

float Block_move[20][20];
float Block_rand=1;
float obstacles_flag[20][20];

int Block_count=5; 

void check_Pos_X();
void check_Pos_Z();
void check_Player_fall();
void check_player_obstacle();

float u=2.0,t=0,g=1.0,angle=0;
int Player_fall=0;

float last_update_time , current_time;

int is_dragging=0;
float x_dragstart,Zoom=1,delta_angle=0;
float cx=1,cy=1,cz=1,cdx=0,cdy=0,cdz=0,lx=0,ly=0,lz=0,ldx=0,ldy=0,ldz=0,ux=0,uy=1,uz=0,udx=0,udy=0,udz=0;
int Player_win=0;

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
 void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
 {

 	if (action == GLFW_RELEASE)
 	{
 		switch (key)
 		{
 			case GLFW_KEY_C:
 			rectangle_rot_status = !rectangle_rot_status;
 			break;
 			case GLFW_KEY_P:
 			triangle_rot_status = !triangle_rot_status;
 			break;
 			case GLFW_KEY_X:
                // do something ..
 			break;
 			default:
 			break;
 		}
 	}
 	else if (action == GLFW_PRESS)
 	{
 		switch (key)
 		{
 			case GLFW_KEY_ESCAPE:
 			quit(window);
 			break;
 			case GLFW_KEY_UP:
 				Player_Z-=0.2;
	 			check_Pos_Z();
 				break;
 			case GLFW_KEY_DOWN:
 				Player_Z+=0.2;
			 	check_Pos_Z();
 				break;
 			case GLFW_KEY_LEFT:
 				Player_X-=0.2;
 				check_Pos_X();
 				break;
 			case GLFW_KEY_RIGHT:
 				Player_X+=0.2;
 				check_Pos_X();
 				break;
 			case GLFW_KEY_Z:
 				Zoom=Zoom+0.2;
 				Matrices.projection = glm::ortho(Zoom*-10.0f, Zoom*10.0f, Zoom*-10.0f, Zoom*10.0f, -10.0f, 10.0f);
 				break;
 			case GLFW_KEY_X:
 				Zoom=Zoom-0.2;
 				Matrices.projection = glm::ortho(Zoom*-10.0f, Zoom*10.0f, Zoom*-10.0f, Zoom*10.0f, -10.0f, 10.0f);
 				break;

 			case GLFW_KEY_SPACE:
 				Player_jump=1;
 				check_Player_fall();
 				break;
 			case GLFW_KEY_T:
 				cdx=-5-cx;
 				cdy=5-cy;
 				cdz=-5-cz;
 				ldx=-5-lx;
 				ldy=-ly;
 				ldz=-5-lz;
 				udy=1-uy;
 				udz=1-uz;
 				break;
 			case GLFW_KEY_N:
 				cdx=0;
 				cdy=0;
 				cdz=0;
 				ldx=0;
 				ldy=0;
 				ldz=0;
 				udy=0;
 				udz=0;
 				break;
 			case GLFW_KEY_F:
 				cdx=Player_X-2.0;
 				cdy=0.0;
 				cdz=Player_Z-1;
 				ldx=Player_X;
 				ldy=Player_Y;
 				ldz=Player_Z;
 				udx=0.0;
 				udy=0.0;
 				udz=0;

 				break;


 			case GLFW_KEY_H:


 				cdx=Player_X-1;
 				cdy=Player_Y-1;
 				cdz=Player_Z-1;
 				ldx=Player_X+10.0;
 				ldy=Player_Y;
 				ldz=Player_Z-10;
 		
 				break;

 			default:
 				break;
 		}
 	}
 	else if (action == GLFW_REPEAT)
 	{
 		switch (key)
 		{
 			case GLFW_KEY_ESCAPE:
 			quit(window);
 			break;
 			case GLFW_KEY_UP:
 				Player_Z-=0.2;
	 			check_Pos_Z();
 				break;
 			case GLFW_KEY_DOWN:
 				Player_Z+=0.2;
			 	check_Pos_Z();
 				break;
 			case GLFW_KEY_LEFT:
 				Player_X-=0.2;
 				check_Pos_X();
 				break;
 			case GLFW_KEY_RIGHT:
 				Player_X+=0.2;
 				check_Pos_X();
 				break;

 			default:
 				break;
 		}
 	} 	
 	if(Player_X >9.8)
 		Player_X=9.79;
 	else if(Player_X <0)
 		Player_X=0.01;
 	if(Player_Z >0)
 		Player_Z=-0.01;
 	else if(Player_Z <-9.8)
 		Player_Z=-9.79;
 	check_Player_fall();
 	check_player_obstacle();
 	if(Player_X>=9.5 && Player_Z <=-9.5)
 	{
 		Player_win=1;
 	}
 	// printf("Player_X:   %f  Player_Z:  %f   \n",Player_X,Player_Z);

 }

void check_Pos_X()
{
	for(int i=0;i<10;i++){
		for(int j=0;j<10;j++){

			if(Block_flag[i][j]==1)
			{
				// printf("Player(X,Z):  (%f,%f)\t BLOCK: (%d,%d)\n",Player_X,Player_Z,i,j);
				if(Player_X >= i && Player_X <= i+1  &&  Player_Z <= -j && Player_Z >= -(j+1))
				{
					// printf("Player_X:  %f , i: %d , Player_Z:  %f  ,  j:  %d \n",Player_X,i,Player_Z,j );
					if(Player_X < i+0.5)
						Player_X=Player_X-0.2;
					else
						Player_X=Player_X+0.2;
				}
			}
		}
	}
}


void check_Pos_Z()
{
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<10;j++)
		{
			if(Block_flag[i][j]==1)
			{

				if(Player_X >= i && Player_X <= i+1  && -(Player_Z) >=j && -(Player_Z) <=j+1)
				{
					// printf("Player_X:  %f , i: %d , Player_Z:  %f  ,  j:      ||| 22222222222222222222222%d \n",Player_X,i,Player_Z,j );
					if(-(Player_Z) < j+0.5)
						Player_Z=Player_Z+0.2;
					else
						Player_Z=Player_Z-0.2;
				}
			}
		}
	}
}


void check_Player_fall()
{
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<10;j++)
		{
			if(Block_dis_flag[i][j]==1)
			{
				// printf("Player(X,Z):  (%f,%f)\t BLOCK: (%d,%d)\n",Player_X,Player_Z,i,j);

				if(Player_X >= i && Player_X <= i+1  && -(Player_Z) >=j && -(Player_Z) <=j+1 && Player_Y <=0.1)
				{
					Player_fall=1;
				}
				// printf("Player_fall: %d \n",Player_fall);
			}
		}
	}
}

void check_player_obstacle()
{
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<10;j++)
		{
			if(obstacles_flag[i][j]==1)
			{
				if(Player_X >= i && Player_X <= i+1  && -(Player_Z) >=j && -(Player_Z) <=j+1 && Player_Y<=0.1)
				{
					Player_fall=1;
				}				
			}
		}
	}
}



/* Executed for character input (like in text boxes) */
 void keyboardChar (GLFWwindow* window, unsigned int key)
 {
 	switch (key) {
 		case 'Q':
 		case 'q':
 		quit(window);
 		break;
 		default:
 		break;
 	}
 }

/* Executed when a mouse button is pressed/released */
 void mouseButton (GLFWwindow* window, int button, int action, int mods)
 {
 	switch (button) {
 		case GLFW_MOUSE_BUTTON_LEFT:
	 		if (action == GLFW_PRESS)
	 		{	is_dragging=1 ;
	 			x_dragstart=lx;
	 		}
	 		else
	 		{
	 			angle+=delta_angle;
	 			is_dragging=0;
	 		}
	 		break;
 		// case GLFW_MOUSE_BUTTON_RIGHT:
 		// if (action == GLFW_RELEASE) {
 		// 	rectangle_rot_dir *= -1;
 		// }
 		// break;
 		default:
 		break;
 	}
 }

void mouse_motion(GLFWwindow* window,double lx,double ly)
{
	if(is_dragging==1)
	{
		delta_angle=(lx- x_dragstart)*0.002;
		ldx=-sin(angle+delta_angle);
	}
}



/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
 void reshapeWindow (GLFWwindow* window, int width, int height)
 {
 	int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
 	glfwGetFramebufferSize(window, &fbwidth, &fbheight);

 	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
 	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
	   Matrices.projection = glm::ortho(Zoom*-10.0f, Zoom*10.0f, Zoom*-10.0f, Zoom*10.0f, -10.0f, 10.0f);
	}

	VAO *triangle, *rectangle, *rectangle6, *circle;
	VAO *cube1;
	VAO *Blocks[20][20];
	VAO *obstacles[20][20];
	glm::mat4 Block_translate[20][20];
	glm::mat4 Block_rotate[20][20];

void createBlocks (float Block_x, float Block_y, float Block_z, float Block_len, float Block_width, float Block_height)
	{
  // GL3 accepts only Triangles. Quads are not supported
		static const GLfloat vertex_buffer_data [] = {


//front face

        Block_x, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z,   Block_x+Block_len, Block_y+Block_height, Block_z,  //lower triangle
        Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y, Block_z, //upper triangle

// left face

		Block_x, Block_y, Block_z,  Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z+Block_width,  //upper triangle
		Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y, Block_z+Block_width,  Block_x, Block_y, Block_z,  //lower triangle

// right face
		Block_x+Block_len, Block_y, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width,  //lower triangle
		Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  //upper triangle 

// back face
		Block_x, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y, Block_z+Block_width,   Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  //lower triangle
        Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y, Block_z+Block_width, //upper triangle

// lower face
        Block_x, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width, //right triangle
        Block_x, Block_y, Block_z,  Block_x, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y, Block_z+Block_width,  // left triangle

// upper face

        Block_x, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width, //right triangle
        Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  // left triangle        

    };

    static const GLfloat color_buffer_data [] = {
        0, 0, 0,   0, 0, 1,   0, 1, 1,     0, 0, 0,   0, 0, 1,   0, 1, 1,   //1
        1, 0, 0,   1, 0, 1,   1, 1, 1,     1, 0, 0,   1, 0, 1,   1, 1, 1,   //2
        0, 0, 0,   0, 0, 1,   1, 0, 1,     0, 0, 0,   0, 0, 1,   1, 0, 1,   //3
        0, 1, 0,   0, 1, 1,   1, 1, 1,     0, 1, 0,   0, 1, 1,   1, 1, 1,   //4
        0, 0, 0,   0, 1, 0,   1, 1, 0,     0, 0, 0,   0, 1, 0,   1, 1, 0,   //5
        0, 0, 1,   0, 1, 1,   1, 1, 1,     0, 0, 1,   0, 1, 1,   1, 1, 1    //6


    };

  // create3DObject creates and returns a handle to a VAO that can be used later
    for(int i=0;i<10;i++)
    {
    	for(int j=0;j<10;j++)
    	{
    		Blocks[i][j] = create3DObject(GL_TRIANGLE_STRIP, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
    	}
    }
}


void createobstacles (float Block_x, float Block_y, float Block_z, float Block_len, float Block_width, float Block_height)
	{
  // GL3 accepts only Triangles. Quads are not supported
		static const GLfloat vertex_buffer_data [] = {


//front face

        Block_x, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z,   Block_x+Block_len, Block_y+Block_height, Block_z,  //lower triangle
        Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y, Block_z, //upper triangle

// left face

		Block_x, Block_y, Block_z,  Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z+Block_width,  //upper triangle
		Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y, Block_z+Block_width,  Block_x, Block_y, Block_z,  //lower triangle

// right face
		Block_x+Block_len, Block_y, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width,  //lower triangle
		Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  //upper triangle 

// back face
		Block_x, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y, Block_z+Block_width,   Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  //lower triangle
        Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y, Block_z+Block_width, //upper triangle

// lower face
        Block_x, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width, //right triangle
        Block_x, Block_y, Block_z,  Block_x, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y, Block_z+Block_width,  // left triangle

// upper face

        Block_x, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width, //right triangle
        Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  // left triangle        

    };

    static const GLfloat color_buffer_data [] = {
        0.7, 0, 0,   0.7, 0, 1,   0.7,1, 0.7,     0.7, 0, 0,   0.7, 0, 1,   0.7, 1, 0.7   //2



    };

  // create3DObject creates and returns a handle to a VAO that can be used later
    for(int i=0;i<10;i++)
    {
    	for(int j=0;j<10;j++)
    	{
    		obstacles[i][j] = create3DObject(GL_TRIANGLE_STRIP, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
    	}
    }
}



void createcube (float Block_x, float Block_y, float Block_z, float Block_len, float Block_width, float Block_height)
{
  // GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {



//front face

        Block_x, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z,   Block_x+Block_len, Block_y+Block_height, Block_z,  //lower triangle
        Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y, Block_z, //upper triangle

// left face

		Block_x, Block_y, Block_z,  Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z+Block_width,  //upper triangle
		Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y, Block_z+Block_width,  Block_x, Block_y, Block_z,  //lower triangle

// right face
		Block_x+Block_len, Block_y, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width,  //lower triangle
		Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  //upper triangle 

// back face
		Block_x, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y, Block_z+Block_width,   Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  //lower triangle
        Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x, Block_y, Block_z+Block_width, //upper triangle

// lower face
        Block_x, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z,  Block_x+Block_len, Block_y, Block_z+Block_width, //right triangle
        Block_x, Block_y, Block_z,  Block_x, Block_y, Block_z+Block_width,  Block_x+Block_len, Block_y, Block_z+Block_width,  // left triangle

// upper face

        Block_x, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width, //right triangle
        Block_x, Block_y+Block_height, Block_z,  Block_x, Block_y+Block_height, Block_z+Block_width,  Block_x+Block_len, Block_y+Block_height, Block_z+Block_width,  // left triangle        

    };

    static const GLfloat color_buffer_data [] = {
        // 0, 0, 0,   0, 0, 1,   0, 1, 1,     0, 0, 0,   0, 0, 1,   0, 1, 1,  //1 
        // 0, 1, 0,   0, 1, 1,   1, 1, 1,     0, 1, 0,   0, 1, 1,   1, 1, 1,   //4
        // 0, 0, 0,   0, 1, 0,   1, 1, 0,     0, 0, 0,   0, 1, 0,   1, 1, 0,   //5
        // 0, 0, 1,   0, 1, 1,   1, 1, 1,     0, 0, 1,   0, 1, 1,   1, 1, 1,  //6
        0.7, 0, 0,   0.7, 0, 1,   0.7,1, 0.7,     0.7, 0, 0,   0.7, 0, 1,   0.7, 1, 0.7   //2
        // 1, 0, 0,   1, 0, 1,   1, 1, 1,     1, 0, 0,   1, 0, 1,   1, 1, 1,   //2
        // 0, 0, 0,   0, 0, 1,   1, 0, 1,     0, 0, 0,   0, 0, 1,   1, 0, 1,   //3
        // 0, 0, 0,   0, 0, 1,   0, 1, 1,     0, 0, 0,   0, 0, 1,   0, 1, 1  //1 

    };

  // create3DObject creates and returns a handle to a VAO that can be used later

    cube1 = create3DObject(GL_TRIANGLE_STRIP, 36, vertex_buffer_data, 1,0,0);
}


// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
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

  // create3DObject creates and returns a handle to a VAO that can be used later
triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
	static const GLfloat vertex_buffer_data [] = {
		-1, -1, -1,   -1, -1,  1,   -1,  1,  1,   -1,  1, -1,
		1, -1, -1,    1, -1,  1,    1,  1,  1,    1,  1, -1,
		-1, -1, -1,   -1, -1,  1,    1, -1,  1,    1, -1, -1,
		-1,  1, -1,   -1,  1,  1,    1,  1,  1,    1,  1, -1,
		-1, -1, -1,   -1,  1, -1,    1,  1, -1,    1, -1, -1,
		-1, -1,  1,   -1,  1,  1,    1,  1,  1,    1, -1,  1

	};

	static const GLfloat color_buffer_data [] = {
		0, 0, 0,   0, 0, 1,   0, 1, 1,   0, 1, 0,
		1, 0, 0,   1, 0, 1,   1, 1, 1,   1, 1, 0,
		0, 0, 0,   0, 0, 1,   1, 0, 1,   1, 0, 0,
		0, 1, 0,   0, 1, 1,   1, 1, 1,   1, 1, 0,
		0, 0, 0,   0, 1, 0,   1, 1, 0,   1, 0, 0,
		0, 0, 1,   0, 1, 1,   1, 1, 1,   1, 0, 1
	};

  // create3DObject creates and returns a handle to a VAO that can be used later
	rectangle6 = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data, color_buffer_data, GL_FILL);
}



/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram (programID);

	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	glm::vec3 target (0, 0, 0);
	glm::vec3 up (0, 1, 0);


  Matrices.view = glm::lookAt(glm::vec3(cx+cdx,cy+cdy,cz+cdz), glm::vec3(lx+ldx,ly+ldy,lz+ldz), glm::vec3(ux+udx,uy+udy,uz+udz)); // Fixed camera for 2D (ortho) in XY plane

  glm::mat4 VP = Matrices.projection * Matrices.view;

  glm::mat4 MVP;	// MVP = Projection * View * Model


//*************************************background rectangle 1**************************
  
//********************moving blocks****************************

   int r1,r2;
  if(Block_rand == 1)
	{
		for(int p=0;p<5;p++)
		{

			r1=rand()%8+1;
			r2=rand()%8+1;
			if(Block_flag[r1][r2]==0 && obstacles_flag[r1][r2]==0)
				Block_flag[r1][r2]=1;
			else
				p--;
		}
	}
	Block_rand = 0;

  for(int i=0;i<10;i++)
  {
  	for(int j=0;j<10;j++)
  	{
  		
			// printf("Block_rand: %f\n",Block_rand );
  		Matrices.model = glm::mat4(1.0f);
  		Block_translate[i][j]  = glm::translate (glm::vec3(i*1.0, Block_flag[i][j]*(Block_move[i][j]), -j*1.0));
	  		// Block_rotate[i][j] = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,1,0)); 
			// Matrices.model *= (Block_translate[i][j]*Block_rotate[i][j]);
  		Matrices.model *= (Block_translate[i][j]);
  		MVP = VP * Matrices.model;
  		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  		if(Block_dis_flag[i][j]==0)
  			draw3DObject(Blocks[i][j]);
  		
  		// if(Block_flag[i][j])
  		// {
  		// 	// printf("block_move: %f\n",Block_move[i][j]);
  		// }
  		if(Block_flag1[i][j] == 1 && Block_flag[i][j]==1)
  		{
  			Block_move[i][j]+=0.01;
  		}
  		else if(Block_flag1[i][j] == 0 && Block_flag[i][j]==1)
  		{
  			Block_move[i][j]-=0.01;
  		}

  		// printf("Block_count:   %d\n",Block_count);
  		if(Block_move[i][j]>=2.0 && Block_flag[i][j]==1)
  		{
  			Block_flag1[i][j]=0;
  		}
  		else if(Block_move[i][j]<=0.0 && Block_flag[i][j]==1)
  		{
  			Block_flag[i][j]=0;
  			Block_flag1[i][j]=1;
  			Block_count--;
  			if(Block_count==0)
  			{
  				Block_rand=1;
  				Block_count=5;
  			}
  		}

  	}
  }	
 //***************************************************************************************

//****************************************** CUBE 1 ****************************************

  // printf("Player_fall : %d\n",Player_fall );
  if(Player_fall==1)
  {
  	while(1)
  	{
  		Player_Y-=0.2;
  		sleep(0.05);
  		if(Player_Y<-12.0)
  			break;  		
  	}
  }

  float x1=Player_X, z1=Player_Z;
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 cubetranslate1  = glm::translate (glm::vec3(0+Player_X, Player_Y, 0+Player_Z));
	  		// Block_rotate[i][j] = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,1,0)); 
			// Matrices.model *= (Block_translate[i][j]*Block_rotate[i][j]);
  Matrices.model *= (cubetranslate1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(cube1);

//*****************************************************************************


//*******************************Obstacles*************************


current_time = glfwGetTime();

if((current_time - last_update_time) >= 10)
{
	for(int i=0;i<10;i++)
	{
		for(int j=0;j<10;j++)		
			obstacles_flag[i][j]=0;		
	}

	for(int i=1;i<9;i++)
	{
		r1=rand()%2;
		for(int j=0;j<r1;j++)
		{
			r2=rand()%8+1;
			if(Block_dis_flag[i][j]==0 && Block_flag[i][i]==0)
			{
				obstacles_flag[i][r2]=1;
			}
		}
	}
	last_update_time=current_time;
}
for(int i=0;i<10;i++)
{
	for(int j=0;j<10;j++)
	{
	  Matrices.model = glm::mat4(1.0f);
	  glm::mat4 cubetranslate2  = glm::translate (glm::vec3(i*1.0, 0, -j*1.0-0.45));

	  Matrices.model *= (cubetranslate2);
	  MVP = VP * Matrices.model;
	  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	  if(obstacles_flag[i][j]==1)
	  	draw3DObject(obstacles[i][j]);
	}
}






  //*******************************************************************







  // ********************************JUMP ******************************

  if(Player_jump==1 && Player_Y >=0.0)
  {
	Player_Y += (u*t/20.0f - g*t*t/40.0f);
	t=t+0.05;
  }
  else
  {
  	Player_Y=0.0;
  	Player_jump=0;
  	t=0;
  }

// ****************************************************************************************
}



/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
    	exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
    	glfwTerminate();
    	exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);


    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
	glfwSetCursorPosCallback(window, mouse_motion);
    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();

	createBlocks(-8.0,-8.0,0.0,1.0,1.00,9.0);
	createcube(-8.0,1.0,0.55,0.4,0.4,0.8);
	createobstacles(-8.0,1.0,0.55,1,1,1);


	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	last_update_time = glfwGetTime();
	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (64/255.0f, 164/255.0f, 233/255.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

}
void zero()
{
	for(int i=0;i<20;i++)
	{
		for(int j=0;j<20;j++)
		{
			Block_flag[i][j]=0; // for multiplying by (block_move+0.7)
			Block_flag1[i][j]=1; // for insert into if condition
			Block_move[i][j]=0;
			Block_dis_flag[i][j]=0;
			obstacles_flag[i][j]=0;
		}
	}
	for(int i=1;i<9;i++)
	{
		int r1=rand()%3;
		for(int j=0;j<r1;j++)
		{
			int r2=rand()%8+1;
			if(Block_dis_flag[i][j]==0 && Block_flag[i][i]==0)
			{
				obstacles_flag[i][r2]=1;
			}
		}
	}
		for(int p=0;p<10;p++)
		{

			int r1=rand()%8+1;
			int r2=rand()%8+1;
			if(Block_flag[r1][r2]==0 && obstacles_flag[r1][r2]==0)
			{
				Block_dis_flag[r1][r2]=1;
			}
		}
	

}

int main (int argc, char** argv)
{
	int width = 1920;
	int height = 1080;

	GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

	zero();

    /* Draw in loop */
	while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
		draw();

        // Swap Frame Buffer in double buffering

		glfwSwapBuffers(window);

		if(Player_fall==1 )
		{
			printf("YOU LOST THE MATCH. TRY AGAIN !!!\n\n\n\n\n\n\n\n\n\n");
			quit(window);
		}
		if(Player_win==1)
		{
			printf("CONGRATULATIONS YOU WON THE MATCH !!!!\n\n\n\n\n\n\n\n\n\n");
			quit(window);
		}

        // Poll for Keyboard and mouse events
		glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)

    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}
