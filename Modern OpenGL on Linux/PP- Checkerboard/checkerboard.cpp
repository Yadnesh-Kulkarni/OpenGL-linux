#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <SOIL/SOIL.h>

#include "vmath.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define checkImageWidth 64
#define checkImageHeight 64

using namespace std;
using namespace vmath;
enum
{
		VDG_ATTRIBUTE_VERTEX = 0,
		VDG_ATTRIBUTE_COLOR,
		VDG_ATTRIBUTE_NORMAL,
		VDG_ATTRIBUTE_TEXTURE0,
};

FILE *gpFile = NULL;

bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth = 800;
int giWindowHeight = 600;

GLXContext gGLXContext;
GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_square;
GLuint gVao_square1;
GLuint gVbo_position;
GLuint gVbo_position1;
GLuint gVbo_square_texture;
GLuint gMVPUniform;

mat4 gPerspectiveProjectionMatrix;

GLuint gTexture_Sampler_Uniform;
GLuint gTexture_name;
GLubyte checkImage[checkImageHeight][checkImageWidth][4];

int width,height;

int main(void){
	void CreateWindow(void);
	void ToggleFullScreen(void);
	void uninitialize();

	void initialize(void);
	void display(void);
	void resize(int,int);

	int winWidth = giWindowWidth;
	int winHeight = giWindowHeight;

	bool bDone = false;

	gpFile = fopen("Log.txt","w");
	if(!gpFile){
		fprintf(gpFile,"\n Unable to open log file \n Exitting now");
		exit(0);
	}else{
		fprintf(gpFile,"\n Log file opened \n");
	}

	CreateWindow();

	XEvent event;
	KeySym keysym;

	while(bDone == false){  // OGL change
		initialize();
		while(XPending(gpDisplay)){  // OGL change

			XNextEvent(gpDisplay,&event);
			switch(event.type){
				case MapNotify:
					break;
				case KeyPress:
				keysym = XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym){
					case XK_Escape:
					  uninitialize();
						exit(0);
						break;
					case XK_F:
					case XK_f:
						if(bFullScreen == false){
							ToggleFullScreen();
							bFullScreen = true;
						}else{
							ToggleFullScreen();
							bFullScreen = false;
						}
						break;
					default:
						break;
				}
				break;
				case ButtonPress:
				switch(event.xbutton.button){
					case 1:
						break;
					case 2:
						break;
					case 3:
						break;
				}
				break;
				case MotionNotify:
					break;
				case ConfigureNotify:
					winWidth = event.xconfigure.width;
					winHeight = event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
				case Expose:
					break;
				case DestroyNotify:
					break;
				case 33:
					uninitialize();
					exit(0);
				default:
					break;
			}
		}

		display();
	}
	uninitialize();
	return(0);
}

void CreateWindow(void){
	void uninitialize(void);

	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	// OGL change
	static int frameBufferAttributes[] = {
		GLX_DOUBLEBUFFER,True,
		GLX_RGBA,
		GLX_RED_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DEPTH_SIZE,24,
		None
	};

	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL){
		printf("ERROR : unable To Open X Display. \nExitting now");
		uninitialize();
		exit(1);
	}
	defaultScreen = XDefaultScreen(gpDisplay);

	gpXVisualInfo = glXChooseVisual(gpDisplay,defaultScreen,frameBufferAttributes);

	winAttribs.border_pixel = 0;
	winAttribs.background_pixmap = 0;

	winAttribs.colormap = XCreateColormap(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo -> screen),gpXVisualInfo -> visual, AllocNone);

	gColormap = winAttribs.colormap;

	winAttribs.background_pixel = BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask |KeyPressMask|PointerMotionMask|StructureNotifyMask;

	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	gWindow = XCreateWindow(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo -> screen),0,0,giWindowWidth,
		giWindowHeight,0,gpXVisualInfo -> depth,InputOutput,gpXVisualInfo -> visual, styleMask,&winAttribs);

	if(!gWindow){
		printf("Failed to create main window");
		uninitialize();
		exit(1);
	}

	XStoreName(gpDisplay,gWindow,"First XWindow");
	Atom windowManagerDelete = XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);
	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);
}

