// R-Graphics-Demos.cpp : Defines the entry point for the application.
//
#include "pch.h"
#include "framework.h"
#include "ECS/World.h"
#include "Test/MoveSystem.h"
#include "Utils/Logger.h"
#include "Utils/SimpleTimer.h"

struct Health
{
    int hp;
    static const uint64_t uid = 1 << 4;
};
R::Test::MoveSystem* mv;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // TODO: Place code here.
    R::ECS::World world;
    R::Utils::Logger logger;
    world.RegisterArchetype<R::ECS::Pos, R::Test::Velocity>();
    world.RegisterArchetype<R::ECS::Pos, R::Test::Velocity, Health>();
    auto e1 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>({ 1.5f, 3.7f, 0.0f }, { 2.0f, 0.0f, 0.0f }, { 100 });
    auto e2 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity>({ -2.0f, 0.0f, 0.0f }, { -2.0f, 0.0f, 0.0f });
    auto e3 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>({ 10.0f, -10.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 50 });
    auto e4 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>({ 2.43f, -6.67f, 0.0f }, { -3.0f, 0.0f, 0.0f }, { 75 });
    auto e5 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity>({ 1.0f, 1.0f, 0.0f }, { 10.0f, 0.0f, 0.0f });
    for (int i = 0; i < 1000; i++)
    {
        R::ECS::Pos p;
        p.x = (float)(rand() % 10);
        auto e1 = world.CreateEntity<R::ECS::Pos, R::Test::Velocity, Health>(p, { 2.f, 0.f, 0.f }, { 100 });
    }

    world.GetComponent<Health>(e3).hp -= 25;
    int updatedHp = world.GetComponent<Health>(e3).hp;

    R::Job::JobSystem jobSys;

    auto moveSys = R::Test::MoveSystem(world, jobSys);
    mv = &moveSys;

    auto pos1 = world.GetComponent<R::ECS::Pos>(e1);
    auto pos2 = world.GetComponent<R::ECS::Pos>(e2);
    auto pos3 = world.GetComponent<R::ECS::Pos>(e3);
    auto pos4 = world.GetComponent<R::ECS::Pos>(e4);
    auto pos5 = world.GetComponent<R::ECS::Pos>(e5);

    std::vector<R::ECS::Interest<2>> posInterest;
    world.InterestedIn<R::ECS::Pos, Health>(posInterest);

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
            R_TIMER(MoveTimer);
            mv->Update(0.16);
            mv->WaitForCompletion();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

