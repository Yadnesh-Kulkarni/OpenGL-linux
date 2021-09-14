#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>


#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <SOIL/SOIL.h>

using namespace std;

bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth = 800;
int giWindowHeight = 600;

int width,height;

GLuint Texture_Stone;
GLuint Texture_Kundali;

GLXContext gGLXContext;

GLfloat angleTri = 0.0f;
GLfloat angleSquare = 0.0f;

int main(void){
	void CreateWindow(void);
	void ToggleFullScreen(void);
	void uninitialize();

	void initialize(void);
	void display(void);
	void resize(int,int);
	void update(void);


	int winWidth = giWindowWidth;
	int winHeight = giWindowHeight;

	bool bDone = false;

	CreateWindow();
	initialize();

	XEvent event;
	KeySym keysym;

	while(bDone == false){  // OGL change
		while(XPending(gpDisplay)){  // OGL change

			XNextEvent(gpDisplay,&event);
			switch(event.type){
				case MapNotify:
					break;
				case KeyPress:
				keysym = XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
				switch(keysym){
					case XK_Escape:
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
		update();
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

	/*defaultDepth = DefaultDepth(gpDisplay,defaultScreen);

	gpXVisualInfo = (XVisualInfo *) malloc(sizeof(XVisualInfo));

	if(gpXVisualInfo == NULL){
		printf("ERROR:Unable to allocate memory for visual info");
		uninitialize();
		exit(1);
	}
	
	XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,gpXVisualInfo);
	if(XMatchVisualInfo(gpDisplay,defaultScreen,defaultDepth,TrueColor,gpXVisualInfo) == 0){
		printf("ERROR:Unable to allocate memory for visual");
		uninitialize();
		exit(1);
	}*/
	
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

	int LoadGLTextures(GLuint *,const char *);

	gGLXContext = glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);
	
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	// Optional but to keep smoothnes
	
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	
	glEnable(GL_TEXTURE_2D);
	LoadGLTextures(&Texture_Stone,"Stone.bmp");
	LoadGLTextures(&Texture_Kundali,"Vijay_Kundali.bmp");

	resize(giWindowWidth,giWindowHeight);
}

int LoadGLTextures(GLuint *texture, const char* Path){

	unsigned char* imageData = NULL;

	imageData = SOIL_load_image(Path,&width,&height,0,SOIL_LOAD_RGB);
	glGenTextures(1,texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	
	glBindTexture(GL_TEXTURE_2D,*texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D,3,width,height,GL_RGB,GL_UNSIGNED_BYTE,(GLvoid *)imageData);

	SOIL_free_image_data(imageData);
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.5f,0.0f,-6.0f);
	glRotatef(angleTri,0.0f,1.0f,0.0f);

	glBindTexture(GL_TEXTURE_2D,Texture_Stone);

	glBegin(GL_TRIANGLES);
 // front face
	glTexCoord2f(0.5f,1.0f);
	glVertex3f(0.0f,1.0f,0.0f);

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0,-1.0,1.0f);

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0f,-1.0f,1.0f);
//right face
	glTexCoord2f(0.5f,1.0f);
	glVertex3f(0.0f,1.0f,0.0f);

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0,-1.0,1.0f);

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(1.0f,-1.0f,-1.0f);
	// back face
	glTexCoord2f(0.5f,1.0f);
	glVertex3f(0.0f,1.0f,0.0f);

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(1.0,-1.0,-1.0f);

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0f,-1.0f,-1.0f);
	// left face
	glTexCoord2f(0.5f,1.0f);
	glVertex3f(0.0f,1.0f,0.0f);

	glTexCoord2f(0.0f,0.0f);
	glVertex3f(-1.0,-1.0,-1.0f);

	glTexCoord2f(1.0f,0.0f);
	glVertex3f(-1.0f,-1.0f,1.0f);

	glEnd();

	glLoadIdentity();
	glTranslatef(1.5f,0.0f,-6.0f);
	glScalef(0.75f,0.75f,0.75f);
	glRotatef(angleSquare,1.0f,1.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D,Texture_Kundali);
	glBegin(GL_QUADS);
	// top face
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(1.0f,1.0f,-1.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-1.0f,1.0f,-1.0f);
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(-1.0f,1.0f,1.0f);
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(1.0f,1.0f,1.0f);

	// bottom face
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(1.0f,-1.0f,1.0f);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-1.0f,-1.0f,1.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(1.0f,-1.0f,-1.0f);

		// front face
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(1.0f,1.0f,1.0f);
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(-1.0f,1.0f,1.0f);
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(-1.0f,-1.0f,1.0f);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(1.0f,-1.0f,1.0f);

		// back face
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(1.0f,1.0f,-1.0f);
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(-1.0f,1.0f,-1.0f);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(1.0f,-1.0f,-1.0f);

		// left face
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-1.0f,1.0f,1.0f);
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(-1.0f,1.0f,-1.0f);
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(-1.0f,-1.0f,-1.0f);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(-1.0f,-1.0f,1.0f);

		// right face
		glTexCoord2f(1.0f,0.0f);
		glVertex3f(1.0f,1.0f,-1.0f);
		glTexCoord2f(1.0f,1.0f);
		glVertex3f(1.0f,1.0f,1.0f);
		glTexCoord2f(0.0f,1.0f);
		glVertex3f(1.0f,-1.0f,1.0f);
		glTexCoord2f(0.0f,0.0f);
		glVertex3f(1.0f,-1.0f,-1.0f);
		
	glEnd();

	glXSwapBuffers(gpDisplay,gWindow);

}

void resize(int width,int height){
	if(height == 0)
		height = 1;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);
}

void update(void){
	angleTri = angleTri+0.01f;
	if(angleTri > 360){
		angleTri = angleTri - 360.0f;
	}

	angleSquare = angleSquare+0.01f;
	if(angleSquare > 360){
		angleSquare = angleSquare - 360.0f;
	}
}

void uninitialize(void){
	
	if(Texture_Stone){
		glDeleteTextures(1,&Texture_Stone);
		Texture_Stone = 0;	
	}
	if(Texture_Kundali){
		glDeleteTextures(1,&Texture_Kundali);
		Texture_Kundali = 0;
	}

	GLXContext currentGLXContext;
	currentGLXContext = glXGetCurrentContext();

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
	