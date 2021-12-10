#include "FilterPushDownSketch.h"

class FilterPushDownSketch {
 public:
  // return the number of tuples in a table given by its identifier
  // get it from the catalog
  size_t getNumTuples(int32_t table_id, Catalog_Namespace::Catalog& cat) {
    size_t num_tuples = 0;
    const auto td = cat.getMetadataForTable(table_id);
    const auto shard_tables = cat.getPhysicalTablesDescriptors(td);
    for (const auto shard_table : shard_tables) {
      const auto& shard_metainfo = shard_table->fragmenter->getFragmentsForQuery();
      num_tuples += shard_metainfo.getPhysicalNumTuples();
    }

    return num_tuples;
  }

  // retrieve join conditions from a set of rel alg nodes and a given expression
  // join conditions are added to the executor
  // return false only in the case of a subquery
  // we have to preserve the columns appearing in a join for each table because
  // we have to build sketches on them
  // we do this with a map [table --> (colIdx, colName)]
  bool extractHashJoinCol(std::vector<std::shared_ptr<RelAlgNode>>& nodes,
                          const RexScalar* rex,
                          RelAlgExecutor* ra_executor) {
    const auto rex_operator = dynamic_cast<const RexOperator*>(rex);
    if (rex_operator) {
      auto oper_size = rex_operator->size();

      // this is the base case of a simple operation with two operands
      if (oper_size == 2) {
        const auto rex_input_lhs = dynamic_cast<const RexInput*>(rex_operator->getOperand(0));
        const auto rex_input_rhs = dynamic_cast<const RexInput*>(rex_operator->getOperand(1));

        if (rex_input_lhs && rex_input_rhs) {
          unsigned t_id_lhs, t_id_rhs;
          std::string col_name_lhs, col_name_rhs;
          unsigned col_id_lhs, col_id_rhs;
          std::tie(t_id_lhs, col_name_lhs, col_id_lhs) =
              findColumnNameByIndex(rex_input_lhs->getSourceNode(), rex_input_lhs->getIndex());
          std::tie(t_id_rhs, col_name_rhs, col_id_rhs) =
              findColumnNameByIndex(rex_input_rhs->getSourceNode(), rex_input_rhs->getIndex());

          // look for the RelScan's order in nodes using t_id
          int t_order_lhs = -1, t_order_rhs = -1, t_order = 0;
          int32_t t_id_lhs_phy = -1, t_id_rhs_phy = -1;
          auto lhs_node_id = -1, rhs_node_id = -1;
          for (auto node : nodes) {
            auto node_scan = std::dynamic_pointer_cast<RelScan>(node);
            if (node_scan) {
              if (t_order_lhs < 0 && t_id_lhs == node_scan->getId()) {
                t_order_lhs = t_order;
                t_id_lhs_phy = node_scan->getTableDescriptor()->tableId;
                lhs_node_id = node_scan->getId();
              }
              if (t_order_rhs < 0 && t_id_rhs == node_scan->getId()) {
                t_order_rhs = t_order;
                t_id_rhs_phy = node_scan->getTableDescriptor()->tableId;
                rhs_node_id = node_scan->getId();
              }
              if ((t_order_lhs != -1) && (t_order_rhs != -1)) {
                break;
              }
              t_order++;
            }
          }
          CHECK_GT(t_order_lhs, -1);
          CHECK_GT(t_order_rhs, -1);
          CHECK_GT(t_id_lhs_phy, -1);
          CHECK_GT(t_id_rhs_phy, -1);
          if (t_order_lhs != t_order_rhs) {

            auto node_lhs = std::stoi(std::to_string(t_id_lhs_phy) + "88" + std::to_string(lhs_node_id));
            auto node_rhs = std::stoi(std::to_string(t_id_rhs_phy) + "88" + std::to_string(rhs_node_id));

            if (ra_executor->getJoinGraph().find(node_lhs) == ra_executor->getJoinGraph().end()) {
              std::set<size_t> adjacent_nodes;
              ra_executor->getJoinGraph()[node_lhs] = adjacent_nodes;
              std::vector<std::tuple<size_t, size_t, size_t, size_t, size_t>> join_columns;
              ra_executor->getNodeJoinColumns()[node_lhs] = join_columns;
            }
            ra_executor->getJoinGraph()[node_lhs].emplace(node_rhs);
            ra_executor->getNodeJoinColumns()[node_lhs].emplace_back(t_id_lhs_phy, col_id_lhs, node_rhs, t_id_rhs_phy, col_id_rhs);

            if (ra_executor->getJoinGraph().find(node_rhs) == ra_executor->getJoinGraph().end()) {
              std::set<size_t> adjacent_nodes;
              ra_executor->getJoinGraph()[node_rhs] = adjacent_nodes;
              std::vector<std::tuple<size_t, size_t, size_t, size_t, size_t>> join_columns;
              ra_executor->getNodeJoinColumns()[node_rhs] = join_columns;
             }
            ra_executor->getJoinGraph()[node_rhs].emplace(node_lhs);
            ra_executor->getNodeJoinColumns()[node_rhs].emplace_back(t_id_rhs_phy, col_id_rhs, node_lhs, t_id_lhs_phy, col_id_lhs);

            // table physical and order indices needed in join ordering logic. See in RelAlgExecutor for more details.
            if (ra_executor->getTableIndexInfo().find(node_lhs) == ra_executor->getTableIndexInfo().end()) {
                ra_executor->getTableIndexInfo().emplace(node_lhs, std::make_pair(t_id_lhs_phy, t_order_lhs));
            }
            if (ra_executor->getTableIndexInfo().find(node_rhs) == ra_executor->getTableIndexInfo().end()) {
                ra_executor->getTableIndexInfo().emplace(node_rhs, std::make_pair(t_id_rhs_phy, t_order_rhs));
            }

            return true;
          }
        }
      }

      // perform recursive extraction from all the parts of a complex expression
      for (size_t i = 0; i < oper_size; i++) {
        const auto operand = rex_operator->getOperand(i);
        bool ret = extractHashJoinCol(nodes, operand, ra_executor);
        if (false == ret)
          return false;
      }

      return true;
    }

    const auto rex_sub_query = dynamic_cast<const RexSubQuery*>(rex);
    if (rex_sub_query)
      return false;

    return true;
  }

