// Windows Header File
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "VK.h"

// vulkan related header file
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// glm related macros and header file
#define GLM_FORCE_RADIAN
#define GLM_FORCE_DEPTH_ZERO_TO_1
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

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
BOOL gbWindowMinimize = FALSE;

HWND ghwnd;
BOOL gbActive = FALSE;

// vulkan related global variables
// instance extension related variables
uint32_t enabledInstanceExtensionCount = 0;
// VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME and VK_EXT_DEBUG_REPORT_EXTENSION_NAME we are using these two macros and going to store in following array
const char *enabledInstanceExtensionNames_array[3];

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

// for semaphores and fences
VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_renderComplete = VK_NULL_HANDLE;
VkFence *vkFence_array = NULL;

// clear color values
VkClearColorValue vkClearColorValue;

// for render step
BOOL bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;

// for validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char *enabledValidationLayerNames_array[1]; // for VK_LAYER_KHRONOS_VALIDATION
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

// vertex buffer related variable
typedef struct 
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} VertexData;

// position
VertexData vertexData_position;

VertexData vertexData_color;

// uniform related declarations
struct MyUniformData
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};

typedef struct UniformData
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
} UniformData;

UniformData uniformData;


// shader related variables
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

// descriptor set layout
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

// pipeline layout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

// descriptor pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;
//descriptor set
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

// for pipeline 
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
    // Function declarations

    VkResult initialize(void);
    void uninitialize(void);
    VkResult display(void);
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
                if(gbWindowMinimize == FALSE)
                {
                    // render
                    vkResult = display();
                    if(vkResult != VK_FALSE && vkResult != VK_SUCCESS && vkResult != VK_ERROR_OUT_OF_DATE_KHR && vkResult != VK_SUBOPTIMAL_KHR)
                    {
                        fprintf(gpFile, "%s()-> display() failed\n\n", __func__);
                        bDone = TRUE;
                    }

                    // update
                    update();
                }
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

    VkResult resize(int, int);

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
        if(wParam == SIZE_MINIMIZED)
        {
            gbWindowMinimize = TRUE;
        }
        else
        {
            gbWindowMinimize = FALSE;
            resize(LOWORD(lParam), HIWORD(lParam));
        }
        break;

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
    VkResult createVertexBuffer(void);
    VkResult createUniformBuffer(void);
    VkResult createShaders(void);
    VkResult createDescriptorSetLayout(void);
    VkResult createPipelineLayout(void);
    VkResult createDescriptorPool(void);
    VkResult createDescriptorSet(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFrameBuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);
    VkResult buildCommandBuffers(void);
    
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

    // create vertex buffer
    vkResult = createVertexBuffer();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createVertexBuffer() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createVertexBuffer() success\n\n", __func__);
    }

    // create 
    vkResult = createUniformBuffer();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createUniformBuffer() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createUniformBuffer() success\n\n", __func__);
    }

    // create shaders
    vkResult = createShaders();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createShaders() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createShaders() success\n\n", __func__);
    }

    // create descriptor set layout
    vkResult = createDescriptorSetLayout();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createDescriptorSetLayout() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createDescriptorSetLayout() success\n\n", __func__);
    }

    // create pipeline layout
    vkResult = createPipelineLayout();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createPipelineLayout() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createPipelineLayout() success\n\n", __func__);
    }

    // create descriptor pool
    vkResult = createDescriptorPool();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createDescriptorPool() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createDescriptorPool() success\n\n", __func__);
    }

    // create descriptor set
    vkResult = createDescriptorSet();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createDescriptorSet() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createDescriptorSet() success\n\n", __func__);
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

    // create pipeline
    vkResult = createPipeline();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createPipeline() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createPipeline() success\n\n", __func__);
    }

    // create frame buffer
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

    // create semaphore
    vkResult = createSemaphores();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createSemaphores() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createSemaphores() success\n\n", __func__);
    }

    // create fences
    vkResult = createFences();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createFences() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> createFences() success\n\n", __func__);
    }

    // initialize clearColorValue
    memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));
    vkClearColorValue.float32[0] = 0.0f;
    vkClearColorValue.float32[1] = 0.0f;
    vkClearColorValue.float32[2] = 0.0f;
    vkClearColorValue.float32[3] = 1.0f; // analogous to glClearColor

    // build command buffer
    vkResult = buildCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> buildCommandBuffers() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> buildCommandBuffers() success\n\n", __func__);
    }

    // initialization is completed
    bInitialized = TRUE;
    fprintf(gpFile, "Initializeation successfully !!!\n\n");
    fprintf(gpFile, "================================== INITIALIZATION END ==================================\n\n");
    return (vkResult);
}

