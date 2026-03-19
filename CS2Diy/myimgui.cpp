#include "myimgui.h"
#include <Windows.h>



//参数1:类名 参数2:标题名 参数3:回调 参数4:字体路径 参数5:字体大小 参数6:垂直同步 参数7:菜单样式 0为黑色,1为白色
int myimgui::CreateWindow_Violet (LPCSTR 类名, LPCSTR title_name, myimgui::myFun fun,const char* Fontsname, float Fonts_size, bool Synclnterval, int Menustyle)
{
    gamewindow.ClassName = 类名;
    gamewindow.TitleName = title_name;

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Violet", nullptr };
    ::RegisterClassExW(&wc);
    mywindow.hwnd = ::CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_POPUP, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!myimgui::CreateDeviceD3D(mywindow.hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(mywindow.hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(mywindow.hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;     

    //菜单样式
    if (Menustyle == 0) { ImGui::StyleColorsDark(); }
    if (Menustyle == 1) { ImGui::StyleColorsLight(); }
    
    ImGui_ImplWin32_Init(mywindow.hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImFont* font = io.Fonts->AddFontFromFileTTF(Fontsname,Fonts_size, nullptr, io.Fonts->GetGlyphRangesChineseFull());//C:\\Windows\\Fonts\\msyh.ttc


    ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 0.f);
    bool done = false;


    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            exit(0);
        //更新窗口
        UpdateWindow();
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        fun(); 
        
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        if (Synclnterval)
        {
            g_pSwapChain->Present(1, 0);
        }
        else
        {
            g_pSwapChain->Present(0, 0);
        }
    }
    


    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(mywindow.hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

//更新窗口
void myimgui::UpdateWindow()
{
    POINT Point{};
    RECT Rect{};

    //查找目标窗口
    gamewindow.hwnd = FindWindowA(gamewindow.ClassName, gamewindow.TitleName);

    //获取目标窗口位置
    GetClientRect(gamewindow.hwnd, &Rect);
    ClientToScreen(gamewindow.hwnd, &Point);

    //更新透明窗口位置和大小
    mywindow.pos = gamewindow.pos = ImVec2((float)Point.x, (float)Point.y);
    mywindow.size = gamewindow.size = ImVec2((float)Rect.right, (float)Rect.bottom);
    SetWindowPos(mywindow.hwnd, HWND_TOPMOST, (int)mywindow.pos.x, (int)mywindow.pos.y, (int)mywindow.size.x, (int)mywindow.size.y, SWP_SHOWWINDOW);

    //获取鼠标位置 同步到imgui中
    POINT MousePos;
    GetCursorPos(&MousePos);
    ScreenToClient(mywindow.hwnd, &MousePos);
    ImGui::GetIO().MousePos.x = (float)MousePos.x;
    ImGui::GetIO().MousePos.y = (float)MousePos.y;

    //鼠标穿透
    if (ImGui::GetIO().WantCaptureMouse)
    {
        //如果鼠标在imgui菜单区域中   那就不把他设置成分层窗口
        //printf("WantCaptureMouse=true\n");        //0xfff7ffff 会剩下当前的属性 但是会筛掉WS_EX_LAYERED属性
        SetWindowLong(mywindow.hwnd, GWL_EXSTYLE, GetWindowLong(mywindow.hwnd, GWL_EXSTYLE) & (~WS_EX_LAYERED));

    }
    else
    {
        //如果不鼠标在imgui菜单区域中  那就把他设置成分层窗口
        //printf("WantCaptureMouse=false\n");
        SetWindowLong(mywindow.hwnd, GWL_EXSTYLE, GetWindowLong(mywindow.hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    }
}

//创建D3D设备
bool myimgui::CreateDeviceD3D(HWND hWnd)
{
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

//清理D3D设备
void myimgui::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

//创建目标渲染视图
void myimgui::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

//清理目标渲染视图
void myimgui::CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

//Win32窗口消息处理函数
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI myimgui::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_CREATE:
    {
        MARGINS Margin = { -1 };
        DwmExtendFrameIntoClientArea(hWnd, &Margin);
        break;
    }
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
