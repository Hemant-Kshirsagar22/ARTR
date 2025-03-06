// Windows Header File
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "VK.h"

// vulkan related header file
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// vulkan related libraries
#pragma comment(lib,"vulkan-1.lib")

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
const char *gpszAppName = "ARTR";

FILE *gpFile = NULL;
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
BOOL gbFullScreen = FALSE;

HWND ghwnd;
BOOL gbActive = FALSE;

// vulkan related global variables
// instance extension related variables
uint32_t enabledInstanceExtensionCount = 0;
// VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME we are using these two macros and going to store in following array
const char *enabledInstanceExtensionNames_array[2];

// vulkan instance
VkInstance vkInstance = VK_NULL_HANDLE;

// vulkan presentation surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

// vulkan physcial device related variables
VkPhysicalDevice vkPhyscialDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Function declarations

    VkResult initialize(void);
    void uninitialize(void);
    void display(void);
    void update(void);

    // Local Variable Declarations
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[255];

    int iResult = 0;

    BOOL bDone = FALSE;

    VkResult vkResult = VK_SUCCESS;

    int SW, SH, WW, WH;


    // code
    gpFile = fopen("LOG.txt", "w");
    if (gpFile == NULL)
    {
        MessageBox(NULL, TEXT("LOG File Cant Create !!!"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
        exit(0);
    }

    fprintf(gpFile, "%s()->Program Started Successfully !!!\n\n", __func__);

    wsprintf(szAppName, TEXT("%s"), gpszAppName);

    // WNDCLASSEX initilization
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = WndProc;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

    // Register WNDLCASSEX
    RegisterClassEx(&wndclass);

    // Create Window

    SW = GetSystemMetrics(SM_CXSCREEN);
    SH = GetSystemMetrics(SM_CYSCREEN);

    SW = SW / 2;
    SH = SH / 2;
    WW = WIN_WIDTH / 2;
    WH = WIN_HEIGHT / 2;

    WW = SW - WW;
    WH = SH - WH;

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        szAppName,
        TEXT("HEMANT GANESH KSHIRSAGAR : Vulkan"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        WW,
        WH,
        WIN_WIDTH,
        WIN_HEIGHT,
        NULL,
        NULL,
        hInstance,
        NULL);

    ghwnd = hwnd;

    // initialization

    vkResult = initialize();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "WinMain()-> initialize() failed !!!\n\n");
        MessageBox(hwnd, TEXT("initialize() Failed !!!"), TEXT("ERROR"), MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "WinMain()-> initialize() success\n\n");
    }

    // Show The Window
    ShowWindow(hwnd, iCmdShow);

    SetForegroundWindow(hwnd);

    SetFocus(hwnd);

    // GameLoop

    while (bDone == FALSE)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (gbActive == TRUE)
            {
                // render
                display();

                // update
                update();
            }
        }
    }

    // uninitialization
    uninitialize();

    return((int)msg.wParam);

}

//CALLBACK Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // Function Prototype / Declarations

    void ToggleFullScreen(void);

    void resize(int, int);

    // code

    switch (iMsg)
    {
    case WM_SETFOCUS: // When Window is in focus (Window is Active)
        gbActive = TRUE;
        break;

    case WM_KILLFOCUS: // When Window Is Not In Focus (Window is deactiveated)
        gbActive = FALSE;
        break;

    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        break;

        //    case WM_ERASEBKGND:
        //        return(0);

    case WM_KEYDOWN:
        switch (LOWORD(wParam))
        {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
            break;
        default:
            break;
        }
        break;

    case WM_CHAR:
        switch (LOWORD(wParam))
        {
        case 'F':
        case 'f':
            if (gbFullScreen == FALSE)
            {
                ToggleFullScreen();
                gbFullScreen = TRUE;
            }
            else
            {
                ToggleFullScreen();
                gbFullScreen = FALSE;
            }
            break;
        default:
            break;
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
    // Local Variable declarations
    MONITORINFO mi = { sizeof(MONITORINFO) };

    // code
    if (gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowPlacement(ghwnd, &wpPrev);

        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

        ShowCursor(TRUE);
    }
}

VkResult initialize(void)
{
    // function declarations
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "================================== INITIALIZATION START ===================================\n\n");
    vkResult = createVulkanInstance();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "initialize()-> createVulkanInstance() failed %d !!!\n\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "initialize()-> createVulkanInstance() success\n\n");
    }

    // create vulkan presentation surface
    vkResult = getSupportedSurface();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "initialize()-> getSupportedSurface() failed %d !!!\n\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "initialize()-> getSupportedSurface() success\n\n");
    }

    // select required physical device and its queue family index
    vkResult = getPhysicalDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "initialize()-> getPhysicalDevice() failed : %d !!!\n\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "initialize()-> getPhysicalDevice() success\n\n");
    }


    fprintf(gpFile, "================================== INITIALIZATION END ==================================\n\n");
    return(vkResult);
}

