#include "visuals.hpp"

#include <imgui.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <framework/entities/entities.hpp>
#include <framework/roblox/visual_engine.hpp>

#undef max
#undef min

namespace
{
    constexpr ImU32 box_outline = IM_COL32( 0, 0, 0, 255 );
    constexpr ImU32 box_fill = IM_COL32( 255, 255, 255, 255 );
    constexpr ImU32 health_fill = IM_COL32( 255, 255, 255, 255 );
    constexpr ImU32 text_fill = IM_COL32( 255, 255, 255, 255 );
    constexpr ImU32 text_outline = IM_COL32( 0, 0, 0, 255 );
    constexpr ImU32 snap_fill = IM_COL32( 255, 255, 255, 255 );
    constexpr ImU32 snap_outline = IM_COL32( 0, 0, 0, 255 );

    constexpr float studs_to_meters = 0.28f;
    constexpr float near_cull_studs = 2.5f;

    void draw_outlined_text( ImDrawList* draw, const char* text, const ImVec2& pos )
    {
        draw->AddText( ImVec2( pos.x + 1.f, pos.y ), text_outline, text );
        draw->AddText( ImVec2( pos.x - 1.f, pos.y ), text_outline, text );
        draw->AddText( ImVec2( pos.x, pos.y + 1.f ), text_outline, text );
        draw->AddText( ImVec2( pos.x, pos.y - 1.f ), text_outline, text );
        draw->AddText( pos, text_fill, text );
    }

    void draw_centered_text( ImDrawList* draw, const char* text, float center_x, float y )
    {
        const ImVec2 size = ImGui::CalcTextSize( text );
        draw_outlined_text( draw, text, ImVec2( center_x - size.x * 0.5f, y ) );
    }

    [[nodiscard]] bool resolve_camera_position( vector3& out )
    {
        if ( !game.datamodel.valid( ) )
            return false;

        const auto workspace = game.datamodel.get_member< std::uint64_t >( Offsets::DataModel::Workspace );
        if ( !workspace )
            return false;

        const auto camera = g_memory.read< std::uint64_t >( workspace + Offsets::Workspace::CurrentCamera );
        if ( !camera )
            return false;

        out = g_memory.read< vector3 >( camera + Offsets::Camera::Position );
        return true;
    }

    [[nodiscard]] bool resolve_local_position( const std::vector< entity_t >& snapshot, vector3& out )
    {
        for ( const auto& entity : snapshot )
        {
            if ( !entity.self )
                continue;

            for ( const auto& bone : entity.bones )
            {
                if ( bone.key == "HumanoidRootPart" && bone.primitive )
                {
                    out = g_memory.read< vector3 >( bone.primitive + Offsets::Primitive::Position );
                    return true;
                }
            }

            for ( const auto& bone : entity.bones )
            {
                if ( bone.key == "Head" && bone.primitive )
                {
                    out = g_memory.read< vector3 >( bone.primitive + Offsets::Primitive::Position );
                    return true;
                }
            }

            for ( const auto& bone : entity.bones )
            {
                if ( bone.primitive )
                {
                    out = g_memory.read< vector3 >( bone.primitive + Offsets::Primitive::Position );
                    return true;
                }
            }
        }

        return false;
    }
}  // namespace

bool visuals_t::calculate_esp( const visual_engine_t& engine, const entity_t& entity, esp_info_t& out )
{
    bool any = false;
    vector3 sum{};
    int prim_count = 0;

    out = esp_info_t{};

    for ( const auto& bone : entity.bones )
    {
        if ( !bone.primitive )
            continue;

        const auto prim = g_memory.read< primitive_t >( bone.primitive + Offsets::Primitive::Rotation );
        const vector3 half = prim.size / 2.f;

        sum += prim.position;
        ++prim_count;

        if ( bone.key == "HumanoidRootPart" )
        {
            out.root = prim.position;
            out.has_root = true;
        }
        else if ( bone.key == "Head" )
        {
            out.head = prim.position;
            out.has_head = true;
        }

        vector2 corners[ 8 ];
        int count = 0;
        bool valid = true;

        for ( int x = -1; x <= 1 && valid; x += 2 )
        {
            for ( int y = -1; y <= 1 && valid; y += 2 )
            {
                for ( int z = -1; z <= 1 && valid; z += 2 )
                {
                    const vector3 local{ half.x * x, half.y * y, half.z * z };
                    const vector3 world = prim.position + prim.rotation.transform( local );

                    if ( !engine.world_to_screen( world, corners[ count ] ) )
                    {
                        valid = false;
                        break;
                    }
                    ++count;
                }
            }
        }

        if ( !valid )
            continue;

        any = true;

        for ( int i = 0; i < count; ++i )
        {
            const auto& corner = corners[ i ];
            out.box.Min.x = std::min( out.box.Min.x, corner.x );
            out.box.Min.y = std::min( out.box.Min.y, corner.y );
            out.box.Max.x = std::max( out.box.Max.x, corner.x );
            out.box.Max.y = std::max( out.box.Max.y, corner.y );
        }
    }

    if ( prim_count > 0 )
        out.center = sum / static_cast< float >( prim_count );

    return any && prim_count > 0 && out.box.Max.x > out.box.Min.x && out.box.Max.y > out.box.Min.y;
}

