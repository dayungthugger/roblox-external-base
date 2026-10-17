#ifndef visual_engine_hpp
#define visual_engine_hpp

#include <cstdint>

#include <framework/math.hpp>
#include <utilities/utilities.hpp>

struct visual_engine_t : object_base_t
{
    using object_base_t::object_base_t;

    [[nodiscard]] static visual_engine_t get( );
    [[nodiscard]] bool world_to_screen( const vector3& pos, vector2& out ) const;
};

#endif  // visual_engine_hpp