VkResult resize(int width, int height)
{
    // local function declarations
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageView(void);
    VkResult createCommandBuffer(void);
    VkResult createPipelineLayout(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFrameBuffers(void);
    VkResult buildCommandBuffers(void);

    // local variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    // code
    if (height <= 0)
    {
        height = 1;
    }

    // check the bInitialized variable
    if(bInitialized == FALSE)
    {
        fprintf(gpFile, "%s()-> Initialization yet not completed or failed !!!\n\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }

    // as recreation of swapchain if needed we are going to repeat many steps of initialize again hence set bInitialized to FALSE again
    bInitialized = FALSE;

    // set global winWidth and winHeight variables
    winWidth = width;
    winHeight = height;

    // wait for device to complete in-hand task
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpFile, "%s()-> vkDeviceWaitIdle is done\n", __func__);
    }

    // check precence of swapchain
    if(vkSwapchainKHR == VK_NULL_HANDLE)
    {
        fprintf(gpFile, "%s()-> swapchain is already NULL can not procced\n", __func__);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
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

    // destroy pipeline
    if(vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
    }

    // destroy vkRenderPass
    if(vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
    }

    // free pipeline layout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
    }

    // destroy command buffers
    for (uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
        vkCommandBuffer_array[i] = NULL;
    }
    
    if (vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
    }

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
    }

    //////////////////////// RECREATE FOR RESIZE ////////////////
    // create swapchain
    vkResult = createSwapchain(VK_FALSE);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createSwapchain() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
    
    // create vulkan images and image view
    vkResult = createImagesAndImageView();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createImagesAndImageView() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // create command buffer
    vkResult = createCommandBuffer();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createCommandBuffer() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
   
    // create pipeline layout
    vkResult = createPipelineLayout();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createPipelineLayout() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

     // create render pass
    vkResult = createRenderPass();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createRenderPass() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

     // create pipeline
    vkResult = createPipeline();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createPipeline() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // create frame buffer
    vkResult = createFrameBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> createFrameBuffers() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }

    // build command buffer
    vkResult = buildCommandBuffers();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> buildCommandBuffers() failed !!! (ERROR CODE : %d)\n\n", __func__, vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return (vkResult);
    }
   
    bInitialized = TRUE;
    return(vkResult);
}