void ToggleFullScreen(void){
	Atom wm_state;
	Atom fullscreen;
	XEvent xev = {0};

	wm_state = XInternAtom(gpDisplay,"_NET_WM_STATE",False);

	memset(&xev,0,sizeof(xev));

	xev.type = ClientMessage;
	xev.xclient.window = gWindow;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = bFullScreen ? 0:1 ;

	fullscreen = XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);
	xev.xclient.data.l[1] = fullscreen;

	XSendEvent(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo -> screen),False,StructureNotifyMask,&xev);
}

void initialize(void){
	void resize(int,int);
	void uninitialize(void);
	int LoadGLTextures();
	gGLXContext = glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	GLenum glew_error = glewInit();
	if(glew_error != GLEW_OK){
		fprintf(gpFile,"\n Failed to initialize GLEW ");
		uninitialize();
		exit(0);
	}

	//Vertex gShader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	const GLchar *vertexShaderSourceCode =
		"#version 450 core"\
		"\n"\
		"in vec4 vPosition;"\
		"in vec2 vTexture0_coord;"\
		"out vec2 out_texture0_coord;"\
		"uniform mat4 u_mvp_matrix;"\
		"void main(void)"\
		"{"\
		"gl_Position = u_mvp_matrix * vPosition;"\
		"out_texture0_coord = vTexture0_coord;"\
		"}";

	glShaderSource(gVertexShaderObject,1,(const GLchar **) &vertexShaderSourceCode,NULL);
	glCompileShader(gVertexShaderObject);
	GLint iInfoLogLength = 0;
	GLint iShaderCompileStatus = 0;
	char *szInfoLog = NULL;

	glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompileStatus);
	if(iShaderCompileStatus == GL_FALSE){
		glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if(iInfoLogLength>0){
			szInfoLog = (char *)malloc(iInfoLogLength);
			if(szInfoLog != NULL){
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject,iInfoLogLength,&written,szInfoLog);
				fprintf(gpFile,"Vertex Shader Compilation Log: %s \n",szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	//Fragment Shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar *fragmentShaderSource =
		"#version 450 core"\
		"\n"\
		"in vec2 out_texture0_coord;"\
		"out vec4 fragColor;"\
		"uniform sampler2D u_texture0_sampler;"\
		"void main(void)"\
		"{"\
		"fragColor = texture(u_texture0_sampler,out_texture0_coord);"\
		"}";

		glShaderSource(gFragmentShaderObject,1,(const GLchar **) &fragmentShaderSource,NULL);
		glCompileShader(gFragmentShaderObject);

	  iInfoLogLength = 0;
		iShaderCompileStatus = 0;
		//szInfoLog = NULL;

		glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompileStatus);
		if(iShaderCompileStatus == GL_FALSE){
			glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
			if(iInfoLogLength >0){
				szInfoLog = (char *)malloc(iInfoLogLength);
				if(szInfoLog != NULL){
					GLsizei written;
					glGetShaderInfoLog(gFragmentShaderObject,iInfoLogLength,&written,szInfoLog);
					fprintf(gpFile, "Fragment Shader Compilation Log: %s\n",szInfoLog);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}

		// Link i.e. shader programm

		gShaderProgramObject = glCreateProgram();
		glAttachShader(gShaderProgramObject,gVertexShaderObject);
		glAttachShader(gShaderProgramObject,gFragmentShaderObject);
		glBindAttribLocation(gShaderProgramObject,VDG_ATTRIBUTE_VERTEX,"vPosition");
		glBindAttribLocation(gShaderProgramObject,VDG_ATTRIBUTE_TEXTURE0,"vTexture0_coord");
		glLinkProgram(gShaderProgramObject);

		GLint iShaderProgramLinkStatus = 0;
		iInfoLogLength = 0;
		//szInfoLog = NULL;
		glGetProgramiv(gShaderProgramObject,GL_LINK_STATUS,&iShaderProgramLinkStatus);
		if(iShaderProgramLinkStatus == GL_FALSE)
		{
			glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
			if(iInfoLogLength>0){
				szInfoLog = (char * )malloc(iInfoLogLength);
				if(szInfoLog != NULL){
					GLsizei written;
					glGetProgramInfoLog(gShaderProgramObject,iInfoLogLength,&written,szInfoLog);
					fprintf(gpFile, "Shader program link log: %s\n",szInfoLog);
					free(szInfoLog);
					uninitialize();
					exit(0);
				}
			}
		}

		gMVPUniform = glGetUniformLocation(gShaderProgramObject,"u_mvp_matrix");
		gTexture_Sampler_Uniform = glGetUniformLocation(gShaderProgramObject,"u_texture0_sampler");


const GLfloat squareVertices[] = {
									-1.0f,1.0f,0.0f,
									-1.0f,-1.0f,0.0f,
									 1.0f,-1.0f,0.0f,
									 1.0f,1.0f,0.0f};

const GLfloat squareVertices1[] = {
					 				1.0f,-1.0f,0.0f,
								 	1.0f,1.0f,0.0f,
				 					2.41421f,1.0f,-1.41421f,
								 	2.41421f,-1.0f,-1.41421f
								 	};

// Cube Vao

glGenVertexArrays(1,&gVao_square);
glBindVertexArray(gVao_square);

// vbo for cube position
glGenBuffers(1,&gVbo_position);
glBindBuffer(GL_ARRAY_BUFFER,gVbo_position);
glBufferData(GL_ARRAY_BUFFER,sizeof(squareVertices),squareVertices,GL_STATIC_DRAW);
glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,3,GL_FLOAT,GL_FALSE,0,NULL);
glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
glBindBuffer(GL_ARRAY_BUFFER,0);

// vbo for cube color
glGenBuffers(1,&gVbo_square_texture);
glBindBuffer(GL_ARRAY_BUFFER,gVbo_square_texture);
glBufferData(GL_ARRAY_BUFFER,4*2*sizeof(float),NULL,GL_DYNAMIC_DRAW);
glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,NULL);
glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
glBindBuffer(GL_ARRAY_BUFFER,0);

glBindVertexArray(0);
// VAO2
glGenVertexArrays(1,&gVao_square1);
glBindVertexArray(gVao_square1);

// vbo for cube position
glGenBuffers(1,&gVbo_position1);
glBindBuffer(GL_ARRAY_BUFFER,gVbo_position1);
glBufferData(GL_ARRAY_BUFFER,sizeof(squareVertices1),squareVertices1,GL_STATIC_DRAW);
glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX,3,GL_FLOAT,GL_FALSE,0,NULL);
glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
glBindBuffer(GL_ARRAY_BUFFER,0);

// vbo for cube color
glGenBuffers(1,&gVbo_square_texture);
glBindBuffer(GL_ARRAY_BUFFER,gVbo_square_texture);
glBufferData(GL_ARRAY_BUFFER,4*2*sizeof(float),NULL,GL_DYNAMIC_DRAW);
glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0,2,GL_FLOAT,GL_FALSE,0,NULL);
glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
glBindBuffer(GL_ARRAY_BUFFER,0);

