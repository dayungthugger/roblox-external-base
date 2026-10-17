#ifndef memory_hpp
#define memory_hpp

#include <Windows.h>

#include <cstddef>
#include <cstdint>
#include <string_view>

#include <TlHelp32.h>

extern "C"
{
    long nt_read_virtual_mem( HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T );
    long nt_write_virtual_mem( HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T );
    long nt_alloc_virtual_mem( HANDLE, PVOID, PSIZE_T, ULONG, ULONG );
}

[[nodiscard]] std::int32_t get_pid_by_name( std::string_view process_name ) noexcept;
[[nodiscard]] std::uint64_t get_image_base_from_pid( std::int32_t pid ) noexcept;

struct memory_t
{
    HANDLE handle = nullptr;
    std::uint64_t base = 0;
    std::int32_t pid = 0;

    memory_t( ) noexcept = default;

    explicit memory_t( std::int32_t pid ) noexcept : pid( pid )
    {
        handle = OpenProcess( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_QUERY_INFORMATION, FALSE, static_cast< DWORD >( pid ) );
        base = get_image_base_from_pid( pid );
    }

    memory_t( const memory_t& ) = delete;
    memory_t& operator=( const memory_t& ) = delete;

    memory_t( memory_t&& other ) noexcept : handle( other.handle ), base( other.base ), pid( other.pid )
    {
        other.handle = nullptr;
        other.base = 0;
        other.pid = 0;
    }

    memory_t& operator=( memory_t&& other ) noexcept
    {
        if ( this != &other )
        {
            close( );
            handle = other.handle;
            base = other.base;
            pid = other.pid;
            other.handle = nullptr;
            other.base = 0;
            other.pid = 0;
        }
        return *this;
    }

    ~memory_t( )
    {
        close( );
    }

    void close( ) noexcept
    {
        if ( handle && handle != INVALID_HANDLE_VALUE )
        {
            CloseHandle( handle );
        }
        handle = nullptr;
        base = 0;
        pid = 0;
    }

    [[nodiscard]] bool valid( ) const noexcept
    {
        return handle && handle != INVALID_HANDLE_VALUE && base != 0;
    }

    template< typename T >
    [[nodiscard]] T read( std::uint64_t address ) const noexcept
    {
        T value{};
        if ( !valid( ) )
            return value;

        SIZE_T bytes_read = 0;
        nt_read_virtual_mem( handle, reinterpret_cast< PVOID >( address ), &value, sizeof( T ), &bytes_read );
        return value;
    }

    template< typename T >
    bool write( std::uint64_t address, const T& value ) noexcept
    {
        if ( !valid( ) )
            return false;

        SIZE_T bytes_written = 0;
        const long status =
            nt_write_virtual_mem( handle, reinterpret_cast< PVOID >( address ), const_cast< PVOID >( reinterpret_cast< const void* >( &value ) ), sizeof( T ), &bytes_written );
        return status == 0 && bytes_written == sizeof( T );
    }

    [[nodiscard]] bool read_buffer( std::uint64_t address, void* buffer, std::size_t size ) const noexcept;
    bool write_buffer( std::uint64_t address, const void* buffer, std::size_t size ) noexcept;
};

#endif  // memory_hpp