VkResult display(void)
{
    // function declration
    VkResult resize(int, int);
    VkResult updateUniformBuffer(void);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code

    // if control comes here before initalization gets completed return FALSE.
    if(bInitialized == FALSE)
    {
        fprintf(gpFile, "%s()-> initialization Yet Not Completed !!!\n\n", __func__);
        return((VkResult)VK_FALSE);
    }

    // aquaire index of next swapchain image
    vkResult = vkAcquireNextImageKHR(vkDevice, 
                                    vkSwapchainKHR,
                                    UINT64_MAX, // time in nano sec, here we are waiting for swapchain next Image, if we don't get image within timout return VK_NOT_READY 
                                    vkSemaphore_backBuffer, // waiting for previous operation to release swapchain image.
                                    VK_NULL_HANDLE,
                                    &currentImageIndex);
    if(vkResult != VK_SUCCESS)
    {
        if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR)
        {
            resize(winWidth, winHeight);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkAcquireNextImageKHR() failed (ERROR CODE : %d) !!!\n\n", __func__, vkResult);
            return(vkResult);
        }
    }

    // use fence to allow host to wait for completion of execution of previous commandBuffer
    vkResult = vkWaitForFences(vkDevice, 
                               1,           // number of fences to wait
                               &vkFence_array[currentImageIndex],
                               VK_TRUE,     // if true wait for all fences to get signaled, if false wait for one fence to get signaled
                               UINT64_MAX   // timeout in nano sec
                              );
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkWaitForFences() failed (ERROR CODE : %d) !!!\n\n", __func__, vkResult);
        return(vkResult);
    }

    // make fences ready the next command buffer
    vkResult = vkResetFences(vkDevice, 
                            1, // number of fences
                            &vkFence_array[currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkResetFences() failed (ERROR CODE : %d) !!!\n\n", __func__, vkResult);
        return(vkResult);
    }
    
    // one of the member of VkSubmitInfoStruct requires array of pipeline stages we have only 1 completion of color attachment, still we need 1 member array
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    // declare, memset and initialize VkSubmitInfo structure
    VkSubmitInfo vkSubmitInfo;
    memset((void *)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backBuffer;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_renderComplete;

    // submit above work to the queue
    vkResult = vkQueueSubmit(vkQueue,
                            1,      // number of vkSubmit info
                            &vkSubmitInfo,
                            vkFence_array[currentImageIndex]);

    // we are going to present render image after declaring and initializing VkPresentInfoKHR structure
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void *)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_renderComplete;

    vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);

    if(vkResult != VK_SUCCESS)
    {
         if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR)
        {
            resize(winWidth, winHeight);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkQueuePresentKHR() failed (ERROR CODE : %d) !!!\n\n", __func__, vkResult);
            return(vkResult);
        }
    }

    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkResetFences() failed (ERROR CODE : %d) !!!\n\n", __func__, vkResult);
        return(vkResult);
    }

    vkDeviceWaitIdle(vkDevice);

    return(vkResult);
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

    // destroy fences
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyFence(vkDevice, vkFence_array[i], NULL);
    }

    if(vkFence_array)
    {
        free(vkFence_array);
        vkFence_array = NULL;
        fprintf(gpFile, "%s()-> vkFence_array freed\n", __func__);
    }

    // destroy semaphores
    if(vkSemaphore_renderComplete)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
        vkSemaphore_renderComplete = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkSemaphore_renderComplete freed\n", __func__);
    }

    if(vkSemaphore_backBuffer)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
        vkSemaphore_backBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkSemaphore_backBuffer freed\n", __func__);
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

    // destroy pipeline
    if(vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkPipeline is freed\n", __func__);
    }

    // destroy vkRenderPass
    if(vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkRenderPass is done\n", __func__);
    }

    // destroy descriptor pool
    // when we destory descriptor pool it will destroy vkDesriptor Set also 
    if(vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        vkDescriptorSet = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkDescriptorPool and vkDescriptorSet destroyed Successfully\n", __func__);
    }

    // free descriptor set layout
    if (vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = NULL;
        fprintf(gpFile, "%s()-> vkDescriptorSetLayout is freed\n", __func__);
    }

    // free pipeline layout
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkPipelineLayout is freed\n", __func__);
    }

    // free fragment shader
    if (vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkShaderModule_fragment_shader is freed\n", __func__);
    }

    // free vertex shader
    if (vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vkShaderModule_vertex_shader is freed\n", __func__);
    }

    // free uniformData.vkDeviceMemory
    if(uniformData.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
        uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> uniformData.vkDeviceMemory is freed\n", __func__);
    }

    // free uniform Buffer
    if(uniformData.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
        uniformData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> uniformData.vkBuffer is freed\n", __func__);
    }

    // free vertexData_color
    if(vertexData_color.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_color.vkDeviceMemory, NULL);
        vertexData_color.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vertexData_color.vkDeviceMemory is freed\n", __func__);
    }

    if(vertexData_color.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_color.vkBuffer, NULL);
        vertexData_color.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vertexData_color.vkBuffer is freed\n", __func__);
    }

    // free vkDevice memory
    if(vertexData_position.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
        vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vertexData_position.vkDeviceMemory is freed\n", __func__);
    }

    if(vertexData_position.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
        vertexData_position.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "%s()-> vertexData_position.vkBuffer is freed\n", __func__);
    }

    // destroy command buffers
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

    // destroy debug report callback
    if(vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
        vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr = NULL;
    }

    // destroy vulkan instance
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpFile, "uninitialize()-> vkDestroyInstance() succeeded\n");
    }

    // close the log file
    fprintf(gpFile, "uninitialize()-> Program Ended Successfully !!!\n\n");
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
    VkResult fillValidationLayerNames(void);
    VkResult createValidationCallbackFunction(void);

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

    if(bValidation == TRUE)
    {    
        //  fill and initalize required validation layer names and count global variables.
        vkResult = fillValidationLayerNames();
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance()-> fillValidationLayerNames() failed !!!\n\n");
            return (vkResult);
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance()-> fillValidationLayerNames() success\n\n");
        }
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
    
    if(bValidation == TRUE)
    {
        vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array;
    }
    else
    {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL; 
    }

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

    // do for validation callbacks
    if(bValidation == TRUE)
    {
        vkResult = createValidationCallbackFunction();
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance()-> createValidationCallbackFunction() failed !!!\n\n");
            return (vkResult);
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance()-> createValidationCallbackFunction() success\n\n");
        }
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
    VkBool32 debugReportExtensionFound = VK_FALSE;

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

        if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            debugReportExtensionFound = VK_TRUE;
            if(bValidation == TRUE)
            {
                enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
            else
            {
                // array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
            }
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

    
    if(debugReportExtensionFound == VK_FALSE)
    {
        if(bValidation == TRUE)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
            fprintf(gpFile, "fillInstanceExtensionNames()-> Validation is on but VK_EXT_DEBUG_REPORT_EXTENSION_NAME NOT SUPPORTED !!!\n\n");
            return (vkResult);
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames()-> Validation is off and VK_EXT_DEBUG_REPORT_EXTENSION_NAME NOT SUPPORTED !!!\n\n");
        }
    }
    else
    {
        if(bValidation == TRUE)
        {
            fprintf(gpFile, "fillInstanceExtensionNames()-> Validation is on and VK_EXT_DEBUG_REPORT_EXTENSION_NAME SUPPORTED\n\n");
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames()-> Validation is off but VK_EXT_DEBUG_REPORT_EXTENSION_NAME SUPPORTED\n\n");
        }
    }
    
    // 8. Print only supported extension names.
    for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
    {
        fprintf(gpFile, "fillInstanceExtensionNames()-> ENABLED VULKAN INSTANCE EXTENTION NAME : %s\n", enabledInstanceExtensionNames_array[i]);
    }

    fprintf(gpFile, "\n\n============================ STEPS FOR INSTANCE EXTENSIONS END ===============================\n\n");
    return (vkResult);
}

