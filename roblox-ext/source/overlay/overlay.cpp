#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <features/visuals.hpp>
#include <framework/globals.hpp>
#include <overlay/overlay.hpp>

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "dcomp.lib" )
#pragma comment( lib, "dwmapi.lib" )

namespace
{
    constexpr const char* window_class = "overlay_cls";
    constexpr const char* window_title = "overlay";
}  // namespace

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

static LRESULT CALLBACK wnd_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    if ( ImGui_ImplWin32_WndProcHandler( hwnd, msg, wparam, lparam ) )
        return true;

    if ( msg == WM_DESTROY )
    {
        PostQuitMessage( 0 );
        return 0;
    }
    return DefWindowProcA( hwnd, msg, wparam, lparam );
}

bool overlay_t::create_window( )
{
    const HINSTANCE instance = GetModuleHandleA( nullptr );

    WNDCLASSEXA wc{};
    wc.cbSize = sizeof( wc );
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = instance;
    wc.lpszClassName = window_class;

    if ( !RegisterClassExA( &wc ) )
        return false;

    width = GetSystemMetrics( SM_CXSCREEN );
    height = GetSystemMetrics( SM_CYSCREEN );
    if ( width < 1 || height < 1 )
        return false;

    hwnd = CreateWindowExA(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        window_class,
        window_title,
        WS_POPUP,
        0,
        0,
        width,
        height,
        nullptr,
        nullptr,
        instance,
        nullptr );

    if ( !hwnd )
        return false;

    SetLayeredWindowAttributes( hwnd, 0, 255, LWA_ALPHA );

    MARGINS margins{ -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea( hwnd, &margins );

    ShowWindow( hwnd, SW_SHOW );
    UpdateWindow( hwnd );

    return true;
}

bool overlay_t::create_device( )
{
    constexpr D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };

    D3D_FEATURE_LEVEL obtained = D3D_FEATURE_LEVEL_11_0;
    const HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        levels,
        ARRAYSIZE( levels ),
        D3D11_SDK_VERSION,
        device.GetAddressOf( ),
        &obtained,
        context.GetAddressOf( ) );

    return SUCCEEDED( hr ) && device && context;
}

bool overlay_t::create_swap_chain( )
{
    Microsoft::WRL::ComPtr< IDXGIDevice > dxgi_device;
    if ( FAILED( device.As( &dxgi_device ) ) )
        return false;

    Microsoft::WRL::ComPtr< IDXGIAdapter > adapter;
    if ( FAILED( dxgi_device->GetAdapter( adapter.GetAddressOf( ) ) ) )
        return false;

    Microsoft::WRL::ComPtr< IDXGIFactory2 > factory;
    if ( FAILED( adapter->GetParent( __uuidof( IDXGIFactory2 ), reinterpret_cast< void** >( factory.GetAddressOf( ) ) ) ) )
        return false;

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = static_cast< UINT >( width );
    desc.Height = static_cast< UINT >( height );
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

    return SUCCEEDED( factory->CreateSwapChainForComposition( device.Get( ), &desc, nullptr, swap_chain.GetAddressOf( ) ) );
}

bool overlay_t::create_dcomp( )
{
    Microsoft::WRL::ComPtr< IDXGIDevice > dxgi_device;
    if ( FAILED( device.As( &dxgi_device ) ) )
        return false;

    if ( FAILED( DCompositionCreateDevice( dxgi_device.Get( ), __uuidof( IDCompositionDevice ), reinterpret_cast< void** >( comp_device.GetAddressOf( ) ) ) ) )
        return false;

    if ( FAILED( comp_device->CreateTargetForHwnd( hwnd, TRUE, comp_target.GetAddressOf( ) ) ) )
        return false;

    if ( FAILED( comp_device->CreateVisual( comp_visual.GetAddressOf( ) ) ) )
        return false;

    if ( FAILED( comp_visual->SetContent( swap_chain.Get( ) ) ) )
        return false;

    if ( FAILED( comp_target->SetRoot( comp_visual.Get( ) ) ) )
        return false;

    return SUCCEEDED( comp_device->Commit( ) );
}

bool overlay_t::create_render_target( )
{
    Microsoft::WRL::ComPtr< ID3D11Texture2D > back_buffer;
    if ( FAILED( swap_chain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( back_buffer.GetAddressOf( ) ) ) ) )
        return false;

    return SUCCEEDED( device->CreateRenderTargetView( back_buffer.Get( ), nullptr, render_target.GetAddressOf( ) ) );
}

void overlay_t::release_render_target( )
{
    render_target.Reset( );
}

bool overlay_t::poll_messages( )
{
    MSG msg{};
    while ( PeekMessageA( &msg, nullptr, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &msg );
        DispatchMessageA( &msg );

        if ( msg.message == WM_QUIT )
            return false;
    }
    return true;
}

