#include "visual_engine.hpp"

#include <framework/globals.hpp>

visual_engine_t visual_engine_t::get( )
{
    return visual_engine_t( g_memory.read< std::uint64_t >( g_memory.base + Offsets::VisualEngine::Pointer ) );
}

bool visual_engine_t::world_to_screen( const vector3& pos, vector2& out ) const
{
    const matrix4& view = game.view_matrix;
    const vector2& dims = game.dimensions;

    const float w = pos.x * view.m[ 3 ][ 0 ] + pos.y * view.m[ 3 ][ 1 ] + pos.z * view.m[ 3 ][ 2 ] + view.m[ 3 ][ 3 ];
    if ( w < 0.1f )
        return false;

    const float x = pos.x * view.m[ 0 ][ 0 ] + pos.y * view.m[ 0 ][ 1 ] + pos.z * view.m[ 0 ][ 2 ] + view.m[ 0 ][ 3 ];
    const float y = pos.x * view.m[ 1 ][ 0 ] + pos.y * view.m[ 1 ][ 1 ] + pos.z * view.m[ 1 ][ 2 ] + view.m[ 1 ][ 3 ];

    const float half_x = dims.x * 0.5f;
    const float half_y = dims.y * 0.5f;

    out.x = half_x + ( x / w ) * half_x;
    out.y = half_y - ( y / w ) * half_y;

    return true;
}
