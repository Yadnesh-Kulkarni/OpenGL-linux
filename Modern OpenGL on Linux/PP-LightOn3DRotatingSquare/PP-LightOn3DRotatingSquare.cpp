#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include<GL/glew.h>
#include<GL/gl.h>
#include<GL/glx.h>

#include "vmath.h"

using namespace std;
using namespace vmath;

bool bFullscreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;

int giWindowWidth = 800;
int giWindowHeight = 600;

GLXContext gGLXContext;

FILE *gpFile = NULL;

enum {
	OVS_ATTRIBUTE_VERTEX = 0,
	OVS_ATTRIBUTE_COLOR,
	OVS_ATTRIBUTE_NORMAL,
	OVS_ATTRIBUTE_TEXTURE0
};

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_Cube;
GLuint gVbo_Cube_Position;
GLuint gVbo_Cube_Normal;
GLuint gMVPUniform;

GLfloat gAngleCube = 0.0f;

GLuint gModelViewMatrixUniform, gProjectionMatrixUniform;
GLuint gLdUniform, gKdUniform, gLightPositionUniform;

GLuint gLKeyPressedUniform;

mat4 gPerspectiveProjectionMatrix;

bool gbAnimate;
bool gbLight;

int main(void){
	void Initialize(void);
	void CreateWindow(void);
	void display(void);
	void ToggleFullscreen(void);
	void resize(int, int);
	void Uninitialize(void);

	int winWidth = giWindowWidth;
	int winHeight = giWindowHeight;

	bool bDone = false;

	gpFile = fopen("Log.txt", "w");
	if(gpFile != NULL){
		fprintf(gpFile, "Log file created successfully.");
	}
	else{
		printf("Failed to create log file.");
		exit(0);
	}

	CreateWindow();

	Initialize();

	XEvent event;
	KeySym keysym;

	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event);
			switch(event.type)
			{
				case MapNotify:
					break;
				case KeyPress:
					keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym)
					{
						case XK_Escape:
							Uninitialize();
							exit(0);
						case XK_F:
						case XK_f:
							if(bFullscreen==false)
							{
								ToggleFullscreen();
								bFullscreen=true;
							}
							else
							{
								ToggleFullscreen();
								bFullscreen=false;
							}
							break;

						case XK_A: // for 'A' or 'a'
						case XK_a:
							if (bIsAKeyPressed == false)
							{
								gbAnimate = true;
								bIsAKeyPressed = true;
							}
							else
							{
								gbAnimate = false;
								bIsAKeyPressed = false;
							}
							break;
						case XK_L: // for 'L' or 'l'
						case XK_l:
							if (bIsLKeyPressed == false)
							{
								gbLight = true;
								bIsLKeyPressed = true;
							}
							else
							{
								gbLight = false;
								bIsLKeyPressed = false;
							}
							break;
						default:
							break;
					}
					break;
				case ButtonPress: 
					switch(event.xbutton.button)
					{
						case 1: 
						    break;
						case 2: 
						    break;
						case 3: 
						    break;
						default:
						    break;
					}
					break;
				case MotionNotify: 
					break;
				case ConfigureNotify: 
					winWidth=event.xconfigure.width;
					winHeight=event.xconfigure.height;
					resize(winWidth,winHeight);
					break;
				case Expose: 
					break;
				case DestroyNotify:
					break;
				case 33:
					bDone=true;
					break;
				default:
					break;
			}
		
		}
		if (gbAnimate == true)
			update();
		display();
	}
	Uninitialize();
	return(0);
}

void CreateWindow(void){
	void Uninitialize(void);
	
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	static int frameBufferAttributes[]=
	{
		GLX_RGBA,GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DEPTH_SIZE,24,
		None
	};

	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL){
		printf("Error: Unable to open X Display....\nExiting now");
		Uninitialize();
		exit(1);
	}

	defaultScreen = XDefaultScreen(gpDisplay);

	defaultDepth = DefaultDepth(gpDisplay, defaultScreen);

	gpXVisualInfo=glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);

	winAttribs.border_pixel = 0;
	winAttribs.background_pixmap = 0;
	winAttribs.colormap = XCreateColormap(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen), gpXVisualInfo->visual, AllocNone);
	gColormap = winAttribs.colormap;
	winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
	winAttribs.event_mask = ExposureMask | VisibilityChangeMask | ButtonPressMask | KeyPressMask | PointerMotionMask | StructureNotifyMask;

	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	gWindow = XCreateWindow(gpDisplay,RootWindow(gpDisplay, gpXVisualInfo->screen),
				0,
				0,
				giWindowWidth,
				giWindowHeight,
				0,
				gpXVisualInfo->depth,
				InputOutput,
				gpXVisualInfo->visual,
				styleMask,
				&winAttribs);

	if(!gWindow){
		printf("Error : Failed to Create Main Window\nExitting Now...");
		Uninitialize();
		exit(1);
	}

	XStoreName(gpDisplay, gWindow, "Perspective Triangle in Programmable Pipeline");

	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete,1);

	XMapWindow(gpDisplay, gWindow);
}

void ToggleFullscreen(void){
	Atom wm_state;
	Atom fullscreen;
	XEvent xev = {0};

	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE", False);
	memset(&xev,0,sizeof(xev));

	xev.type = ClientMessage;
	xev.xclient.window = gWindow;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1;
	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN", False);
	xev.xclient.data.l[1] = fullscreen;

	XSendEvent(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen),
			False,
			StructureNotifyMask,
			&xev);
}

