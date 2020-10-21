#include <Storages/System/StorageSystemCurrentResourcePool.h>
#include <DataTypes/DataTypeString.h>
#include <DataTypes/DataTypesNumber.h>
#include <Columns/ColumnString.h>
#include <Columns/ColumnsNumber.h>
#include <Access/User.h>
#include <Access/EnabledResourcePool.h>
#include <Interpreters/Context.h>


namespace DB
{

NamesAndTypesList StorageSystemCurrentResourcePool::getNamesAndTypes()
{
    NamesAndTypesList names_and_types{
        {"name", std::make_shared<DataTypeString>()},
        {"query_concurrency", std::make_shared<DataTypeUInt32>()},
    };
    return names_and_types;
}


void StorageSystemCurrentResourcePool::fillData(MutableColumns & res_columns, const Context & context, const SelectQueryInfo &) const
{
    auto resource_pool = context.getResourcePool(); // getResourcePoolInfo?
    auto user = context.getUser();
    if (!resource_pool || !user)
        return;

    size_t column_index = 0;
    auto & column_name = assert_cast<ColumnString &>(*res_columns[column_index++]);
    auto & column_query_concurrency = assert_cast<ColumnUInt32 &>(*res_columns[column_index++]).getData();

    auto add_row = [&](const String & name, const UInt32 query_concurrency)
    {
        column_name.insertData(name.data(), name.length());
        column_query_concurrency.push_back(query_concurrency);
    };

    add_row(resource_pool->name, resource_pool->query_concurrency);

    //    for (const auto & role_id : roles_info->current_roles)
//    {
//        const String & role_name = roles_info->names_of_roles.at(role_id);
//        bool admin_option = roles_info->enabled_roles_with_admin_option.count(role_id);
//        bool is_default = user->default_roles.match(role_id);
//        add_row(role_name, admin_option, is_default);
//    }
}

}
