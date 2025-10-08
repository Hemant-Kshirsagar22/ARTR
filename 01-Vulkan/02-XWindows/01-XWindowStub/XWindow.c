// standard header files
#include <stdio.h>  // for printf()
#include <stdlib.h> // for exit()
#include <memory.h> // for memset()

// Xlib header files
#include <X11/Xlib.h>   // for Xlib API
#include <X11/Xutil.h>  // for XVisualInfo and related API
#include <X11/XKBlib.h> // for XkbKeycodeToKeysym
#include <X11/keysym.h> // for KeySym
#include <X11/Xatom.h>  // for XA_ATOM

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global variables
const char *gpszAppName = "ARTR";
Display *gpDisplay = NULL;
XVisualInfo *gpXVisualInfo = NULL;
Colormap colormap;
Window window;
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;

Bool bActiveWindow = False;
Bool bEscapeKeyIsPressed = False;
Bool bFullScreen = False;
Bool bWindowMinimized = False;

FILE *gpFile = NULL;

// entry-point function
int main(int argc, char *argv[])
{
    // function declarations
    void toggleFullScreen(void);
    Bool isWindowMinimized(void);
    int initialize(void);
    void resize(int, int);
    void display(void);
    void update(void);
    void uninitialize(void);

    // local variables
    XVisualInfo xVisualInfo;
    int iNumFBConfig = 0;
    XSetWindowAttributes windowAttributes;
    int defaultScreen = 0;
    int defaultDepth = 0;
    int styleMask = 0;
    Atom windowManagerDeleteAtom;
    int screenWidth = 0;
    int screenHeight = 0;
    XEvent event;
    KeySym keySym;
    char keys[26];
    Bool bDone = False;

    // code
    gpFile = fopen("LOG.txt", "w");
    if(gpFile == NULL)
    {
        printf("%s() -> Failed to open log.txt !!!\n\n", __func__);
        exit(EXIT_FAILURE);
    }
    else
    {
        fprintf(gpFile, "%s() -> Program started Successfully !!!\n\n", __func__);
    }

    // open connection with xserver and get display interface
    gpDisplay = XOpenDisplay(NULL);
    if(gpDisplay == NULL)
    {
        fprintf(gpFile, "%s() -> XOpenDisplay() failed !!!\n\n", __func__);
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // get default screen
    defaultScreen = XDefaultScreen(gpDisplay);

    // initialize local XVisualInfo
    memset((void *)&xVisualInfo, 0, sizeof(XVisualInfo));

    xVisualInfo.screen = defaultScreen;
    gpXVisualInfo = XGetVisualInfo(gpDisplay, VisualScreenMask, &xVisualInfo, &iNumFBConfig);
    if(gpXVisualInfo == NULL)
    {
        fprintf(gpFile, "%s() -> XGetVisualInfo() failed !!!\n\n", __func__);
        uninitialize();
        exit(EXIT_FAILURE);
    }

    fprintf(gpFile, "%s() -> found number of matching FBConfigs : %d\n\n", __func__, iNumFBConfig);

    // create colormap
    colormap = XCreateColormap(gpDisplay, XRootWindow(gpDisplay, xVisualInfo.screen),  gpXVisualInfo->visual, AllocNone);
    // initialize window attributes
    memset((void *)&windowAttributes, 0, sizeof(XSetWindowAttributes));
    windowAttributes.border_pixel = 0;
    windowAttributes.background_pixel = XBlackPixel(gpDisplay, defaultScreen);
    windowAttributes.background_pixmap = 0;
    windowAttributes.colormap = colormap;
    windowAttributes.event_mask = ExposureMask | VisibilityChangeMask | StructureNotifyMask | KeyPressMask | ButtonPressMask | FocusChangeMask | PropertyChangeMask;
    
    styleMask = CWBorderPixel | CWBackPixel | CWColormap | CWEventMask;

    // create window
    window = XCreateWindow(
        gpDisplay,
        XRootWindow(gpDisplay, xVisualInfo.screen),
        0,
        0,
        WIN_WIDTH,
        WIN_HEIGHT,
        0,
        gpXVisualInfo->depth,
        InputOutput,
        gpXVisualInfo->visual,
        styleMask,
        &windowAttributes
    );

    if(!window)
    {
        fprintf(gpFile, "%s() -> XCreateWindow() failed !!!\n\n", __func__);
        uninitialize();
        exit(EXIT_FAILURE);
    }

    // set window caption
    XStoreName(gpDisplay, window, "HGK : Vulkan");

    windowManagerDeleteAtom = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(gpDisplay, window, &windowManagerDeleteAtom, 1);

    // show window
    XMapWindow(gpDisplay, window);

    // cernter the window
    screenWidth = XWidthOfScreen(XScreenOfDisplay(gpDisplay, defaultScreen));
    screenHeight = XHeightOfScreen(XScreenOfDisplay(gpDisplay, defaultScreen));
    XMoveWindow(gpDisplay, window, (screenWidth - WIN_WIDTH) / 2, (screenHeight - WIN_HEIGHT) / 2);

    int iResult = initialize();
    if(iResult != 0)
    {
        fprintf(gpFile, "%s() -> initialize() failed !!!\n\n", __func__);
        uninitialize();
        exit(EXIT_FAILURE);
    }
    else 
    {
        fprintf(gpFile, "%s() -> initialize() success\n\n", __func__);
    }

    while(bDone == False)
    {
        while(XPending(gpDisplay))
        {
            XNextEvent(gpDisplay, &event);
            switch(event.type)
            {
                case MapNotify:
                    break;

                case FocusIn: // similar to WM_SETFOCUS in windows
                    bActiveWindow = True;
                    break;

                case FocusOut: // similar to kill focus
                    bActiveWindow = False;
                    break;

                case KeyPress:
                {
                    keySym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
                    switch(keySym)
                    {
                        case XK_Escape:
                            bEscapeKeyIsPressed = True;
                            break;

                        default:
                            break;
                    }

                    XLookupString(&event.xkey, 
                    keys,
                    sizeof(keys),
                    NULL, NULL);

                    switch(keys[0])
                    {
                        case 'F':
                        case 'f':
                            if(bFullScreen == False)
                            {
                                toggleFullScreen();
                                bFullScreen = True;
                            }
                            else
                            {
                                toggleFullScreen();
                                bFullScreen = False;
                            }
                            break;
                        
                        default :
                        break;
                    }
                }
                break;

                case ConfigureNotify: // similar to WM_SIZE
                    resize(event.xconfigure.width, event.xconfigure.height);
                    winWidth = event.xconfigure.width;
                    winHeight = event.xconfigure.height;
                    break;

                case PropertyNotify:
                    if(isWindowMinimized() == True)
                    {
                        bWindowMinimized = True;
                        fprintf(gpFile, "bWindowMinimized : True\n\n");
                    }
                    else 
                    {
                        bWindowMinimized = False;
                        fprintf(gpFile, "bWindowMinimized : False\n\n");
                    }
                    break;

                case DestroyNotify:
                    break;
                
                case 33:
                    bDone = True;
                    break;
                
                default:
                    break;
            }
        }

        if(bActiveWindow == True)
        {
            if(bEscapeKeyIsPressed)
            {
                bDone = True;
            }

            // display
            if(bWindowMinimized == False)
            {
                display();

                update();
            }
        }
    }

    uninitialize();
    return(0);
}

void toggleFullScreen(void)
{
    // local variable declaration
    Atom windowManagerStateNormal;
    Atom windowManagerStateFullScreen;
    XEvent event;

    //code
    windowManagerStateNormal = XInternAtom(gpDisplay, "_NET_WM_STATE", False);

    windowManagerStateFullScreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);

    // use memset to initialize all members of XEvent to 0 and give above values of Atom
    memset((void*)&event, 0, sizeof(XEvent));
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = windowManagerStateNormal;
    event.xclient.format = 32;
    event.xclient.data.l[0] = bFullScreen ? 0 : 1;
    event.xclient.data.l[1] = windowManagerStateFullScreen;

    // send event
    XSendEvent(gpDisplay,
    XRootWindow(gpDisplay, gpXVisualInfo->screen),
    False,
    SubstructureNotifyMask,
    &event);
}

