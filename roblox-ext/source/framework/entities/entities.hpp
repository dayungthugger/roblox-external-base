#ifndef entities_hpp
#define entities_hpp

#include <atomic>
#include <shared_mutex>
#include <string>
#include <vector>

#include <framework/globals.hpp>

struct bone_t
{
    std::string key;
    std::uint64_t primitive = 0;
};

struct entity_t
{
    bool self = false;
    std::string name;
    std::uint64_t humanoid = 0;
    std::vector< bone_t > bones;
};

namespace cache
{
    void update( );
    void tick( );
    void stop( );

    [[nodiscard]] std::vector< entity_t > get_snapshot( );
}  // namespace cache

#endif  // entities_hpp
