#ifndef instance_hpp
#define instance_hpp

#include <string>
#include <string_view>
#include <vector>

#include <utilities/utilities.hpp>

struct instance_t : object_base_t
{
    using object_base_t::object_base_t;

    [[nodiscard]] std::string get_name( ) const;
    [[nodiscard]] std::string get_class_name( ) const;

    [[nodiscard]] std::vector< instance_t > get_children( ) const;

    [[nodiscard]] instance_t find_first_child( std::string_view name ) const;
    [[nodiscard]] instance_t find_first_child_of_class( std::string_view class_name ) const;
};

#endif  // instance_hpp
