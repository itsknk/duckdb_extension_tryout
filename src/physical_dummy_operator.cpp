#include "physical_dummy_operator.hpp"

namespace duckdb {

PhysicalDummyOperator::PhysicalDummyOperator(vector<LogicalType> types, idx_t estimated_cardinality)
    : PhysicalOperator(PhysicalOperatorType::FILTER, std::move(types), estimated_cardinality) {
}

OperatorResultType PhysicalDummyOperator::Execute(ExecutionContext &context,
                                                DataChunk &input,
                                                DataChunk &chunk,
                                                GlobalOperatorState &gstate,
                                                OperatorState &state) const {
    chunk.Reference(input);
    return OperatorResultType::NEED_MORE_INPUT;
}

} // namespace duckdb