  bool evaluateAndPushDown(std::vector<std::shared_ptr<RelAlgNode>>& nodes,
                           RelAlgExecutor* ra_executor,
                           std::vector<std::pair<const RelAlgNode*, size_t>>& table_sizes) {

    size_t node_index = -1;
    for (auto it_nodes = nodes.begin(); it_nodes != nodes.end(); ++it_nodes) {
      node_index++;
      auto node_scan = std::dynamic_pointer_cast<RelScan>(*it_nodes);
      unsigned table_size = -1;
      if (node_scan) {
        auto node_value = std::stoi(std::to_string(node_scan->getTableDescriptor()->tableId) + "88" + std::to_string(node_index + 1));
        if (!std::dynamic_pointer_cast<RelScan>(*(std::next(it_nodes, 1))) &&
                !std::dynamic_pointer_cast<RelJoin>(*(std::next(it_nodes, 1)))) { continue; }
        for (auto table_info = table_sizes.begin(); table_info != table_sizes.end(); ++table_info) {
          if (node_scan.get() == table_info->first) {
            table_size = table_info->second;
            break;
          }
        }

        std::vector<std::pair<unsigned, std::string>> cols_to_project;
        auto filter_expr_new = findFilterAndProject(nodes, node_index, cols_to_project);
        if (!filter_expr_new) { continue; }
        if (!ra_executor->getSessionInfo().get_PRE_PROCESSING_STATUS() &&
            table_size < ra_executor->getSessionInfo().get_FPD_MIN_TABLE_SIZE()) { continue; }
        ra_executor->getTablesWithPredicates().emplace(node_value);
      }
    }

    node_index = -1;
    for (auto it_nodes = nodes.begin(); it_nodes != nodes.end(); ++it_nodes) {
      node_index++;
      auto node_scan = std::dynamic_pointer_cast<RelScan>(*it_nodes);
      if (node_scan) {
        if (!std::dynamic_pointer_cast<RelScan>(*(std::next(it_nodes, 1))) &&
                !std::dynamic_pointer_cast<RelJoin>(*(std::next(it_nodes, 1)))) { continue; }
        auto node_value = std::stoi(std::to_string(node_scan->getTableDescriptor()->tableId) + "88" + std::to_string(node_index + 1));
        if (ra_executor->getTablesWithPredicates().find(node_value) == ra_executor->getTablesWithPredicates().end()) {
          findSketchFromTemplates(node_value, node_scan->getTableDescriptor()->tableId, ra_executor, true);  // pre-build sketch templates
        } else {
          findSketchFromTemplates(node_value, node_scan->getTableDescriptor()->tableId, ra_executor, false);  // sketches with selection predicates
        }
      }
    }

    std::cout << "------ BEFORE pushDownFilterPredicates ------" << std::endl;
    // std::cout << tree_string(nodes.back().get(), 0) << std::endl;

    // look for each RelScan in ascending order in terms of size so that we can deal with the largest relation at the
    // end to ignore selection push-down on the largest relation if the number of filtered rows is still greater than
    // the second largest table's, i.e., does not affect join ordering
    auto it_table_sizes = table_sizes.begin();
    size_t max_size_so_far = 0;
    size_t i = 0;
    bool skip_filter;  // flag for skip selection push-down
    while (it_table_sizes != table_sizes.end()) {
      auto node_scan = std::dynamic_pointer_cast<RelScan>(nodes[i]);
      // auto table_size = table_sizes[ts_count];
      if (node_scan.get() == it_table_sizes->first) {
        // if next node is neither scan nor join,
        // this is already a derived table, skip
        if (!std::dynamic_pointer_cast<RelScan>(nodes[i + 1]) && !std::dynamic_pointer_cast<RelJoin>(nodes[i + 1])) {
          continue;
        }
        // reset the flag for each RelScan
        skip_filter = false;
        // find filter predicates to push-down and columns to project for this table
        // if there is no filter predicate, skip selection push-down
        std::vector<std::pair<unsigned, std::string>> cols_to_project;
        auto filter_expr_new = findFilterAndProject(nodes, i, cols_to_project);
        // if the size of relation is too small, it is not worth of materialization
        if (!filter_expr_new) {
          skip_filter = true;
        } else if (!ra_executor->getSessionInfo().get_PRE_PROCESSING_STATUS() &&
            it_table_sizes->second < ra_executor->getSessionInfo().get_FPD_MIN_TABLE_SIZE()) {
          skip_filter = true;
        }

        std::string table_name = node_scan->getTableDescriptor()->tableName;
        unsigned table_id = node_scan->getTableDescriptor()->tableId;
        unsigned column_id = 0;
        std::cout << "\nTABLE INFO:: table_name: " << table_name
            << ", table_id: " << table_id << ", node_id: " << node_scan->getId() << "------" << std::endl;
        for (auto col_name : node_scan->getFieldNames()) {
            std::cout << "\t" << column_id << ". " << col_name << std::endl;
            column_id++;
        }
        std::cout << std::endl;

        if (skip_filter) {  // generate a sketch without filter
          auto clock_begin = timer_start();
          // make cols_to_project contain only join attributes
          std::set<size_t> cols_to_join;
          auto node_value = std::stoi(std::to_string(table_id) + "88" + std::to_string(node_scan->getId()));
          std::cout << "------ table: " << table_id << " " << node_scan->getId() << "------" << std::endl;
          for (auto it_cols = ra_executor->getNodeJoinColumns()[node_value].begin();
            it_cols != ra_executor->getNodeJoinColumns()[node_value].end(); ++it_cols) {
            auto column_id = std::get<1>(*it_cols);
            cols_to_join.insert(column_id);
          }
          CHECK(!cols_to_join.empty());
          if (cols_to_join.size() != cols_to_project.size()) {
            auto it_proj = cols_to_project.begin();
            auto it_join = cols_to_join.begin();
            while (it_proj != cols_to_project.end()) {
              if (it_proj->first == *it_join) {
                ++it_proj;
                ++it_join;
              } else {  // not a join attribute: erase
                cols_to_project.erase(it_proj);
              }
            }
          }
          CHECK_EQ(cols_to_join.size(), cols_to_project.size());
          // create RelProject
          std::vector<std::unique_ptr<const RexScalar>> scalar_exprs;
          std::vector<std::string> fields;
          for (auto col : cols_to_project) {
            scalar_exprs.push_back(std::unique_ptr<const RexScalar>(new RexInput(node_scan.get(), col.first)));
            fields.push_back(col.second);
          }
          size_t no_columns = ra_executor->getJoinGraph().find(node_value)->second.size();
          ra_executor->getSketchComplexity().emplace_back(node_value, no_columns * it_table_sizes->second);
          max_size_so_far = std::max(max_size_so_far, it_table_sizes->second);
          auto queue_time_ms = timer_stop(clock_begin);
          std::cout << "time taken for updating sketch without filter: " << queue_time_ms << " ms" << std::endl;
        } else {
          auto clock_begin = timer_start();
          // add new push-down filter from filter_expr_new
          auto node_filter_new = std::shared_ptr<RelAlgNode>(new RelFilter(filter_expr_new, nodes[i]));
          // look for the parent node to replace its input into new RelFilter
          size_t input_idx = 0, dist = 1;
          if (std::dynamic_pointer_cast<RelScan>(nodes[i + dist])) {
            while (true) {  // current node is left input of next join
              dist++;
              if (std::dynamic_pointer_cast<RelJoin>(nodes[i + dist])) {
                break;
              }
            }
          } else if (std::dynamic_pointer_cast<RelJoin>(nodes[i + dist])) {
            input_idx = 1;  // current node is right input of right next join
          }

          auto node_output = std::dynamic_pointer_cast<RelAlgNode>(nodes[i + dist]);
          auto old_input = node_output->getAndOwnInput(input_idx);
          node_output->replaceInput(old_input, node_filter_new);

          nodes.insert(nodes.begin() + i + 1, node_filter_new);  // add new RelFilter to nodes
          dist++;
          std::cout << "------ AFTER findFilterAndProject ------" << std::endl;

          // make RelCompound and insert into nodes
          makeAndInsertCompound(nodes, i + 1, i + dist, cols_to_project);
          std::cout << "------ AFTER makeAndInsertCompound ------" << std::endl;

          std::pair<int32_t, int> table_id_phy = std::make_pair(node_scan->getTableDescriptor()->tableId, node_scan->getId());
          std::cout << "------ table: " << table_id_phy.first << " " << table_id_phy.second << "------" << std::endl;
          size_t node_value = std::stoi(std::to_string(table_id) + "88" + std::to_string(node_scan->getId()));

          ssize_t num_tuples_filtered = executeFilterAndEvaluate(nodes[i + 1].get(), it_table_sizes->second,
                  std::next(it_table_sizes) == table_sizes.end() ? max_size_so_far : -1, table_id_phy, ra_executor, cols_to_project);

          // if num_tuples_filtered is -1, then push-down was reverted
          if (num_tuples_filtered > 0) {  // update rex_input in post-join nodes
            updatePostJoinExprs(nodes, i, i + dist, input_idx, cols_to_project);
            max_size_so_far = std::max(max_size_so_far, (size_t)num_tuples_filtered);
            std::cout << "------ AFTER updatePostJoinExprs ------" << std::endl;
            schema_map_.clear();
            ra_executor->getMaterializedStatuses().emplace(node_value);
          } else {  // remove filter/compound
            auto node_to_erase = node_output->getAndOwnInput(input_idx);
            node_output->replaceInput(node_to_erase, old_input);
            nodes.erase(nodes.begin() + i + 1);
            max_size_so_far = std::max(max_size_so_far, it_table_sizes->second);
            std::cout << "SELECTION WAS REVERTED: " << table_name << std::endl;
            // since selection is reverted, we still don't use pre-calculated sketches

            size_t no_columns = ra_executor->getJoinGraph().find(node_value)->second.size();
            unsigned idx_val = -1;
            for (auto it : ra_executor->getSketchComplexity()) {
              idx_val++;
              if (it.first == node_value) { break; }
            }
            ra_executor->getSketchComplexity()[idx_val] = std::make_pair(node_value, no_columns * it_table_sizes->second);
          }
          auto queue_time_ms = timer_stop(clock_begin);
          std::cout << "time taken for updating sketch using filter: " << queue_time_ms << " ms" << std::endl;
        }
        ++it_table_sizes;  // advance iterator and reset counter to look up next larger relation
        i = 0;
        continue;
      }
      i++;  // keep iterate over nodes until we find the target node
    }
    std::cout << "------ SKETCH-BASED JOIN SIZE ESTIMATION ------" << std::endl;
    std::cout << "------ AFTER pushDownFilterPredicates ------" << std::endl;

    // ra_executor->printAllInfo();

    // this is true only when we create pre-calculated sketch templates
    if (ra_executor->getSessionInfo().get_PRE_PROCESSING_STATUS()) {
      ra_executor->getSessionInfo().saveSketchTemplate(ra_executor->getNodeJoinColumns(), ra_executor->getSketchInstances());
    }

    return true;
  }