Bool isWindowMinimized(void)
{
    // function declarations
    void uninitialize(void);

    // local variable declarations
    Bool windowMinimized = False;
    int iResult = -1;
    
    Atom returned_property_type = None;
    int returned_property_format = -1;
    unsigned long number_of_returned_item = 0;
    unsigned long number_of_bytes_remained = 0;
    Atom *returned_property_data_array;
    
    // code
    Atom wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", True);

    if(wm_state == None)
    {
        fprintf(gpFile, "%s() -> XInternAtom() failed for _NET_WM_STATE !!!\n\n", __func__);
        uninitialize();
        exit(EXIT_FAILURE);
    }

    Atom wm_state_hidden = XInternAtom(gpDisplay, "_NET_WM_STATE_HIDDEN", True);
    if(wm_state_hidden == None)
    {
        fprintf(gpFile, "%s() -> XInternAtom() failed for _NET_WM_STATE_HIDDEN !!!\n\n", __func__);
        uninitialize();
        exit(EXIT_FAILURE);
    }

    iResult = XGetWindowProperty(
        gpDisplay,
        window,
        wm_state,
        0l,
        1024,
        False,
        XA_ATOM,
        &returned_property_type,
        &returned_property_format,
        &number_of_returned_item,
        &number_of_bytes_remained,
        (unsigned char **)&returned_property_data_array
    );

    if(iResult != Success || returned_property_data_array == NULL)
    {
        if(returned_property_data_array != NULL)
        {
            XFree(returned_property_data_array);
            returned_property_data_array = NULL;
        }
        return(False);
    }
    else
    {
        // looped the retun array for required return property
        for(unsigned long i = 0; i < number_of_returned_item; i++)
        {
            // check whether array contains "hidden" property or not
            if(returned_property_data_array[i] == wm_state_hidden)
            {
                windowMinimized = True;
                break;
            }
        }    
    }

    if(returned_property_data_array != NULL)
    {
        XFree(returned_property_data_array);
        returned_property_data_array = NULL;
    }

    return(windowMinimized);
}

