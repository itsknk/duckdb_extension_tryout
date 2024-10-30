#pragma once
#include "duckdb.hpp"
#include "duckdb/execution/physical_operator.hpp"

namespace duckdb {

struct DummyFunctionData : public TableFunctionData {
    DummyFunctionData() = default;
    shared_ptr<Relation> plan;
    unique_ptr<QueryResult> res;
    unique_ptr<Connection> conn;
};

class DummyExtensionExtension : public Extension {
public:
    void Load(DuckDB &db) override;
    std::string Name() override;
    std::string Version() const override { return "v0.0.1"; }
};

void InitializeDummyFunction(const Connection &con);

extern "C" {
    void InjectDummyOperator(unique_ptr<PhysicalOperator> &plan);
}

} // namespace duckdb
