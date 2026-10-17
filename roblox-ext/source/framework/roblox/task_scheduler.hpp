#ifndef task_scheduler_hpp
#define task_scheduler_hpp

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <utilities/utilities.hpp>

struct job_t : object_base_t
{
    using object_base_t::object_base_t;

    [[nodiscard]] std::string get_name( ) const;
};

struct task_scheduler_t : object_base_t
{
    using object_base_t::object_base_t;

    [[nodiscard]] bool is_loaded( ) const;
    [[nodiscard]] std::vector< job_t > get_jobs( ) const;
    [[nodiscard]] std::uint64_t find_first_job( std::string_view name ) const;
};

#endif  // task_scheduler_hpp
