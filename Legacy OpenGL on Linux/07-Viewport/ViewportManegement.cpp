#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

using namespace std;

bool bFullscreen=false;
Display *gpDisplay=NULL;
XVisualInfo *gpXVisualInfo=NULL;
Colormap gColormap;
Window gWindow;
int giWindowWidth = 800;
int giWindowHeight = 600;

float angleTri = 0.0f;
float angleRect = 0.0f;
int giViewportFlag = -1;
int giWidth = giWindowWidth, giHeight = giWindowHeight;

//**
GLXContext gGLXContext;

int main(void)
{
	void CreateWindow(void);
	void ToggleFullscreen(void);
	
	//**
	void initialize();
	void display();
	void resize(int, int);
	void spin(void);
 
	void uninitialize();
	
	int winWidth = giWindowWidth;
	int winHeight = giWindowHeight;

	//**
	bool bDone=false;

	CreateWindow();

	//**
	initialize();

	XEvent event;
	KeySym keysym;

	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay, &event);
			switch(event.type)
			{
				case MapNotify:
					break;

				case KeyPress:
					keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
					switch(keysym)
					{
						case XK_Escape:
							bDone=true;
							break;
						case XK_F:
						case XK_f:
							if( bFullscreen==false )
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
						case XK_1:
							giViewportFlag = 1;
							resize(giWidth, giHeight);
							break;
						case XK_2:
							giViewportFlag = 2;
							resize(giWidth, giHeight);
							break;
						case XK_3:
							giViewportFlag = 3;
							resize(giWidth, giHeight);
							break;
						case XK_4:
							giViewportFlag = 4;
							resize(giWidth, giHeight);
							break;
						case XK_5:
							giViewportFlag = 5;
							resize(giWidth, giHeight);
							break;
						case XK_6:
							giViewportFlag = 6;
							resize(giWidth, giHeight);
							break;
						case XK_7:
							giViewportFlag = 7;
							resize(giWidth, giHeight);
							break;
						case XK_8:
							giViewportFlag = 8;							
							resize(giWidth, giHeight);
							break;
						case XK_9:
							giViewportFlag = 9;								
							resize(giWidth, giHeight);						
							break; 
						default:	
							printf("\n giViewportFlag = %d", giViewportFlag);
							giViewportFlag = 0;
							resize(giWidth, giHeight);
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
					resize(winWidth, winHeight);
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
		
		spin();
		display();
	}
	
	uninitialize();
	return(0);
}

void CreateWindow(void)
{
	void uninitialize(void);
	
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int defaultDepth;
	int styleMask;

	static int frameBufferAttributes[]=
	{
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		GLX_DOUBLEBUFFER, True,
		None
	};

	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("ERROR: Unable to open X display. \n Exitting now..\n");
		uninitialize();
		exit(1);
	}

	defaultScreen=XDefaultScreen(gpDisplay);

	gpXVisualInfo=glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);	

	winAttribs.border_pixel=0;
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay, RootWindow(gpDisplay, gpXVisualInfo->screen),
				gpXVisualInfo->visual, AllocNone);
	gColormap=winAttribs.colormap;

	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);

	winAttribs.event_mask= ExposureMask | VisibilityChangeMask | ButtonPressMask 
				| KeyPressMask | PointerMotionMask | StructureNotifyMask;

	styleMask=CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	gWindow=XCreateWindow(gpDisplay,
				RootWindow(gpDisplay, gpXVisualInfo->screen),
				0, 0,
				giWindowWidth,
				giWindowHeight,
				0,
				gpXVisualInfo->depth,
				InputOutput,
				gpXVisualInfo->visual,
				styleMask,
				&winAttribs);

	if(!gWindow)
	{
		printf("ERROR: failed to create main window. \n Exiting now..\n");
		uninitialize();	
		exit(1);
	}

	XStoreName(gpDisplay, gWindow, "First XWindow");

	Atom windowManagerDelete=XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &windowManagerDelete, 1);

	XMapWindow(gpDisplay, gWindow);
}	

void ToggleFullscreen(void)
{
	Atom wm_state;
	Atom fullscreen;
	XEvent xev={0};

	wm_state=XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&xev, 0, sizeof(xev));

	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.message_type=wm_state;
	xev.xclient.format=32;
	xev.xclient.data.l[0]=bFullscreen ? 0 : 1;

	fullscreen=XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xev.xclient.data.l[1]=fullscreen;

	XSendEvent(gpDisplay,
			RootWindow(gpDisplay, gpXVisualInfo->screen),
			False,
			StructureNotifyMask,
			&xev);
}

void initialize(void)
{
	void resize(int, int);
	
	gGLXContext=glXCreateContext(gpDisplay, gpXVisualInfo, NULL, GL_TRUE);
	
	glXMakeCurrent(gpDisplay, gWindow, gGLXContext);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


	resize(giWindowWidth, giWindowHeight);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_TRIANGLES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);

	glEnd();

	glXSwapBuffers(gpDisplay, gWindow);
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;

	giWidth = width;
	giHeight = height;
	
	if(giViewportFlag == 1)
		glViewport(0, (GLsizei)height/2, (GLsizei)width/2, (GLsizei)height/2);
	else if(giViewportFlag == 2)
		glViewport((GLsizei)width / 2, (GLsizei)height / 2, (GLsizei)width / 2, (GLsizei)height / 2);
	else if (giViewportFlag == 3)
		glViewport(0, 0, (GLsizei)width / 2, (GLsizei)height / 2);
	else if (giViewportFlag == 4)
		glViewport((GLsizei)width / 2, 0, (GLsizei)width / 2, (GLsizei)height / 2);
	else if (giViewportFlag == 5)
		glViewport(0, 0, (GLsizei)width / 2, (GLsizei)height);
	else if (giViewportFlag == 6)
		glViewport((GLsizei)width / 2, 0, (GLsizei)width / 2, (GLsizei)height);
	else if (giViewportFlag == 7)
		glViewport(0, (GLsizei)height / 2, (GLsizei)width, (GLsizei)height/2);
	else if (giViewportFlag == 8)
		glViewport(0, 0, (GLsizei)width, (GLsizei)height / 2);
	else if (giViewportFlag == 9)
		glViewport((GLsizei)(width / 2)/2, (GLsizei)(height / 2)/2, (GLsizei)width / 2, (GLsizei)height / 2);
	else
		glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}

void uninitialize(void)
{
	GLXContext currentGLXContext;
	currentGLXContext=glXGetCurrentContext();

	if(currentGLXContext!=NULL && currentGLXContext==gGLXContext)
	{
		glXMakeCurrent(gpDisplay, 0, 0);
	}

	if(gGLXContext)
		glXDestroyContext(gpDisplay, gGLXContext);

	if(gWindow)
		XDestroyWindow(gpDisplay, gWindow);

	if(gColormap)
		XFreeColormap(gpDisplay, gColormap);

	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}
	
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
}

void spin(void)
{
	angleTri = angleTri + 1.0f;
	if( angleTri >= 360.0f )
		angleTri = 0.0f;

	angleRect = angleRect - 1.0f;
	if( angleRect <= -360.0f )
		angleRect = 0.0f;
}
