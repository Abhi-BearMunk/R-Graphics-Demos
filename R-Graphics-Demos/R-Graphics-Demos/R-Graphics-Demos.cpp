// R-Graphics-Demos.cpp : Defines the entry point for the application.
//
#include "pch.h"
#include "framework.h"
#include "ECS/World.h"
#include "Test/MoveSystem.h"
#include "Rendering/RenderSystem.h"
#include "Utils/Logger.h"
#include "Utils/SimpleTimer.h"

struct Health
{
    int hp;
    static const uint64_t uid = 1 << 4;
};
R::Test::MoveSystem* mv;
R::Rendering::RenderSystem* rs;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // TODO: Place code here.
    R::ECS::World world;
    R::Utils::Logger logger;
    world.RegisterArchetype<R::ECS::Pos, R::Test::Velocity>();
    world.RegisterArchetype<R::ECS::Pos, R::Test::Velocity, Health>();
    auto e1 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>({ 0.75f, 0.37f, 0.0f }, { 0.02f, 0.0f, 0.0f }, { 100 });
    auto e2 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity>({ -0.20f, 0.0f, 0.0f }, { -0.02f, 0.0f, 0.0f });
    auto e3 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>({ 2.10f, -0.10f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 50 });
    auto e4 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>({ 0.643f, -0.67f, 0.0f }, { -0.03f, 0.0f, 0.0f }, { 75 });
    auto e5 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity>({ 0.1f, 0.10f, 0.0f }, { 0.10f, 0.0f, 0.0f });
    uint32_t count = 40000;
    for (uint32_t i = 0; i < count; i++)
    {
        R::ECS::Pos p;
        p.x = (float)(rand() % 100) / 50.0f - 1.f;
        p.y = (float)(rand() % 100) / 50.0f - 1.f;
        p.z = (float)(rand() % 100) / 50.0f - 1.f;
        R::Test::Velocity v;
        v.x = (float)(rand() % 100) / 500.0f - 0.1f;
        v.y = (float)(rand() % 100) / 500.0f - 0.1f;
        v.z = (float)(rand() % 100) / 500.0f - 0.1f;
        if (i < count / 2)
        {
            world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>(p, v, { 100 });
        }
        else
        {
            world.CreateEntity<R::ECS::Pos, R::Test::Velocity>(p, v);
        }
    }

    //world.GetComponent<Health>(e3).hp -= 25;
    //int updatedHp = world.GetComponent<Health>(e3).hp;

    R::Job::JobSystem jobSys;

    auto moveSys = R::Test::MoveSystem(world, jobSys);
    mv = &moveSys;

    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = L"DXSampleClass";
    RegisterClassEx(&windowClass);

    RECT windowRect = { 0, 0, 1920, 1080 };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // Create the window and store a handle to it.
    HWND m_hwnd = CreateWindow(
        windowClass.lpszClassName,
        L"Reality Demo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr,        // We have no parent window.
        nullptr,        // We aren't using menus.
        hInstance,
        //pSample);
        nullptr);

    // Initialize the sample. OnInit is defined in each child-implementation of DXSample.
    //pSample->OnInit();

    ShowWindow(m_hwnd, nCmdShow);

    auto rendSys = R::Rendering::RenderSystem(1920, 1080, m_hwnd, world, jobSys);
    rs = &rendSys;

    // Main sample loop.
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    //pSample->OnDestroy();

    // Return this part of the WM_QUIT message to Windows.
    return static_cast<char>(msg.wParam);
}

// Main message handler for the sample.
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //DXSample* pSample = reinterpret_cast<DXSample*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message)
    {
    case WM_CREATE:
    {
        // Save the DXSample* passed in to CreateWindow.
        //LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        //SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
    }
    return 0;

    case WM_KEYDOWN:
        /*if (pSample)
        {
            pSample->OnKeyDown(static_cast<UINT8>(wParam));
        }*/
        return 0;

    case WM_KEYUP:
        /*if (pSample)
        {
            pSample->OnKeyUp(static_cast<UINT8>(wParam));
        }*/
        return 0;

    case WM_PAINT:
        /*if (pSample)
        {
            pSample->OnUpdate();
            pSample->OnRender();
        }*/
        {
            R_DEBUG_TIMER(TotalUpdateTimer);
            {
                R_DEBUG_TIMER(MoveTimer);
                mv->Update(0.16f);
                mv->WaitForCompletion();
            }
            {
                R_DEBUG_TIMER(RenderTimer);
                rs->Render();
                rs->Update(0.16f);
                rs->WaitForUpdate();
            }
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

