#ifndef datamodel_hpp
#define datamodel_hpp

#include <framework/roblox/instance.hpp>

struct datamodel_t : instance_t
{
    using instance_t::instance_t;

    [[nodiscard]] std::uint64_t get_place_id( ) const;
    [[nodiscard]] std::uint64_t get_creator_id( ) const;
    [[nodiscard]] std::uint64_t get_job_id( ) const;
    [[nodiscard]] std::uint64_t get_game_id( ) const;

    [[nodiscard]] bool is_in_game( ) const;
    [[nodiscard]] bool is_game_loaded( ) const;

    [[nodiscard]] static datamodel_t get( );

    static void tick( );
    static void stop( );
};

#endif  // datamodel_hpp
