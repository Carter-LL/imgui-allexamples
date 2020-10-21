// Direct3D Learning Grounds.cpp : This file contains the 'main' function. Program execution begins and ends there.
//http://www.directxtutorial.com/Lesson.aspx?lessonid=9-4-1
//Basic Includes
#include <windows.h>
#include <windowsx.h>
#include <iostream>
#include <stdio.h>

//Basic Libraries
#pragma comment(lib, "windowscodecs.lib")

#pragma region OpenGL Setup

//OpenGL Options
bool useOpenGLGraphics = false;
bool useOpenGL2 = false;
bool useOpenGL3 = false;

//Includes OpenGL
#include <GLFW/glfw3.h>
#include "imgui_impl_glfw.h"
#include <gl3w.h>  

//OpenGL library file
#pragma comment(lib, "legacy_stdio_definitions.lib")

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

//Global declarations
GLFWwindow* window = NULL;
const char* glsl_version = NULL;

#pragma endregion

#pragma region DirectX Setup

//DirectX Options
bool useDirectXGraphics = true;
bool useDirectX9 = false;
bool useDirectX10 = false;
bool useDirectX11 = true;
bool useDirectX12 = false;

//Includes DirectX
#include <d3d9.h> //Contains various declarations
#include <d3d10_1.h> //Contains various declarations
#include <d3d11.h> //Contains various declarations
#include <d3d12.h> //Contains various declarations
#include <dxgi1_4.h>

//Input type
#define DIRECTINPUT_VERSION 0x0800

//Direct3D library file
#pragma comment (lib, "d3d9.lib") //Contains the libraries
#pragma comment (lib, "d3d10.lib") //Contains the libraries
#pragma comment (lib, "d3d11.lib") //Contains the libraries
#pragma comment (lib, "d3d12.lib") //Contains the libraries
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

//Global declarations
//DirectX9
LPDIRECT3D9 Direct3DInterface9; //Pointer to Direct3D interface *Long pointer*
LPDIRECT3DDEVICE9 Direct3DDevice9; //Pointer to device class *Long pointer to class which stores the information* Device Interface Holds information
D3DPRESENT_PARAMETERS Direct3Dpp9 = {};

//DirectX10
ID3D10Device* g_pd3dDevice10 = NULL;
IDXGISwapChain* g_pSwapChain10 = NULL;
ID3D10RenderTargetView* g_mainRenderTargetView10 = NULL;

//DirectX11
ID3D11Device* g_pd3dDevice11 = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext11 = NULL;
IDXGISwapChain* g_pSwapChain11 = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView11 = NULL;

//DirectX12
struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue;
};
static int const NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT g_frameIndex = 0;
static int const NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = NULL;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
static ID3D12Fence* g_fence = NULL;
static HANDLE g_fenceEvent = NULL;
static UINT64 g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = NULL;
static HANDLE g_hSwapChainWaitableObject = NULL;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

//Function prototyes
bool createDeviceD3D(HWND hWnd); //Setup and initialize Direct3D
void render_frame(void); //Render a single frame
void resetDevice(); //Reset Direct3D device
void cleanupDeviceD3D(); //Clean the device
void createRenderTarget(); //Render targets
void cleanupRenderTarget(); //Clean render targets
void waitForLastSubmittedFrame(); //Waits for frame
FrameContext* waitForNextFrameResources(); //Wait for frame resources
void resizeSwapChain(HWND hWnd, int width, int height); //Change swapchain size
void glfw_error_callback(int error, const char* description); //Error callback
void ImGuiContent(); //Main ImGui Content

#pragma endregion

#pragma region ImGui Setup

//Includes ImGui
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_dx10.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl2.h"
#include "imgui_impl_win32.h"
#include <dinput.h>
#include <tchar.h>

void initImGui(HWND hWnd);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma endregion

#pragma region Windows Setup

//WindowProc function prototype
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma endregion

#pragma region MyRegion

//Declarations
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

#pragma endregion

#pragma region Main