  void findSketchFromTemplates(size_t node, size_t table_id, RelAlgExecutor* ra_executor, bool no_selection) {
    ra_executor->getProcessedNodes().emplace(node);

    auto joins = ra_executor->getNodeJoinColumns()[node];
    auto attribute_nbr = joins.size();

    auto row_no = ra_executor->getSessionInfo().getRowNumbers();
    auto bucket_no = ra_executor->getSessionInfo().getBucketSize();
    FAGMS_Sketch* sketch_instance = new FAGMS_Sketch(bucket_no, row_no, attribute_nbr);

    unsigned join_column_index = 0;
    for (auto join_info = joins.begin(); join_info != joins.end(); ++join_info) {
      auto column_id = std::get<1>(*join_info);
      auto join_table_id = std::get<3>(*join_info);
      auto join_column_id = std::get<4>(*join_info);

      std::string key_value = std::to_string(table_id) + "--" + std::to_string(column_id) + "--"
        + std::to_string(join_table_id) + "--" + std::to_string(join_column_id);
      auto sketch_template = ra_executor->getSessionInfo().getTableSketchTemplates()->find(key_value);
      if (sketch_template == ra_executor->getSessionInfo().getTableSketchTemplates()->end()) {

        std::string other_key_value = std::to_string(join_table_id) + "--" + std::to_string(join_column_id) + "--"
          + std::to_string(table_id) + "--" + std::to_string(column_id);

        auto join_new_sketch_seed = ra_executor->getNewSeeds().find(other_key_value);
        if (join_new_sketch_seed != ra_executor->getNewSeeds().end()) {
          sketch_instance->Copy_Sketch_Seeds(&join_new_sketch_seed->second, join_column_index);
        } else {
          std::cout << "TABLE-SKETCH-NOT-FOUND-NO-PREDICATE :: " << key_value << std::endl;
          auto new_seeds = generateSeeds(ra_executor);
          sketch_instance->Copy_Sketch_Seeds(&new_seeds, join_column_index);
          ra_executor->getNewSeeds()[key_value] = new_seeds;
        }
      } else {
        if (no_selection) {
          sketch_instance->Copy_Sketch_Template(sketch_template->second, join_column_index, false);
        } else {
          sketch_instance->Copy_Sketch_Template(sketch_template->second, join_column_index, true);
        }
      }

      join_column_index++;
    }
    ra_executor->getSketchInstances()[node] = sketch_instance;
  }

 private:
  // find the attribute name of a column given by its index in the rel alg node
  // tuple: node_id, column_name and column_id
  std::tuple<unsigned, std::string, unsigned> findColumnNameByIndex(const RelAlgNode* source, unsigned c_id) {
    std::vector<std::pair<unsigned, unsigned>> t_num_cols;
    std::vector<std::string> schema;
    std::tie(t_num_cols, schema) = getSchemaFromSource(source);
    CHECK(!t_num_cols.empty());

    std::tuple<int, std::string, int> c_info;
    unsigned num_cols_so_far = 0;
    for (auto it = t_num_cols.begin(); it != t_num_cols.end(); ++it) {
      if (c_id < num_cols_so_far + it->second) {
        auto col_id = c_id - num_cols_so_far;
        c_info = std::make_tuple(it->first, schema[c_id], col_id);
        break;
      } else {
        num_cols_so_far += it->second;
      }
    }

    CHECK_GT(std::get<0>(c_info), -1);

    return c_info;
  }

  // get the schema of a node in the relational algebra tree
  std::pair<std::vector<std::pair<unsigned, unsigned>>, std::vector<std::string>> getSchemaFromSource(
      const RelAlgNode* node) {
    auto it = schema_map_.find(node);
    if (it != schema_map_.end())
      return it->second;
    else {
      auto schema_new = buildSchemaFromSource(node);
      schema_map_[node] = schema_new;
      return schema_new;
    }
  }

  // build schema for a node
  // schema consists of a pair of (node_id, num_atts) pairs and a vector of att names
  std::pair<std::vector<std::pair<unsigned, unsigned>>, std::vector<std::string>> buildSchemaFromSource(
      const RelAlgNode* node) {
    std::pair<std::vector<std::pair<unsigned, unsigned>>, std::vector<std::string>> schema;
    std::vector<std::pair<unsigned, unsigned>> t_num_cols;

    // RelScan
    const auto node_scan = dynamic_cast<const RelScan*>(node);
    if (node_scan) {
      t_num_cols.emplace_back(node_scan->getId(), node_scan->size());
      schema = std::make_pair(t_num_cols, node_scan->getFieldNames());
    }

    // RelFilter
    const auto node_filter = dynamic_cast<const RelFilter*>(node);
    if (node_filter) {
      schema = buildSchemaFromSource(node_filter->getInput(0));
    }

    // RelProject
    const auto node_project = dynamic_cast<const RelProject*>(node);
    if (node_project) {
      unsigned int t_source_id = findBaseTableId(node_project->getInput(0));
      t_num_cols.emplace_back(t_source_id, node_project->size());
      schema = std::make_pair(t_num_cols, node_project->getFields());
    }

    // RelAggregate
    const auto node_aggregate = dynamic_cast<const RelAggregate*>(node);
    if (node_aggregate) {
      unsigned int t_source_id = findBaseTableId(node_aggregate->getInput(0));
      t_num_cols.emplace_back(t_source_id, node_aggregate->size());
      schema = std::make_pair(t_num_cols, node_aggregate->getFields());
    }

    // RelSort
    const auto node_sort = dynamic_cast<const RelSort*>(node);
    if (node_sort) {
      schema = buildSchemaFromSource(node_sort->getInput(0));
    }

    // RelJoin
    const auto node_join = dynamic_cast<const RelJoin*>(node);
    if (node_join) {
      std::vector<std::pair<unsigned, unsigned>> t_num_cols_lhs, t_num_cols_rhs;
      std::vector<std::string> schema_lhs, schema_rhs;
      std::tie(t_num_cols_lhs, schema_lhs) = buildSchemaFromSource(node_join->getInput(0));
      std::tie(t_num_cols_rhs, schema_rhs) = buildSchemaFromSource(node_join->getInput(1));

      t_num_cols_lhs.insert(std::end(t_num_cols_lhs), std::begin(t_num_cols_rhs), std::end(t_num_cols_rhs));
      schema_lhs.insert(std::end(schema_lhs), std::begin(schema_rhs), std::end(schema_rhs));
      schema = std::make_pair(t_num_cols_lhs, schema_lhs);
    }

    // RelCompound
    const auto node_compound = dynamic_cast<const RelCompound*>(node);
    if (node_compound) {
      unsigned int t_source_id = findBaseTableId(node_compound->getInput(0));
      t_num_cols.emplace_back(t_source_id, node_compound->size());
      schema = std::make_pair(t_num_cols, node_compound->getFields());
    }

    CHECK(node_scan || node_filter || node_project || node_aggregate || node_sort || node_join || node_compound);

    return schema;
  }