VkResult fillValidationLayerNames(void)
{
    // // variable declarations
    VkResult vkResult = VK_SUCCESS;
    uint32_t validationLayerCount = 0;
    VkLayerProperties *vkLayerProperties_array = NULL;
    char **validationLayerNames_array = NULL;

    // code
    fprintf(gpFile, "\n======================== STEPS FOR FILL VALIDATION LAYER NAMES START ================================\n\n");

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames()-> 1st called to vkEnumerateInstanceLayerProperties() failed !!!\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames()-> 1st called to vkEnumerateInstanceLayerProperties() success\n");
    }

    vkLayerProperties_array = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * validationLayerCount);

    if(vkLayerProperties_array == NULL)
    {
        fprintf(gpFile, "%s()-> malloc() failed for vkLayerProperties_array !!!\n\n", __func__);        
		return(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }
    else
    {
        fprintf(gpFile, "%s()-> malloc() success for vkLayerProperties_array\n\n", __func__);
    }

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames()-> 2nd called to vkEnumerateInstanceLayerProperties() failed !!!\n");
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames()-> 2nd called to vkEnumerateInstanceLayerProperties() success\n");
    }

    validationLayerNames_array = (char **)malloc(sizeof(char *) * validationLayerCount);

    if(validationLayerNames_array == NULL)
    {
        fprintf(gpFile, "%s()-> malloc() failed for validationLayerNames_array !!!\n\n", __func__);        
		return(VK_ERROR_OUT_OF_DEVICE_MEMORY);
    }
    else
    {
        fprintf(gpFile, "%s()-> malloc() success for validationLayerNames_array\n\n", __func__);
    }

    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char *)malloc(sizeof(char) * (strlen(vkLayerProperties_array[i].layerName) + 1));

        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, (strlen(vkLayerProperties_array[i].layerName) + 1));

        fprintf(gpFile, "fillValidationLayerNames()-> vlkan validation layer name : i = %d => %s\n", i, validationLayerNames_array[i]);
    }

    if(vkLayerProperties_array)
    {
        free(vkLayerProperties_array);
        vkLayerProperties_array = NULL;
    }

    // // for required validation layer 
    VkBool32 validationLayerFound = VK_FALSE;

    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        if (strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
        {
            validationLayerFound = VK_TRUE;
            enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }

    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        if(validationLayerNames_array[i])
        {
            free(validationLayerNames_array[i]);
            validationLayerNames_array[i] = NULL;
        }
    }

    if(validationLayerNames_array)
    {
       free(validationLayerNames_array);
       validationLayerNames_array = NULL; 
    }

    if(bValidation == TRUE)
    {
        if(validationLayerFound == VK_FALSE)
        {
            vkResult = VK_ERROR_INITIALIZATION_FAILED; // return hardcoded failure
            fprintf(gpFile, "fillValidationLayerNames()-> VK_LAYER_KHRONOS_validation NOT SUPPORTED !!!\n\n");
            return (vkResult);
        }
        else
        {
            fprintf(gpFile, "fillValidationLayerNames()-> VK_LAYER_KHRONOS_validation SUPPORTED !!!\n\n");
        }
    }

    for(uint32_t i = 0; i < enabledValidationLayerCount; i++)
    {
        fprintf(gpFile, "fillValidationLayerNames()-> ENABLED VULKAN VALIDATION LAYER NAME : %s\n\n", enabledValidationLayerNames_array[i]);
    }

    fprintf(gpFile, "\n======================== STEPS FOR FILL VALIDATION LAYER NAMES END ================================\n\n");
    return (vkResult);
}

VkResult createValidationCallbackFunction(void)
{
    // local function declarations
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char *, const char *, void *);

    // variable declarations
    VkResult vkResult = VK_SUCCESS;
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL; 
    // code
    fprintf(gpFile, "\n======================== STEPS FOR CREATE VALIDATION CALLBACK FUNCTION START ================================\n\n");

    // get the required function pointer
    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");

    if(vkCreateDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "%s()-> vkGetInstanceProcAddr() failed to get function pointer for vkCreateDebugReportCallbackEXT\n\n", __func__);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetInstanceProcAddr() function pointer for vkCreateDebugReportCallbackEXT found\n\n", __func__);
    }

    // for destory
    vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");

    if(vkDestroyDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "%s()-> vkGetInstanceProcAddr() failed to get function pointer for vkDestroyDebugReportCallbackEXT\n\n", __func__);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkGetInstanceProcAddr() function pointer for vkDestroyDebugReportCallbackEXT found\n\n", __func__);
    }

    // get the vulkan debug callback object
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset((void *)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));

    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;

    vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);

    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateDebugReportCallbackEXT_fnptr() failed !!!\n\n", __func__);
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateDebugReportCallbackEXT_fnptr() success\n\n", __func__);
    }

    fprintf(gpFile, "\n======================== STEPS FOR CREATE VALIDATION CALLBACK FUNCTION END ================================\n\n");
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

        vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
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