int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    std::cout << "Starting Direct3D Application\n";

    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
    ::RegisterClassEx(&wc);

    HWND hWnd = NULL;
    if (useDirectX9) {
        hWnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui Directx9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    }
    if (useDirectX10) {
        hWnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui Directx10 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    }
    if (useDirectX11) {
        hWnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui Directx11 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    }
    if (useDirectX12) {
        hWnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui Directx12 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1920, 1080, NULL, NULL, wc.hInstance, NULL);
    }
    if (useOpenGL2) {
        // Setup window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return 1;
        window = glfwCreateWindow(1920, 1080, "Dear ImGui GLFW+OpenGL2 example", NULL, NULL);
        if (window == NULL)
            return 1;
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync
    }
    if (useOpenGL3) {
        // Setup window
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return 1;

        // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
        glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
        glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
        window = glfwCreateWindow(1920, 1080, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
        if (window == NULL)
            return 1;
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
        bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
        bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
        bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
        bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
        bool err = false;
        glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
        bool err = false;
        glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
        bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
        if (err)
        {
            fprintf(stderr, "Failed to initialize OpenGL loader!\n");
            return 1;
        }
    }

    // Initialize Direct3D
    if (useDirectXGraphics) {
        if (!createDeviceD3D(hWnd))
        {
            cleanupDeviceD3D();
            ::UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }
    }

    // Show the window
    ::ShowWindow(hWnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hWnd);

    //Setup ImGui
    initImGui(hWnd);

    if (useDirectXGraphics) {
        //Enter main loop
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT)
        {

            if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
                continue;
            }

            //ImGui new frames
            if (useDirectX9) {
                ImGui_ImplDX9_NewFrame();
            }
            if (useDirectX10) {
                ImGui_ImplDX10_NewFrame();
            }
            if (useDirectX11) {
                ImGui_ImplDX11_NewFrame();
            }
            if (useDirectX12) {
                ImGui_ImplDX12_NewFrame();
            }

            ImGui_ImplWin32_NewFrame();

            ImGuiContent();

            //Device Modifiers
            if (useDirectX9) {
                Direct3DDevice9->SetRenderState(D3DRS_ZENABLE, FALSE);
                Direct3DDevice9->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                Direct3DDevice9->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
            }

            //Direct3D frame
            render_frame();

            //Begin new scene for ImGui
            if (useDirectX9) {
                if (Direct3DDevice9->BeginScene() >= 0)
                {
                    ImGui::Render();
                    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                    Direct3DDevice9->EndScene();
                }
                HRESULT result = Direct3DDevice9->Present(NULL, NULL, NULL, NULL);

                // Handle loss of D3D9 device
                if (result == D3DERR_DEVICELOST && Direct3DDevice9->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
                    resetDevice();
            }
            if (useDirectX10) {
                ImGui::Render();
                g_pd3dDevice10->OMSetRenderTargets(1, &g_mainRenderTargetView10, NULL);
                g_pd3dDevice10->ClearRenderTargetView(g_mainRenderTargetView10, (float*)&clear_color);
                ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());

                g_pSwapChain10->Present(1, 0); // Present with vsync
            }
            if (useDirectX11) {
                ImGui::Render();
                g_pd3dDeviceContext11->OMSetRenderTargets(1, &g_mainRenderTargetView11, NULL);
                g_pd3dDeviceContext11->ClearRenderTargetView(g_mainRenderTargetView11, (float*)&clear_color);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

                g_pSwapChain11->Present(1, 0); // Present with vsync
            }
            if (useDirectX12) {
                FrameContext* frameCtxt = NULL;
                frameCtxt = waitForNextFrameResources();
                UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
                frameCtxt->CommandAllocator->Reset();

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

                g_pd3dCommandList->Reset(frameCtxt->CommandAllocator, NULL);
                g_pd3dCommandList->ResourceBarrier(1, &barrier);
                g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], (float*)&clear_color, 0, NULL);
                g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
                g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
                ImGui::Render();
                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
                g_pd3dCommandList->ResourceBarrier(1, &barrier);
                g_pd3dCommandList->Close();

                g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

                g_pSwapChain->Present(1, 0); // Present with vsync
                //g_pSwapChain->Present(0, 0); // Present without vsync

                UINT64 fenceValue = g_fenceLastSignaledValue + 1;
                g_pd3dCommandQueue->Signal(g_fence, fenceValue);
                g_fenceLastSignaledValue = fenceValue;
                frameCtxt->FenceValue = fenceValue;
            }
        }
    }

    if (useOpenGLGraphics) {
        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            glfwPollEvents();

            // Start the Dear ImGui frame
            if (useOpenGL2) {
                ImGui_ImplOpenGL2_NewFrame();
            }
            if (useOpenGL3) {
                ImGui_ImplOpenGL3_NewFrame();
            }
            ImGui_ImplGlfw_NewFrame();

            ImGuiContent();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            if (useOpenGL3) {
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
            // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using imgui_impl_opengl3.cpp!!),
            // you may need to backup/reset/restore current shader using the commented lines below.
            //GLint last_program;
            //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
            //glUseProgram(0);
            if (useOpenGL2) {
                ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            }
            //glUseProgram(last_program);
            if (useOpenGL2) {
                glfwMakeContextCurrent(window);
            }
            glfwSwapBuffers(window);
        }
    }

    //Shutdown ImGui
    if (useDirectX9) {
        ImGui_ImplDX9_Shutdown();
    }
    if (useDirectX10) {
        ImGui_ImplDX10_Shutdown();
    }
    if (useDirectX11) {
        ImGui_ImplDX11_Shutdown();
    }
    if (useDirectX12) {
        waitForLastSubmittedFrame();
        ImGui_ImplDX12_Shutdown();
    }
    if (useDirectXGraphics) {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        //Clean up DirectX and COM
        cleanupDeviceD3D();
        ::DestroyWindow(hWnd);
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    }
    if (useOpenGLGraphics) {
        if (useOpenGL2) {
            ImGui_ImplOpenGL2_Shutdown();
        }
        if (useOpenGL3) {
            ImGui_ImplOpenGL3_Shutdown();
        }
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    return 0;
}
#pragma endregion