void resize(int width, int height)
{
    // code

    if (height <= 0)
    {
        height = 1;
    }
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
    // Function Declarations

    void ToggleFullScreen(void);

    // code
    // if Application is exitting in fullscreen
    if (gbFullScreen == TRUE)
    {
        ToggleFullScreen();
        gbFullScreen = FALSE;
    }

    // Destroy Window
    if (ghwnd)
    {
        DestroyWindow(ghwnd);
        ghwnd = NULL;
    }

    // uninitialze presentation surface
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL); // this method is generic for all surfaces
        vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "uninitialize()-> vkDestroySurfaceKHR() succeeded\n");
    }

    // destroy vulkan instance
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpFile, "uninitialize()-> vkDestroyInstance() succeeded\n");

    }

    // close the log file
    fprintf(gpFile, "uninitialize()->Program Ended Successfully !!!\n\n");
    if (gpFile)
    {
        fclose(gpFile);
        gpFile = NULL;
    }
}
// ----------------------------------------------------------------------------------------------
//                        VULKAN RELATED FUNCTION DEFINATIONS
// ----------------------------------------------------------------------------------------------

VkResult createVulkanInstance(void)
{
    // Function Declarations
    VkResult fillInstanceExtensionNames(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code

    // STEPS FOR INSTANCE CREATION
    // 1. fill and initalize required extension names and count global variables.
    fprintf(gpFile, "\n======================== STEPS FOR INSTANCE CREATION START ================================\n\n");
    vkResult = fillInstanceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanInstance()-> fillInstanceExtensionNames() failed !!!\n\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "createVulkanInstance()-> fillInstanceExtensionNames() success\n\n");
    }

    // 2. initialize VkApplicationInfo.
    VkApplicationInfo vkApplicationInfo;
    memset((void *)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkApplicationInfo.pNext = NULL;
    vkApplicationInfo.pApplicationName = gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpszAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_3;

    //  3. initialize struct VkInstanceCreateInfo by using infromation from step 1 and 2.
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void *)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;

    // 4. call vkCreateInstance() to get VkInstance in a global variable and do error checking.
    vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
    if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gpFile, "\ncreateVulkanInstance()-> vkCreateInstance() failed due to incompatible driver(ERROR CODE : %d)\n", vkResult);
        return(vkResult);
    }
    else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gpFile, "\ncreateVulkanInstance()-> vkCreateInstance() failed due to requred extension not present (ERROR CODE : %d)\n", vkResult);
        return(vkResult);
    }
    else if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "\ncreateVulkanInstance()-> vkCreateInstance() failed due to unkown reason (ERROR CODE : %d)\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "\ncreateVulkanInstance()-> vkCreateInstance() Succeeded\n");
    }
    fprintf(gpFile, "\n======================== STEPS FOR INSTANCE CREATION END ================================\n\n");
    return(vkResult);
}

