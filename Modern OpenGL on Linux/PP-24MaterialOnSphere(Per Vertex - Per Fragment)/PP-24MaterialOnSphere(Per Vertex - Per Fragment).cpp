#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include"Sphere.h"

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

GLuint gVao_sphere;
GLuint gVbo_sphere_position;
GLuint gVbo_sphere_normal;
GLuint gVbo_sphere_element;
GLuint gMVPUniform;

GLuint gNumVertices;
GLuint gNumElements;

GLfloat sin_red = 0.0f;
GLfloat cos_red = 0.0f;

GLfloat sin_green = 0.0f;
GLfloat cos_green = 0.0f;

GLfloat sin_blue = 0.0f;
GLfloat cos_blue = 0.0f;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

GLfloat rotationAngle = 0.0f;

GLuint gViewMatrixUniform, gModelMatrixUniform, gProjectionMatrixUniform, view_matrix_uniform;
GLuint gLdUniform, gKdUniform, glightPosition_RedUniform;

GLuint gLKeyPressedUniform;

GLuint L_KeyPressed_uniform;
GLuint F_KeyPressed_uniform;
GLuint V_KeyPressed_uniform;

GLuint La_uniform_red;
GLuint Ld_uniform_red;
GLuint Ls_uniform_red;
GLuint light_position_uniform_red;

GLuint La_uniform_green;
GLuint Ld_uniform_green;
GLuint Ls_uniform_green;
GLuint light_position_uniform_green;

GLuint La_uniform_blue;
GLuint Ld_uniform_blue;
GLuint Ls_uniform_blue;
GLuint light_position_uniform_blue;

GLuint Ka_uniform;
GLuint Kd_uniform;
GLuint Ks_uniform;
GLuint material_shininess_uniform;

mat4 gPerspectiveProjectionMatrix;

bool gbVertexLighting = false;
bool gbFragmentLighting = false;

GLuint gwidth = 0;
GLuint gheight = 0;

