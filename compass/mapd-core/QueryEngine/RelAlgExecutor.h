/*
 * Copyright 2017 MapD Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef QUERYENGINE_RELALGEXECUTOR_H
#define QUERYENGINE_RELALGEXECUTOR_H

#include "../Shared/scope.h"
#include "Distributed/AggregatedResult.h"
#include "Execute.h"
#include "InputMetadata.h"
#include "QueryRewrite.h"
#include "RelAlgExecutionDescriptor.h"
#include "SpeculativeTopN.h"
#include "StreamingTopN.h"

#include <ctime>

enum class MergeType { Union, Reduce };

struct FirstStepExecutionResult {
  ExecutionResult result;
  const MergeType merge_type;
  const unsigned node_id;
  bool is_outermost_query;
};

class RelAlgExecutor {
 public:
  RelAlgExecutor(Executor* executor, const Catalog_Namespace::SessionInfo& session)
      : executor_(executor),
        session_(session),
        cat_(session.get_catalog()),
        now_(0),
        queue_time_ms_(0),
        fpd_enabled_(session.fpd_enabled()),
        node_traverse_bound_(session.get_NODE_TRAVERSE_BOUND()),
        sketch_sparsity_bound_(session.get_SKETCH_SPARSITY_BOUND()) {}

  RelAlgExecutor(Executor* executor, const Catalog_Namespace::SessionInfo& session, const bool fpd_enabled)
      : executor_(executor),
        session_(session),
        cat_(session.get_catalog()),
        now_(0),
        queue_time_ms_(0),
        fpd_enabled_(fpd_enabled),
        node_traverse_bound_(session.get_NODE_TRAVERSE_BOUND()),
        sketch_sparsity_bound_(session.get_SKETCH_SPARSITY_BOUND()) {}

  ExecutionResult executeRelAlgQuery(const std::string& query_ra,
                                     const CompilationOptions& co,
                                     const ExecutionOptions& eo,
                                     RenderInfo* render_info);

  FirstStepExecutionResult executeRelAlgQueryFirstStep(const RelAlgNode* ra,
                                                       const CompilationOptions& co,
                                                       const ExecutionOptions& eo,
                                                       RenderInfo* render_info);

  void prepareLeafExecution(const AggregatedColRange& agg_col_range,
                            const StringDictionaryGenerations& string_dictionary_generations,
                            const TableGenerations& table_generations);

  ExecutionResult executeRelAlgSubQuery(const RexSubQuery* subquery,
                                        const CompilationOptions& co,
                                        const ExecutionOptions& eo);

  ExecutionResult executeRelAlgSeq(std::vector<RaExecutionDesc>& ed_list,
                                   const CompilationOptions& co,
                                   const ExecutionOptions& eo,
                                   RenderInfo* render_info,
                                   const int64_t queue_time_ms);

  void addLeafResult(const unsigned id, const AggregatedResult& result) {
    const auto it_ok = leaf_results_.emplace(id, result);
    CHECK(it_ok.second);
  }

  void registerSubquery(RexSubQuery* subquery) noexcept { subqueries_.push_back(subquery); }

  const std::vector<RexSubQuery*>& getSubqueries() const noexcept { return subqueries_; };

  AggregatedColRange computeColRangesCache(const RelAlgNode* ra);

  StringDictionaryGenerations computeStringDictionaryGenerations(const RelAlgNode* ra);

  TableGenerations computeTableGenerations(const RelAlgNode* ra);

  Executor* getExecutor() const;

  const Catalog_Namespace::SessionInfo& getSessionInfo() { return session_; }

  const bool fpd_enabled() { return fpd_enabled_; }

  ExecutionResult executeRelAlgQueryFPD(const RelAlgNode* ra,
                                        ssize_t fpd_max_count,
                                        const std::pair<int32_t, int> table_id_phy);

  ssize_t addPushDownFilter(const int table_id, ExecutionResult& result) {
    ssize_t num_filtered_rows = -1;
    auto row_set = boost::get<RowSetPtr>(&result.getDataPtr());
    if (row_set) {
      CHECK_LT(size_t(0), (*row_set)->colCount());
      num_filtered_rows = (*row_set)->rowCount();
    }
    CHECK_LT(table_id, 0);
    ExecutionResult result_copy(std::move(result));
    const auto it_ok = push_down_filters_.insert(std::make_pair(table_id, std::move(result_copy)));
    CHECK(it_ok.second);
    return num_filtered_rows;
  }

  std::unordered_multimap<size_t, std::pair<size_t, size_t>>& getTableIndexInfo() { return table_index_info; }
  std::vector<std::pair<size_t, unsigned int>>& getSketchComplexity() { return table_sketch_complexity; }
  std::set<size_t>& getMaterializedStatuses() { return table_materialized_status; }
  std::unordered_map<size_t, std::set<size_t>>& getJoinGraph() { return join_graph; }
  std::unordered_map<size_t, std::vector<std::tuple<size_t, size_t, size_t, size_t, size_t>>>& getNodeJoinColumns() { return node_join_columns; }
  std::set<size_t>& getTablesWithPredicates() { return nodes_with_selection_predicates; }
  std::unordered_map<size_t, FAGMS_Sketch*>& getSketchInstances() { return tables_fagms_sketches; }
  std::unordered_map<std::string, std::pair<Xi_CW2B*, Xi_EH3*>>& getNewSeeds() { return new_sketch_seeds; }
  std::set<size_t>& getProcessedNodes() { return already_processed_nodes; }
  unsigned& getNodeTraverseBound() { return node_traverse_bound_; }
  unsigned& getSketchSparsityBound() { return sketch_sparsity_bound_; }
  std::chrono::steady_clock::time_point get_query_start_time() { return query_start_time; }
  void set_query_start_time(std::chrono::steady_clock::time_point new_time) { query_start_time = new_time; }

  void printAllInfo() {
    //////////////////////////////////////START PRINT//////////////////////////////////////////////////////////////
    std::cout << "\nINIT Join Graph" << std::endl;
    for (auto node = getJoinGraph().begin(); node != getJoinGraph().end(); ++node) {
      std::cout << "\t" << node->first << " ::";
      for (auto adj_node = node->second.begin(); adj_node != node->second.end(); ++adj_node) {
        std::cout << " " << *adj_node;
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;

    std::cout << "INIT Join Connections" << std::endl;
    for (auto node = getNodeJoinColumns().begin(); node != getNodeJoinColumns().end(); ++node) {
      std::cout << "\tlhs_table_node: " << node->first << std::endl;
      for (auto join_info = node->second.begin(); join_info != node->second.end(); ++join_info) {
        auto table_id = std::get<0>(*join_info);
        auto column_id = std::get<1>(*join_info), join_table_node = std::get<2>(*join_info);
        auto join_table_id = std::get<3>(*join_info), join_column_id = std::get<4>(*join_info);
        std::cout << "\t\tlhs_table_id: " << table_id << ", lhs_column_id: " << column_id << ", rhs_table_node: "
            << join_table_node << ", rhs_table_id: " << join_table_id << ", rhs_column_id: " << join_column_id << std::endl;
      }
    }
    std::cout << std::endl;

    std::cout << "INIT SKETCHES::" << std::endl;
    for (auto node = getSketchInstances().begin(); node != getSketchInstances().end(); ++node) {
      std::cout << "\tnode: " << node->first << " ++++ ";
      node->second->PrintSketch();
      std::cout << std::endl;
    }
    std::cout << std::endl;
    //////////////////////////////////////END PRINT////////////////////////////////////////////////////////////////
  }

 private:
  ExecutionResult executeRelAlgQueryNoRetry(const std::string& query_ra,
                                            const CompilationOptions& co,
                                            const ExecutionOptions& eo,
                                            RenderInfo* render_info);

  void executeRelAlgStep(const size_t step_idx,
                         std::vector<RaExecutionDesc>&,
                         const CompilationOptions&,
                         const ExecutionOptions&,
                         RenderInfo*,
                         const int64_t queue_time_ms);

  ExecutionResult executeCompound(const RelCompound*,
                                  const CompilationOptions&,
                                  const ExecutionOptions&,
                                  RenderInfo*,
                                  const int64_t queue_time_ms);

  ExecutionResult executeAggregate(const RelAggregate* aggregate,
                                   const CompilationOptions& co,
                                   const ExecutionOptions& eo,
                                   RenderInfo* render_info,
                                   const int64_t queue_time_ms);

  ExecutionResult executeProject(const RelProject*,
                                 const CompilationOptions&,
                                 const ExecutionOptions&,
                                 RenderInfo*,
                                 const int64_t queue_time_ms);

  ExecutionResult executeFilter(const RelFilter*,
                                const CompilationOptions&,
                                const ExecutionOptions&,
                                RenderInfo*,
                                const int64_t queue_time_ms);

  ExecutionResult executeSort(const RelSort*,
                              const CompilationOptions&,
                              const ExecutionOptions&,
                              RenderInfo*,
                              const int64_t queue_time_ms);

  ExecutionResult executeJoin(const RelJoin*,
                              const CompilationOptions&,
                              const ExecutionOptions&,
                              RenderInfo*,
                              const int64_t queue_time_ms);

  ExecutionResult executeLogicalValues(const RelLogicalValues*, const ExecutionOptions&);

  // TODO(alex): just move max_groups_buffer_entry_guess to RelAlgExecutionUnit once
  //             we deprecate the plan-based executor paths and remove WorkUnit
  struct WorkUnit {
    RelAlgExecutionUnit exe_unit;
    const RelAlgNode* body;
    const size_t max_groups_buffer_entry_guess;
    std::unique_ptr<QueryRewriter> query_rewriter;
  };

  WorkUnit createSortInputWorkUnit(const RelSort*, const bool just_explain);

  ExecutionResult executeWorkUnit(const WorkUnit& work_unit,
                                  const std::vector<TargetMetaInfo>& targets_meta,
                                  const bool is_agg,
                                  const CompilationOptions& co,
                                  const ExecutionOptions& eo,
                                  RenderInfo*,
                                  const int64_t queue_time_ms);

  size_t getNDVEstimation(const WorkUnit& work_unit,
                          const bool is_agg,
                          const CompilationOptions& co,
                          const ExecutionOptions& eo);

  ssize_t getFilteredCountAll(const WorkUnit& work_unit,
                              const bool is_agg,
                              const CompilationOptions& co,
                              const ExecutionOptions& eo);

  ssize_t getFilteredCountAllAndUpdateSketch(const WorkUnit& work_unit,
                                             const bool is_agg,
                                             const CompilationOptions& co,
                                             const ExecutionOptions& eo);

  bool isRowidLookup(const WorkUnit& work_unit);

  ExecutionResult renderWorkUnit(const RelAlgExecutor::WorkUnit& work_unit,
                                 const std::vector<TargetMetaInfo>& targets_meta,
                                 RenderInfo* render_info,
                                 const int32_t error_code,
                                 const int64_t queue_time_ms);

  void executeUnfoldedMultiJoin(const RelAlgNode* user,
                                RaExecutionDesc& exec_desc,
                                const CompilationOptions& co,
                                const ExecutionOptions& eo,
                                const int64_t queue_time_ms);

  ExecutionResult handleRetry(const int32_t error_code_in,
                              const RelAlgExecutor::WorkUnit& work_unit,
                              const std::vector<TargetMetaInfo>& targets_meta,
                              const bool is_agg,
                              const CompilationOptions& co,
                              const ExecutionOptions& eo,
                              const int64_t queue_time_ms);

  static void handlePersistentError(const int32_t error_code);

  static std::string getErrorMessageFromCode(const int32_t error_code);

  WorkUnit createWorkUnit(const RelAlgNode*, const SortInfo&, const bool just_explain);

  WorkUnit createCompoundWorkUnit(const RelCompound*, const SortInfo&, const bool just_explain);

  WorkUnit createAggregateWorkUnit(const RelAggregate*, const SortInfo&, const bool just_explain);

  WorkUnit createProjectWorkUnit(const RelProject*, const SortInfo&, const bool just_explain);

  WorkUnit createFilterWorkUnit(const RelFilter*, const SortInfo&, const bool just_explain);

  WorkUnit createJoinWorkUnit(const RelJoin*, const SortInfo&, const bool just_explain);

  void addTemporaryTable(const int table_id, const ResultPtr& result) {
    auto row_set = boost::get<RowSetPtr>(&result);
    if (row_set) {
      CHECK_LT(size_t(0), (*row_set)->colCount());
    }
    CHECK_LT(table_id, 0);
    const auto it_ok = temporary_tables_.emplace(table_id, result);
    CHECK(it_ok.second);
  }

  void handleNop(const RelAlgNode*);

  JoinQualsPerNestingLevel translateLeftDeepJoinFilter(
      const RelLeftDeepInnerJoin* join,
      const std::vector<InputDescriptor>& input_descs,
      const std::unordered_map<const RelAlgNode*, int>& input_to_nest_level,
      const bool just_explain);

  // Transform the provided `join_condition` to conjunctive form, find composite
  // key opportunities and finally translate it to an Analyzer expression.
  std::list<std::shared_ptr<Analyzer::Expr>> makeJoinQuals(
      const RexScalar* join_condition,
      const std::vector<JoinType>& join_types,
      const std::unordered_map<const RelAlgNode*, int>& input_to_nest_level,
      const bool just_explain) const;

  Executor* executor_;
  const Catalog_Namespace::SessionInfo& session_;
  const Catalog_Namespace::Catalog& cat_;
  TemporaryTables temporary_tables_;
  std::unordered_map<int, const ExecutionResult> push_down_filters_;

  // key = table_id 88 node_id, value = pair of table_id and node_id
  std::unordered_multimap<size_t, std::pair<size_t, size_t>> table_index_info;
  // key = table_id 88 node_id, value = number of survived tuples * number of columns
  std::vector<std::pair<size_t, unsigned int>> table_sketch_complexity;
  // key = table_id 88 node_id, value = whether the table is materialized
  std::set<size_t> table_materialized_status;
  // key = table_id 88 node_id, value = set of table_id 88 node_id
  std::unordered_map<size_t, std::set<size_t>> join_graph;
  // key = table_id 88 node_id, value = tuple of table_id_1, column_id_1, table_id 88 node_id, table_id_2, column_id_2
  std::unordered_map<size_t, std::vector<std::tuple<size_t, size_t, size_t, size_t, size_t>>> node_join_columns;
  // set of table_id 88 node_id
  std::set<size_t> nodes_with_selection_predicates;
  // set of table_id 88 node_id to check for missing newly generated seeds
  std::set<size_t> already_processed_nodes;
  // set of table_id 88 node_id, value = FastAGMS
  std::unordered_map<size_t, FAGMS_Sketch*> tables_fagms_sketches;
  // key = lhs_table_id--lhs_col_id--rhs_table_id--rhs_node_id, value = new seed pair
  std::unordered_map<std::string, std::pair<Xi_CW2B*, Xi_EH3*>> new_sketch_seeds;
  std::chrono::steady_clock::time_point query_start_time;

  time_t now_;
  std::vector<std::shared_ptr<Analyzer::Expr>> target_exprs_owned_;  // TODO(alex): remove
  std::vector<RexSubQuery*> subqueries_;
  std::unordered_map<unsigned, AggregatedResult> leaf_results_;
  int64_t queue_time_ms_;
  static SpeculativeTopNBlacklist speculative_topn_blacklist_;
  static const size_t max_groups_buffer_entry_default_guess{16384};
  const bool fpd_enabled_;
  unsigned node_traverse_bound_;
  unsigned sketch_sparsity_bound_;
};

#endif  // QUERYENGINE_RELALGEXECUTOR_H