VkResult fillInstanceExtensionNames(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t instanceExtensionCount = 0;
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    char **instanceExtensionNames_array = NULL;
    //  STEPS FOR INSTANCE EXTENSIONS
    // 1 : find how many instance extensions are supported by the vulkan driver of this version and keep it in a local variable.
    fprintf(gpFile, "\n======================== STEPS FOR INSTANCE EXTENSIONS START ================================\n\n");
    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> 1st called to vkEnumerateInstanceExtensionProperties() failed !!!\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> 1st called to vkEnumerateInstanceExtensionProperties() success\n");
    }

    // 2. allocate and fill struct VkExtension proprties array corrosponding to above count.
    vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
    if (vkExtensionProperties_array == NULL)
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> malloc() failed for vkExtensionProperties_array\n");
        return(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> 2nd called to vkEnumerateInstanceExtensionProperties() failed !!!\n\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> 2nd called to vkEnumerateInstanceExtensionProperties() success\n\n");
    }

    // 3. fill and display a local string array of extension names obtained from VkExtension properties.
    instanceExtensionNames_array = (char **)malloc(sizeof(char *) * instanceExtensionCount);
    if (vkExtensionProperties_array == NULL)
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> malloc() failed for instanceExtensionName_array\n");
        return(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "fillInstanceExtensionNames()-> vlkan extension name : %s\n", instanceExtensionNames_array[i]);
    }

    // 4. As not required here onwords free VkExtension Array.
    if (vkExtensionProperties_array)
    {
        free(vkExtensionProperties_array);
        vkExtensionProperties_array = NULL;
    }

    // 5. find whether above extension names contain our required two extentions (VK_KHR_SURFACE_EXTENSION_NAME macro of VK_KHR_surface, VK_KHR_WIN32_SURFACE_EXTENSION_NAME macro of VK_KHR_win32_surface) accordingly set two global variable.
    VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
    VkBool32 win32SurfaceExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }

        if (strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            win32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }

    }

    // 6. As not needed hencefore free local string array.
    for (uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if (instanceExtensionNames_array[i])
        {
            free(instanceExtensionNames_array[i]);
            instanceExtensionNames_array[i] = NULL;
        }
    }

    if (instanceExtensionNames_array)
    {
        free(instanceExtensionNames_array);
        instanceExtensionNames_array = NULL;
    }

    // 7. Print whether our valkan driver support our required extensions found or not
    if (vulkanSurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(gpFile, "\n\nfillInstanceExtensionNames()-> VK_KHR_SURFACE_EXTENSION_NAME NOT FOUND !!!\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "\n\nfillInstanceExtensionNames()-> VK_KHR_SURFACE_EXTENSION_NAME FOUND !!!\n");

    }

    if (win32SurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(gpFile, "fillInstanceExtensionNames()-> VK_KHR_WIN32_SURFACE_EXTENSION_NAME NOT FOUND !!!\n\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> VK_KHR_WIN32_SURFACE_EXTENSION_NAME FOUND !!!\n\n");
    }

    // 8. Print only supported extension names.
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> ENABLED VULKAN INSTANCE EXTENTION NAME : %s\n", enabledInstanceExtensionNames_array[i]);
    }

    fprintf(gpFile, "\n\n============================ STEPS FOR INSTANCE EXTENSIONS END ===============================\n\n");
    return(vkResult);
}

VkResult getSupportedSurface(void)
{
    VkResult vkResult = VK_SUCCESS;
    // code
    fprintf(gpFile, "\n\n============================ STEPS FOR PRESENTATION SURFACE START ===============================\n\n");
    // 1. Declare and memset platform specific (windows,linux,android,etc) surface create info structure.

    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;

    memset((void *)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

    // 2. Initalize it perticularly its hinstance and hwnd members.
    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE); // other method getModuleHandle(NULL) returns hInstance
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    // 3. Now Call vkCreateWin32SurfaceKHR() to create persentation suface object.
    vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getSupportedSurface()-> vkCreateWin32SufaceKHR() FAILED !!!\n\n");
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "getSupportedSurface()-> vkCreateWin32SufaceKHR() Success !!!\n\n");
    }

    fprintf(gpFile, "\n\n============================ STEPS FOR PRESENTATION SURFACE END ===============================\n\n");

    return(vkResult);
}