VkResult createVertexBuffer(void)
{
    fprintf(gpFile, "\n======================== CREATE VERTEX BUFFER START ================================\n\n");
    // local variable declarations
    VkResult vkResult = VK_SUCCESS;
    
    float triangle_position[] = 
    {
        0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f
    };

    float triangle_color[] = 
    {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    // code
    
    // ------- Vertex position buffer

    // memset our global vertexData_position struct.
    memset((void *)&vertexData_position, 0, sizeof(VertexData));

    //  declare and memset struct VkBufferCreateInfo
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // flags are used for scatterd / sparce buffer
    vkBufferCreateInfo.size = sizeof(triangle_position);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // call vkCreateBuffer() vulkan api in the .vkBuffer member of our global struct.
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateBuffer() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateBuffer() call success\n\n", __func__);
    }

    VkMemoryRequirements vkMemoryRequirements;
    memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);


    VkMemoryAllocateInfo vkMemeoryAllocateInfo;
    memset((void *)&vkMemeoryAllocateInfo, 0, sizeof(vkMemeoryAllocateInfo));

    vkMemeoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemeoryAllocateInfo.allocationSize = vkMemoryRequirements.size;

    vkMemeoryAllocateInfo.memoryTypeIndex = 0; // inital value before entring loop

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemeoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemeoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkAllocateMemory() for vertexData_position.vkDeviceMemory failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkAllocateMemory() call success for vertexData_position.vkDeviceMemory\n\n", __func__);
    }

    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkBindBufferMemory() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkBindBufferMemory() call success\n\n", __func__);
    }

    void *data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemeoryAllocateInfo.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkMapMemory() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkMapMemory() call success\n\n", __func__);
    }

    // actual memeory mapped io
    memcpy(data, triangle_position, sizeof(triangle_position));

    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

     // ------- Vertex color buffer

    // memset our global vertexData_color struct.
    memset((void *)&vertexData_color, 0, sizeof(VertexData));

    //  declare and memset struct VkBufferCreateInfo
    memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // flags are used for scatterd / sparce buffer
    vkBufferCreateInfo.size = sizeof(triangle_color);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    // call vkCreateBuffer() vulkan api in the .vkBuffer member of our global struct.
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_color.vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateBuffer() failed for vertexData_color!!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateBuffer() call success for vertexData_color\n\n", __func__);
    }

    vkMemoryRequirements;
    memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, vertexData_color.vkBuffer, &vkMemoryRequirements);

    memset((void *)&vkMemeoryAllocateInfo, 0, sizeof(vkMemeoryAllocateInfo));

    vkMemeoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemeoryAllocateInfo.allocationSize = vkMemoryRequirements.size;

    vkMemeoryAllocateInfo.memoryTypeIndex = 0; // inital value before entring loop

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemeoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemeoryAllocateInfo, NULL, &vertexData_color.vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkAllocateMemory() for vertexData_color.vkDeviceMemory failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkAllocateMemory() call success for vertexData_color.vkDeviceMemory\n\n", __func__);
    }

    vkResult = vkBindBufferMemory(vkDevice, vertexData_color.vkBuffer, vertexData_color.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkBindBufferMemory() failed for vertexData_color !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkBindBufferMemory() call success for vertexData_color\n\n", __func__);
    }

    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_color.vkDeviceMemory, 0, vkMemeoryAllocateInfo.allocationSize, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkMapMemory() failed for vertexData_color!!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkMapMemory() call success for vertexData_color\n\n", __func__);
    }

    // actual memeory mapped io
    memcpy(data, triangle_color, sizeof(triangle_color));

    vkUnmapMemory(vkDevice, vertexData_color.vkDeviceMemory);

    fprintf(gpFile, "\n======================== CREATE VERTEX BUFFER END ================================\n\n");
    return(vkResult);
}

VkResult createUniformBuffer(void)
{
    // function declarations
    VkResult updateUniformBuffer(void);
    // code
    VkResult vkResult = VK_SUCCESS;

    // memset our global vertexData_position struct.
    memset((void *)&uniformData, 0, sizeof(UniformData));

    //  declare and memset struct VkBufferCreateInfo
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0; // flags are used for scatterd / sparce buffer
    vkBufferCreateInfo.size = sizeof(MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    // call vkCreateBuffer() vulkan api in the .vkBuffer member of our global struct.
    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData.vkBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateBuffer() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateBuffer() call success\n\n", __func__);
    }

    VkMemoryRequirements vkMemoryRequirements;
    memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

    vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, &vkMemoryRequirements);


    VkMemoryAllocateInfo vkMemeoryAllocateInfo;
    memset((void *)&vkMemeoryAllocateInfo, 0, sizeof(vkMemeoryAllocateInfo));

    vkMemeoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemeoryAllocateInfo.allocationSize = vkMemoryRequirements.size;

    vkMemeoryAllocateInfo.memoryTypeIndex = 0; // inital value before entring loop

    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemeoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }

        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemeoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkAllocateMemory() for uniformData.vkDeviceMemory failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkAllocateMemory() call success for uniformData.vkDeviceMemory\n\n", __func__);
    }

    vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, uniformData.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkBindBufferMemory() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkBindBufferMemory() call success\n\n", __func__);
    }

    // call update uniform buffer
    vkResult = updateUniformBuffer();
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> updateUniformBuffer() failed !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> updateUniformBuffer() call success\n\n", __func__);
    }

    return(vkResult);
}

VkResult updateUniformBuffer(void)
{
    // local variable declarations
    VkResult vkResult = VK_SUCCESS;
    MyUniformData myUniformData;

    // code
    memset((void *)&myUniformData, 0, sizeof(MyUniformData));

    // update matricies
    myUniformData.modelMatrix = glm::mat4(1.0f);
    myUniformData.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    myUniformData.viewMatrix  = glm::mat4(1.0f);

    glm::mat4 perspectivePojectionMatrix = glm::mat4(1.0f);
    
    perspectivePojectionMatrix = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);
    perspectivePojectionMatrix[1][1] = perspectivePojectionMatrix[1][1] * (-1.0f);
    myUniformData.projectionMatrix = perspectivePojectionMatrix;

    // map uniform buffer
    void *data = NULL;
    vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
    
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkMapMemory() failed !!!\n\n", __func__);
        return (vkResult);
    }

    // copy the data to mapped buffer
    memcpy(data, &myUniformData, sizeof(MyUniformData));

    vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);
    return(vkResult);
}