void Initialize(void){
	void resize(int, int);
	void Uninitialize(void);

	gGLXContext = glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);
	
	glXMakeCurrent(gpDisplay, gWindow, gGLXContext);
	
	glewInit();
	
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar *vertexShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_LKeyPressed;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Kd;" \
		"uniform vec4 u_light_position;" \
		"out vec3 diffuse_light;" \
		"void main(void)" \
		"{" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates = u_model_view_matrix * vPosition;" \
		"vec3 tnorm = normalize(mat3(u_model_view_matrix) * vNormal);" \
		"vec3 s = normalize(vec3(u_light_position - eyeCoordinates));" \
		"diffuse_light = u_Ld * u_Kd * max(dot(s, tnorm), 0.0);" \
		"}" \
		"gl_Position = u_projection_matrix * u_model_view_matrix * vPosition;" \
		"}";
	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	// Compile the shader
	glCompileShader(gVertexShaderObject);

	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;

	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE) {
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0) {
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				Uninitialize();
				exit(0);
			}
		}
	}

	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	GLchar *fragmentShaderSourceCode =
		"#version 450 core" \
		"\n" \
		"in vec3 diffuse_light;" \
		"out vec4 FragColor;" \
		"uniform int u_LKeyPressed;" \
		"void main(void)" \
		"{" \
		"vec4 color;" \
		"if (u_LKeyPressed == 1)" \
		"{" \
		"color = vec4(diffuse_light,1.0);" \
		"}" \
		"else" \
		"{" \
		"color = vec4(1.0, 1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = color;" \
		"}";

	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	// Compile the shader
	glCompileShader(gFragmentShaderObject);

	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE) {
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0) {
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				Uninitialize();
				exit(0);
			}
		}
	}

	gShaderProgramObject = glCreateProgram();

	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	glBindAttribLocation(gShaderProgramObject, OVS_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, OVS_ATTRIBUTE_NORMAL, "vNormal");

	glLinkProgram(gShaderProgramObject);

	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE) {
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0) {
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL) {
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Shader program link log : %s\n", szInfoLog);
				free(szInfoLog);
				Uninitialize();
				exit(0);
			}
		}
	}

	gModelViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	gLKeyPressedUniform = glGetUniformLocation(gShaderProgramObject, "u_LKeyPressed");

	gLdUniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	gKdUniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	gLightPositionUniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");;

	GLfloat CubeVertices[] =
	{
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,

		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f
	};

	const GLfloat CubeNormals[] =
	{
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 0.0f,

		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,

		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	for (int i = 0; i < 72; i++) {
		if (CubeVertices[i] > 0.0f)
			CubeVertices[i] = CubeVertices[i] - 0.25f;
		else
			CubeVertices[i] = CubeVertices[i];
	}

	// Cube
	glGenVertexArrays(1, &gVao_Cube);
	glBindVertexArray(gVao_Cube);

	glGenBuffers(1, &gVbo_Cube_Position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_Cube_Position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeVertices), CubeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(OVS_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(OVS_ATTRIBUTE_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// For Vbo_Color
	glGenBuffers(1, &gVbo_Cube_Normal);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_Cube_Normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeNormals), CubeNormals, GL_STATIC_DRAW);

	glVertexAttribPointer(OVS_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(OVS_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	gPerspectiveProjectionMatrix = mat4::identity();

	gbAnimate = false;
	gbLight = false;

	resize(giWindowWidth, giWindowHeight);
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);

	if (gbLight == true)
	{
		glUniform1i(gLKeyPressedUniform, 1);

		glUniform3f(gLdUniform, 1.0f, 1.0f, 1.0f);
		glUniform3f(gKdUniform, 0.5f, 0.5f, 0.5f);

		float lightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };
		glUniform4fv(gLightPositionUniform, 1, (GLfloat *)lightPosition);
	}
	else
	{
		glUniform1i(gLKeyPressedUniform, 0);
	}

	// Cube
	mat4 modelViewMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	mat4 modelMatrix = mat4::identity();

	modelMatrix = translate(0.0f, 0.0f, -5.0f);
	rotationMatrix = rotate(gAngleCube, gAngleCube, gAngleCube);
	modelViewMatrix = modelMatrix * rotationMatrix;

	glUniformMatrix4fv(gModelViewMatrixUniform, 1, GL_FALSE, modelViewMatrix);

	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	glBindVertexArray(gVao_Cube);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
	glDrawArrays(GL_TRIANGLE_FAN, 20, 4);
	glBindVertexArray(0);

	glUseProgram(0);

	glXSwapBuffers(gpDisplay, gWindow);
}

void resize(int width, int height){
	if(height == 0)
		height = 1;

	glViewport(0,0,(GLsizei)width, (GLsizei)height);
	
	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
}

void update(void) {
	gAngleCube = gAngleCube + 0.01f;

	if (gAngleCube > 360.0f) {
		gAngleCube = 0.0f;
	}
}

void Uninitialize(void){
	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();
	
	if (gVao_Cube)
	{
		glDeleteVertexArrays(1, &gVao_Cube);
		gVao_Cube = 0;
	}

	if (gVbo_Cube_Position)
	{
		glDeleteBuffers(1, &gVbo_Cube_Position);
		gVbo_Cube_Position = 0;
	}

	if (gVbo_Cube_Normal)
	{
		glDeleteBuffers(1, &gVbo_Cube_Normal);
		gVbo_Cube_Normal = 0;
	}
	
	glDetachShader(gShaderProgramObject, gVertexShaderObject);

	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;

	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	glUseProgram(0);
	
	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext){
		glXMakeCurrent(gpDisplay,0,0);
	}

	if(gGLXContext){
		glXDestroyContext(gpDisplay, gGLXContext);
	}

	if(gWindow){
		XDestroyWindow(gpDisplay, gWindow);
	}

	if(gColormap){
		XFreeColormap(gpDisplay, gColormap);
	}

	if(gpXVisualInfo){
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;
	}

	if(gpDisplay){
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}
	
	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
























