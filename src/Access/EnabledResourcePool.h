#pragma once

#include <boost/noncopyable.hpp>


namespace DB
{
class EnabledResourcePool : public boost::noncopyable
{
public:
    String name;
    UInt32 max_query_concurrency;

    mutable std::atomic<UInt32> query_concurrency;

    struct QueryHandle
    {
        const EnabledResourcePool & p;
        explicit QueryHandle(const EnabledResourcePool & _p) : p(_p)
        {
            // stupid code to prove a point
            auto c = p.query_concurrency.fetch_add(1);
            if (p.max_query_concurrency && c >= p.max_query_concurrency)
            {
                p.query_concurrency.fetch_sub(1);

                throw Exception("Resource pool " + p.name
                        + " exhausted query concurrency limit"
                        + " (max: " + toString(p.max_query_concurrency) + ")",
                    42);
            }
        }
        ~QueryHandle() { p.query_concurrency.fetch_sub(1); }
    };


    std::unique_ptr<QueryHandle> getQueryHandle() const {
        return std::make_unique<QueryHandle>(*this);
    }
};

}