#pragma region Window

// this is the main message handler for the program
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (useDirectX9) {
            if (Direct3DDevice9 != NULL && wParam != SIZE_MINIMIZED)
            {
                Direct3Dpp9.BackBufferWidth = LOWORD(lParam);
                Direct3Dpp9.BackBufferHeight = HIWORD(lParam);
                resetDevice();
            }
        }
        if (useDirectX10) {
            if (g_pd3dDevice10 != NULL && wParam != SIZE_MINIMIZED)
            {
                cleanupRenderTarget();
                g_pSwapChain10->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                createRenderTarget();
            }
        }
        if (useDirectX11) {
            if (g_pd3dDevice11 != NULL && wParam != SIZE_MINIMIZED)
            {
                cleanupRenderTarget();
                g_pSwapChain11->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                createRenderTarget();
            }
        }
        if (useDirectX12) {
            if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
            {
                waitForLastSubmittedFrame();
                ImGui_ImplDX12_InvalidateDeviceObjects();
                cleanupRenderTarget();
                resizeSwapChain(hWnd, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
                createRenderTarget();
                ImGui_ImplDX12_CreateDeviceObjects();
            }
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

#pragma endregion

#pragma region ImGui

void ImGuiContent() {
    ImGui::NewFrame();
    //Inner content begins here

    //Showing demo wind

    ImGui::Text("Hello, world!");

    //End frame
    ImGui::EndFrame();
}

void initImGui(HWND hWnd) {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    if (useDirectXGraphics) {
        ImGui_ImplWin32_Init(hWnd);
    }
    if (useOpenGLGraphics) {
        ImGui_ImplGlfw_InitForOpenGL(window, true);
    }

    if (useDirectX9) {
        ImGui_ImplDX9_Init(Direct3DDevice9);
    }
    if (useDirectX10) {
        ImGui_ImplDX10_Init(g_pd3dDevice10);
    }
    if (useDirectX11) {
        ImGui_ImplDX11_Init(g_pd3dDevice11, g_pd3dDeviceContext11);
    }
    if (useDirectX12) {
        ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
            DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
            g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());
    }
    if (useOpenGL2) {
        ImGui_ImplOpenGL2_Init();
    }
    if (useOpenGL3) {
        ImGui_ImplOpenGL3_Init(glsl_version);
    }
}

#pragma endregion

#pragma region OpenGL

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#pragma endregion

#pragma region Direct3D

//Setup Direct3D for use, Create interface & graphics device
bool createDeviceD3D(HWND hWnd) {
    if (useDirectX9) {
        if ((Direct3DInterface9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
            return false;

        // Create the D3DDevice
        ZeroMemory(&Direct3Dpp9, sizeof(Direct3Dpp9));
        Direct3Dpp9.Windowed = TRUE;
        Direct3Dpp9.SwapEffect = D3DSWAPEFFECT_DISCARD;
        Direct3Dpp9.BackBufferFormat = D3DFMT_UNKNOWN;
        Direct3Dpp9.EnableAutoDepthStencil = TRUE;
        Direct3Dpp9.AutoDepthStencilFormat = D3DFMT_D16;
        Direct3Dpp9.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
        //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
        if (Direct3DInterface9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Direct3Dpp9, &Direct3DDevice9) < 0)
            return false;

        return true;
    }
    if (useDirectX10) {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
        if (D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, D3D10_SDK_VERSION, &sd, &g_pSwapChain10, &g_pd3dDevice10) != S_OK)
            return false;

        createRenderTarget();
        return true;
    }
    if (useDirectX11) {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT createDeviceFlags = 0;
        //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
        if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain11, &g_pd3dDevice11, &featureLevel, &g_pd3dDeviceContext11) != S_OK)
            return false;

        createRenderTarget();
        return true;
    }
    if (useDirectX12) {
        // Setup swap chain
        DXGI_SWAP_CHAIN_DESC1 sd;
        {
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = NUM_BACK_BUFFERS;
            sd.Width = 0;
            sd.Height = 0;
            sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            sd.Scaling = DXGI_SCALING_STRETCH;
            sd.Stereo = FALSE;
        }

        // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
        ID3D12Debug* pdx12Debug = NULL;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
            pdx12Debug->EnableDebugLayer();
#endif

        // Create device
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        if (D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
            return false;

        // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
        if (pdx12Debug != NULL)
        {
            ID3D12InfoQueue* pInfoQueue = NULL;
            g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            pInfoQueue->Release();
            pdx12Debug->Release();
        }
#endif

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            desc.NumDescriptors = NUM_BACK_BUFFERS;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            desc.NodeMask = 1;
            if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
                return false;

            SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            {
                g_mainRenderTargetDescriptor[i] = rtvHandle;
                rtvHandle.ptr += rtvDescriptorSize;
            }
        }

        {
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 1;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
                return false;
        }

        {
            D3D12_COMMAND_QUEUE_DESC desc = {};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            desc.NodeMask = 1;
            if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
                return false;
        }

        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
            if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
                return false;

        if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
            g_pd3dCommandList->Close() != S_OK)
            return false;

        if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
            return false;

        g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (g_fenceEvent == NULL)
            return false;

        {
            IDXGIFactory4* dxgiFactory = NULL;
            IDXGISwapChain1* swapChain1 = NULL;
            if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK ||
                dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1) != S_OK ||
                swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
                return false;
            swapChain1->Release();
            dxgiFactory->Release();
            g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
            g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
        }

        createRenderTarget();
        return true;
    }
}
//Change swapchain size
void resizeSwapChain(HWND hWnd, int width, int height)
{
    if (useDirectX12) {
        DXGI_SWAP_CHAIN_DESC1 sd;
        g_pSwapChain->GetDesc1(&sd);
        sd.Width = width;
        sd.Height = height;

        IDXGIFactory4* dxgiFactory = NULL;
        g_pSwapChain->GetParent(IID_PPV_ARGS(&dxgiFactory));

        g_pSwapChain->Release();
        CloseHandle(g_hSwapChainWaitableObject);

        IDXGISwapChain1* swapChain1 = NULL;
        dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1);
        swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain));
        swapChain1->Release();
        dxgiFactory->Release();

        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);

        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
        assert(g_hSwapChainWaitableObject != NULL);
    }
}

