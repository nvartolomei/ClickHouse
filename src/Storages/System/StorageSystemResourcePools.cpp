#include <Storages/System/StorageSystemResourcePools.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <DataTypes/DataTypeUUID.h>
#include <Columns/ColumnString.h>
#include <Columns/ColumnsNumber.h>
#include <Access/AccessControlManager.h>
#include <Access/ResourcePool.h>
#include <Access/AccessFlags.h>
#include <Interpreters/Context.h>


namespace DB
{

NamesAndTypesList StorageSystemResourcePools::getNamesAndTypes()
{
    NamesAndTypesList names_and_types{
        {"name", std::make_shared<DataTypeString>()},
        {"id", std::make_shared<DataTypeUUID>()},
        {"storage", std::make_shared<DataTypeString>()},
        {"max_query_concurrency", std::make_shared<DataTypeUInt32>()},
    };
    return names_and_types;
}


void StorageSystemResourcePools::fillData(MutableColumns & res_columns, const Context & context, const SelectQueryInfo &) const
{
    context.checkAccess(AccessType::SHOW_RESOURCE_POOLS);
    const auto & access_control = context.getAccessControlManager();
    std::vector<UUID> ids = access_control.findAll<ResourcePool>();

    size_t column_index = 0;
    auto & column_name = assert_cast<ColumnString &>(*res_columns[column_index++]);
    auto & column_id = assert_cast<ColumnUInt128 &>(*res_columns[column_index++]).getData();
    auto & column_storage = assert_cast<ColumnString &>(*res_columns[column_index++]);
    auto & column_max_query_concurrency = assert_cast<ColumnUInt32 &>(*res_columns[column_index++]).getData();

    auto add_row = [&](const String & name,
                       const UUID & id,
                       const String & storage_name,
                       const UInt32 max_q_c)
    {
        column_name.insertData(name.data(), name.length());
        column_id.push_back(id);
        column_storage.insertData(storage_name.data(), storage_name.length());
        column_max_query_concurrency.push_back(max_q_c);
    };

    for (const auto & id : ids)
    {
        auto pool = access_control.tryRead<ResourcePool>(id);
        if (!pool)
            continue;

        auto storage = access_control.findStorage(id);
        if (!storage)
            continue;

        add_row(pool->getName(), id, storage->getStorageName(),
                pool->max_query_concurrency);
    }
}

}
