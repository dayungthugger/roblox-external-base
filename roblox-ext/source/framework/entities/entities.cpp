#include "entities.hpp"

#include <chrono>
#include <thread>

namespace
{
    inline std::vector< entity_t > g_entities;
    inline std::shared_mutex g_mutex;
    inline std::atomic_bool g_running{ false };

    [[nodiscard]] bool is_valid_part( const instance_t& part )
    {
        const std::string class_name = part.get_class_name( );
        return class_name == "Part" || class_name == "MeshPart";
    }
}  // namespace

void cache::update( )
{
    const std::vector< instance_t > players = game.players.get_children( );

    std::vector< entity_t > fresh;
    fresh.reserve( players.size( ) );

    for ( const auto& player : players )
    {
        const auto model_addr = g_memory.read< std::uint64_t >( player.address + Offsets::Player::ModelInstance );
        if ( !model_addr )
            continue;

        const instance_t model( model_addr );
        const std::vector< instance_t > parts = model.get_children( );

        entity_t entity{};
        entity.bones.reserve( parts.size( ) );

        for ( const auto& part : parts )
        {
            if ( !is_valid_part( part ) )
                continue;

            bone_t bone{};
            bone.key = part.get_name( );
            bone.primitive = g_memory.read< std::uint64_t >( part.address + Offsets::BasePart::Primitive );
            if ( !bone.primitive )
                continue;

            entity.bones.emplace_back( std::move( bone ) );
        }

        entity.humanoid = model.find_first_child_of_class( "Humanoid" ).address;
        entity.name = player.get_name( );
        entity.self = ( player.address == game.local_player.address );

        fresh.emplace_back( std::move( entity ) );
    }

    {
        std::unique_lock lock( g_mutex );
        g_entities = std::move( fresh );
    }
}

void cache::tick( )
{
    if ( g_running.exchange( true ) )
        return;

    std::thread(
        []( )
        {
            while ( g_running.load( ) )
            {
                update( );
                std::this_thread::sleep_for( std::chrono::milliseconds( 1500 ) );
            }
        } )
        .detach( );
}

void cache::stop( )
{
    g_running.store( false );
}

std::vector< entity_t > cache::get_snapshot( )
{
    std::shared_lock lock( g_mutex );
    return g_entities;
}