VkResult createShaders(void)
{
    // local variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE SHADERS START ================================\n\n");

    // for vertex shader
    const char *szFileName = "Shader.vert.spv";
    FILE *fp = NULL;
    size_t size;

    // first open the shader file
    fp = fopen(szFileName, "rb");

    if (fp == NULL)
    {
        fprintf(gpFile, "%s()-> failed to open %s file !!!\n\n", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> success to open %s file !!!\n\n", __func__, szFileName);
    }

    // set the file pointer at end of file
    fseek(fp, 0, SEEK_END);

    // find the byte size of shader file data
    size = ftell(fp);
    if (size == 0)
    {
        fprintf(gpFile, "%s()-> %s file size is 0 !!!\n\n", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }


    // reset the file pointer at the beginning of the file
    fseek(fp, 0, SEEK_SET);

    // allocate a character buffer of file size
    char *shaderData = (char *)malloc(sizeof(char) * size);

    // read shader file data into it
    size_t retVal = fread(shaderData, size, 1, fp);

    if (retVal != 1)
    {
        fprintf(gpFile, "%s()-> failed to read %s file !!!\n\n", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> success to read %s file !!!\n\n", __func__, szFileName);
    }

    // close the file
    fclose(fp);

    // declare and memset struct VkShaderModuleCreateInfo and specify above file size and buffer while initializing it.
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; // reserved must be 0
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

    // call vkCreateShaderModule() vulkan api, pass above struct pointer to it as parameter and obtain shader module object in global variable 
    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateShaderModule() failed for vertex shader !!! \n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateShaderModule() call success for vertex shader\n\n", __func__);
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }

    fprintf(gpFile, "%s()-> vertex shader module successfully created\n\n", __func__);
    //---------------------------------------------------------------------------------
    // for fragment shader

    szFileName = "Shader.frag.spv";
    fp = NULL;
    size = 0;

    // first open the shader file
    fp = fopen(szFileName, "rb");

    if (fp == NULL)
    {
        fprintf(gpFile, "%s()-> failed to open %s file !!!\n\n", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> success to open %s file !!!\n\n", __func__, szFileName);
    }

    // set the file pointer at end of file
    fseek(fp, 0, SEEK_END);

    // find the byte size of shader file data
    size = ftell(fp);
    if (size == 0)
    {
        fprintf(gpFile, "%s()-> %s file size is 0 !!!\n\n", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }


    // reset the file pointer at the beginning of the file
    fseek(fp, 0, SEEK_SET);

    // allocate a character buffer of file size
    shaderData = (char *)malloc(sizeof(char) * size);

    // read shader file data into it
    retVal = fread(shaderData, size, 1, fp);

    if (retVal != 1)
    {
        fprintf(gpFile, "%s()-> failed to read %s file !!!\n\n", __func__, szFileName);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return(vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> success to read %s file !!!\n\n", __func__, szFileName);
    }

    // close the file
    fclose(fp);

    // memset struct VkShaderModuleCreateInfo and specify above file size and buffer while initializing it.
    memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; // reserved must be 0
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

    // call vkCreateShaderModule() vulkan api, pass above struct pointer to it as parameter and obtain shader module object in global variable 
    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateShaderModule() failed for fragment shader !!!\n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateShaderModule() call success for fragment shader\n\n", __func__);
    }

    if (shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }

    fprintf(gpFile, "%s()-> fragment shader module successfully created\n\n", __func__);

    fprintf(gpFile, "\n======================== CREATE SHADERS END ================================\n\n");
    return(vkResult);
}

VkResult createDescriptorSetLayout(void)
{
    // local variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE DESRIPTOR SET LAYOUT START ================================\n\n");

    // initialize descriptor set binding
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
    
    memset((void *)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));

    vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding.binding = 0; // this 0 related with the shader binding point in vertex shader
    vkDescriptorSetLayoutBinding.descriptorCount = 1;
    vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;

    // declare memset and initialize struct VkDecriptorSetLayoutCreateInfo
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void *)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0; // reserved hence 0
    vkDescriptorSetLayoutCreateInfo.bindingCount = 1;
    vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding;

    vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateDescriptorSetLayout() failed !!! \n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateDescriptorSetLayout() call success \n\n", __func__);
    }

    fprintf(gpFile, "\n======================== CREATE DESRIPTOR SET LAYOUT END ================================\n\n");
    return(vkResult);
}

VkResult createPipelineLayout(void)
{
    // local variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE PIPELINE LAYOUT START ================================\n\n");

    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void *)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0; // reserved hence 0
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateDescriptorSetLayout() failed !!! \n\n", __func__);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateDescriptorSetLayout() call success \n\n", __func__);
    }

    fprintf(gpFile, "\n======================== CREATE PIPELINE LAYOUT END ================================\n\n");
    return(vkResult);
}