glBindVertexArray(0);


 	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Optional but to keep smoothnes

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	//glEnable(GL_CULL_FACE);

	LoadGLTextures();

	glEnable(GL_TEXTURE_2D);

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	gPerspectiveProjectionMatrix = mat4::identity();

	resize(giWindowWidth,giWindowHeight);
}

int LoadGLTextures(void){
	void makeCheckImage(void);

	makeCheckImage();

	glGenTextures(1,&gTexture_name);
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);

	glBindTexture(GL_TEXTURE_2D,gTexture_name);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,checkImageWidth,checkImageHeight,0,GL_BGR,GL_UNSIGNED_BYTE,checkImage);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	return 0;
}

void makeCheckImage(void){
	int i,j,c;

	for(i=0;i<checkImageHeight;i++){
		for(j = 0; j<checkImageWidth; j++){
			c = (((i & 0x8) == 0)^((j & 0x8) == 0))*255;
			checkImage[i][j][0] = (GLubyte)c;
			checkImage[i][j][2] = (GLubyte)c;
			checkImage[i][j][3] = (GLubyte)c;
			checkImage[i][j][4] = (GLubyte)255;
		}
	}
}


void display(void){

	glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);

	const GLfloat squareTexCoords[] = {
		0.0f,0.0f,
		0.0f,1.0f,
		1.0f,1.0f,
		1.0f,0.0f
	};

	// pyramid display
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = vmath::translate(-1.0f,0.0f,-6.0f);

modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix;
glUniformMatrix4fv(gMVPUniform,1,GL_FALSE,modelViewProjectionMatrix);

//Texture
glBindBuffer(GL_ARRAY_BUFFER,gVbo_square_texture);
glBufferData(GL_ARRAY_BUFFER,sizeof(squareTexCoords),squareTexCoords,GL_DYNAMIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER,0);

//Texture
glActiveTexture(GL_TEXTURE0);
glUniform1i(gTexture_Sampler_Uniform,0);
glBindTexture(GL_TEXTURE_2D,gTexture_name);

glBindVertexArray(gVao_square);

glDrawArrays(GL_TRIANGLE_FAN,0,4);

glBindVertexArray(0);

// ------------------------------Bind VAO2--------------------------------------
modelViewMatrix = mat4::identity();
modelViewProjectionMatrix = mat4::identity();

modelViewMatrix = vmath::translate(1.0f,0.0f,-6.0f);

modelViewProjectionMatrix = gPerspectiveProjectionMatrix * modelViewMatrix;
glUniformMatrix4fv(gMVPUniform,1,GL_FALSE,modelViewProjectionMatrix);

//Texture
glBindBuffer(GL_ARRAY_BUFFER,gVbo_square_texture);
glBufferData(GL_ARRAY_BUFFER,sizeof(squareTexCoords),squareTexCoords,GL_DYNAMIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER,0);

//Texture
glActiveTexture(GL_TEXTURE0);
glUniform1i(gTexture_Sampler_Uniform,0);
glBindTexture(GL_TEXTURE_2D,gTexture_name);

glBindVertexArray(gVao_square1);

glDrawArrays(GL_TRIANGLE_FAN,0,4);

glBindVertexArray(0);

//------------------------------------------------------------------------------
glUseProgram(0);

glXSwapBuffers(gpDisplay,gWindow);
}

void resize(int width,int height){
	if(height == 0)
		height = 1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	gPerspectiveProjectionMatrix = vmath::perspective(60.0f,(GLfloat)width/(GLfloat)height,1.0f,30.0f);
}

void uninitialize(void){

	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();

	if(gpFile)
	{
		fprintf(gpFile,"\nlog file closed \n");
		fclose(gpFile);
		gpFile = NULL;
	}
	if(gVao_square){
		glDeleteVertexArrays(1,&gVao_square);
		gVao_square = 0;
	}
	if(gVbo_position){
		glDeleteBuffers(1,&gVbo_position);
		gVbo_position = 0;
	}
	if(gVbo_square_texture){
		glDeleteBuffers(1,&gVbo_square_texture);
		gVbo_square_texture = 0;
	}

	glDetachShader(gShaderProgramObject, gVertexShaderObject);
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject =  0;

	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	glUseProgram(0);

	if(currentGLXContext!=NULL && currentGLXContext == gGLXContext){
		glXMakeCurrent(gpDisplay,0,0);
	}

	if(gGLXContext){
		glXDestroyContext(gpDisplay,gGLXContext);
	}

	if(gWindow){
		XDestroyWindow(gpDisplay,gWindow);
	}
	if(gColormap){
		XFreeColormap(gpDisplay,gColormap);
	}
	if(gpXVisualInfo){
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}
	if(gpDisplay){
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}

}
