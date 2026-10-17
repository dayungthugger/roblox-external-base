#include <framework/roblox/task_scheduler.hpp>

#include <framework/globals.hpp>
#include <framework/memory/memory.hpp>

std::string job_t::get_name( ) const
{
    return read_string( address + Offsets::TaskScheduler::JobName );
}

bool task_scheduler_t::is_loaded( ) const
{
    const auto container = get_member< std::uint64_t >( Offsets::TaskScheduler::JobStart );
    if ( !container )
        return false;

    const auto vec = g_memory.read< vector_t >( container );
    if ( vec.empty( ) )
        return false;

    return g_memory.read< std::uint64_t >( vec.first ) != 0;
}

std::vector< job_t > task_scheduler_t::get_jobs( ) const
{
    std::vector< job_t > jobs;

    const auto container = get_member< std::uint64_t >( Offsets::TaskScheduler::JobStart );
    const auto addresses = read_shared_addresses( container );

    jobs.reserve( addresses.size( ) );
    for ( const std::uint64_t addr : addresses )
        jobs.emplace_back( addr );

    return jobs;
}

std::uint64_t task_scheduler_t::find_first_job( std::string_view name ) const
{
    for ( const auto& entry : get_jobs( ) )
    {
        if ( entry.get_name( ) == name )
            return entry.address;
    }

    return 0;
}
