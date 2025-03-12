// Windows Header File
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "VK.h"

// vulkan related header file
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
const char *gpszAppName = "ARTR";

FILE *gpFile = NULL;
DWORD dwStyle = 0;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
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

// vulkan pringVKInfo
uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevice_array = NULL;

// Device extensions
// Device extension related variables
uint32_t enabledDeviceExtensionCount = 0;
// for VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char *enabledDeviceExtensionNames_array[1];

// vulkan (logical) device
VkDevice vkDevice = NULL;

// device queue
VkQueue vkQueue = VK_NULL_HANDLE;

// Color format and color space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// present mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

// swapchain related global variables
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

// swapchain images and view related global variables
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

// command pool related global variables
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

// command buffer related global variables
VkCommandBuffer *vkCommandBuffer_array = NULL;

// renderpass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

// framebuffers
VkFramebuffer *vkFramebuffer_array = NULL;

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

    return ((int)msg.wParam);
}

// CALLBACK Function
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

    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen(void)
{
    // Local Variable declarations
    MONITORINFO mi = {sizeof(MONITORINFO)};

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
    VkResult printVKInfo(void);
    VkResult createVulkanDevice(void);
    void getDeivceQueue(void);
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageView(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffer(void);
    VkResult createRenderPass(void);
    VkResult createFrameBuffers(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "================================== INITIALIZATION START ===================================\n\n");
    vkResult = createVulkanInstance();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "initialize()-> createVulkanInstance() failed %d !!!\n\n", vkResult);
        return (vkResult);
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
        return (vkResult);
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
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "initialize()-> getPhysicalDevice() success\n\n");
    }

    // print vulkan info using printVKInfo()
    vkResult = printVKInfo();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "initialize()-> printVKInfo() failed : %d !!!\n\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "initialize()-> printVKInfo() success\n\n");
    }

    // create vulkan (logical) device
    vkResult = createVulkanDevice();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createVulkanDevice() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createVulkanDevice() success\n\n", __func__);
    }

    // get device queue
    getDeivceQueue();

    // create swapchain
    vkResult = createSwapchain(VK_FALSE);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createSwapchain() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createSwapchain() success\n\n", __func__);
    }

    // create vulkan images and image view
    vkResult = createImagesAndImageView();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createImagesAndImageView() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createImagesAndImageView() success\n\n", __func__);
    }

    // create command pool
    vkResult = createCommandPool();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createCommandPool() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createCommandPool() success\n\n", __func__);
    }

    // create command buffer
    vkResult = createCommandBuffer();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createCommandBuffer() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createCommandBuffer() success\n\n", __func__);
    }

    // create render pass
    vkResult = createRenderPass();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createRenderPass() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createRenderPass() success\n\n", __func__);
    }


    vkResult = createFrameBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createFrameBuffers() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createFrameBuffers() success\n\n", __func__);
    }

    fprintf(gpFile, "================================== INITIALIZATION END ==================================\n\n");
    return (vkResult);
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

    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpFile, "%s()-> vkDeviceWaitIdle is done\n", __func__);
    }

    // destroy framebuffers
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
    }

    if(vkFramebuffer_array)
    {
        free(vkFramebuffer_array);
        vkFramebuffer_array = NULL;
    }

    // destroy vkRenderPass
    if(vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkRenderPass is done\n", __func__);
    }

    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
        vkCommandBuffer_array[i] = NULL;
    }
    
    if (vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        fprintf(gpFile, "%s()-> vkCommandBuffer_array is done\n", __func__);
    }

    // free command pool
    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = NULL;
        fprintf(gpFile, "%s()-> vkCommandPool is done\n", __func__);
    }

    // no need to destroy or uninitialize device queue

    // destroy image view
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
        swapchainImageView_array[i] = NULL;
    }

    if (swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        fprintf(gpFile, "%s()-> swapchainImageView_array is done\n", __func__);
    }

    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
    }

    // destroy swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkSwapchinKHR is uninitialized\n", __func__);
    }

    // no need to destroy the selected physical device

    // destroy vulkan device
    if (vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkDevice is uninitialized\n", __func__);
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
        return (vkResult);
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
        return (vkResult);
    }
    else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gpFile, "\ncreateVulkanInstance()-> vkCreateInstance() failed due to requred extension not present (ERROR CODE : %d)\n", vkResult);
        return (vkResult);
    }
    else if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "\ncreateVulkanInstance()-> vkCreateInstance() failed due to unkown reason (ERROR CODE : %d)\n", vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "\ncreateVulkanInstance()-> vkCreateInstance() Succeeded\n");
    }
    fprintf(gpFile, "\n======================== STEPS FOR INSTANCE CREATION END ================================\n\n");
    return (vkResult);
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
        return (vkResult);
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
        return (VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> 2nd called to vkEnumerateInstanceExtensionProperties() failed !!!\n\n");
        return (vkResult);
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
        return (VK_ERROR_OUT_OF_DEVICE_MEMORY);
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
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "\n\nfillInstanceExtensionNames()-> VK_KHR_SURFACE_EXTENSION_NAME FOUND !!!\n");
    }

    if (win32SurfaceExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(gpFile, "fillInstanceExtensionNames()-> VK_KHR_WIN32_SURFACE_EXTENSION_NAME NOT FOUND !!!\n\n");
        return (vkResult);
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
    return (vkResult);
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
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "getSupportedSurface()-> vkCreateWin32SufaceKHR() Success !!!\n\n");
    }

    fprintf(gpFile, "\n\n============================ STEPS FOR PRESENTATION SURFACE END ===============================\n\n");

    return (vkResult);
}

