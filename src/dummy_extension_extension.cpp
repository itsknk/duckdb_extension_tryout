#define DUCKDB_EXTENSION_MAIN

#include "dummy_extension_extension.hpp"
#include "physical_dummy_operator.hpp"
#include "duckdb/main/config.hpp"
#include "duckdb/execution/operator/scan/physical_table_scan.hpp"
#include "duckdb/execution/operator/filter/physical_filter.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/main/relation/table_relation.hpp"
#include "duckdb/catalog/catalog.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/parser/parsed_data/create_table_function_info.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"

namespace duckdb {

extern "C" {
    void InjectDummyOperator(unique_ptr<PhysicalOperator> &plan) {
        for (auto &child : plan->children) {
            InjectDummyOperator(child);  // Recursively traverse children
        }

        if (plan->type == PhysicalOperatorType::TABLE_SCAN) {
            // Create a dummy operator
            auto dummy = make_uniq<PhysicalDummyOperator>(plan->types, plan->estimated_cardinality);

            // Create a filter operator with a true constant expression
            vector<unique_ptr<Expression>> filter_expressions;
            filter_expressions.push_back(
            make_uniq<BoundConstantExpression>(Value::BOOLEAN(true))
            );

            auto filter_op = make_uniq<PhysicalFilter>(plan->types, std::move(filter_expressions), plan->estimated_cardinality);

            // Set the original TABLE_SCAN as a child of the filter
            filter_op->children.push_back(std::move(plan));

            // Add the filter as a child of the dummy operator
            dummy->children.push_back(std::move(filter_op));

            // Replace the original plan with the dummy operator
            plan = std::move(dummy);
        }
    }
}



// Dummy function to fetch data
static void DummyFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
    auto &data = data_p.bind_data->CastNoConst<DummyFunctionData>();
    if (!data.res) {
        data.res = data.plan->Execute();  // Execute the plan
    }
    auto result_chunk = data.res->Fetch();  // Fetch result chunk
    if (!result_chunk) {
        return;
    }
    output.Move(*result_chunk);  // Move data to output
}

// Bind function for the dummy operator
static unique_ptr<FunctionData> DummyBind(ClientContext &context, 
                                          TableFunctionBindInput &input,
                                          vector<LogicalType> &return_types, 
                                          vector<string> &names) {
    auto result = make_uniq<DummyFunctionData>();

    // Create a connection to the database
    result->conn = make_uniq<Connection>(*context.db);

    // Use TableRelation to create the plan from the query string
    string query = input.inputs[0].GetValue<string>();
    auto table_description = result->conn->TableInfo(query);  // Fetch table description

    if (!table_description) {
        throw std::runtime_error("Table not found: " + query);
    }

    // Create a TableRelation from the table description
    result->plan = duckdb::make_shared_ptr<TableRelation>(result->conn->context, std::move(table_description));

    // Extract columns from the TableRelation
    for (auto &column : result->plan->Columns()) {
        return_types.emplace_back(column.Type());
        names.emplace_back(column.Name());
    }

    return std::move(result);
}

// Initialize the dummy function in the DuckDB catalog
void InitializeDummyFunction(const Connection &con) {
    auto &catalog = Catalog::GetSystemCatalog(*con.context);
    TableFunction dummy_func("dummy_function", {LogicalType::VARCHAR}, DummyFunction, DummyBind);
    CreateTableFunctionInfo dummy_info(dummy_func);
    catalog.CreateTableFunction(*con.context, dummy_info);
}

// Load the extension and register the dummy function
void DummyExtensionExtension::Load(DuckDB &db) {
    Connection con(db);
    con.BeginTransaction();
    InitializeDummyFunction(con);
    con.Commit();
}

// Extension name and version
std::string DummyExtensionExtension::Name() {
    return "dummy_extension";
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void dummy_extension_init(duckdb::DatabaseInstance &db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::DummyExtensionExtension>();
}

DUCKDB_EXTENSION_API const char *dummy_extension_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
