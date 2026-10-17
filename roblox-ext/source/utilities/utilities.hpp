#ifndef utilities_hpp
#define utilities_hpp

#include <cstdint>
#include <string>
#include <vector>

#include <framework/memory/memory.hpp>
#include <utilities/structs.hpp>
#include <framework/offsets.hpp>

inline memory_t g_memory;

struct object_base_t
{
    std::uint64_t address = 0;

    constexpr object_base_t( ) noexcept = default;
    constexpr explicit object_base_t( std::uint64_t addr ) noexcept : address( addr )
    {
    }

    [[nodiscard]] bool valid( ) const noexcept
    {
        return address != 0;
    }

    [[nodiscard]] bool operator==( const object_base_t& other ) const noexcept
    {
        return address == other.address;
    }

    [[nodiscard]] bool operator!=( const object_base_t& other ) const noexcept
    {
        return !( *this == other );
    }

    template< typename T >
    [[nodiscard]] T get_member( std::uint64_t offset ) const noexcept
    {
        return g_memory.read< T >( address + offset );
    }
};

inline std::string read_string( std::uint64_t address )
{
    constexpr std::size_t max_length = 256;

    const auto sso = g_memory.read< sso_string_t >( address );
    if ( sso.length == 0 || sso.length > max_length )
        return "unknown";

    if ( sso.length < sizeof( sso.string.buffer ) )
        return std::string( sso.string.buffer, sso.length );

    std::string result( sso.length, '\0' );
    if ( !g_memory.read_buffer( sso.string.ptr, result.data( ), result.size( ) ) )
        return "unknown";

    return result.empty( ) ? "unknown" : result;
}

inline std::vector< std::uint64_t > read_shared_addresses( std::uint64_t container )
{
    std::vector< std::uint64_t > result;
    if ( !container )
        return result;

    const auto vec = g_memory.read< vector_t >( container );
    if ( vec.empty( ) )
        return result;

    constexpr std::size_t max_count = 4000;
    const std::size_t count = vec.byte_size( ) / sizeof( shared_pointer_t );
    if ( count == 0 || count > max_count )
        return result;

    std::vector< shared_pointer_t > buffer( count );
    if ( !g_memory.read_buffer( vec.first, buffer.data( ), buffer.size( ) * sizeof( shared_pointer_t ) ) )
        return result;

    result.reserve( count );
    for ( const auto& slot : buffer )
    {
        if ( !slot.object )
            break;
        result.push_back( slot.object );
    }

    return result;
}

#endif  // utilities_hpp
