#pragma once

#include <Access/IAccessEntity.h>
#include <Access/RolesOrUsersSet.h>
#include <Access/SettingsProfileElement.h>


namespace DB
{
/// Represents a settings resource pool created by command
/// CREATE RESOURCE_POOL name ... TO roles
struct ResourcePool : public IAccessEntity
{
    UInt32 max_query_concurrency;

    /// Which roles or users should use this resource pool.
    RolesOrUsersSet to_roles;

    bool equal(const IAccessEntity & other) const override;
    std::shared_ptr<IAccessEntity> clone() const override { return cloneImpl<ResourcePool>(); }
    static constexpr const Type TYPE = Type::RESOURCE_POOL;
    Type getType() const override { return TYPE; }
};

using ResourcePoolPtr = std::shared_ptr<const ResourcePool>;
}
