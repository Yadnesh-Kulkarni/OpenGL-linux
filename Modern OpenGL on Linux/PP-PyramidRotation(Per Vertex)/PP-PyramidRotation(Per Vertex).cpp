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

GLuint gVao_Pyramid;
GLuint gVbo_Position;
GLuint gVbo_Normals;
GLuint gMVPUniform;

GLuint La_uniform_left;
GLuint Ld_uniform_left;
GLuint Ls_uniform_left;
GLuint left_light_position_uniform;

GLuint La_uniform_right;
GLuint Ld_uniform_right;
GLuint Ls_uniform_right;
GLuint right_light_position_uniform;

GLuint Ka_uniform;
GLuint Kd_uniform;
GLuint Ks_uniform;
GLuint material_shininess_uniform;

GLuint model_matrix_uniform, view_matrix_uniform, projection_matrix_uniform;

GLuint L_KeyPressed_uniform;

mat4 gPerspectiveProjectionMatrix;

bool gbAnimate;
bool gbLight;

GLfloat gAnglePyramid = 0.0f;

GLfloat lightAmbient_left[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse_left[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightSpecular_left[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition_left[] = { -2.0f, 1.0f, 1.0f, 0.0f };

GLfloat lightAmbient_right[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse_right[] = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat lightSpecular_right[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition_right[] = { 2.0f, 1.0f, 1.0f, 0.0f };

GLfloat material_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_diffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat material_specular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat material_shininess = 50.0f;

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

							case XK_A:
							case XK_a; 
								if (!bIsAKeyPressed) {
									gbAnimate = true;
									bIsAKeyPressed = true;
								}
								else {
									gbAnimate = false;
									bIsAKeyPressed = false;
								}
								break;

							case XK_L:
							case XK_l:
								if (!bIsLKeyPressed) {
									gbLight = true;
									bIsLKeyPressed = true;
								}
								else {
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
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_lighting_enabled;" \
		"uniform int u_LKeyPressed;" \
		"uniform vec3 u_La, u_Ld, u_Ls, u_Ka, u_Kd, u_Ks;" \
		"uniform vec3 u_La2, u_Ld2, u_Ls2;" \
		"uniform vec4 u_light_position, u_light_position2;" \
		"uniform float u_material_shininess;" \
		"out vec3 out_phong_ads_color;" \
		"vec3 phong_ads_color;" \
		"vec3 CalculateLight(vec3 La, vec3 Ld, vec3 Ls, vec4 light_position)"
		"{" \
		"vec4 eyeCoordinates = u_view_matrix * u_model_matrix * vPosition;" \
		"vec3 transformed_normals = normalize(mat3(u_view_matrix * u_model_matrix) * vNormal);" \
		"vec3 light_direction = normalize(vec3(light_position) - eyeCoordinates.xyz);" \
		"float tn_dot_ld = max(dot(transformed_normals, light_direction), 0.0);" \
		"vec3 ambient = La * u_Kd;" \
		"vec3 diffuse = Ld * u_Kd * tn_dot_ld;" \
		"vec3 reflection_vector = reflect(-light_direction, transformed_normals);" \
		"vec3 viewer_vector = normalize(-eyeCoordinates.xyz);" \
		"vec3 specular = Ls * u_Ks * pow(max(dot(reflection_vector, viewer_vector), 0.0), u_material_shininess);" \
		"phong_ads_color = ambient + diffuse + specular;" \
		"return phong_ads_color;" \
		"}" \
		"void main(void)" \
		"{" \
		"if (u_lighting_enabled == 1)" \
		"{" \
		"vec3 lightleft = CalculateLight(u_La, u_Ld, u_Ls, u_light_position);" \
		"vec3 lightright = CalculateLight(u_La2, u_Ld2, u_Ls2, u_light_position2);" \
		"out_phong_ads_color = lightleft + lightright;" \
		"}" \
		"else" \
		"{" \
		"out_phong_ads_color = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
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
		"in vec3 out_phong_ads_color;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = vec4(out_phong_ads_color, 1.0f);" \
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

	model_matrix_uniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	view_matrix_uniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	projection_matrix_uniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	L_KeyPressed_uniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled");

	La_uniform_right = glGetUniformLocation(gShaderProgramObject, "u_La");
	Ld_uniform_right = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	Ls_uniform_right = glGetUniformLocation(gShaderProgramObject, "u_Ls");
	right_light_position_uniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");

	La_uniform_left = glGetUniformLocation(gShaderProgramObject, "u_La2");
	Ld_uniform_left = glGetUniformLocation(gShaderProgramObject, "u_Ld2");
	Ls_uniform_left = glGetUniformLocation(gShaderProgramObject, "u_Ls2");
	left_light_position_uniform = glGetUniformLocation(gShaderProgramObject, "u_light_position2");

	Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	material_shininess_uniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");

	const GLfloat pyramidVertices[] = {
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f,-1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,

		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f,-1.0f,
		-1.0f, -1.0f, 1.0f
	};

	const GLfloat pyramidNormals[] = {
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		1.0f, 0.0f, 0.0,
		1.0f, 0.0f, 0.0,
		1.0f, 0.0f, 0.0,

		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, -1.0f,

		-1.0f, 0.0f, 0.0,
		-1.0f, 0.0f, 0.0,
		-1.0f, 0.0f, 0.0
	};

	// Vertex Array Object
	glGenVertexArrays(1, &gVao_Pyramid);
	glBindVertexArray(gVao_Pyramid);

	glGenBuffers(1, &gVbo_Position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_Position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(OVS_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(OVS_ATTRIBUTE_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &gVbo_Normals);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_Normals);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);

	glVertexAttribPointer(OVS_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(OVS_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glShadeModel(GL_FLAT);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	gPerspectiveProjectionMatrix = mat4::identity();

	gbLight = false;

	resize(giWindowWidth, giWindowHeight);
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);

	if (gbLight == true)
	{
		glUniform1i(L_KeyPressed_uniform, 1);

		// Set light
		glUniform3fv(La_uniform_right, 1, lightAmbient_right);
		glUniform3fv(Ld_uniform_right, 1, lightDiffuse_right);
		glUniform3fv(Ls_uniform_right, 1, lightSpecular_right);
		glUniform4fv(right_light_position_uniform, 1, lightPosition_right);

		glUniform3fv(La_uniform_left, 1, lightAmbient_left);
		glUniform3fv(Ld_uniform_left, 1, lightDiffuse_left);
		glUniform3fv(Ls_uniform_left, 1, lightSpecular_left);
		glUniform4fv(left_light_position_uniform, 1, lightPosition_left);

		// Set material
		glUniform3fv(Ka_uniform, 1, material_ambient);
		glUniform3fv(Kd_uniform, 1, material_diffuse);
		glUniform3fv(Ks_uniform, 1, material_specular);
		glUniform1f(material_shininess_uniform, material_shininess);
	}
	else
	{
		glUniform1i(L_KeyPressed_uniform, 0);
	}

	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();

	modelMatrix = translate(0.0f, 0.0f, -6.0f);

	rotationMatrix = rotate(gAnglePyramid, 0.0f, 1.0f, 0.0f);

	modelMatrix = viewMatrix * modelMatrix * rotationMatrix;

	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	// Bind vao
	glBindVertexArray(gVao_Pyramid);

	glDrawArrays(GL_TRIANGLES, 0, 12);

	glBindVertexArray(0);

	glUseProgram(0);
	glXSwapBuffers(gpDisplay, gWindow);
}

void update(void) {
	gAnglePyramid = gAnglePyramid + 0.01f;
	if (gAnglePyramid >= 360.0f) {
		gAnglePyramid = 0.0f;
	}
}

void resize(int width, int height){
	if(height == 0)
		height = 1;

	glViewport(0,0,(GLsizei)width, (GLsizei)height);
	
	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
}

void Uninitialize(void){
	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();
	
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
