VkResult getPhysicalDevice(void)
{
    // code
    VkResult vkResult = VK_SUCCESS;

    fprintf(gpFile, "\n\n============================ STEPS FOR PHYSICAL DEVICE START ===============================\n\n");
    //  2. call vkEnumeratePhysicalDevices() to get physical device count.
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice()-> 1st call vkEnumeratePhysicalDevices() FAILED %d !!!\n\n", vkResult);
        return (vkResult);
    }
    else if (physicalDeviceCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevice()-> vkEnumeratePhysicalDevices() : physicalDeviceCount = 0 !!!\n\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice()-> 1st call vkEnumeratePhysicalDevices() Success !!!\n\n");
    }

    //  3. allocate VkPhysicalDevice array according to above count.
    vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

    //  4. Call vkEnumeratePhysicalDevices() again to fill above array.
    vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice()-> 2nd call vkEnumeratePhysicalDevices() FAILED %d !!!\n\n", vkResult);
        return (vkResult);
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
            fprintf(gpFile, "%s()-> vkPhyscialDevice_array free\n", __func__);
        }

        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
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

    return (vkResult);
}

VkResult printVKInfo(void)
{
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n\n============================ STEPS FOR PRINT VK INFO START ===============================\n\n");

    fprintf(gpFile, "\t\t\t\t\tVULKAN INFORMATION\n\n");

    for (uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void *)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

        // api version
        fprintf(gpFile, "\t\t\tVULKAN API VERSION : %d.%d.%d\n", majorVersion, minorVersion, patchVersion);

        // device name
        fprintf(gpFile, "\t\t\tVULKAN DEVICE NAME : %s\n", vkPhysicalDeviceProperties.deviceName);

        // device type
        switch (vkPhysicalDeviceProperties.deviceType)
        {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            fprintf(gpFile, "\t\t\tVULKAN DEVICE TYPE : INTEGRATED GPU (iGPU)\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            fprintf(gpFile, "\t\t\tVULKAN DEVICE TYPE : DISCRETE GPU (dGPU)\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            fprintf(gpFile, "\t\t\tVULKAN DEVICE TYPE : VIRTUAL GPU (vGPU)\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            fprintf(gpFile, "\t\t\tVULKAN DEVICE TYPE : CPU\n");
            break;

        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            fprintf(gpFile, "\t\t\tVULKAN DEVICE TYPE : OTHER\n");
            break;

        default:
            fprintf(gpFile, "\t\t\tVULKAN DEVICE TYPE : UNKNOWN\n");
            break;
        }

        // vendor id
        fprintf(gpFile, "\t\t\tVULKAN VENDER ID   : 0x%04x\n", vkPhysicalDeviceProperties.vendorID);

        // device id
        fprintf(gpFile, "\t\t\tVULKAN DEVICE ID   : 0x%04x\n\n\n", vkPhysicalDeviceProperties.deviceID);
    }

    // free global physical device array
    if (vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        fprintf(gpFile, "%s()-> vkPhyscialDevice_array free\n", __func__);
    }

    fprintf(gpFile, "\n\n============================ STEPS FOR PRINT VK INFO END ===============================\n\n");
    return (vkResult);
}

VkResult fillDeviceExtensionNames(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t deviceExtensionCount = 0;
    VkExtensionProperties *vkExtensionProperties_array = NULL;
    char **deviceExtensionNames_array = NULL;

    //  STEPS FOR INSTANCE EXTENSIONS
    // 1 : find how many instance extensions are supported by the vulkan driver of this version and keep it in a local variable.
    fprintf(gpFile, "\n======================== STEPS FOR DEVICE EXTENSIONS START ================================\n\n");
    vkResult = vkEnumerateDeviceExtensionProperties(vkPhyscialDevice_selected, NULL, &deviceExtensionCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> 1st called to vkEnumerateDeviceExtensionProperties() failed !!!\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> 1st called to vkEnumerateDeviceExtensionProperties() success\n", __func__);
        fprintf(gpFile, "\n%s()-> Device Extensions Count : %u\n\n", __func__, deviceExtensionCount);
    }

    // 2. allocate and fill struct VkExtension proprties array corrosponding to above count.
    vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
    if (vkExtensionProperties_array == NULL)
    {
        fprintf(gpFile, "%s()-> malloc() failed for vkExtensionProperties_array\n", __func__);
        return (VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhyscialDevice_selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> 2nd called to vkEnumerateDeviceExtensionProperties() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> 2nd called to vkEnumerateDeviceExtensionProperties() success\n\n", __func__);
    }

    // 3. fill and display a local string array of extension names obtained from VkExtension properties.
    deviceExtensionNames_array = (char **)malloc(sizeof(char *) * deviceExtensionCount);
    if (deviceExtensionNames_array == NULL)
    {
        fprintf(gpFile, "%s()-> malloc() failed for deviceExtensionNames_array\n", __func__);
    }

    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "%s()-> vulkan device extension name : %s\n", __func__, deviceExtensionNames_array[i]);
    }

    // 4. As not required here onwords free VkExtension Array.
    if (vkExtensionProperties_array)
    {
        free(vkExtensionProperties_array);
        vkExtensionProperties_array = NULL;
    }

    // 5. find whether above extension names contain our required extentions (VK_KHR_SWAPCHAIN_EXTENSION_NAME) accordingly set two global variable.
    VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;

    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if (strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapchainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }

    // 6. As not needed hencefore free local string array.
    for (uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if (deviceExtensionNames_array[i])
        {
            free(deviceExtensionNames_array[i]);
            deviceExtensionNames_array[i] = NULL;
        }
    }

    if (deviceExtensionNames_array)
    {
        free(deviceExtensionNames_array);
        deviceExtensionNames_array = NULL;
    }

    // 7. Print whether our valkan driver support our required extensions found or not
    if (vulkanSwapchainExtensionFound == VK_FALSE)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
        fprintf(gpFile, "\n\n%s()-> VK_KHR_SWAPCHAIN_EXTENSION_NAME NOT FOUND !!!\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "\n\n%s()-> VK_KHR_SWAPCHAIN_EXTENSION_NAME FOUND !!!\n", __func__);
    }

    // 8. Print only supported extension names.
    for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
    {
        fprintf(gpFile, "%s()-> ENABLED VULKAN DEVICE EXTENTION NAME : %s\n", __func__, enabledDeviceExtensionNames_array[i]);
    }

    fprintf(gpFile, "\n\n============================ STEPS FOR DEVICE EXTENSIONS END ===============================\n\n");
    return (vkResult);
}