void visuals_t::draw_box( ImDrawList* draw, const ImRect& bb ) const
{
    if ( box_style == box_style_t::corner )
        draw_corner_box( draw, bb );
    else
        draw_full_box( draw, bb );
}

void visuals_t::draw_full_box( ImDrawList* draw, const ImRect& bb ) const
{
    draw->AddRect( bb.Min, bb.Max, box_outline, 0.f, 0, 3.f );
    draw->AddRect( bb.Min, bb.Max, box_fill, 0.f, 0, 1.f );
}

void visuals_t::draw_corner_box( ImDrawList* draw, const ImRect& bb ) const
{
    const float w = bb.Max.x - bb.Min.x;
    const float h = bb.Max.y - bb.Min.y;
    if ( w <= 0.f || h <= 0.f )
        return;

    const float len_x = std::clamp( w * 0.25f, 4.f, 32.f );
    const float len_y = std::clamp( h * 0.25f, 4.f, 32.f );

    const ImVec2 tl{ bb.Min.x, bb.Min.y };
    const ImVec2 tr{ bb.Max.x, bb.Min.y };
    const ImVec2 bl{ bb.Min.x, bb.Max.y };
    const ImVec2 br{ bb.Max.x, bb.Max.y };

    for ( int pass = 0; pass < 2; ++pass )
    {
        const ImU32 col = pass == 0 ? box_outline : box_fill;
        const float thickness = pass == 0 ? 3.f : 1.f;

        draw->AddLine( tl, ImVec2( tl.x + len_x, tl.y ), col, thickness );
        draw->AddLine( tl, ImVec2( tl.x, tl.y + len_y ), col, thickness );

        draw->AddLine( tr, ImVec2( tr.x - len_x, tr.y ), col, thickness );
        draw->AddLine( tr, ImVec2( tr.x, tr.y + len_y ), col, thickness );

        draw->AddLine( bl, ImVec2( bl.x + len_x, bl.y ), col, thickness );
        draw->AddLine( bl, ImVec2( bl.x, bl.y - len_y ), col, thickness );

        draw->AddLine( br, ImVec2( br.x - len_x, br.y ), col, thickness );
        draw->AddLine( br, ImVec2( br.x, br.y - len_y ), col, thickness );
    }
}

void visuals_t::draw_health_bar( ImDrawList* draw, const ImRect& bb, const entity_t& entity ) const
{
    if ( !entity.humanoid )
        return;

    const float health = g_memory.read< float >( entity.humanoid + Offsets::Humanoid::Health );
    const float max_health = g_memory.read< float >( entity.humanoid + Offsets::Humanoid::MaxHealth );
    if ( max_health <= 0.f )
        return;

    const float ratio = std::clamp( health / max_health, 0.f, 1.f );
    const float filled = std::floorf( ( bb.Max.y - bb.Min.y ) * ratio );
    if ( filled < 1.f )
        return;

    draw->AddRect( ImVec2( bb.Min.x - 5.f, bb.Min.y - 1.f ), ImVec2( bb.Min.x - 2.f, bb.Max.y + 1.f ), box_outline );
    draw->AddRectFilled( ImVec2( bb.Min.x - 4.f, bb.Max.y - filled ), ImVec2( bb.Min.x - 3.f, bb.Max.y ), health_fill );
}