  // recursive method to find the base table on a left subtree
  unsigned int findBaseTableId(const RelAlgNode* node) {
    const auto node_scan = dynamic_cast<const RelScan*>(node);
    if (node_scan)
      return node_scan->getId();
    else
      return findBaseTableId(node->getInput(0));
  }

  std::unique_ptr<const RexScalar> findFilterAndProject(
      std::vector<std::shared_ptr<RelAlgNode>>& nodes,
      size_t scan_idx,
      std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    std::unique_ptr<const RexScalar> filter_expr_new;
    for (auto it_nodes = nodes.rbegin(); it_nodes != nodes.rend(); ++it_nodes) {
      auto node_project = std::dynamic_pointer_cast<RelProject>(*it_nodes);
      if (node_project) {
        if (dynamic_cast<const RelAggregate*>(node_project->getInput(0)) ||
            dynamic_cast<const RelProject*>(node_project->getInput(0))) {
          continue;
        }
        for (size_t i = 0; i < node_project->size(); i++) {
          getProjectFromRex(node_project->getProjectAt(i), nodes[scan_idx], cols_to_project);
        }
        continue;
      }
      auto node_filter = std::dynamic_pointer_cast<RelFilter>(*it_nodes);
      if (node_filter) {
        filter_expr_new = findFilterAndProjectFromRex(node_filter->getCondition(), nodes[scan_idx], cols_to_project);
        continue;
      }
      if (std::dynamic_pointer_cast<RelJoin>(*it_nodes)) {
        break;
      }
    }
    return filter_expr_new;
  }

  void getProjectFromRex(const RexScalar* rex,
                         std::shared_ptr<RelAlgNode>& node_scan,
                         std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    const auto rex_input = dynamic_cast<const RexInput*>(rex);
    if (rex_input) {
      auto c_info = getColumnInfoFromScan(rex_input, node_scan->getId());
      if (!c_info.second.empty()) {
        auto it = std::lower_bound(cols_to_project.begin(), cols_to_project.end(), c_info, [](auto lhs, auto rhs) {
          return lhs.first < rhs.first;
        });
        if (it == cols_to_project.end() || !(*it == c_info)) {
          cols_to_project.emplace(it, std::move(c_info));
        }
      }
    }
    const auto rex_operator = dynamic_cast<const RexOperator*>(rex);
    if (rex_operator) {
      for (size_t i = 0; i < rex_operator->size(); i++) {
        const auto operand = rex_operator->getOperand(i);
        getProjectFromRex(operand, node_scan, cols_to_project);
      }
    }

    const auto rex_case = dynamic_cast<const RexCase*>(rex);
    if (rex_case) {
      for (size_t i = 0; i < rex_case->branchCount(); i++) {
        getProjectFromRex(rex_case->getWhen(i), node_scan, cols_to_project);
        getProjectFromRex(rex_case->getThen(i), node_scan, cols_to_project);
      }
      getProjectFromRex(rex_case->getElse(), node_scan, cols_to_project);
    }
  }

  std::unique_ptr<const RexScalar> findFilterAndProjectFromRex(
      const RexScalar* rex,
      std::shared_ptr<RelAlgNode>& node_scan,
      std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    const auto rex_input = dynamic_cast<const RexInput*>(rex);
    if (rex_input) {
      auto c_info = getColumnInfoFromScan(rex_input, node_scan->getId());
      if (!c_info.second.empty()) {
        return std::unique_ptr<const RexScalar>(new RexInput(node_scan.get(), c_info.first));
      } else {
        return nullptr;
      }
    }
    const auto rex_literal = dynamic_cast<const RexLiteral*>(rex);
    if (rex_literal) {
      return rex_literal->deepCopy();
    }
    const auto rex_func_operator = dynamic_cast<const RexFunctionOperator*>(rex);
    if (rex_func_operator) {
      std::vector<std::unique_ptr<const RexScalar>> operands_new;
      std::vector<std::pair<unsigned, std::string>> c_infos_tmp;
      auto oper_size = rex_func_operator->size();
      for (size_t i = 0; i < oper_size; i++) {
        auto operand_new = findFilterAndProjectFromFuncOper(rex_func_operator->getOperand(i), node_scan, c_infos_tmp);
        if (operand_new) {
          operands_new.push_back(std::move(operand_new));
        }
      }
      if (operands_new.size() != oper_size) {
        for (auto& c_info : c_infos_tmp) {
          auto it = std::lower_bound(cols_to_project.begin(), cols_to_project.end(), c_info, [](auto lhs, auto rhs) {
            return lhs.first < rhs.first;
          });
          if (it == cols_to_project.end() || !(*it == c_info)) {
            cols_to_project.emplace(it, std::move(c_info));
          }
        }
        return nullptr;
      } else {
        return std::unique_ptr<const RexScalar>(
            new RexFunctionOperator(rex_func_operator->getName(), operands_new, rex_func_operator->getType()));
      }
    }
    const auto rex_operator = dynamic_cast<const RexOperator*>(rex);
    if (rex_operator) {
      std::vector<std::unique_ptr<const RexScalar>> operands_new;
      auto oper_size = rex_operator->size();
      if (oper_size >= 2) {
        std::vector<std::pair<unsigned, std::string>> c_infos_tmp;
        unsigned t_id_prev = 0;
        bool has_joint_condition = false;
        for (size_t i = 0; i < oper_size; i++) {
          const auto rex_input = dynamic_cast<const RexInput*>(rex_operator->getOperand(i));
          if (rex_input) {
            auto t_id = std::get<0>(findColumnNameByIndex(rex_input->getSourceNode(), rex_input->getIndex()));
            if (t_id_prev == 0) {
              t_id_prev = t_id;
            } else if (!has_joint_condition && t_id_prev != t_id) {
              has_joint_condition = true;
            }
            auto c_info = getColumnInfoFromScan(rex_input, node_scan->getId());
            if (!c_info.second.empty()) {
              c_infos_tmp.push_back(std::move(c_info));
            }
          }
        }
        if (has_joint_condition) {
          for (auto& c_info : c_infos_tmp) {
            auto it = std::lower_bound(cols_to_project.begin(), cols_to_project.end(), c_info, [](auto lhs, auto rhs) {
              return lhs.first < rhs.first;
            });
            if (it == cols_to_project.end() || !(*it == c_info)) {
              cols_to_project.emplace(it, std::move(c_info));
            }
          }
          return nullptr;
        }
      }
      for (size_t i = 0; i < oper_size; i++) {
        auto operand_new = findFilterAndProjectFromRex(rex_operator->getOperand(i), node_scan, cols_to_project);
        if (operand_new) {
          operands_new.push_back(std::move(operand_new));
        }
      }
      if (operands_new.size() > 1) {
        return std::unique_ptr<const RexScalar>(
            new RexOperator(rex_operator->getOperator(), operands_new, rex_operator->getType()));
      } else if (operands_new.size() == 1) {
        // preserve the internal operator in case of NOT and ISNOTNULL operators
        if (rex_operator->getOperator() == kNOT || rex_operator->getOperator() == kISNOTNULL
                                                   || rex_operator->getOperator() == kISNULL) {
          return std::unique_ptr<const RexScalar>(
              new RexOperator(rex_operator->getOperator(), operands_new, rex_operator->getType()));
        } else if (dynamic_cast<const RexOperator*>(operands_new[0].get()) ||
                   dynamic_cast<const RexCase*>(operands_new[0].get())) {
          return std::move(operands_new[0]);
        } else {
          return nullptr;
        }
      } else {
        return nullptr;
      }
    }
    const auto rex_case = dynamic_cast<const RexCase*>(rex);
    if (rex_case) {
      std::vector<std::pair<std::unique_ptr<const RexScalar>, std::unique_ptr<const RexScalar>>> expr_pair_list_new;
      for (size_t i = 0; i < rex_case->branchCount(); i++) {
        auto oper_when_new = findFilterAndProjectFromRex(rex_case->getWhen(i), node_scan, cols_to_project);
        auto oper_then_new = findFilterAndProjectFromRex(rex_case->getThen(i), node_scan, cols_to_project);
        if (oper_when_new && oper_then_new) {
          auto expr_pair_new = std::make_pair(std::move(oper_when_new), std::move(oper_then_new));
          expr_pair_list_new.push_back(std::move(expr_pair_new));
        }
      }
      if (!expr_pair_list_new.empty()) {
        auto else_expr_new = findFilterAndProjectFromRex(rex_case->getElse(), node_scan, cols_to_project);
        if (else_expr_new) {
          return std::unique_ptr<const RexScalar>(new RexCase(expr_pair_list_new, else_expr_new));
        }
      }
      return nullptr;
    }
    const auto rex_ref = dynamic_cast<const RexRef*>(rex);
    if (rex_ref) {
      return rex_ref->deepCopy();
    }
    const auto rex_sub_query = dynamic_cast<const RexSubQuery*>(rex);
    if (rex_sub_query) {
      return nullptr;
    }
    CHECK(rex_input || rex_literal || rex_operator || rex_case || rex_ref || rex_sub_query);
    return nullptr;
  }

