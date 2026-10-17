#include <framework/memory/memory.hpp>

#include <cstring>
#include <memory>

namespace
{
    struct handle_guard
    {
        HANDLE handle = nullptr;

        explicit handle_guard( HANDLE h ) noexcept : handle( h )
        {
        }

        ~handle_guard( )
        {
            if ( handle && handle != INVALID_HANDLE_VALUE )
                CloseHandle( handle );
        }

        handle_guard( const handle_guard& ) = delete;
        handle_guard& operator=( const handle_guard& ) = delete;
    };
}  // namespace

std::int32_t get_pid_by_name( std::string_view process_name ) noexcept
{
    handle_guard snapshot( CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 ) );
    if ( !snapshot.handle || snapshot.handle == INVALID_HANDLE_VALUE )
        return 0;

    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof( entry );

    if ( !Process32First( snapshot.handle, &entry ) )
        return 0;

    do
    {
        if ( _stricmp( entry.szExeFile, process_name.data( ) ) == 0 )
            return static_cast< std::int32_t >( entry.th32ProcessID );
    } while ( Process32Next( snapshot.handle, &entry ) );

    return 0;
}

std::uint64_t get_image_base_from_pid( std::int32_t pid ) noexcept
{
    if ( pid <= 0 )
        return 0;

    handle_guard snapshot( CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, static_cast< DWORD >( pid ) ) );
    if ( !snapshot.handle || snapshot.handle == INVALID_HANDLE_VALUE )
        return 0;

    MODULEENTRY32 entry{};
    entry.dwSize = sizeof( entry );

    if ( !Module32First( snapshot.handle, &entry ) )
        return 0;

    return reinterpret_cast< std::uint64_t >( entry.modBaseAddr );
}

bool memory_t::read_buffer( std::uint64_t address, void* buffer, std::size_t size ) const noexcept
{
    if ( !valid( ) || !buffer || size == 0 )
        return false;

    SIZE_T bytes_read = 0;
    const long status = nt_read_virtual_mem( handle, reinterpret_cast< PVOID >( address ), buffer, size, &bytes_read );
    return status == 0 && bytes_read == size;
}

bool memory_t::write_buffer( std::uint64_t address, const void* buffer, std::size_t size ) noexcept
{
    if ( !valid( ) || !buffer || size == 0 )
        return false;

    SIZE_T bytes_written = 0;
    const long status =
        nt_write_virtual_mem( handle, reinterpret_cast< PVOID >( address ), const_cast< PVOID >( buffer ), size, &bytes_written );
    return status == 0 && bytes_written == size;
}