void visuals_t::draw_name( ImDrawList* draw, const ImRect& bb, const entity_t& entity ) const
{
    if ( entity.name.empty( ) )
        return;

    const float center_x = ( bb.Min.x + bb.Max.x ) * 0.5f;
    const ImVec2 size = ImGui::CalcTextSize( entity.name.c_str( ) );
    draw_centered_text( draw, entity.name.c_str( ), center_x, bb.Min.y - size.y - 2.f );
}

void visuals_t::draw_distance( ImDrawList* draw, const ImRect& bb, float distance_m ) const
{
    char buffer[ 32 ];
    if ( distance_m < 10.f )
        std::snprintf( buffer, sizeof( buffer ), "%.1fm", distance_m );
    else
        std::snprintf( buffer, sizeof( buffer ), "%.0fm", distance_m );

    const float center_x = ( bb.Min.x + bb.Max.x ) * 0.5f;
    draw_centered_text( draw, buffer, center_x, bb.Max.y + 2.f );
}

void visuals_t::draw_snap_line( ImDrawList* draw, const ImRect& bb, const ImVec2& origin ) const
{
    const ImVec2 dest{ ( bb.Min.x + bb.Max.x ) * 0.5f, bb.Max.y };
    draw->AddLine( origin, dest, snap_outline, 3.f );
    draw->AddLine( origin, dest, snap_fill, 1.f );
}

ImVec2 visuals_t::resolve_snap_origin( ) const noexcept
{
    switch ( snap_origin )
    {
        case snap_origin_t::mouse:
            return ImGui::GetMousePos( );
        case snap_origin_t::bottom:
            return ImVec2( game.dimensions.x * 0.5f, game.dimensions.y );
        case snap_origin_t::top:
        default:
            return ImVec2( game.dimensions.x * 0.5f, 0.f );
    }
}

void visuals_t::render( )
{
    if ( !any_enabled( ) )
        return;

    const auto engine = visual_engine_t::get( );
    if ( !engine.valid( ) )
        return;

    game.dimensions = engine.get_member< vector2 >( Offsets::VisualEngine::Dimensions );
    game.view_matrix = engine.get_member< matrix4 >( Offsets::VisualEngine::ViewMatrix );

    if ( game.dimensions.x < 1.f || game.dimensions.y < 1.f )
        return;

    const std::vector< entity_t > snapshot = cache::get_snapshot( );
    if ( snapshot.empty( ) )
        return;

    const bool want_distance = distance;
    const bool want_snap = snap_lines;

    vector3 camera_pos{};
    const bool has_camera = resolve_camera_position( camera_pos );
    if ( has_camera )
        game.camera_position = camera_pos;

    vector3 local_pos{};
    const bool has_local = !want_distance || resolve_local_position( snapshot, local_pos );

    const ImVec2 snap_from = want_snap ? resolve_snap_origin( ) : ImVec2( 0.f, 0.f );

    ImDrawList* draw = ImGui::GetBackgroundDrawList( );
    if ( !draw )
        return;

    for ( const auto& entity : snapshot )
    {
        if ( !self && entity.self )
            continue;

        esp_info_t info{};
        if ( !calculate_esp( engine, entity, info ) )
            continue;

        info.box.Min.x = std::floorf( info.box.Min.x );
        info.box.Min.y = std::floorf( info.box.Min.y );
        info.box.Max.x = std::floorf( info.box.Max.x );
        info.box.Max.y = std::floorf( info.box.Max.y );

        if ( has_camera && info.center.dist_to( camera_pos ) < near_cull_studs )
            continue;

        float meters_dist = 0.f;
        bool has_dist = false;

        if ( want_distance && has_local )
        {
            float studs = info.has_root ? info.root.dist_to( local_pos ) : info.center.dist_to( local_pos );
            if ( studs < 0.1f && info.has_head )
                studs = info.head.dist_to( local_pos );

            meters_dist = studs * studs_to_meters;

            if ( meters_dist > max_distance )
                continue;

            has_dist = true;
        }

        if ( boxes )
            draw_box( draw, info.box );

        if ( health_bar )
            draw_health_bar( draw, info.box, entity );

        if ( names )
            draw_name( draw, info.box, entity );

        if ( has_dist )
            draw_distance( draw, info.box, meters_dist );

        if ( want_snap )
            draw_snap_line( draw, info.box, snap_from );
    }
}
