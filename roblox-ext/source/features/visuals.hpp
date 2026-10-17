#ifndef visuals_hpp
#define visuals_hpp

#include <cfloat>

#include <framework/globals.hpp>
#include <imgui_internal.h>

enum class snap_origin_t : int
{
    mouse = 0,
    bottom,
    top
};

enum class box_style_t : int
{
    full = 0,
    corner
};

struct esp_info_t
{
    ImRect box{ FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX };
    vector3 center{};
    vector3 root{};
    vector3 head{};
    bool has_root = false;
    bool has_head = false;
};

struct visuals_t
{
    bool self = false;
    bool names = true;
    bool boxes = true;
    bool health_bar = true;
    bool distance = true;
    bool snap_lines = false;

    snap_origin_t snap_origin = snap_origin_t::bottom;

    box_style_t box_style = box_style_t::full;

    float max_distance = 500.f;

    [[nodiscard]] bool any_enabled( ) const noexcept
    {
        return boxes || health_bar || names || distance || snap_lines;
    }

    [[nodiscard]] bool calculate_esp( const visual_engine_t& engine, const entity_t& entity, esp_info_t& out );
    void render( );

private:
    void draw_box( ImDrawList* draw, const ImRect& bb ) const;
    void draw_full_box( ImDrawList* draw, const ImRect& bb ) const;
    void draw_corner_box( ImDrawList* draw, const ImRect& bb ) const;
    void draw_health_bar( ImDrawList* draw, const ImRect& bb, const entity_t& entity ) const;
    void draw_name( ImDrawList* draw, const ImRect& bb, const entity_t& entity ) const;
    void draw_distance( ImDrawList* draw, const ImRect& bb, float distance_m ) const;
    void draw_snap_line( ImDrawList* draw, const ImRect& bb, const ImVec2& origin ) const;

    [[nodiscard]] ImVec2 resolve_snap_origin( ) const noexcept;
};

inline visuals_t visuals{};

#endif  // visuals_hpp