int initialize(void)
{
    // code
    fprintf(gpFile, "\n\n============================ %s() START ===============================\n\n", __func__);

    fprintf(gpFile, "\n\n============================ %s() END ===============================\n\n", __func__);
    return(0);
}

void resize(int width, int height)
{
    // code
}

void display(void)
{
    // code
}

void update(void)
{
    // code
}

void uninitialize(void)
{
    // function declarations
    void toggleFullScreen(void);

    // code
    fprintf(gpFile, "\n\n============================ %s() START ===============================\n\n", __func__);;
    // restored if full screen
    if(bFullScreen == True)
    {
        toggleFullScreen();
        bFullScreen = False;
    }

    if(window)
    {
        XDestroyWindow(gpDisplay, window);
    }

    if(colormap)
    {
        XFreeColormap(gpDisplay, colormap);
    }

    if(gpXVisualInfo)
    {
        XFree((void *)gpXVisualInfo);
        gpXVisualInfo = NULL;
    }

    if(gpDisplay)
    {
        XCloseDisplay(gpDisplay);
        gpDisplay = NULL;
    }

    if(gpFile)
    {
        fprintf(gpFile, "program terminated successfully !!!");
        fprintf(gpFile, "\n\n============================ %s() END ===============================\n\n", __func__);
        fclose(gpFile);
        gpFile = NULL;
    }
}