  std::unique_ptr<const RexScalar> findFilterAndProjectFromFuncOper(
      const RexScalar* rex,
      std::shared_ptr<RelAlgNode>& node_scan,
      std::vector<std::pair<unsigned, std::string>>& c_infos_tmp) {
    const auto rex_input = dynamic_cast<const RexInput*>(rex);
    if (rex_input) {
      auto c_info = getColumnInfoFromScan(rex_input, node_scan->getId());
      if (!c_info.second.empty()) {
        c_infos_tmp.push_back(std::move(c_info));
        return std::unique_ptr<const RexScalar>(new RexInput(node_scan.get(), c_info.first));
      } else {
        return nullptr;
      }
    }
    const auto rex_literal = dynamic_cast<const RexLiteral*>(rex);
    if (rex_literal) {
      return rex_literal->deepCopy();
    }
    const auto rex_func_operator = dynamic_cast<const RexFunctionOperator*>(rex);
    if (rex_func_operator) {
      std::vector<std::unique_ptr<const RexScalar>> operands_new;
      auto oper_size = rex_func_operator->size();
      for (size_t i = 0; i < oper_size; i++) {
        auto operand_new = findFilterAndProjectFromFuncOper(rex_func_operator->getOperand(i), node_scan, c_infos_tmp);
        if (operand_new) {
          operands_new.push_back(std::move(operand_new));
        }
      }
      if (operands_new.size() != oper_size) {
        return nullptr;
      } else {
        return std::unique_ptr<const RexScalar>(
            new RexFunctionOperator(rex_func_operator->getName(), operands_new, rex_func_operator->getType()));
      }
    }
    CHECK(rex_input || rex_literal || rex_func_operator);
    return nullptr;
  }

  std::pair<unsigned, std::string> getColumnInfoFromScan(const RexInput* rex_input, unsigned scan_id) {
    const auto source = rex_input->getSourceNode();
    const auto c_id = rex_input->getIndex();
    std::vector<std::pair<unsigned, unsigned>> t_num_cols;
    std::vector<std::string> schema;
    std::tie(t_num_cols, schema) = getSchemaFromSource(source);
    unsigned num_cols_so_far = 0;
    for (auto t_num_col : t_num_cols) {
      if (t_num_col.first == scan_id) {
        if (c_id < (num_cols_so_far + t_num_col.second) && c_id >= num_cols_so_far) {
          return std::make_pair(c_id - num_cols_so_far, schema[c_id]);
        }
        break;
      } else {
        num_cols_so_far += t_num_col.second;
      }
    }
    // this input doesn't belong to the given table
    return std::make_pair(-1, "");
  }

  void makeAndInsertCompound(std::vector<std::shared_ptr<RelAlgNode>>& nodes,
                             size_t filter_idx,
                             size_t output_idx,
                             std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    std::unique_ptr<const RexScalar> filter_rex;
    std::vector<std::unique_ptr<const RexScalar>> scalar_sources;
    size_t groupby_count{0};
    std::vector<std::string> fields;
    std::vector<const RexAgg*> agg_exprs;
    std::vector<const Rex*> target_exprs;
    // extract filter
    const auto node_filter = std::dynamic_pointer_cast<RelFilter>(nodes[filter_idx]);
    if (node_filter) {
      CHECK(!filter_rex);
      filter_rex.reset(node_filter->getAndReleaseCondition());
      CHECK(filter_rex);
    } else {
      CHECK(false);
    }
    // extract project
    for (auto col : cols_to_project) {
      auto rex_col = std::unique_ptr<const RexScalar>(new RexInput(nodes[filter_idx]->getInput(0), col.first));
      scalar_sources.push_back(std::move(rex_col));
      target_exprs.push_back(scalar_sources.back().get());
      fields.push_back(col.second);
    }
    // create compound
    auto node_compound = std::make_shared<RelCompound>(
        filter_rex, target_exprs, groupby_count, agg_exprs, fields, scalar_sources, false);
    CHECK_EQ(size_t(1), nodes[filter_idx]->inputCount());
    node_compound->addManagedInput(nodes[filter_idx]->getAndOwnInput(0));
    nodes[output_idx]->replaceInput(nodes[filter_idx], node_compound);
    // remove filter and project and insert compound
    nodes.erase(nodes.begin() + filter_idx);
    nodes.insert(nodes.begin() + filter_idx, node_compound);
  }

  RelCompound makeCompound(RelAlgNode* node, std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    std::unique_ptr<const RexScalar> filter_rex;
    std::vector<std::unique_ptr<const RexScalar>> scalar_sources;
    size_t groupby_count{0};
    std::vector<std::string> fields;
    std::vector<const RexAgg*> agg_exprs;
    std::vector<const Rex*> target_exprs;

    // extract project
    for (auto col : cols_to_project) {
      auto rex_col = std::unique_ptr<const RexScalar>(new RexInput(node->getInput(0), col.first));
      scalar_sources.push_back(std::move(rex_col));
      target_exprs.push_back(scalar_sources.back().get());
      fields.push_back(col.second);
    }
    // create compound
    RelCompound node_compound =
        RelCompound(filter_rex, target_exprs, groupby_count, agg_exprs, fields, scalar_sources, false);
    CHECK_EQ(size_t(1), node->inputCount());
    node_compound.addManagedInput(node->getAndOwnInput(0));
    return node_compound;
  }

