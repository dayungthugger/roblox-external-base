#include "instance.hpp"

std::string instance_t::get_name( ) const
{
    const auto name_container = get_member< std::uint64_t >( Offsets::Instance::NameContainer );
    if ( !name_container )
        return "unknown";

    return read_string( name_container + 0x8 );
}

std::string instance_t::get_class_name( ) const
{
    const auto class_desc = get_member< std::uint64_t >( Offsets::Instance::ClassDescriptor );
    if ( !class_desc )
        return "unknown";

    const auto class_name = g_memory.read< std::uint64_t >( class_desc + 0x8 );
    if ( !class_name )
        return "unknown";

    return read_string( class_name );
}

std::vector< instance_t > instance_t::get_children( ) const
{
    std::vector< instance_t > children;

    const auto container = get_member< std::uint64_t >( Offsets::Instance::ChildrenStart );
    const auto addresses = read_shared_addresses( container );

    children.reserve( addresses.size( ) );
    for ( const std::uint64_t addr : addresses )
        children.emplace_back( addr );

    return children;
}

instance_t instance_t::find_first_child( std::string_view name ) const
{
    for ( const auto& child : get_children( ) )
    {
        if ( child.get_name( ) == name )
            return child;
    }
    return instance_t{};
}

instance_t instance_t::find_first_child_of_class( std::string_view class_name ) const
{
    for ( const auto& child : get_children( ) )
    {
        if ( child.get_class_name( ) == class_name )
            return child;
    }
    return instance_t{};
}
