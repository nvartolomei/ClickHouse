#pragma once

#include <ext/shared_ptr_helper.h>
#include <Storages/System/IStorageSystemOneBlock.h>


namespace DB
{
class Context;

class StorageSystemCurrentResourcePool final : public ext::shared_ptr_helper<StorageSystemCurrentResourcePool>, public IStorageSystemOneBlock<StorageSystemCurrentResourcePool>
{
public:
    std::string getName() const override { return "SystemCurrentResourcePool"; }
    static NamesAndTypesList getNamesAndTypes();

protected:
    friend struct ext::shared_ptr_helper<StorageSystemCurrentResourcePool>;
    using IStorageSystemOneBlock::IStorageSystemOneBlock;
    void fillData(MutableColumns & res_columns, const Context & context, const SelectQueryInfo &) const override;
};

}