GLfloat lightAmbient_red[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffuse_red[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat lightSpecular_red[] = { 1.0f,0.0f,0.0f,1.0f };
GLfloat lightPosition_red[] = { -2.0f,1.0f,0.0f,1.0f };

GLfloat lightAmbient_green[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffuse_green[] = { 0.0f,1.0f,0.0f,1.0f };
GLfloat lightSpecular_green[] = { 0.0f,1.0f,0.0f,1.0f };
GLfloat lightPosition_green[] = { 2.0f,1.0f,0.0f,1.0f };

GLfloat lightAmbient_blue[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat lightDiffuse_blue[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat lightSpecular_blue[] = { 0.0f,0.0f,1.0f,1.0f };
GLfloat lightPosition_blue[] = { 0.0f,0.0f,2.0f,1.0f };

//Material
// 1 x 1
GLfloat material_ambient_1_1[] = { 0.0215f,0.1745f,0.0215f,1.0f };
GLfloat material_diffuse_1_1[] = { 0.07568f,0.61424f,0.07568f,1.0f };
GLfloat material_specular_1_1[] = { 0.633f,0.727811f,0.633f,1.0f };
GLfloat material_shininess_1_1 = { 0.6f * 128.0f };

// 1 x 2
GLfloat material_ambient_1_2[] = { 0.135f,0.2225f,0.1575f,1.0f };
GLfloat material_diffuse_1_2[] = { 0.54f,0.89f,0.63f,1.0f };
GLfloat material_specular_1_2[] = { 0.316228f,0.316228f,0.316228f,1.0f };
GLfloat material_shininess_1_2 = { 0.1f * 128.0f };

// 1 x 3
GLfloat material_ambient_1_3[] = { 0.05357f,0.05f,0.06625f,1.0f };
GLfloat material_diffuse_1_3[] = { 0.18275f,0.17f,0.22525f,1.0f };
GLfloat material_specular_1_3[] = { 0.332741f,0.328634f,0.346435f,1.0f };
GLfloat material_shininess_1_3 = { 0.3f * 128.0f };

// 1 x 4
GLfloat material_ambient_1_4[] = { 0.25f,0.20725f,0.20725f,1.0f };
GLfloat material_diffuse_1_4[] = { 1.0f,0.829f,0.829f,1.0f };
GLfloat material_specular_1_4[] = { 0.296648f,0.296648f,0.296648f,1.0f };
GLfloat material_shininess_1_4 = { 0.088f * 128.0f };

// 1 x 5
GLfloat material_ambient_1_5[] = { 0.1745f,0.01175f,0.01175f,1.0f };
GLfloat material_diffuse_1_5[] = { 0.61424f,0.04136f,0.04136f,1.0f };
GLfloat material_specular_1_5[] = { 0.727811f,0.626959f,0.626959f,1.0f };
GLfloat material_shininess_1_5 = { 0.6f * 128.0f };

// 1 x 6
GLfloat material_ambient_1_6[] = { 0.1f,0.18725f,0.1745f,1.0f };
GLfloat material_diffuse_1_6[] = { 0.369f,0.74151f,0.69102f,1.0f };
GLfloat material_specular_1_6[] = { 0.297254f,0.30829f,0.306678f,1.0f };
GLfloat material_shininess_1_6 = { 0.1f * 128.0f };

// 2 x 1
GLfloat material_ambient_2_1[] = { 0.329412f,0.223529f,0.027451f,1.0f };
GLfloat material_diffuse_2_1[] = { 0.780392f,0.568627f,0.113725f,1.0f };
GLfloat material_specular_2_1[] = { 0.992157f,0.941176f,0.807843f,1.0f };
GLfloat material_shininess_2_1 = { 0.21794872f * 128.0f };

// 2 x 2
GLfloat material_ambient_2_2[] = { 0.2125f,0.1275f,0.054f,1.0f };
GLfloat material_diffuse_2_2[] = { 0.714f,0.4284f,0.18144f,1.0f };
GLfloat material_specular_2_2[] = { 0.393548f,0.271906f,0.166721f,1.0f };
GLfloat material_shininess_2_2 = { 0.2f * 128.0f };

// 2 x 3
GLfloat material_ambient_2_3[] = { 0.25f,0.25f,0.25f,1.0f };
GLfloat material_diffuse_2_3[] = { 0.4f,0.4f,0.4f,1.0f };
GLfloat material_specular_2_3[] = { 0.774597f,0.774597f,0.774597f,1.0f };
GLfloat material_shininess_2_3 = { 0.6f * 128.0f };

// 2 x 4
GLfloat material_ambient_2_4[] = { 0.19125f,0.0735f,0.0225f,1.0f };
GLfloat material_diffuse_2_4[] = { 0.7038f,0.27048f,0.0828f,1.0f };
GLfloat material_specular_2_4[] = { 0.256777f,0.137622f,0.086014f,1.0f };
GLfloat material_shininess_2_4 = { 0.1f * 128.0f };

// 2 x 5
GLfloat material_ambient_2_5[] = { 0.24725f,0.1995f,0.0745f,1.0f };
GLfloat material_diffuse_2_5[] = { 0.75164f,0.60648f,0.22648f,1.0f };
GLfloat material_specular_2_5[] = { 0.628281f,0.555802f,0.366065f,1.0f };
GLfloat material_shininess_2_5 = { 0.4f * 128.0f };

// 2 x 6
GLfloat material_ambient_2_6[] = { 0.19255f,0.19225f,0.19225f,1.0f };
GLfloat material_diffuse_2_6[] = { 0.50754f,0.50754f,0.50754f,1.0f };
GLfloat material_specular_2_6[] = { 0.58273f,0.508273f,0.508273f,1.0f };
GLfloat material_shininess_2_6 = { 0.4f * 128.0f };

// 3 x 1
GLfloat material_ambient_3_1[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_diffuse_3_1[] = { 0.01f,0.01f,0.01f,1.0f };
GLfloat material_specular_3_1[] = { 0.50f,0.50f,0.50f,1.0f };
GLfloat material_shininess_3_1 = { 0.25f * 128.0f };

// 3 x 2
GLfloat material_ambient_3_2[] = { 0.0f,0.1f,0.06f,1.0f };
GLfloat material_diffuse_3_2[] = { 0.0f,0.50980392f,0.50980392f,1.0f };
GLfloat material_specular_3_2[] = { 0.50196078f,0.50196078f,0.50196078f,1.0f };
GLfloat material_shininess_3_2 = { 0.25f * 128.0f };

// 3 x 3
GLfloat material_ambient_3_3[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_diffuse_3_3[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat material_specular_3_3[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat material_shininess_3_3 = { 0.25f * 128.0f };

// 3 x 4
GLfloat material_ambient_3_4[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_diffuse_3_4[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat material_specular_3_4[] = { 0.7f,0.6f,0.6f,1.0f };
GLfloat material_shininess_3_4 = { 0.25f * 128.0f };

// 3 x 5
GLfloat material_ambient_3_5[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_diffuse_3_5[] = { 0.55f,0.55f,0.55f,1.0f };
GLfloat material_specular_3_5[] = { 0.70f,0.70f,0.70f,1.0f };
GLfloat material_shininess_3_5 = { 0.25f * 128.0f };

// 3 x 6
GLfloat material_ambient_3_6[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_diffuse_3_6[] = { 0.5f,0.5f,0.0f,1.0f };
GLfloat material_specular_3_6[] = { 0.60f,0.60f,0.50f,1.0f };
GLfloat material_shininess_3_6 = { 0.25f * 128.0f };

// 4 x 1
GLfloat material_ambient_4_1[] = { 0.02f,0.02f,0.02f,1.0f };
GLfloat material_diffuse_4_1[] = { 0.01f,0.01f,0.01f,1.0f };
GLfloat material_specular_4_1[] = { 0.4f,0.4f,0.4f,1.0f };
GLfloat material_shininess_4_1 = { 0.078125f * 128.0f };

// 4 x 2
GLfloat material_ambient_4_2[] = { 0.0f,0.05f,0.05f,1.0f };
GLfloat material_diffuse_4_2[] = { 0.4f,0.5f,0.5f,1.0f };
GLfloat material_specular_4_2[] = { 0.04f,0.7f,0.7f,1.0f };
GLfloat material_shininess_4_2 = { 0.78125f * 128.0f };

// 4 x 3
GLfloat material_ambient_4_3[] = { 0.0f,0.05f,0.0f,1.0f };
GLfloat material_diffuse_4_3[] = { 0.4f,0.5f,0.4f,1.0f };
GLfloat material_specular_4_3[] = { 0.04f,0.7f,0.04f,1.0f };
GLfloat material_shininess_4_3 = { 0.078125f * 128.0f };

// 4 x 4
GLfloat material_ambient_4_4[] = { 0.05f,0.0f,0.0f,1.0f };
GLfloat material_diffuse_4_4[] = { 0.5f,0.4f,0.4f,1.0f };
GLfloat material_specular_4_4[] = { 0.7f,0.04f,0.04f,1.0f };
GLfloat material_shininess_4_4 = { 0.078125f * 128.0f };

// 4 x 5
GLfloat material_ambient_4_5[] = { 0.05f,0.05f,0.05f,1.0f };
GLfloat material_diffuse_4_5[] = { 0.5f,0.5f,0.5f,1.0f };
GLfloat material_specular_4_5[] = { 0.7f,0.7f,0.7f,1.0f };
GLfloat material_shininess_4_5 = { 0.078125f * 128.0f };

// 4 x 6
GLfloat material_ambient_4_6[] = { 0.05f,0.05f,0.0f,1.0f };
GLfloat material_diffuse_4_6[] = { 0.5f,0.5f,0.4f,1.0f };
GLfloat material_specular_4_6[] = { 0.7f,0.7f,0.04f,1.0f };
GLfloat material_shininess_4_6 = { 0.078125f * 128.0f };


int main(void){
	void Initialize(void);
	void CreateWindow(void);
	void display(void);
	void spin(void);
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
						case XK_Q:
						case XK_q:
							Uninitialize();
							exit(0);

						case XK_F:
						case XK_f:
							gbVertexLighting = false;
							gbFragmentLighting = true;
							break;

						case XK_V: // V
						case XK_v:
							gbVertexLighting = true;
							gbFragmentLighting = false;
							break;

						case XK_Escape:
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
		spin();
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
		"uniform int u_f_lighting_enabled;" \
		"uniform int u_v_lighting_enabled;" \
		"uniform vec3 u_La_red;" \
		"uniform vec3 u_Ld_red;" \
		"uniform vec3 u_Ls_red;" \
		"uniform vec4 u_light_position_red;" \
		"uniform vec3 u_La_green;" \
		"uniform vec3 u_Ld_green;" \
		"uniform vec3 u_Ls_green;" \
		"uniform vec4 u_light_position_green;" \
		"uniform vec3 u_La_blue;" \
		"uniform vec3 u_Ld_blue;" \
		"uniform vec3 u_Ls_blue;" \
		"uniform vec4 u_light_position_blue;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"vec3 phong_ads_color;" \
		"out vec3 out_phong_ads_color;" \
		"out vec3 transformed_normals;" \
		"out vec3 light_direction_red;" \
		"out vec3 light_direction_green;" \
		"out vec3 light_direction_blue;" \
		"out vec3 viewer_vector;" \
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
		"if(u_v_lighting_enabled == 1)" \
		"{" \
		"vec3 light_red = CalculateLight(u_La_red, u_Ld_red, u_Ls_red, u_light_position_red);" \
		"vec3 light_green = CalculateLight(u_La_green, u_Ld_green, u_Ls_green, u_light_position_green);" \
		"vec3 light_blue = CalculateLight(u_La_blue, u_Ld_blue, u_Ls_blue, u_light_position_blue);" \
		"out_phong_ads_color = light_red + light_green + light_blue;" \
		"}" \
		"if(u_f_lighting_enabled == 1)" \
		"{" \
		"vec4 eye_coordinates=u_view_matrix * u_model_matrix * vPosition;" \
		"transformed_normals=mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction_red = vec3(u_light_position_red) - eye_coordinates.xyz;" \
		"light_direction_green = vec3(u_light_position_green) - eye_coordinates.xyz;" \
		"light_direction_blue = vec3(u_light_position_blue) - eye_coordinates.xyz;" \
		"viewer_vector = -eye_coordinates.xyz;" \
		"}" \
		"gl_Position=u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
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
		"in vec3 transformed_normals;" \
		"in vec3 light_direction_red;" \
		"in vec3 light_direction_green;" \
		"in vec3 light_direction_blue;" \
		"in vec3 viewer_vector;" \
		"in vec3 out_phong_ads_color;" \
		"out vec4 FragColor;" \
		"uniform vec3 u_La_red;" \
		"uniform vec3 u_Ld_red;" \
		"uniform vec3 u_Ls_red;" \
		"uniform vec3 u_La_green;" \
		"uniform vec3 u_Ld_green;" \
		"uniform vec3 u_Ls_green;" \
		"uniform vec3 u_La_blue;" \
		"uniform vec3 u_Ld_blue;" \
		"uniform vec3 u_Ls_blue;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"uniform int u_lighting_enabled;" \
		"uniform int u_v_lighting_enabled;" \
		"uniform int u_f_lighting_enabled;" \
		"vec3 CalculateLight(vec3 u_La, vec3 u_Ld, vec3 u_Ls, vec3 light_direction)" \
		"{" \
		"vec3 phong_ads_color;" \
		"vec3 normalized_transformed_normals=normalize(transformed_normals);" \
		"vec3 normalized_light_direction=normalize(light_direction);" \
		"vec3 normalized_viewer_vector=normalize(viewer_vector);" \
		"vec3 ambient = u_La * u_Ka;" \
		"float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction),0.0);" \
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);" \
		"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), u_material_shininess);" \
		"phong_ads_color=ambient + diffuse + specular;" \
		"return phong_ads_color;" \
		"}" \
		"void main(void)" \
		"{" \
		"if(u_v_lighting_enabled == 1)" \
		"{" \
		"FragColor = vec4(out_phong_ads_color, 1.0);" \
		"}" \
		"if(u_f_lighting_enabled == 1)" \
		"{" \
		"vec3 light_red = CalculateLight(u_La_red, u_Ld_red, u_Ls_red, light_direction_red);" \
		"vec3 light_green = CalculateLight(u_La_green, u_Ld_green, u_Ls_green, light_direction_green);" \
		"vec3 light_blue = CalculateLight(u_La_blue, u_Ld_blue, u_Ls_blue, light_direction_blue);" \
		"vec3 phong_ads_color = light_red + light_green + light_blue;" \
		"FragColor = vec4(phong_ads_color, 1.0);" \
		"}" \
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

	gModelMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	F_KeyPressed_uniform = glGetUniformLocation(gShaderProgramObject, "u_f_lighting_enabled");
	V_KeyPressed_uniform = glGetUniformLocation(gShaderProgramObject, "u_v_lighting_enabled");

	La_uniform_red = glGetUniformLocation(gShaderProgramObject, "u_La_red");
	Ld_uniform_red = glGetUniformLocation(gShaderProgramObject, "u_Ld_red");
	Ls_uniform_red = glGetUniformLocation(gShaderProgramObject, "u_Ls_red");
	light_position_uniform_red = glGetUniformLocation(gShaderProgramObject, "u_light_position_red");

	La_uniform_green = glGetUniformLocation(gShaderProgramObject, "u_La_green");
	Ld_uniform_green = glGetUniformLocation(gShaderProgramObject, "u_Ld_green");
	Ls_uniform_green = glGetUniformLocation(gShaderProgramObject, "u_Ls_green");
	light_position_uniform_green = glGetUniformLocation(gShaderProgramObject, "u_light_position_green");

	La_uniform_blue = glGetUniformLocation(gShaderProgramObject, "u_La_blue");
	Ld_uniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Ld_blue");
	Ls_uniform_blue = glGetUniformLocation(gShaderProgramObject, "u_Ls_blue");
	light_position_uniform_blue = glGetUniformLocation(gShaderProgramObject, "u_light_position_blue");

	Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	material_shininess_uniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");

	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	glGenVertexArrays(1, &gVao_sphere);
	glBindVertexArray(gVao_sphere);

	// Position
	glGenBuffers(1, &gVbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(OVS_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(OVS_ATTRIBUTE_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals
	glGenBuffers(1, &gVbo_sphere_normal);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(OVS_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(OVS_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Elements
	glGenBuffers(1, &gVbo_sphere_element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.25f, 0.25f, 0.25f, 0.0f);

	gPerspectiveProjectionMatrix = mat4::identity();

	resize(giWindowWidth, giWindowHeight);
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);

	sin_red = cos(3.1415 * rotationAngle / 180.0f) * 10.0f;
	cos_red = sin(3.1415 * rotationAngle / 180.0f) * 10.0f;

	sin_green = cos(3.1415 * rotationAngle / 180.0f) * 10.0f;
	cos_green = sin(3.1415 * rotationAngle / 180.0f) * 10.0f;

	sin_blue = cos(3.1415 * rotationAngle / 180.0f) * 10.0f;
	cos_blue = sin(3.1415 * rotationAngle / 180.0f) * 10.0f;

	glUniform1i(V_KeyPressed_uniform, 1);

	if (gbFragmentLighting == true) {
		glUniform1i(F_KeyPressed_uniform, 1);
		glUniform1i(V_KeyPressed_uniform, 0);
	}
	else {
		glUniform1i(F_KeyPressed_uniform, 0);
		glUniform1i(V_KeyPressed_uniform, 1);
	}

	glUniform3fv(La_uniform_red, 1, lightAmbient_red);
	glUniform3fv(Ld_uniform_red, 1, lightDiffuse_red);
	glUniform3fv(Ls_uniform_red, 1, lightSpecular_red);
	lightPosition_red[0] = cos_red;
	lightPosition_red[1] = 0.0f;
	lightPosition_red[2] = sin_red;
	glUniform4fv(light_position_uniform_red, 1, lightPosition_red);

	glUniform3fv(La_uniform_green, 1, lightAmbient_green);
	glUniform3fv(Ld_uniform_green, 1, lightDiffuse_green);
	glUniform3fv(Ls_uniform_green, 1, lightSpecular_green);
	lightPosition_green[0] = 0.0f;
	lightPosition_green[1] = cos_green;
	lightPosition_green[2] = sin_green;
	glUniform4fv(light_position_uniform_green, 1, lightPosition_green);

	glUniform3fv(La_uniform_blue, 1, lightAmbient_blue);
	glUniform3fv(Ld_uniform_blue, 1, lightDiffuse_blue);
	glUniform3fv(Ls_uniform_blue, 1, lightSpecular_blue);
	lightPosition_blue[0] = cos_blue;
	lightPosition_blue[1] = sin_blue;
	lightPosition_blue[2] = 0.0f;
	glUniform4fv(light_position_uniform_blue, 1, lightPosition_blue);

	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();

	modelMatrix = translate(0.0f, 0.0f, -2.0f);

	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	GLint currentWidth = 0;
	GLint currentHeight = 0;

	glBindVertexArray(gVao_sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glViewport(0, 0, gwidth/6, gheight/4);
	glUniform3fv(Ka_uniform, 1, material_ambient_1_1);
	glUniform3fv(Kd_uniform, 1, material_diffuse_1_1);
	glUniform3fv(Ks_uniform, 1, material_specular_1_1);
	glUniform1f(material_shininess_uniform, material_shininess_1_1);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, 0, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_1_2);
	glUniform3fv(Kd_uniform, 1, material_diffuse_1_2);
	glUniform3fv(Ks_uniform, 1, material_specular_1_2);
	glUniform1f(material_shininess_uniform, material_shininess_1_2);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, 0, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_1_3);
	glUniform3fv(Kd_uniform, 1, material_diffuse_1_3);
	glUniform3fv(Ks_uniform, 1, material_specular_1_3);
	glUniform1f(material_shininess_uniform, material_shininess_1_3);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, 0, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_1_4);
	glUniform3fv(Kd_uniform, 1, material_diffuse_1_4);
	glUniform3fv(Ks_uniform, 1, material_specular_1_4);
	glUniform1f(material_shininess_uniform, material_shininess_1_4);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, 0, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_1_5);
	glUniform3fv(Kd_uniform, 1, material_diffuse_1_5);
	glUniform3fv(Ks_uniform, 1, material_specular_1_5);
	glUniform1f(material_shininess_uniform, material_shininess_1_5);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, 0, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_1_6);
	glUniform3fv(Kd_uniform, 1, material_diffuse_1_6);
	glUniform3fv(Ks_uniform, 1, material_specular_1_6);
	glUniform1f(material_shininess_uniform, material_shininess_1_6);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);


	// 2nd Row
	currentHeight += gheight / 4;
	currentWidth = 0;

	glViewport(0, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_2_1);
	glUniform3fv(Kd_uniform, 1, material_diffuse_2_1);
	glUniform3fv(Ks_uniform, 1, material_specular_2_1);
	glUniform1f(material_shininess_uniform, material_shininess_2_1);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_2_2);
	glUniform3fv(Kd_uniform, 1, material_diffuse_2_2);
	glUniform3fv(Ks_uniform, 1, material_specular_2_2);
	glUniform1f(material_shininess_uniform, material_shininess_2_2);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_2_3);
	glUniform3fv(Kd_uniform, 1, material_diffuse_2_3);
	glUniform3fv(Ks_uniform, 1, material_specular_2_3);
	glUniform1f(material_shininess_uniform, material_shininess_2_3);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_2_4);
	glUniform3fv(Kd_uniform, 1, material_diffuse_2_4);
	glUniform3fv(Ks_uniform, 1, material_specular_2_4);
	glUniform1f(material_shininess_uniform, material_shininess_2_4);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_2_5);
	glUniform3fv(Kd_uniform, 1, material_diffuse_2_5);
	glUniform3fv(Ks_uniform, 1, material_specular_2_5);
	glUniform1f(material_shininess_uniform, material_shininess_2_5);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_2_6);
	glUniform3fv(Kd_uniform, 1, material_diffuse_2_6);
	glUniform3fv(Ks_uniform, 1, material_specular_2_6);
	glUniform1f(material_shininess_uniform, material_shininess_2_6);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);


	// 3rd Row
	currentHeight += gheight / 4;
	currentWidth = 0;

	glViewport(0, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_3_1);
	glUniform3fv(Kd_uniform, 1, material_diffuse_3_1);
	glUniform3fv(Ks_uniform, 1, material_specular_3_1);
	glUniform1f(material_shininess_uniform, material_shininess_3_1);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_3_2);
	glUniform3fv(Kd_uniform, 1, material_diffuse_3_2);
	glUniform3fv(Ks_uniform, 1, material_specular_3_2);
	glUniform1f(material_shininess_uniform, material_shininess_3_2);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_3_3);
	glUniform3fv(Kd_uniform, 1, material_diffuse_3_3);
	glUniform3fv(Ks_uniform, 1, material_specular_3_3);
	glUniform1f(material_shininess_uniform, material_shininess_3_3);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_3_4);
	glUniform3fv(Kd_uniform, 1, material_diffuse_3_4);
	glUniform3fv(Ks_uniform, 1, material_specular_3_4);
	glUniform1f(material_shininess_uniform, material_shininess_3_4);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_3_5);
	glUniform3fv(Kd_uniform, 1, material_diffuse_3_5);
	glUniform3fv(Ks_uniform, 1, material_specular_3_5);
	glUniform1f(material_shininess_uniform, material_shininess_3_5);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_3_6);
	glUniform3fv(Kd_uniform, 1, material_diffuse_3_6);
	glUniform3fv(Ks_uniform, 1, material_specular_3_6);
	glUniform1f(material_shininess_uniform, material_shininess_3_6);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);


	// 4th Row
	currentHeight += gheight / 4;
	currentWidth = 0;

	glViewport(0, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_4_1);
	glUniform3fv(Kd_uniform, 1, material_diffuse_4_1);
	glUniform3fv(Ks_uniform, 1, material_specular_4_1);
	glUniform1f(material_shininess_uniform, material_shininess_4_1);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_4_2);
	glUniform3fv(Kd_uniform, 1, material_diffuse_4_2);
	glUniform3fv(Ks_uniform, 1, material_specular_4_2);
	glUniform1f(material_shininess_uniform, material_shininess_4_2);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_4_3);
	glUniform3fv(Kd_uniform, 1, material_diffuse_4_3);
	glUniform3fv(Ks_uniform, 1, material_specular_4_3);
	glUniform1f(material_shininess_uniform, material_shininess_4_3);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_4_4);
	glUniform3fv(Kd_uniform, 1, material_diffuse_4_4);
	glUniform3fv(Ks_uniform, 1, material_specular_4_4);
	glUniform1f(material_shininess_uniform, material_shininess_4_4);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_4_5);
	glUniform3fv(Kd_uniform, 1, material_diffuse_4_5);
	glUniform3fv(Ks_uniform, 1, material_specular_4_5);
	glUniform1f(material_shininess_uniform, material_shininess_4_5);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	currentWidth += gwidth / 6;

	glViewport(currentWidth, currentHeight, gwidth / 6, gheight / 4);
	glUniform3fv(Ka_uniform, 1, material_ambient_4_6);
	glUniform3fv(Kd_uniform, 1, material_diffuse_4_6);
	glUniform3fv(Ks_uniform, 1, material_specular_4_6);
	glUniform1f(material_shininess_uniform, material_shininess_4_6);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

	glUseProgram(0);
	glXSwapBuffers(gpDisplay, gWindow);
}

void spin(void) {
	rotationAngle += 0.1f;
	if (rotationAngle >= 360.0f)
		rotationAngle = 0.0f;
}


void resize(int width, int height){
	if(height == 0)
		height = 1;


	gwidth = width;
	gheight = height;

	glViewport(0,0,(GLsizei)width, (GLsizei)height);
	
	gPerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
}

void Uninitialize(void){
	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();

	if (gVao_sphere)
	{
		glDeleteVertexArrays(1, &gVao_sphere);
		gVao_sphere = 0;
	}

	if (gVbo_sphere_position)
	{
		glDeleteBuffers(1, &gVbo_sphere_position);
		gVbo_sphere_position = 0;
	}

	if (gVbo_sphere_normal)
	{
		glDeleteBuffers(1, &gVbo_sphere_normal);
		gVbo_sphere_normal = 0;
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
























