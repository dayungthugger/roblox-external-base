#ifndef overlay_hpp
#define overlay_hpp

#include <Windows.h>

#include <d3d11.h>
#include <dcomp.h>
#include <dxgi1_2.h>
#include <dwmapi.h>

#include <wrl/client.h>

struct overlay_t
{
    HWND hwnd = nullptr;
    int width = 0;
    int height = 0;

    bool vsync = true;
    bool menu_open = false;
    bool insert_was_down = false;

    Microsoft::WRL::ComPtr< ID3D11Device > device;
    Microsoft::WRL::ComPtr< ID3D11DeviceContext > context;
    Microsoft::WRL::ComPtr< IDXGISwapChain1 > swap_chain;
    Microsoft::WRL::ComPtr< ID3D11RenderTargetView > render_target;

    Microsoft::WRL::ComPtr< IDCompositionDevice > comp_device;
    Microsoft::WRL::ComPtr< IDCompositionTarget > comp_target;
    Microsoft::WRL::ComPtr< IDCompositionVisual > comp_visual;

    overlay_t( ) = default;

    overlay_t( const overlay_t& ) = delete;
    overlay_t& operator=( const overlay_t& ) = delete;

    [[nodiscard]] bool init( );
    [[nodiscard]] bool begin_frame( );
    void end_frame( );
    void shutdown( );

private:
    [[nodiscard]] bool create_window( );
    [[nodiscard]] bool create_device( );
    [[nodiscard]] bool create_swap_chain( );
    [[nodiscard]] bool create_dcomp( );
    [[nodiscard]] bool create_render_target( );
    void release_render_target( );

    [[nodiscard]] bool poll_messages( );
    void update_menu_toggle( );
    void render_menu( );
};

inline overlay_t g_overlay;

#endif  // overlay_hpp