VkResult createDescriptorPool(void)
{
    VkResult vkResult = VK_SUCCESS;

    // code
    // before creating actual descriptor pool vulkan expects descriptor pool size
    VkDescriptorPoolSize vkDescriptorPoolSize;
    memset((void *)&vkDescriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));

    vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize.descriptorCount = 1;

    // create the pool
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void *)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));

    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = 1;
    vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;
    vkDescriptorPoolCreateInfo.maxSets = 1;

    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateDescriptorPool() failed (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateDescriptorPool() call success\n\n", __func__);
    }

    return(vkResult);
}

VkResult createDescriptorSet(void)
{
    // variable declaration
    VkResult vkResult = VK_SUCCESS;
    // code

    // initialize descriptor allocation info
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;

    memset((void *)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkAllocateDescriptorSets() failed (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkAllocateDescriptorSets() call success\n\n", __func__);
    }

    // describe whether we want buffer as uniform or image as uniform
    VkDescriptorBufferInfo vkDescriptorBufferInfo;

    memset((void *)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));

    vkDescriptorBufferInfo.buffer = uniformData.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);

    // now update the above descriptor set directly to the shader
    // two ways to update writing directly to shader or copy one shader to another shader, we will prefer directly wrinting to the shader this require initializetion of following structure
    VkWriteDescriptorSet vkWriteDescriptorSet;
    memset((void *)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));

    vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet.pNext = NULL;
    vkWriteDescriptorSet.dstSet = vkDescriptorSet;
    vkWriteDescriptorSet.dstArrayElement = 0;
    vkWriteDescriptorSet.descriptorCount = 1;
    vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet.pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet.pImageInfo = NULL;
    vkWriteDescriptorSet.pTexelBufferView = NULL;
    vkWriteDescriptorSet.dstBinding = 0;  // our uniform at binding index 0 in shader

    vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);

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

VkResult createPipeline(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE PIPELINE START ================================\n\n");

    // vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2];
    memset((void *)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

    // for position
    vkVertexInputBindingDescription_array[0].binding = 0; // corrospoing to layout = 0 in vertex shader
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // for color
    vkVertexInputBindingDescription_array[1].binding = 1; // corrospoing to layout = 1 in vertex shader
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2];
    memset((void *)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

    // for position
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;

    // for color
    vkVertexInputAttributeDescription_array[1].binding = 1;
    vkVertexInputAttributeDescription_array[1].location = 1;
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[1].offset = 0;

    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void *)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

    // input assembly state
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void *)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // rastrization state
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void *)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

    // color blend state
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void *)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void *)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

    // view port scisor state
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void *)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1;

    memset((void *)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0;
    vkViewport.y = 0;
    vkViewport.width = (float)vkExtent2D_swapchain.width;
    vkViewport.height = (float)vkExtent2D_swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;
    
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
    vkPipelineViewportStateCreateInfo.scissorCount = 1;

    memset((void *)&vkRect2D_scissor, 0, sizeof(VkRect2D));

    vkRect2D_scissor.offset.x = 0;
    vkRect2D_scissor.offset.y = 0;
    vkRect2D_scissor.extent.width = vkExtent2D_swapchain.width;
    vkRect2D_scissor.extent.height = vkExtent2D_swapchain.height;

    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;

    // depth Stencil state
    // as we don't have depth yet we can omit this step

    // dynamic state
    // we don't have dynamic state

    // multisampling state
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void *)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // get validation if not initialized

    // shader stage state
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    memset((void *)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

    // vertex shader
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main";
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;

    // fragment shader
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main";
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;

    // Tessellation state
    // we dont have Tessellation so we can omit this state

    // as pipeline are created from pipeline cache we will create pipeline cache object
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void *)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;

    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
    
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreatePipelineCache() failed (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreatePipelineCache() call success\n\n", __func__);
    }

    // create the actual graphics pipeline
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void *)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pMultisampleState = NULL;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0;
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;
    
    // create the pipeline
    vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateGraphicsPipelines() failed (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        if(vkPipelineCache)
        {
            vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
            vkPipelineCache = VK_NULL_HANDLE;
        }
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateGraphicsPipelines() call success\n\n", __func__);
    }

    if(vkPipelineCache)
    {
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
    }

    fprintf(gpFile, "\n======================== CREATE PIPELINE END ================================\n\n");
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
    vkFramebufferCreateInfo.layers = 1;

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
        else
        {
            fprintf(gpFile, "%s()-> vkCreateFramebuffer() success for i = %d\n\n", __func__, i);
        }
    }

    fprintf(gpFile, "\n======================== CREATE FRAMEBUFFERS END ================================\n\n");
    
    return(vkResult);
}

VkResult createSemaphores(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE SEMAPHORES START ================================\n\n");

    // declare, memset and initialize VkSemaphoreCreateInfo structure.
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void *)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;
    vkSemaphoreCreateInfo.flags = 0; // must be zero because it is reserved

    // 1st call to vkCreateSemaphore() for vkSemaphore_backBuffer
    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backBuffer);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateSemaphore() for vkSemaphore_backBuffer failed (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateSemaphore() call for vkSemaphore_backBuffer success\n\n", __func__);
    }

    // 2st call to vkCreateSemaphore() for vkSemaphore_backBuffer
    vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_renderComplete);

    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "%s()-> vkCreateSemaphore() failed for vkSemaphore_renderComplete (ERROR CODE : %d )!!!\n\n", __func__, vkResult);
        return (vkResult);
    }
    else
    {
        fprintf(gpFile, "%s()-> vkCreateSemaphore() call for vkSemaphore_renderComplete success\n\n", __func__);
    }

    fprintf(gpFile, "\n======================== CREATE SEMAPHORES END ================================\n\n");

    return(vkResult);
}