  ssize_t executeFilterAndEvaluate(const RelAlgNode* node,
                                   size_t num_tuples,
                                   ssize_t max_size_so_far,
                                   const std::pair<int32_t, int> table_id_phy,
                                   RelAlgExecutor* ra_executor,
                                   std::vector<std::pair<unsigned, std::string>> cols_to_project) {
    auto executor = Executor::getExecutor(ra_executor->getSessionInfo().get_catalog().get_currentDB().dbId);
    size_t fpd_max_count = num_tuples * ra_executor->getSessionInfo().get_FPD_MAX_SELECTIVITY();
    if (fpd_max_count > ra_executor->getSessionInfo().get_FPD_MAX_SIZE())
        fpd_max_count = ra_executor->getSessionInfo().get_FPD_MAX_SIZE();
//    if (max_size_so_far >= 0) {
//      fpd_max_count = std::min(fpd_max_count, (size_t)max_size_so_far);
//    }
    auto execution_result = ra_executor->executeRelAlgQueryFPD(node, fpd_max_count, table_id_phy);
    // sample code for reading raw data from ExecutionResult
    // need cols_to_project to match with join attributes for sketch update // TODO: elaboration is needed.
    if (!execution_result.getTargetsMeta().empty()) {
      return ra_executor->addPushDownFilter(-node->getId(), execution_result);
    } else {
      return -1;
    }
  }

  void updatePostJoinExprs(std::vector<std::shared_ptr<RelAlgNode>>& nodes,
                           size_t scan_idx,
                           size_t output_idx,
                           size_t input_idx,
                           std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    for (auto it_nodes = nodes.rbegin(); it_nodes != nodes.rend(); ++it_nodes) {
      auto node_project = std::dynamic_pointer_cast<RelProject>(*it_nodes);
      if (node_project) {
        if (dynamic_cast<const RelAggregate*>(node_project->getInput(0)) ||
            dynamic_cast<const RelProject*>(node_project->getInput(0))) {
          continue;
        }
        std::vector<std::unique_ptr<const RexScalar>> exprs_new;
        for (size_t i = 0; i < node_project->size(); i++) {
          auto rex = node_project->getProjectAtAndRelease(i);
          auto rex_new = buildNewProjectExpr(rex, nodes[scan_idx], nodes[output_idx], input_idx, cols_to_project);
          exprs_new.push_back(std::move(rex_new));
        }
        node_project->setExpressions(exprs_new);
        continue;
      }
      auto node_filter = std::dynamic_pointer_cast<RelFilter>(*it_nodes);
      if (node_filter) {
        auto rex = node_filter->getAndReleaseCondition();
        auto rex_new = buildNewFilterExpr(rex, nodes[scan_idx], nodes[output_idx], input_idx, cols_to_project);
        // empty filter expr handling
        // 1. insert always-true condition: easy
        // if(!rex_new) {
        //   rex_new = std::unique_ptr<const RexScalar>(
        //     new RexLiteral(true, kBOOLEAN, kBOOLEAN, -1, 1, -1, 1));
        // }
        // node_filter->setCondition(rex_new);
        // 2. remove filter from nodes and RexInputs and connect filter's child and parent
        // pro: save time to execute always-true filter
        // con: take extra time to modify tree
        if (rex_new) {
          node_filter->setCondition(rex_new);
        } else {
          removeEmptyFilter(nodes, node_filter.get());
        }
        continue;
      }
      if (std::dynamic_pointer_cast<RelJoin>(*it_nodes)) {
        break;
      }
    }
  }

  std::unique_ptr<const RexScalar> buildNewProjectExpr(const RexScalar* rex,
                                                       std::shared_ptr<RelAlgNode>& node_scan,
                                                       std::shared_ptr<RelAlgNode>& node_output,
                                                       size_t input_idx,
                                                       std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    const auto rex_input = dynamic_cast<const RexInput*>(rex);
    if (rex_input) {
      auto source = rex_input->getSourceNode();
      auto c_id = rex_input->getIndex();
      if (!hasConnection(node_output->getInput(input_idx), source) &&
          !hasConnection(source, node_output->getInput(input_idx))) {
        return std::unique_ptr<const RexScalar>(new RexInput(source, c_id));
      }
      std::vector<std::pair<unsigned, unsigned>> t_num_cols;
      std::vector<std::string> schema;
      std::tie(t_num_cols, schema) = getSchemaFromSource(source);
      int num_cols_so_far = 0;
      int num_modifier = 0;
      for (auto it = t_num_cols.begin(); it != t_num_cols.end(); ++it) {
        if (c_id < num_cols_so_far + it->second) {
          if (it->first == node_scan->getId()) {
            auto it_find =
                std::find_if(cols_to_project.begin(), cols_to_project.end(), [&c_id, &num_cols_so_far](auto& col) {
                  return col.first == (c_id - num_cols_so_far);
                });
            CHECK(it_find != cols_to_project.end());
            auto c_id_new = std::distance(cols_to_project.begin(), it_find) + num_cols_so_far;
            if (node_output->getInput(input_idx)->getInput(0) == source) {
              return std::unique_ptr<const RexScalar>(new RexInput(node_output->getInput(input_idx), c_id_new));
            } else {
              return std::unique_ptr<const RexScalar>(new RexInput(source, c_id_new));
            }
          } else {
            auto c_id_new = c_id - num_modifier;
            return std::unique_ptr<const RexScalar>(new RexInput(source, c_id_new));
          }
        } else {
          num_cols_so_far += it->second;
          if (it->first == node_scan->getId()) {
            num_modifier = it->second - cols_to_project.size();
          }
        }
      }
      CHECK(false);  // something went wrong...
    }
    const auto rex_literal = dynamic_cast<const RexLiteral*>(rex);
    if (rex_literal) {
      return rex_literal->deepCopy();
    }
    const auto rex_operator = dynamic_cast<const RexOperator*>(rex);
    if (rex_operator) {
      std::vector<std::unique_ptr<const RexScalar>> operands_new;
      for (size_t i = 0; i < rex_operator->size(); i++) {
        auto oper_new = buildNewProjectExpr(
            rex_operator->getOperandAndRelease(i), node_scan, node_output, input_idx, cols_to_project);
        operands_new.push_back(std::move(oper_new));
      }
      const auto rex_func_operator = dynamic_cast<const RexFunctionOperator*>(rex);
      if (rex_func_operator) {
        return std::unique_ptr<const RexScalar>(
            new RexFunctionOperator(rex_func_operator->getName(), operands_new, rex_func_operator->getType()));
      } else {
        return std::unique_ptr<const RexScalar>(
            new RexOperator(rex_operator->getOperator(), operands_new, rex_operator->getType()));
      }
    }
    const auto rex_case = dynamic_cast<const RexCase*>(rex);
    if (rex_case) {
      std::vector<std::pair<std::unique_ptr<const RexScalar>, std::unique_ptr<const RexScalar>>> expr_pair_list_new;
      for (size_t i = 0; i < rex_case->branchCount(); i++) {
        auto oper_when_new =
            buildNewProjectExpr(rex_case->getWhen(i), node_scan, node_output, input_idx, cols_to_project);
        auto oper_then_new =
            buildNewProjectExpr(rex_case->getThen(i), node_scan, node_output, input_idx, cols_to_project);
        auto expr_pair_new = std::make_pair(std::move(oper_when_new), std::move(oper_then_new));
        expr_pair_list_new.push_back(std::move(expr_pair_new));
      }
      auto else_expr_new = buildNewProjectExpr(rex_case->getElse(), node_scan, node_output, input_idx, cols_to_project);
      return std::unique_ptr<const RexScalar>(new RexCase(expr_pair_list_new, else_expr_new));
    }
    const auto rex_ref = dynamic_cast<const RexRef*>(rex);
    if (rex_ref) {
      return rex_ref->deepCopy();
    }
    const auto rex_sub_query = dynamic_cast<const RexSubQuery*>(rex);
    if (rex_sub_query) {
      return nullptr;
    }
    CHECK(rex_input || rex_literal || rex_operator || rex_case || rex_ref || rex_sub_query);
    return nullptr;
  }

