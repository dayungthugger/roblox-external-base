#include "datamodel.hpp"

#include <atomic>
#include <chrono>
#include <thread>

#include <framework/globals.hpp>

namespace
{
    inline std::atomic_bool g_datamodel_running{ false };
    constexpr const char* roblox_process = "RobloxPlayerBeta.exe";
}  // namespace

std::uint64_t datamodel_t::get_place_id( ) const
{
    return get_member< std::uint64_t >( Offsets::DataModel::PlaceId );
}

std::uint64_t datamodel_t::get_creator_id( ) const
{
    return get_member< std::uint64_t >( Offsets::DataModel::CreatorId );
}

std::uint64_t datamodel_t::get_job_id( ) const
{
    return get_member< std::uint64_t >( Offsets::DataModel::JobId );
}

std::uint64_t datamodel_t::get_game_id( ) const
{
    return get_member< std::uint64_t >( Offsets::DataModel::GameId );
}

bool datamodel_t::is_in_game( ) const
{
    return get_name( ) == "Ugc";
}

bool datamodel_t::is_game_loaded( ) const
{
    return get_member< bool >( Offsets::DataModel::GameLoaded );
}

datamodel_t datamodel_t::get( )
{
    const auto fake = g_memory.read< std::uint64_t >( g_memory.base + Offsets::FakeDataModel::Pointer );
    if ( !fake )
        return datamodel_t{};

    const auto real = g_memory.read< std::uint64_t >( fake + Offsets::FakeDataModel::RealDataModel );
    return datamodel_t( real );
}

void datamodel_t::tick( )
{
    if ( g_datamodel_running.exchange( true ) )
        return;

    std::thread(
        []( )
        {
            while ( g_datamodel_running.load( ) )
            {
                const auto pid = get_pid_by_name( roblox_process );
                if ( !pid )
                {
                    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
                    continue;
                }

                if ( g_memory.pid != pid )
                {
                    g_memory.close( );
                    g_memory.pid = pid;
                    g_memory.handle = OpenProcess( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, static_cast< DWORD >( pid ) );
                    g_memory.base = get_image_base_from_pid( pid );
                }

                if ( !g_memory.valid( ) )
                {
                    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
                    continue;
                }

                const auto current = datamodel_t::get( );
                if ( current.valid( ) && ( !game.players.valid( ) || current.address != game.last_datamodel ) )
                {
                    game.datamodel = current;

                    const auto players = current.find_first_child_of_class( "Players" );
                    if ( players.valid( ) )
                    {
                        game.local_player = instance_t( g_memory.read< std::uint64_t >( players.address + Offsets::Player::LocalPlayer ) );
                        game.players = players;
                        game.last_datamodel = current.address;
                    }
                }

                std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
            }
        } )
        .detach( );
}

void datamodel_t::stop( )
{
    g_datamodel_running.store( false );
}