VkResult createVulkanDevice(void)
{
    // function declarations
    VkResult fillDeviceExtensionNames(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== STEPS CREATING VULKAN DEVICE START ================================\n\n");

    // a. fill device extensions
    vkResult = fillDeviceExtensionNames();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> fillDeviceExtensionNames() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> fillDeviceExtensionNames() success\n\n", __func__);
    }

    // newly added code for Vulkan Device Queue
    float queuePriorities[1];
    queuePriorities[0] = 1.0f;

    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void *)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));

    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;

    // b. Declare and initialize VkDeviceCreateInfoSturcture
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void *)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    // c. initialize this structure.
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.enabledLayerCount = 0;      // deprecated in latest version
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL; // deprecated in latest version
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

    // d. Now call vkCreateDevice() vulkan api to actually create the vulkan logical device and do error checking.
    vkResult = vkCreateDevice(vkPhyscialDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateDevice() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateDevice() Created !!!\n\n", __func__);
    }
    fprintf(gpFile, "\n======================== STEPS CREATING VULKAN DEVICE END ================================\n\n");
    return (vkResult);
}

void getDeivceQueue(void)
{
    // code
    fprintf(gpFile, "\n======================== STEPS FOR GETTING VULKAN DEVICE QUEUE START ================================\n\n");
    vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);

    if (vkQueue == VK_NULL_HANDLE)
    {
        fprintf(gpFile, "%s()-> vkGetDeviceQueue() failed !!!\n\n", __func__);
        return;
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetDeviceQueue() succeeded !!!\n\n", __func__);
    }
    fprintf(gpFile, "\n======================== STEPS FOR GETTING VULKAN DEVICE QUEUE END ================================\n\n");
}

VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    // local variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t formatCount = 0;

    // code
    fprintf(gpFile, "\n======================== STEPS FOR SURFACE FORMAT AND COLOR SPACE START ================================\n\n");
    // get the count of supported surface color format
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhyscialDevice_selected, vkSurfaceKHR, &formatCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfaceFormatsKHR() 1st call failed !!!\n\n", __func__);
        return (vkResult);
    }
    else if (formatCount == 0)
    {
        fprintf(gpFile, "%s()-> format count is zero\n\n", __func__);
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfaceFormatsKHR() 1st call success\n\n", __func__);
    }

    // Declare and allocate array of VkSurfaceFormatStructure currosponding to above count
    VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));

    // fill the array
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhyscialDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfaceFormatsKHR() 2nd call failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfaceFormatsKHR() 2nd call success\n\n", __func__);
    }

    // decide the surface color format first
    if (formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
    {
        vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
    }
    else
    {
        vkFormat_color = vkSurfaceFormatKHR_array[0].format;
    }

    // decide the color space
    vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

    if (vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        fprintf(gpFile, "\n%s()-> vkSurfaceFormatKHR_array is freed\n\n", __func__);
    }

    fprintf(gpFile, "\n======================== STEPS FOR SURFACE FORMAT AND COLOR SPACE END ================================\n\n");

    return (vkResult);
}

VkResult getPhysicalDevicePresentMode(void)
{
    // local variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t presentModeCount = 0;

    // code
    fprintf(gpFile, "\n======================== STEPS FOR PRESENT START ================================\n\n");

    // get count of present modes
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhyscialDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfacePresentModesKHR() 1st call failed !!!\n\n", __func__);
        return (vkResult);
    }
    else if (presentModeCount == 0)
    {
        fprintf(gpFile, "%s()-> presentModeCount is zero\n\n", __func__);
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfacePresentModesKHR() 1st call success\n\n", __func__);
    }

    // Declare and allocate araay of VkPresentationModeKHR Enum currospoding to above count.
    VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

    // fill the array using above function
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhyscialDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfacePresentModesKHR() 2nd call failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfacePresentModesKHR() 2nd call success\n\n", __func__);
    }

    // decide the Present Mode
    for (uint32_t i = 0; i < presentModeCount; i++)
    {
        if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            fprintf(gpFile, "\nvkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR Get.\n");
            break;
        }
    }

    if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gpFile, "\nvkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR Get.\n");
    }

    // free array
    if (vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(gpFile, "\n%s()-> vkPresentModeKHR_array is freed\n\n", __func__);
    }

    fprintf(gpFile, "\n======================== STEPS FOR PRESENT END ================================\n\n");
    return (vkResult);
}