  std::unique_ptr<const RexScalar> buildNewFilterExpr(const RexScalar* rex,
                                                      std::shared_ptr<RelAlgNode>& node_scan,
                                                      std::shared_ptr<RelAlgNode>& node_output,
                                                      size_t input_idx,
                                                      std::vector<std::pair<unsigned, std::string>>& cols_to_project) {
    const auto rex_input = dynamic_cast<const RexInput*>(rex);
    if (rex_input) {
      auto source = rex_input->getSourceNode();
      auto c_id = rex_input->getIndex();
      if (!hasConnection(node_output->getInput(input_idx), source) &&
          !hasConnection(source, node_output->getInput(input_idx))) {
        return std::unique_ptr<const RexScalar>(new RexInput(source, c_id));
      }
      std::vector<std::pair<unsigned, unsigned>> t_num_cols;
      std::vector<std::string> schema;
      std::tie(t_num_cols, schema) = getSchemaFromSource(source);
      int num_cols_so_far = 0;
      int num_modifier = 0;
      for (auto it = t_num_cols.begin(); it != t_num_cols.end(); ++it) {
        if (c_id < num_cols_so_far + it->second) {
          if (it->first == node_scan->getId()) {
            auto it_find =
                std::find_if(cols_to_project.begin(), cols_to_project.end(), [&c_id, &num_cols_so_far](auto& col) {
                  return col.first == (c_id - num_cols_so_far);
                });
            if (it_find == cols_to_project.end()) {
              return nullptr;  // this column is just for filter, which should be pushed down already
            }
            auto c_id_new = std::distance(cols_to_project.begin(), it_find) + num_cols_so_far;
            if (node_output->getInput(input_idx)->getInput(0)->getId() == source->getId()) {
              return std::unique_ptr<const RexScalar>(new RexInput(node_output->getInput(input_idx), c_id_new));
            } else {
              return std::unique_ptr<const RexScalar>(new RexInput(source, c_id_new));
            }
          } else {
            auto c_id_new = c_id - num_modifier;
            return std::unique_ptr<const RexScalar>(new RexInput(source, c_id_new));
          }
        } else {
          num_cols_so_far += it->second;
          if (it->first == node_scan->getId()) {
            num_modifier = it->second - cols_to_project.size();
          }
        }
      }
    }
    const auto rex_literal = dynamic_cast<const RexLiteral*>(rex);
    if (rex_literal) {
      return rex_literal->deepCopy();
    }
    const auto rex_func_operator = dynamic_cast<const RexFunctionOperator*>(rex);
    if (rex_func_operator) {
      std::vector<std::unique_ptr<const RexScalar>> operands_new;
      auto oper_size = rex_func_operator->size();
      for (size_t i = 0; i < oper_size; i++) {
        auto operand_new =
            buildNewFilterExpr(rex_func_operator->getOperand(i), node_scan, node_output, input_idx, cols_to_project);
        if (operand_new) {
          operands_new.push_back(std::move(operand_new));
        }
      }
      if (oper_size == operands_new.size()) {
        return std::unique_ptr<const RexScalar>(
            new RexFunctionOperator(rex_func_operator->getName(), operands_new, rex_func_operator->getType()));
      } else {
        return nullptr;
      }
    }
    const auto rex_operator = dynamic_cast<const RexOperator*>(rex);
    if (rex_operator) {
      std::vector<std::unique_ptr<const RexScalar>> operands_new;
      auto oper_size = rex_operator->size();
      if (oper_size >= 2) {
        std::vector<std::pair<unsigned, std::string>> c_infos_tmp;
        unsigned t_id_prev = 0;
        bool has_joint_condition = false;
        for (size_t i = 0; i < oper_size; i++) {
          const auto rex_input = dynamic_cast<const RexInput*>(rex_operator->getOperand(i));
          if (rex_input) {
            auto t_id = std::get<0>(findColumnNameByIndex(rex_input->getSourceNode(), rex_input->getIndex()));
            if (t_id_prev == 0) {
              t_id_prev = t_id;
            } else if (!has_joint_condition && t_id_prev != t_id) {
              has_joint_condition = true;
            }
            auto c_info = getColumnInfoFromScan(rex_input, node_scan->getId());
            if (!c_info.second.empty()) {
              c_infos_tmp.push_back(std::move(c_info));
            }
          }
        }
        if (!has_joint_condition) {
          if (!c_infos_tmp.empty()) {  // at least a single column is on this table
            return nullptr;
          }
        }
      }
      for (size_t i = 0; i < oper_size; i++) {
        auto operand_new =
            buildNewFilterExpr(rex_operator->getOperand(i), node_scan, node_output, input_idx, cols_to_project);
        if (operand_new) {
          operands_new.push_back(std::move(operand_new));
        }
      }
      if (operands_new.size() > 1) {
        return std::unique_ptr<const RexScalar>(
            new RexOperator(rex_operator->getOperator(), operands_new, rex_operator->getType()));
      } else if (operands_new.size() == 1) {
        // preserve the internal operator in case of NOT and ISNOTNULL operators
        if (rex_operator->getOperator() == kNOT || rex_operator->getOperator() == kISNOTNULL
                                                   || rex_operator->getOperator() == kISNULL) {
          return std::unique_ptr<const RexScalar>(
              new RexOperator(rex_operator->getOperator(), operands_new, rex_operator->getType()));
        } else if (dynamic_cast<const RexOperator*>(operands_new[0].get()) ||
                   dynamic_cast<const RexCase*>(operands_new[0].get())) {
          return std::move(operands_new[0]);
        } else {
          return nullptr;
        }
      } else {
        return nullptr;
      }
    }
    const auto rex_case = dynamic_cast<const RexCase*>(rex);
    if (rex_case) {
      std::vector<std::pair<std::unique_ptr<const RexScalar>, std::unique_ptr<const RexScalar>>> expr_pair_list_new;
      for (size_t i = 0; i < rex_case->branchCount(); i++) {
        auto oper_when_new =
            buildNewFilterExpr(rex_case->getWhen(i), node_scan, node_output, input_idx, cols_to_project);
        auto oper_then_new =
            buildNewFilterExpr(rex_case->getThen(i), node_scan, node_output, input_idx, cols_to_project);
        auto expr_pair_new = std::make_pair(std::move(oper_when_new), std::move(oper_then_new));
        expr_pair_list_new.push_back(std::move(expr_pair_new));
      }
      auto else_expr_new = buildNewFilterExpr(rex_case->getElse(), node_scan, node_output, input_idx, cols_to_project);
      return std::unique_ptr<const RexScalar>(new RexCase(expr_pair_list_new, else_expr_new));
    }
    const auto rex_ref = dynamic_cast<const RexRef*>(rex);
    if (rex_ref) {
      return rex_ref->deepCopy();
    }
    const auto rex_sub_query = dynamic_cast<const RexSubQuery*>(rex);
    if (rex_sub_query) {
      return nullptr;
    }
    CHECK(rex_input || rex_literal || rex_operator || rex_case || rex_ref || rex_sub_query);
    return nullptr;
  }

  bool hasConnection(const RelAlgNode* lhs, const RelAlgNode* rhs) {
    if (lhs == rhs) {
      return true;
    }
    for (size_t i = 0; i < rhs->inputCount(); i++) {
      if (lhs == rhs->getInput(i)) {
        return true;
      } else if (hasConnection(lhs, rhs->getInput(i))) {
        return true;
      }
    }
    return false;
  }