//Render new targets
void createRenderTarget()
{
    if (useDirectX10) {
        ID3D10Texture2D* pBackBuffer;
        g_pSwapChain10->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice10->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView10);
        pBackBuffer->Release();
    }
    if (useDirectX11) {
        ID3D11Texture2D* pBackBuffer;
        g_pSwapChain11->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice11->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView11);
        pBackBuffer->Release();
    }
    if (useDirectX12) {
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            ID3D12Resource* pBackBuffer = NULL;
            g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, g_mainRenderTargetDescriptor[i]);
            g_mainRenderTargetResource[i] = pBackBuffer;
        }
    }
}
//Wait for frame resources
FrameContext* waitForNextFrameResources()
{
    if (useDirectX12) {
        UINT nextFrameIndex = g_frameIndex + 1;
        g_frameIndex = nextFrameIndex;

        HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, NULL };
        DWORD numWaitableObjects = 1;

        FrameContext* frameCtxt = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
        UINT64 fenceValue = frameCtxt->FenceValue;
        if (fenceValue != 0) // means no fence was signaled
        {
            frameCtxt->FenceValue = 0;
            g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
            waitableObjects[1] = g_fenceEvent;
            numWaitableObjects = 2;
        }

        WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

        return frameCtxt;
    }
}