VkResult getPhysicalDevice(void)
{
    // code
    uint32_t physicalDeviceCount = 0;
    VkResult vkResult = VK_SUCCESS;

    fprintf(gpFile, "\n\n============================ STEPS FOR PHYSICAL DEVICE START ===============================\n\n");
    //  2. call vkEnumeratePhysicalDevices() to get physical device count.
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice()-> 1st call vkEnumeratePhysicalDevices() FAILED %d !!!\n\n", vkResult);
        return(vkResult);
    }
    else if (physicalDeviceCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevice()-> vkEnumeratePhysicalDevices() : physicalDeviceCount = 0 !!!\n\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice()-> 1st call vkEnumeratePhysicalDevices() Success !!!\n\n");
    }

    //  3. allocate VkPhysicalDevice array according to above count.
    VkPhysicalDevice *vkPhysicalDevice_array = NULL;
    vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

    //  4. Call vkEnumeratePhysicalDevices() again to fill above array.
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice()-> 2nd call vkEnumeratePhysicalDevices() FAILED %d !!!\n\n", vkResult);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice()-> 2nd call vkEnumeratePhysicalDevices() Success !!!\n\n");
    }

    // 5. Start a loop Using Physical Device count and physicalDevice array (Note
    VkBool32 bFound = VK_FALSE;

    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        // a. declare a local variable to queue count.
        uint32_t queueCount = UINT32_MAX;

        // b. get queue count
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);

        // c. allocate VkQueueFamilyProperty array according to above count.
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;

        vkQueueFamilyProperties_array = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queueCount);

        // d. call vkGetPhyscialDeviceQueueFamilyProperties() to fill above array.
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

        // e. Declare VkBool32 type array and allocate it using the same above queue count.
        VkBool32 *isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32 *)malloc(sizeof(VkBool32) * queueCount);

        // f. Start a nested loop and fill above VkBool32 type array by calling vkGetPhysicalDeviceSurfaceSupportKHR().
        for (uint32_t j = 0; j < queueCount; j++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);
        }

        // g. start another nested loop (not nested in nested above loop), check whether physical device in its array with its queue family has graphics bit or not if yes then this is a selected physical device assign it to global variable similarly this index is selected queue family index assign it to global variable 2 and set bFound = true and break from the second nested loop.
        for (uint32_t j = 0; j < queueCount; j++)
        {
            if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if (isQueueSurfaceSupported_array[j] == VK_TRUE)
                {
                    vkPhyscialDevice_selected = vkPhysicalDevice_array[i];
                    graphicsQueueFamilyIndex_selected = j;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }

        // h. now we are back in main loop so free the queue family array and VkBool32 array.
        if (isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
        }

        // i. free vkQueueFamilyProperties
        if (vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
        }

        // j. according to bFound variable break out from main loop.
        if (bFound == VK_TRUE)
        {
            break;
        }


    }

    // 6. Do error checking according to the value of the bFound.
    if (bFound == VK_TRUE)
    {
        fprintf(gpFile, "getPhysicalDevice()-> get physical device selected with graphic enable\n\n");
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice()-> get physical device selected with graphic enable failed !!!\n\n");

        if (vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            vkPhysicalDevice_array = NULL;
        }

        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }

    if (vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
    }



    // 7. memset the global pyscial device property structure.
    memset((void *)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

    // 8. initialize above structure by using vkGetPhysicalDeviceMemoryProperties().
    vkGetPhysicalDeviceMemoryProperties(vkPhyscialDevice_selected, &vkPhysicalDeviceMemoryProperties);

    // 9. Declare the a local structure variable VkPhysicalDeviceFeatures, memset it and initialize it by calling vkGetPhysicalDeviceFeatures()
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void *)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

    vkGetPhysicalDeviceFeatures(vkPhyscialDevice_selected, &vkPhysicalDeviceFeatures);

    // 10. By Using "tessilationShaderMember" of above structure check selected device tessilation shader support.
    if (vkPhysicalDeviceFeatures.tessellationShader)
    {
        fprintf(gpFile, "getPhysicalDevice()-> tessellationShader Supported\n\n");
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice()-> tessellationShader NOT Supported !!!\n\n");
    }

    // 11. By Using "geometryShaderMember" of above structure check selected device geometry shader support.
    if (vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gpFile, "getPhysicalDevice()-> geometryShader Supported\n\n");
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice()-> geometryShader NOT Supported !!!\n\n");
    }

    fprintf(gpFile, "\n\n============================ STEPS FOR PHYSICAL DEVICE END ===============================\n\n");

    return(vkResult);

}