VkResult createSwapchain(VkBool32 vsync)
{
    // function declarations
    VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult getPhysicalDevicePresentMode(void);

    // local variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE SWAPCHAIN START ================================\n\n");

    // get surface format and color space
    vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> getPhysicalDeviceSurfaceFormatAndColorSpace() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> getPhysicalDeviceSurfaceFormatAndColorSpace() success\n\n", __func__);
    }

    // 2. Get Physical device surface capabilities by using vulkan API vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;

    memset((void *)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhyscialDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() success\n\n", __func__);
    }

    // find out desired number of swapchain image count
    uint32_t testingNumberOfSwapchainImages = 0;
    testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;

    uint32_t desiredNumberOfSwapchainImages = 0;

    if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
    {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    }
    else
    {
        desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }

    // choose size of swapchain image
    memset((void *)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));

    if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;

        fprintf(gpFile, "%s()-> swapchain image width x heght : %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }
    else
    {
        // if surface size is already defined then swapchain image size must match with it.
        VkExtent2D vkExtent2D;
        memset((void *)&vkExtent2D, 0, sizeof(VkExtent2D));

        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;

        vkExtent2D_swapchain.width = max(vkSurfaceCapabilitiesKHR.minImageExtent.width, min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        vkExtent2D_swapchain.height = max(vkSurfaceCapabilitiesKHR.minImageExtent.height, min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
    }

    // set swapchain image usage flag enum
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    // whether to consider pretransformed/flipping or not enum
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;

    if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
    }

    // get present mode
    vkResult = getPhysicalDevicePresentMode();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> getPhysicalDevicePresentMode() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> getPhysicalDevicePresentMode() success\n\n", __func__);
    }

    // initialize VkCreateSwapchainCreateInfo structure
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void *)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

    vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateSwapchainKHR() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateSwapchainKHR() success\n\n", __func__);
    }

    fprintf(gpFile, "\n======================== CREATE SWAPCHAIN END ================================\n\n");
    return (vkResult);
}

VkResult createImagesAndImageView(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE IMAGES AND IMAGE VIEW START ================================\n\n");

    // Get Swapchain image count in a global variable using VkGetSwapchainImagesKHR().
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateImageView() 1st call failed !!!\n\n", __func__);
        return (vkResult);
    }
    else if (swapchainImageCount == 0)
    {
        fprintf(gpFile, "%s()-> swapchainImageCount is zero 1st call\n\n", __func__);
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateImageView() 1st call success\n\n", __func__);
        fprintf(gpFile, "%s()-> swapchainimage count : %d\n\n", __func__, swapchainImageCount);
    }

    // allocate the swapchainImage_array
    swapchainImage_array = (VkImage *)malloc(sizeof(VkImage) * swapchainImageCount);

    // fill array with swapchain images
    vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateImageView() 2st call failed !!!\n\n", __func__);
        return (vkResult);
    }
    else if (swapchainImageCount == 0)
    {
        fprintf(gpFile, "%s()-> swapchainImageCount is zero 2nd call\n\n", __func__);
        return (VK_ERROR_INITIALIZATION_FAILED);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateImageView() 2st call success\n\n", __func__);
        fprintf(gpFile, "%s()-> swapchainimage count : %d\n\n", __func__, swapchainImageCount);
    }

    // allocate array of swapchain image view
    swapchainImageView_array = (VkImageView *)malloc(sizeof(VkImageView) * swapchainImageCount);

    // initialize vkImageViewCreateInfo structure
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void *)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    // fill image view array using above struct
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = swapchainImage_array[i];
        vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "%s()-> vkCreateImageViews() failed  for i = %d (ERROR CODE : %d )!!!\n\n", __func__, i, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkCreateImageViews() call success for i = %d\n\n", __func__, i);
        }
    }

    fprintf(gpFile, "\n======================== CREATE IMAGES AND IMAGE VIEW END ================================\n\n");
    return (vkResult);
}

VkResult createCommandPool(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE COMMAND POOL START ================================\n\n");

    // declare and initialize VkCommandPoolCreateInfo sruct
    VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
    memset((void *)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

    vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    vkCommandPoolCreateInfo.pNext = NULL;
    vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

    // call vkCreateCommandPool() to create the commandPool
    vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateCommandPool() failed (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateCommandPool() call success\n\n", __func__);
    }

    fprintf(gpFile, "\n======================== CREATE COMMAND POOL END ================================\n\n");

    return (vkResult);
}

VkResult createCommandBuffer(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE COMMAND BUFFER START ================================\n\n");
    
    // initialize vkCommandBuffer
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
    memset((void *)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

    vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo.pNext = NULL;
    vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    vkCommandBufferAllocateInfo.commandBufferCount = 1;

    vkCommandBuffer_array = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);

    // allocate command buffer
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);

        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "%s()-> vkAllocateCommandBuffers() failed  for i = %d (ERROR CODE : %d )!!!\n\n", __func__, i, vkResult);
            return (vkResult);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkAllocateCommandBuffers() call success for i = %d\n\n", __func__, i);
        }
    }

    fprintf(gpFile, "\n========================== CREATE COMMAND BUFFER END ================================\n\n");
    return(vkResult);
}