//Wait for frame
void waitForLastSubmittedFrame() {
    if (useDirectX12) {
        FrameContext* frameCtxt = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

        UINT64 fenceValue = frameCtxt->FenceValue;
        if (fenceValue == 0)
            return; // No fence was signaled

        frameCtxt->FenceValue = 0;
        if (g_fence->GetCompletedValue() >= fenceValue)
            return;

        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        WaitForSingleObject(g_fenceEvent, INFINITE);
    }
        
}

//Clean render targets
void cleanupRenderTarget()
{
    if (useDirectX10) {
        if (g_mainRenderTargetView10) { g_mainRenderTargetView10->Release(); g_mainRenderTargetView10 = NULL; }
    }
    if (useDirectX11) {
        if (g_mainRenderTargetView11) { g_mainRenderTargetView11->Release(); g_mainRenderTargetView11 = NULL; }
    }
    if (useDirectX12) {
        waitForLastSubmittedFrame();

        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = NULL; }
    }
}

//Render single frames
void render_frame(void) {
    if (useDirectX9) {
        //Clear the window to deep blue
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(ImVec4(0.45f, 0.55f, 0.60f, 1.00f).x * 255.0f), (int)(ImVec4(0.45f, 0.55f, 0.60f, 1.00f).y * 255.0f), (int)(ImVec4(0.45f, 0.55f, 0.60f, 1.00f).z * 255.0f), (int)(ImVec4(0.45f, 0.55f, 0.60f, 1.00f).w * 255.0f));
        Direct3DDevice9->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

        //Direct3DDevice->BeginScene(); //Starts the 3D scene

        //3D rendering on back buffer begin here

        //Direct3DDevice->EndScene(); //Ends 3D scene

        //Direct3DDevice->Present(NULL, NULL, NULL, NULL); //Display the created frame
    }
}

//Clean device
void cleanupDeviceD3D()
{
    if (useDirectX9) {
        if (Direct3DDevice9) { Direct3DDevice9->Release(); Direct3DDevice9 = NULL; }
        if (Direct3DInterface9) { Direct3DInterface9->Release(); Direct3DInterface9 = NULL; }
    }
    if (useDirectX10) {
        cleanupRenderTarget();
        if (g_pSwapChain10) { g_pSwapChain10->Release(); g_pSwapChain10 = NULL; }
        if (g_pd3dDevice10) { g_pd3dDevice10->Release(); g_pd3dDevice10 = NULL; }
    }
    if (useDirectX11) {
        cleanupRenderTarget();
        if (g_pSwapChain11) { g_pSwapChain11->Release(); g_pSwapChain11 = NULL; }
        if (g_pd3dDeviceContext11) { g_pd3dDeviceContext11->Release(); g_pd3dDeviceContext11 = NULL; }
        if (g_pd3dDevice11) { g_pd3dDevice11->Release(); g_pd3dDevice11 = NULL; }
    }
    if (useDirectX12) {
        cleanupRenderTarget();
        if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
        if (g_hSwapChainWaitableObject != NULL) { CloseHandle(g_hSwapChainWaitableObject); }
        for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
            if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = NULL; }
        if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = NULL; }
        if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = NULL; }
        if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = NULL; }
        if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = NULL; }
        if (g_fence) { g_fence->Release(); g_fence = NULL; }
        if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = NULL; }
        if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }

#ifdef DX12_ENABLE_DEBUG_LAYER
        IDXGIDebug1* pDebug = NULL;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
            pDebug->Release();
        }
#endif
    }
}

//Reset Direct3D device
void resetDevice()
{
    if (useDirectX9) {
        ImGui_ImplDX9_InvalidateDeviceObjects();
        HRESULT hr = Direct3DDevice9->Reset(&Direct3Dpp9);
        if (hr == D3DERR_INVALIDCALL)
            IM_ASSERT(0);
        ImGui_ImplDX9_CreateDeviceObjects();
    }
}

#pragma endregion
// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