  void removeEmptyFilter(std::vector<std::shared_ptr<RelAlgNode>>& nodes, const RelAlgNode* node_filter) {
    for (auto it_nodes = nodes.rbegin(); it_nodes != nodes.rend(); ++it_nodes) {
      auto node_project = std::dynamic_pointer_cast<RelProject>(*it_nodes);
      if (node_project) {
        if (node_project->getInput(0) == node_filter) {
          std::vector<std::unique_ptr<const RexScalar>> exprs_new;
          for (size_t i = 0; i < node_project->size(); i++) {
            auto rex = node_project->getProjectAtAndRelease(i);
            auto rex_new = replaceEmptyFilterSource(rex, node_filter, node_filter->getInput(0));
            exprs_new.push_back(std::move(rex_new));
          }
          node_project->setExpressions(exprs_new);
          node_project->replaceInput(node_project->getAndOwnInput(0), node_filter->getAndOwnInput(0));
          nodes.erase(it_nodes.base() - 2);
          return;
        }
      }
    }
  }

  std::unique_ptr<const RexScalar> replaceEmptyFilterSource(const RexScalar* rex,
                                                            const RelAlgNode* source_old,
                                                            const RelAlgNode* source_new) {
    const auto rex_input = dynamic_cast<const RexInput*>(rex);
    if (rex_input) {
      if (rex_input->getSourceNode() == source_old) {
        return std::unique_ptr<const RexScalar>(new RexInput(source_new, rex_input->getIndex()));
      }
    }
    const auto rex_literal = dynamic_cast<const RexLiteral*>(rex);
    if (rex_literal) {
      return rex_literal->deepCopy();
    }
    const auto rex_operator = dynamic_cast<const RexOperator*>(rex);
    if (rex_operator) {
      std::vector<std::unique_ptr<const RexScalar>> operands_new;
      for (size_t i = 0; i < rex_operator->size(); i++) {
        auto oper_new = replaceEmptyFilterSource(rex_operator->getOperandAndRelease(i), source_old, source_new);
        operands_new.push_back(std::move(oper_new));
      }
      const auto rex_func_operator = dynamic_cast<const RexFunctionOperator*>(rex);
      if (rex_func_operator) {
        return std::unique_ptr<const RexScalar>(
            new RexFunctionOperator(rex_func_operator->getName(), operands_new, rex_func_operator->getType()));
      } else {
        return std::unique_ptr<const RexScalar>(
            new RexOperator(rex_operator->getOperator(), operands_new, rex_operator->getType()));
      }
    }
    const auto rex_case = dynamic_cast<const RexCase*>(rex);
    if (rex_case) {
      std::vector<std::pair<std::unique_ptr<const RexScalar>, std::unique_ptr<const RexScalar>>> expr_pair_list_new;
      for (size_t i = 0; i < rex_case->branchCount(); i++) {
        auto oper_when_new = replaceEmptyFilterSource(rex_case->getWhen(i), source_old, source_new);
        auto oper_then_new = replaceEmptyFilterSource(rex_case->getThen(i), source_old, source_new);
        auto expr_pair_new = std::make_pair(std::move(oper_when_new), std::move(oper_then_new));
        expr_pair_list_new.push_back(std::move(expr_pair_new));
      }
      auto else_expr_new = replaceEmptyFilterSource(rex_case->getElse(), source_old, source_new);
      return std::unique_ptr<const RexScalar>(new RexCase(expr_pair_list_new, else_expr_new));
    }
    const auto rex_ref = dynamic_cast<const RexRef*>(rex);
    if (rex_ref) {
      return rex_ref->deepCopy();
    }
    const auto rex_sub_query = dynamic_cast<const RexSubQuery*>(rex);
    if (rex_sub_query) {
      return nullptr;
    }
    CHECK(rex_input || rex_literal || rex_operator || rex_case || rex_ref || rex_sub_query);
    return nullptr;
  }

  // schema at each node in the query execution tree
  std::unordered_map<const RelAlgNode*, std::pair<std::vector<std::pair<unsigned, unsigned>>, std::vector<std::string>>>
      schema_map_;

  // generating seeds for sketches
  std::pair<Xi_CW2B*, Xi_EH3*> generateSeeds(RelAlgExecutor* ra_executor) {
    auto row_no = ra_executor->getSessionInfo().getRowNumbers();
    Xi_CW2B* xi_b_seed = new Xi_CW2B[row_no];
    Xi_EH3* xi_pm1_seed = new Xi_EH3[row_no];

    unsigned int I1, I2;

    for (unsigned i = 0; i < row_no; i++) {
      I1 = RandomGenerate();
      I2 = RandomGenerate();
      Xi_CW2B tb(I1, I2, ra_executor->getSessionInfo().getBucketSize());
      xi_b_seed[i] = tb;

      I1 = RandomGenerate();
      I2 = RandomGenerate();
      Xi_EH3 tp(I1, I2);
      xi_pm1_seed[i] = tp;
    }

    return std::make_pair(xi_b_seed, xi_pm1_seed);
  }
};

bool push_down_filter_predicates_sketch(std::vector<std::shared_ptr<RelAlgNode>>& nodes, RelAlgExecutor* ra_executor) {
  std::cout << "\npd start time ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
  auto all_push_down_begin = timer_start();
  ra_executor->set_query_start_time(all_push_down_begin);

  FilterPushDownSketch fpd;
  std::vector<std::pair<const RelAlgNode*, size_t>> table_sizes;
//  srand(194321);
  srand(time(0));

  for (auto it_nodes = nodes.begin(); it_nodes != nodes.end(); ++it_nodes) {
    auto node_scan = std::dynamic_pointer_cast<RelScan>(*it_nodes);
    if (node_scan) {
      if (!std::dynamic_pointer_cast<RelScan>(*(std::next(it_nodes, 1))) &&
          !std::dynamic_pointer_cast<RelJoin>(*(std::next(it_nodes, 1)))) {
        continue;
      }
      auto num_tuples =
          fpd.getNumTuples(node_scan->getTableDescriptor()->tableId, ra_executor->getSessionInfo().get_catalog());
      table_sizes.emplace_back(node_scan.get(), num_tuples);
    }

    auto node_filter = std::dynamic_pointer_cast<RelFilter>(*it_nodes);
    if (node_filter) {
      const auto input = node_filter->getInput(0);
      if (dynamic_cast<const RelJoin*>(input)) {
        auto extract_hash_join_begin = timer_start();
        if (!fpd.extractHashJoinCol(nodes, node_filter->getCondition(), ra_executor)) {
          std::cout << "EXTRACT HASH JOIN COL returned false" << std::endl;
          return false;
        }
        auto extract_hash_join_stop = timer_stop(extract_hash_join_begin);
        std::cout << "time taken for extract hash join col: " << extract_hash_join_stop << " ms" << std::endl;

        // make table_sizes order by size ASC to deal with smaller relation first
        auto filter_evaluate_begin = timer_start();
        // this is sorted only once
        std::sort(table_sizes.begin(), table_sizes.end(), [](auto& lhs, auto& rhs) { return lhs.second < rhs.second; });
        bool status = fpd.evaluateAndPushDown(nodes, ra_executor, table_sizes);
        auto filter_evaluate_stop = timer_stop(filter_evaluate_begin);
        std::cout << std::endl << "time taken for all push down and sketch update: " << filter_evaluate_stop << " ms" << std::endl;

        auto all_push_down_stop = timer_stop(all_push_down_begin);
        std::cout << "time taken for entire push down logic: " << all_push_down_stop << " ms" << std::endl;
        std::cout << "pd end time ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl << std::endl;
        return status;
      }
    }
  }
  auto all_push_down_stop = timer_stop(all_push_down_begin);
  std::cout << "time taken for entire push down logic (returned false): " << all_push_down_stop << " ms" << std::endl;
  std::cout << "pd end time ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl << std::endl;
  return false;
}
