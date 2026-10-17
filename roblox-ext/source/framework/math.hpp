#ifndef math_hpp
#define math_hpp

#include <cmath>

struct vector2
{
    float x = 0.f;
    float y = 0.f;

    constexpr vector2( ) noexcept = default;
    constexpr vector2( float x, float y ) noexcept : x( x ), y( y )
    {
    }

    [[nodiscard]] constexpr vector2 operator+( const vector2& other ) const noexcept
    {
        return { x + other.x, y + other.y };
    }

    [[nodiscard]] constexpr vector2 operator-( const vector2& other ) const noexcept
    {
        return { x - other.x, y - other.y };
    }

    [[nodiscard]] constexpr vector2 operator*( float scalar ) const noexcept
    {
        return { x * scalar, y * scalar };
    }

    [[nodiscard]] constexpr vector2 operator/( float scalar ) const noexcept
    {
        return { x / scalar, y / scalar };
    }

    vector2& operator+=( const vector2& other ) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    vector2& operator-=( const vector2& other ) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    vector2& operator*=( float scalar ) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    vector2& operator/=( float scalar ) noexcept
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==( const vector2& other ) const noexcept
    {
        return x == other.x && y == other.y;
    }

    [[nodiscard]] constexpr bool operator!=( const vector2& other ) const noexcept
    {
        return !( *this == other );
    }

    [[nodiscard]] constexpr float length_squared( ) const noexcept
    {
        return x * x + y * y;
    }

    [[nodiscard]] float length( ) const noexcept
    {
        return std::sqrt( length_squared( ) );
    }

    [[nodiscard]] vector2 normalized( ) const noexcept
    {
        const float len = length( );
        if ( len == 0.f )
            return {};
        return { x / len, y / len };
    }

    void normalize( ) noexcept
    {
        const float len = length( );
        if ( len == 0.f )
            return;
        x /= len;
        y /= len;
    }

    [[nodiscard]] constexpr float dot( const vector2& other ) const noexcept
    {
        return x * other.x + y * other.y;
    }

    [[nodiscard]] constexpr float distance_squared( const vector2& other ) const noexcept
    {
        const float dx = x - other.x;
        const float dy = y - other.y;
        return dx * dx + dy * dy;
    }

    [[nodiscard]] float distance( const vector2& other ) const noexcept
    {
        return std::sqrt( distance_squared( other ) );
    }
};

struct vector3
{
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    constexpr vector3( ) noexcept = default;
    constexpr vector3( float x, float y, float z ) noexcept : x( x ), y( y ), z( z )
    {
    }

    [[nodiscard]] constexpr vector3 operator+( const vector3& other ) const noexcept
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    [[nodiscard]] constexpr vector3 operator-( const vector3& other ) const noexcept
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    [[nodiscard]] constexpr vector3 operator*( float scalar ) const noexcept
    {
        return { x * scalar, y * scalar, z * scalar };
    }

    [[nodiscard]] constexpr vector3 operator/( float scalar ) const noexcept
    {
        return { x / scalar, y / scalar, z / scalar };
    }

    vector3& operator+=( const vector3& other ) noexcept
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    vector3& operator-=( const vector3& other ) noexcept
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    vector3& operator*=( float scalar ) noexcept
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    vector3& operator/=( float scalar ) noexcept
    {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==( const vector3& other ) const noexcept
    {
        return x == other.x && y == other.y && z == other.z;
    }

    [[nodiscard]] constexpr bool operator!=( const vector3& other ) const noexcept
    {
        return !( *this == other );
    }

    [[nodiscard]] constexpr float length_squared( ) const noexcept
    {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] float length( ) const noexcept
    {
        return std::sqrt( length_squared( ) );
    }

    [[nodiscard]] vector3 normalized( ) const noexcept
    {
        const float len = length( );
        if ( len == 0.f )
            return {};
        return { x / len, y / len, z / len };
    }

    void normalize( ) noexcept
    {
        const float len = length( );
        if ( len == 0.f )
            return;
        x /= len;
        y /= len;
        z /= len;
    }

    [[nodiscard]] constexpr float dot( const vector3& other ) const noexcept
    {
        return x * other.x + y * other.y + z * other.z;
    }

    [[nodiscard]] constexpr float distance_squared( const vector3& other ) const noexcept
    {
        const float dx = x - other.x;
        const float dy = y - other.y;
        const float dz = z - other.z;
        return dx * dx + dy * dy + dz * dz;
    }

    [[nodiscard]] float distance( const vector3& other ) const noexcept
    {
        return std::sqrt( distance_squared( other ) );
    }

    [[nodiscard]] float dist_to( const vector3& other ) const noexcept
    {
        return distance( other );
    }
};

struct matrix3
{
    float m[ 3 ][ 3 ]{};

    [[nodiscard]] constexpr vector3 transform( const vector3& v ) const noexcept
    {
        return {
            m[ 0 ][ 0 ] * v.x + m[ 0 ][ 1 ] * v.y + m[ 0 ][ 2 ] * v.z,
            m[ 1 ][ 0 ] * v.x + m[ 1 ][ 1 ] * v.y + m[ 1 ][ 2 ] * v.z,
            m[ 2 ][ 0 ] * v.x + m[ 2 ][ 1 ] * v.y + m[ 2 ][ 2 ] * v.z,
        };
    }
};

struct matrix4
{
    float m[ 4 ][ 4 ]{};
};

#endif  // math_hpp