VkResult createRenderPass(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE RenderPass START ================================\n\n");

    // declare and initialize VkAttachmentDescriptor
    VkAttachmentDescription vkAttachmentDescription_array[1];

    memset((void *)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

    vkAttachmentDescription_array[0].flags = 0;
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // it is also depth loadOp
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // it is also depth storeOp
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // declare and initialize VkAttachmentReference structure
    VkAttachmentReference vkAttachmentReference;
    memset((void *)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

    vkAttachmentReference.attachment = 0; // above attachment index number
    vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    //  Declare and initialize VkSubpassDescription structure
    VkSubpassDescription vkSubpassDescription;
    memset((void *)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = NULL;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;

    // Declare and initialize VkRenderPassCreateInfo structure
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void *)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;

    // call create render pass
    vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateRenderPass() failed (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateRenderPass() call success\n\n", __func__);
    }

    fprintf(gpFile, "\n======================== CREATE RenderPass END ================================\n\n");
    return(vkResult);
}

VkResult createFrameBuffers(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE FRAMEBUFFERS START ================================\n\n");

    VkImageView vkImageView_attachments_array[1];
    memset((void *)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

    //
    VkFramebufferCreateInfo vkFramebufferCreateInfo;
    memset((void *)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.pNext = NULL;
    vkFramebufferCreateInfo.flags = 0;
    vkFramebufferCreateInfo.renderPass = vkRenderPass;
    vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
    vkFramebufferCreateInfo.pAttachments = vkImageView_attachments_array;
    vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
    vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
    vkFramebufferCreateInfo.layers = vkExtent2D_swapchain.layers;

    //  Allocate the framebuffer array by malloc equal to the sizeof swapchain image count.
    vkFramebuffer_array = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapchainImageCount);

    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageView_attachments_array[0] = swapchainImageView_array[i];
        vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[i]);

        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "%s()-> vkCreateFramebuffer() failed for i = %d (ERROR CODE : %d)\n\n", __func__, i, vkResult);
            return(vkResult);
        }
    }

    fprintf(gpFile, "\n======================== CREATE FRAMEBUFFERS START ================================\n\n");
    
    return(VkResult);
}

/**
-----------------------------------------------------------------------------------------------
 
 STEPS FOR RENDER PASS
 ---------------------
 1. declare and initialize VkAttachmentDescription array (number of array elements depends upon number of attachments) although we have only one attachment i.e colorattachment we will consider it as array.
 2. declare and initialize VkAttachmentReference structure which will have information about the attachment we describe above.
 3. Declare and initialize VkSubpassDescription structure and keep information about above VkAttachmentReference structure.
 4. Declare and initialize VkRenderPassCreateInfo structure and refer VkAttachmentDescription and VkSubpassDescription into it.
    Remeber : here also we need to specify interdependency of subpasses if needed and also attachment information in the form of image views which will used by framebuffer letter to create the actual RenderPass.
 5. in uninitialze destroy the renderpass by using vkDestroyRenderPass().
 
 STEPS FOR FRAME BUFFERS
 -----------------------
 1. Declare an array of vkImageView equal to number of attachment means in our example array of one attachment.
 2. Declare and inistalize VkFrameBufferCreateInfo structure.
 3. Allocate the framebuffer array by malloc equal to the sizeof swapchain image count.
 4. start a loop for swapchain image count and call vkCreateFrameBuffer() to create FrameBuffers. 
 5. In uninitialize destroy framebuffer in a loop for swapchain image count.

 -----------------------------------------------------------------------------------------------
 Notes:
 all struct, enum and types names = Vk...
 all apis starts from vk....

 // build commands for windows build.bat
 cls
 cl.exe /c /EHsc /I C:VulkanSDK\Valkan\include vk.c
 rc.exe vk.rc
 link.exe vk.obj vk.res /LIBPATH:C:VulkanSDK\valkan\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

 */
