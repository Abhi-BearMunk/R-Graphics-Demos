// R-Graphics-Demos.cpp : Defines the entry point for the application.
//
#include "pch.h"
//#include "ECS/World.h"
#include "Utils/AssetImporter.h"
#include "Test/MoveSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/RenderableManager.h"
#include "Utils/Logger.h"
#include "Utils/SimpleTimer.h"

struct Health
{
    int hp;
    static const std::uint64_t uid = 1 << 4;
};
R::Test::MoveSystem* mv;
R::Rendering::RenderSystem* rs;
R::Rendering::RenderableManager* rm;

XMFLOAT3 UP{ 0, 1, 0 };
XMFLOAT4 RQuat, LQuat;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // TODO: Place code here.
    R::ECS::World world;
    R::Utils::Logger logger;
    world.RegisterArchetype<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity>();
    world.RegisterArchetype<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity, Health>();
    auto e1 = world.CreateEntity<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity, Health>({ 0.0f, 0.0f, 10.0f }, R::ECS::Rotation(0.f, 0.f, 0.f), {1.f, 1.f, 1.f}, { 2.f, 0.0f, 0.0f }, { 100 });
    auto e2 = world.CreateEntity<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity>({ -0.20f, 0.0f, 5.0f }, R::ECS::Rotation(-60.f, 0.f, 0.f), {1.f, 4.f, 7.f }, { -0.02f, 0.0f, 0.0f });
    auto e3 = world.CreateEntity<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity, Health>({ 2.10f, -0.10f, 10.0f }, R::ECS::Rotation(45.f, 0.f, 45.f), { 3.f, 2.f, 1.f }, { 0.0f, 0.0f, 0.0f }, { 50 });
    auto e4 = world.CreateEntity<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity, Health>({ 0.643f, -0.67f, 20.0f }, R::ECS::Rotation(0.f, -750.f, 0.f), { 0.1f, 0.1f, 0.1f }, { -0.03f, 0.0f, 0.0f }, { 75 });
    auto e5 = world.CreateEntity<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity>({ 0.1f, 0.10f, 40.0f }, R::ECS::Rotation(0.f, 0.f, 30.f), { 4.f, 4.f, 4.f }, { 0.10f, 0.0f, 0.0f });
    std::uint32_t count = 25000;
    for (std::uint32_t i = 0; i < count; i++)
    {
        R::ECS::Pos p;
        p.x = (float)(rand() % 100) - 50;
        p.y = (float)(rand() % 100) - 50;
        p.z = (float)(rand() % 100);
        R::ECS::Rotation r((float)(rand() % 360), (float)(rand() % 360), (float)(rand() % 360));
        R::ECS::Scale s;
        s.x = (float)(rand() % 10) / 5.f + 0.5f;
        s.y = (float)(rand() % 10) / 5.f + 0.5f;
        s.z = (float)(rand() % 10) / 5.f + 0.5f;
        R::Test::Velocity v;
        v.x = (float)(rand() % 10) / 20.0f - 0.25f;
        v.y = (float)(rand() % 10) / 20.0f - 0.25f;
        v.z = (float)(rand() % 10) / 20.0f - 0.25f;
        if (i < count / 2)
        {
            world.CreateEntity<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity, Health>(p, r, s, v, { 100 });
        }
        else
        {
            world.CreateEntity<R::ECS::Pos, R::ECS::Rotation, R::ECS::Scale, R::Test::Velocity>(p, r, s, v);
        }
    }

    //world.GetComponent<Health>(e3).hp -= 25;
    //int updatedHp = world.GetComponent<Health>(e3).hp;

    R::Job::JobSystem jobSys;

    R::Test::MoveSystem moveSys(world, jobSys);
    mv = &moveSys;

    R::Utils::AssetImporter assetImporter(jobSys);
    R::Rendering::TextureID testTex;
    R::Utils::TextureAssetDesc testDesc{ R::Utils::TextureAssetDesc::E_Albedo, L"D:\\Work\\Github\\R-Graphics-Demos\\R-Graphics-Demos\\R-Graphics-Demos\\Assets\\Textures\\space-crate1-albedo.png", 1, &testTex };
    R::Utils::TextureAssetDesc assets[] = { testDesc };
    assetImporter.ImportTextures(1, assets);

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

    R::Rendering::RenderSystem rendSys(1920u, 1080u, m_hwnd, jobSys, assetImporter.GetResourceData());
    rs = &rendSys;

    R::Rendering::RenderableManager rendMan(world, jobSys);
    rm = &rendMan;

    XMStoreFloat4(&RQuat, XMQuaternionRotationAxis(XMLoadFloat3(&UP), 2.f * XM_PI / 180.f));
    XMStoreFloat4(&LQuat, XMQuaternionRotationAxis(XMLoadFloat3(&UP), -2.f * XM_PI / 180.f));


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
        // TODO :: Very Hacky, but still very performant;
        XMVECTOR p = XMLoadFloat3(&rs->GetRenderContext()->GetCamera()->position);
        XMVECTOR f = XMLoadFloat3(&rs->GetRenderContext()->GetCamera()->forward);
        XMVECTOR r = XMLoadFloat3(&rs->GetRenderContext()->GetCamera()->right);

        switch (wParam)
        {
        case 0x57:  // W
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->position, XMVectorAdd(p, f));
            break;
        case 0x41:  // A
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->position, XMVectorSubtract(p, r));
            break;
        case 0x53:  // S
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->position, XMVectorSubtract(p, f));
            break;
        case 0x44:  // D
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->position, XMVectorAdd(p, r));
            break;
        case 0x51:  // Q
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->forward, XMVector3Rotate(f, XMLoadFloat4(&RQuat)));
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->right, XMVector3Rotate(r, XMLoadFloat4(&RQuat)));

            break;
        case 0x45:  // E
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->forward, XMVector3Rotate(f, XMLoadFloat4(&LQuat)));
            XMStoreFloat3(&rs->GetRenderContext()->GetCamera()->right, XMVector3Rotate(r, XMLoadFloat4(&LQuat)));

            break;
        default:
            break;
        }
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
                rm->Update(rs->GetRenderContext());
                rm->WaitForUpdate();
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