/*
 -------------------------------------------------------------------------------------------------
 STEPS FOR INSTANCE EXTENSIONS
 -----------------------------
 1. find how many instance extensions are supported by the vulkan driver of this version and keep it in a local variable.
 2. allocate and fill struct VkExtension proprties array corrosponding to above count.
 3. fill and display a local string array of extension names obtained from VkExtension properties.
 4. As not required here onwords free VkExtension Array.
 5. find whether above extension names contain our required two extentions (VK_KHR_SURFACE_EXTENSION_NAME macro of VK_KHR_surface, VK_KHR_WIN32_SURFACE_EXTENSION_NAME macro of VK_KHR_win32_surface) accordingly set two global variable.
        a. required extension count.
        b. required extension name array.
 6. As not needed hencefore free local string array.
 7. Print whether our valkan driver support our required extensions found or not
 8. Print only supported extension names.

 -------------------------------------------------------------------------------------------------
 STEPS FOR INSTANCE CREATION
 ---------------------------
 (Do below 4 steps in initialize() and last step in uninitialize())
 1. As Explain above fill and initalize required extension names and count global variables.
 2. initialize VkApplicationInfo.
 3. initialize struct VkInstanceCreateInfo by using infromation from step 1 and 2.
 4. call vkCreateInstance() to get VkInstance in a global variable and do error checking.
 5. destroy vkInstance in uninitialize() function.

 -------------------------------------------------------------------------------------------------
 STEPS FOR PRESENTATION SURFACE
 -----------------------------
 1. Declare a global variable to hold presentation surface object.
 2. Declare and memset platform specific (windows,linux,android,etc) surface create info structure.
 3. Initalize it perticularly its hinstance and hwnd members.
 4. Now Call vkCreateWin32SufaceKHR() to create persentation suface object.

 -------------------------------------------------------------------------------------------------
 STEPS FOR PHYSICAL DEVICE
 -------------------------
 1. Declare 3 global variables for selected physical device, selected queue family index, for physcial device's memory property(required letter).
 2. call vkEnumeratePhysicalDevices() to get physical device count.
 3. allocate VkPhysicalDevice array according to above count.
 4. Call vkEnumeratePhysicalDevices() again to fill above array.
 5. Start a loop Using Physical Device count and physicalDevice array (Note : Declare a boolean bFound Variable before this loop which will decied whether we found desired physical device or not)
    Inside this loop:
    a. declare a local variable to queue count.
    b. call vkGetPhysicalDeviceQueueFamilyProperties() to queue count variable.
    c. allocate VkQueueFamilyProperty array according to above count.
    d. call vkGetPhyscialDeviceQueueFamilyProperties() to fill above array.
    e. Declare VkBool32 type array and allocate it using the same above queue count.
    f. Start a nested loop and fill above VkBool32 type array by calling vkGetPhysicalDeviceSurfaceSupportKHR().
    g. start another nested loop (not nested in nested above loop), check whether physical device in its array with its queue family has graphics bit or not if yes then this is a selected physical device assign it to global variable similarly this index is selected queue family index assign it to global variable 2 and set bFound = true and break from the second nested loop.
    h. now we are back in main loop so free the queue family array and VkBool32 array.
    i. according to bFound variable break out from main loop.
    j. free physcial device array.
 6. Do error checking according to the value of the bFound.
 7. memset the global pyscial device property structure.
 8. initialize above structure by using vkGetPhysicalDeviceMemoryProperties().
 9. Declare the a local structure variable VkPhysicalDeviceFeatures, memset it and initialize it by calling vkGetPhysicalDeviceFeatures()
 10. By Using "tessellationShaderMember" of above structure check selected device tessellation Shader support
 11. By Using "geometryShaderMember" of above structure check selected device geometry shader support
  12. There is no need to free / destroy / uninitialize selected physical device. Because letter we will create vulkan logical device which we need to destroy and its destruction will automatically destroy the selected physical device.
 -------------------------------------------------------------------------------------------------


 Notes:
 all struct, enum and types names = Vk...
 all apis starts from vk....

 // build commands for windows build.bat
 cls
 cl.exe /c /EHsc /I C:VulkanSDK\Valkan\include vk.c
 rc.exe vk.rc
 link.exe vk.obj vk.res /LIBPATH:C:VulkanSDK\valkan\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

 */

