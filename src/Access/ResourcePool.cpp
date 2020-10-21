#include <Access/ResourcePool.h>


namespace DB
{

bool ResourcePool::equal(const IAccessEntity & other) const
{
    if (!IAccessEntity::equal(other))
        return false;
    const auto & other_profile = typeid_cast<const ResourcePool &>(other);
    return (/*elements == other_profile.elements*/true) && (to_roles == other_profile.to_roles);
}

}
