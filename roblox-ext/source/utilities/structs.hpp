#ifndef structs_hpp
#define structs_hpp

#include <cstddef>
#include <cstdint>
#include <framework/math.hpp>
#include <framework/offsets.hpp>

struct vector_t
{
    std::uint64_t first = 0;
    std::uint64_t last = 0;
    std::uint64_t end = 0;

    [[nodiscard]] bool empty( ) const noexcept
    {
        return first == 0 || last == 0 || last <= first;
    }

    [[nodiscard]] std::size_t byte_size( ) const noexcept
    {
        return empty( ) ? 0 : static_cast< std::size_t >( last - first );
    }
};

struct shared_pointer_t
{
    std::uint64_t object = 0;
    std::uint64_t ref = 0;
};

struct sso_string_t
{
    union
    {
        char buffer[ 16 ];
        std::uint64_t ptr = 0;
    } string;

    std::size_t length = 0;
    std::size_t capacity = 0;
};

struct primitive_t
{
    matrix3 rotation;
    vector3 position;
    vector3 linear_velocity;
    vector3 angular_velocity;
    std::uint8_t pad_1[ Offsets::Primitive::Flags - ( Offsets::Primitive::AssemblyAngularVelocity + sizeof( vector3 ) ) ];
    std::uint8_t flags = 0;
    std::uint8_t pad_2[ Offsets::Primitive::Size - ( Offsets::Primitive::Flags + 1 ) ];
    vector3 size;
};

static_assert( offsetof( primitive_t, rotation ) == 0 );
static_assert( offsetof( primitive_t, flags ) == Offsets::Primitive::Flags - Offsets::Primitive::Rotation );
static_assert( offsetof( primitive_t, size ) == Offsets::Primitive::Size - Offsets::Primitive::Rotation );

#endif  // structs_hpp