VkResult createFences(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== CREATE FENCES START ================================\n\n");

    //  declare, memset and initialze VkFenceCreateInfo structure.
    VkFenceCreateInfo vkFenceCreateInfo;
    memset((void *)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // allocate our global fence array to the size of swapchain image count using malloc().
    vkFence_array = (VkFence *)malloc(sizeof(VkFence) * swapchainImageCount);
    
    // Now in a loop call vkCreateFence() to initialize our global fences array
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
        
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "%s()-> vkCreateFence() failed for i = %d (ERROR CODE : %d)\n\n", __func__, i, vkResult);
            return(vkResult);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkCreateFence() success for i = %d\n\n", __func__, i);
        }
    }

    fprintf(gpFile, "\n======================== CREATE FENCES END ================================\n\n");

    return(vkResult);
}

VkResult buildCommandBuffers(void)
{
    // variable declarations
    VkResult vkResult = VK_SUCCESS;

    // code
    fprintf(gpFile, "\n======================== BUILD COMMAND BUFFERS START ================================\n\n");
    
    // loop per swapchainImageCount
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        // reset command buffers
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0); // 0 means dont release the resources created by commandPool by these command buffers

        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "%s()-> vkResetCommandBuffer() failed for i = %d (ERROR CODE : %d)\n\n", __func__, i, vkResult);
            return(vkResult);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkResetCommandBuffer() success for i = %d\n", __func__, i);
        }

        fprintf(gpFile,"\n\n");

        // 
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void *)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0; // indecates we are only PRIMARY COMMAND BUFFER or we are not going to use this command buffer between multiple threads.
        
        vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
        
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "%s()-> vkBeginCommandBuffer() failed for i = %d (ERROR CODE : %d)\n\n", __func__, i, vkResult);
            return(vkResult);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkBeginCommandBuffer() success for i = %d\n", __func__, i);
        }
        
        fprintf(gpFile,"\n\n");

        // set clear value
        VkClearValue vkClearValue_array[1];
        memset((void *)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

        vkClearValue_array[0].color = vkClearColorValue;

        // declare memset and initialize VkRenderPassBeginInfo structure.
        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void *)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[i];

        // begin the renderPass
        vkCmdBeginRenderPass(vkCommandBuffer_array[i],&vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // bind our descriptor set to pipeline
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, NULL);

        // bind with the pipeline
        vkCmdBindPipeline(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

        // bind with the vertex buffer
        // for position
        VkDeviceSize vkDeviceSize_offset_position[1];
        memset((void *)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));

        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_position.vkBuffer, vkDeviceSize_offset_position);

        // for color
        VkDeviceSize vkDeviceSize_offset_color[1];
        memset((void *)vkDeviceSize_offset_color, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_color));

        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 1, 1, &vertexData_color.vkBuffer, vkDeviceSize_offset_color);

        // here we should call vulkan drawing functions
        vkCmdDraw(vkCommandBuffer_array[i], 
            3, // no of vertices
            1, // no of instance
            0, // first vertex
            0 // first instace
        );

        // end render pass
        vkCmdEndRenderPass(vkCommandBuffer_array[i]);

        // end command buffer record
        vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "%s()-> vkEndCommandBuffer() failed for i = %d (ERROR CODE : %d)\n\n", __func__, i, vkResult);
            return(vkResult);
        }
        else
        {
            fprintf(gpFile, "%s()-> vkEndCommandBuffer() success for i = %d\n", __func__, i);
        }

        fprintf(gpFile,"\n\n");
    }
    
    fprintf(gpFile, "\n======================== BUILD COMMAND BUFFERS END ================================\n\n");
    return(vkResult);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT,
    VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
    uint64_t object,
    size_t location,
    int32_t messageCode,
    const char *pLayerPrefix, 
    const char *pMessage, 
    void *pUserData)
{
    // code
    fprintf(gpFile, "\n======================== DEBUG REPORT CALLBACK START ================================\n\n");

    fprintf(gpFile, "HGK_VALIDATION : %s()-> %s (%d) = %s\n\n", __func__, pLayerPrefix, messageCode, pMessage);

    fprintf(gpFile, "\n======================== DEBUG REPORT CALLBACK END ================================\n\n");

    return(VK_FALSE);
}

/**
-----------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------

 Notes:
 all struct, enum and types names = Vk...
 all apis starts from vk....

 semaphores are used for inter-queue synchronization operations
 fences are used for host to device synchronization

 // build commands for windows build.bat
 cls
 cl.exe /c /EHsc /I C:VulkanSDK\Valkan\include vk.c
 rc.exe vk.rc
 link.exe vk.obj vk.res /LIBPATH:C:VulkanSDK\valkan\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

 // commands for shader compilation SPIR-V (Standard for Portable Intermediate Representation for Vulkan)

 C:\VulkanSDK\Vulkan\Bin\glslangValidator.exe -V -H -o Shader.vert.spv Shader.vert

 -V : for vulkan
 -H : give human readable dump
 -o : for name of output name

 */
