#pragma once
#include "duckdb/execution/physical_operator.hpp"

namespace duckdb {
class PhysicalDummyOperator : public PhysicalOperator {
public:
    PhysicalDummyOperator(vector<LogicalType> types, idx_t estimated_cardinality);
    
    OperatorResultType Execute(ExecutionContext &context, 
                             DataChunk &input, 
                             DataChunk &chunk,
                             GlobalOperatorState &gstate,
                             OperatorState &state) const override;
};
} // namespace duckdb
