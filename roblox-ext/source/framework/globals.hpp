#ifndef globals_hpp
#define globals_hpp

#include <cstdint>

#include <framework/math.hpp>
#include <framework/roblox/datamodel.hpp>

struct game_t
{
    std::uint64_t last_datamodel = 0;
    datamodel_t datamodel{};
    instance_t local_player{};
    instance_t players{};

    matrix4 view_matrix{};
    vector2 dimensions{};
    vector3 camera_position{};
};

struct entity_t;
struct visual_engine_t;

inline game_t game{};

#endif  // globals_hpp