void overlay_t::update_menu_toggle( )
{
    const bool insert_down = ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) != 0;
    if ( insert_down && !insert_was_down )
    {
        menu_open = !menu_open;

        LONG_PTR style = GetWindowLongPtrA( hwnd, GWL_EXSTYLE );
        if ( menu_open )
        {
            style &= ~( WS_EX_TRANSPARENT | WS_EX_NOACTIVATE );
            SetWindowLongPtrA( hwnd, GWL_EXSTYLE, style );
            SetForegroundWindow( hwnd );
            SetFocus( hwnd );
        }
        else
        {
            style |= WS_EX_TRANSPARENT | WS_EX_NOACTIVATE;
            SetWindowLongPtrA( hwnd, GWL_EXSTYLE, style );
        }
    }
    insert_was_down = insert_down;
}

void overlay_t::render_menu( )
{
    ImGui::SetNextWindowSize( ImVec2( 300.f, 300.f ), ImGuiCond_FirstUseEver );
    ImGui::Begin( "menu", nullptr, ImGuiWindowFlags_NoCollapse );

    ImGui::Checkbox( "VSync", &vsync );
    ImGui::Separator( );

    ImGui::Checkbox( "self", &visuals.self );
    ImGui::Checkbox( "boxes", &visuals.boxes );

    if ( visuals.boxes )
    {
        constexpr const char* styles[] = { "full", "corner" };
        int style = static_cast< int >( visuals.box_style );
        if ( ImGui::Combo( "box style", &style, styles, IM_ARRAYSIZE( styles ) ) )
            visuals.box_style = static_cast< box_style_t >( style );
    }

    ImGui::Checkbox( "health bar", &visuals.health_bar );
    ImGui::Checkbox( "names", &visuals.names );
    ImGui::Checkbox( "distance", &visuals.distance );

    if ( visuals.distance )
        ImGui::SliderFloat( "max dist", &visuals.max_distance, 10.f, 2000.f, "%.0fm" );

    ImGui::Checkbox( "snap lines", &visuals.snap_lines );

    if ( visuals.snap_lines )
    {
        constexpr const char* origins[] = { "mouse", "bottom", "top" };
        int origin = static_cast< int >( visuals.snap_origin );
        if ( ImGui::Combo( "origin", &origin, origins, IM_ARRAYSIZE( origins ) ) )
            visuals.snap_origin = static_cast< snap_origin_t >( origin );
    }

    ImGui::End( );
}

bool overlay_t::init( )
{
    if ( !create_window( ) || !create_device( ) || !create_swap_chain( ) || !create_dcomp( ) || !create_render_target( ) )
    {
        shutdown( );
        return false;
    }

    IMGUI_CHECKVERSION( );
    ImGui::CreateContext( );

    ImGuiIO& io = ImGui::GetIO( );
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark( );

    ImGui_ImplWin32_Init( hwnd );
    ImGui_ImplDX11_Init( device.Get( ), context.Get( ) );

    return true;
}

bool overlay_t::begin_frame( )
{
    if ( !poll_messages( ) )
        return false;

    update_menu_toggle( );

    ImGui_ImplDX11_NewFrame( );
    ImGui_ImplWin32_NewFrame( );
    ImGui::NewFrame( );

    visuals.render( );

    if ( menu_open )
        render_menu( );

    ImGui::Render( );

    if ( !context || !render_target )
        return true;

    constexpr float clear[ 4 ] = { 0.f, 0.f, 0.f, 0.f };
    ID3D11RenderTargetView* rtv = render_target.Get( );
    context->ClearRenderTargetView( rtv, clear );
    context->OMSetRenderTargets( 1, &rtv, nullptr );

    D3D11_VIEWPORT vp{};
    vp.Width = static_cast< float >( width );
    vp.Height = static_cast< float >( height );
    vp.MaxDepth = 1.f;
    context->RSSetViewports( 1, &vp );

    ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

    return true;
}

void overlay_t::end_frame( )
{
    if ( swap_chain )
        swap_chain->Present( vsync ? 1 : 0, 0 );
}

void overlay_t::shutdown( )
{
    if ( ImGui::GetCurrentContext( ) )
    {
        ImGui_ImplDX11_Shutdown( );
        ImGui_ImplWin32_Shutdown( );
        ImGui::DestroyContext( );
    }

    release_render_target( );

    comp_visual.Reset( );
    comp_target.Reset( );
    comp_device.Reset( );
    swap_chain.Reset( );
    context.Reset( );
    device.Reset( );

    if ( hwnd )
    {
        DestroyWindow( hwnd );
        hwnd = nullptr;
    }

    UnregisterClassA( window_class, GetModuleHandleA( nullptr ) );

    width = 0;
    height = 0;
}
