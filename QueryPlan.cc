#include "QueryPlan.h"

QueryPlan::QueryPlan(string stat_file) {
  // Initialize statistics from file.
  stat_.Read(stat_file.c_str());

  // Convert AndList to ParseVector.
  parse_vector_ = ParseUtil::AndListToVector(boolean);

  // Copy relations needed for the query.
  for (auto &temp : tables) {
    stat_.CopyRel(temp.first.c_str(), temp.second.c_str());
    table_names_.push_back(temp.first);
    alias_[temp.first] = temp.second;
  }

  UpdateJoinOrder();
  BuildJoinNodes();
  BuildGroupByNode();
  BuildProjectNode();
  BuildDistinctNode();

  auto write_node =
      make_unique<WriteOutNode>(move(root_node_), GetOutMode(), pipes_);
  root_node_ = move(write_node);
}

void QueryPlan::UpdateJoinOrder() {
  // Only process if there is more than a single table.
  if (table_names_.size() < 2)
    return;

  // Store the lowest cost query order seen till now.
  vector<string> best_table_names;
  double min_result = LONG_MAX;

  sort(table_names_.begin(), table_names_.end());
  do {
    Statistics temp_stat = stat_;
    // If i bit is set in selector, it means we have already processed
    // i th or_list in ParseVector.
    BitSet selector;

    double result = 0.0;
    vector<string> relNames;
    unique_ptr<Schema> schema, new_schema, combined_schema;
    for (auto table_name : table_names_) {
      relNames.push_back(alias_[table_name]);

      // Combine schema for join node.
      new_schema = make_unique<Schema>(catalog, table_name, alias_[table_name]);
      if (schema) {
        combined_schema = make_unique<Schema>(schema.get(), new_schema.get());
        schema = move(combined_schema);
      } else {
        schema = move(new_schema);
      }
      // Get Bitset for current selection.
      auto curr_selector =
          ParseUtil::FilterParseVector(parse_vector_, *schema.get(), selector);
      result += temp_stat.Apply(parse_vector_, relNames, curr_selector);
      if (result > min_result)
        break;
    }
    if (result < min_result) {
      min_result = result;
      best_table_names.clear();
      best_table_names = table_names_;
    }
  } while (next_permutation(begin(table_names_), end(table_names_)));
  table_names_ = best_table_names;
}

void QueryPlan::BuildJoinNodes() {
  if (table_names_.size() == 0)
    return;

  BitSet selector;

  auto schema =
      make_unique<Schema>(catalog, table_names_[0], alias_[table_names_[0]]);
  auto curr_selector =
      ParseUtil::FilterParseVector(parse_vector_, *schema.get(), selector);
  pipes_[curr_pipe_id_] = make_unique<Pipe>();
  root_node_ =
      make_unique<SelectFileNode>(parse_vector_, curr_selector, table_names_[0],
                                  move(schema), curr_pipe_id_++, pipes_);

  for (int i = 1; i < table_names_.size(); i++) {
    schema =
        make_unique<Schema>(catalog, table_names_[i], alias_[table_names_[i]]);
    auto curr_selector =
        ParseUtil::FilterParseVector(parse_vector_, *schema.get(), selector);

    auto join_schema =
        make_unique<Schema>(root_node_->GetSchema(), schema.get());
    auto join_selector = ParseUtil::FilterParseVector(
        parse_vector_, *join_schema.get(), selector);

    pipes_[curr_pipe_id_] = make_unique<Pipe>();
    auto sf_node = make_unique<SelectFileNode>(parse_vector_, curr_selector,
                                               table_names_[i], move(schema),
                                               curr_pipe_id_++, pipes_);

    pipes_[curr_pipe_id_] = make_unique<Pipe>();
    auto join_node = make_unique<JoinNode>(
        parse_vector_, join_selector, move(join_schema), move(root_node_),
        move(sf_node), curr_pipe_id_++, pipes_);
    root_node_ = move(join_node);
  }
}

void QueryPlan::BuildGroupByNode() {
  if (groupingAtts.size() > 0) {
    pipes_[curr_pipe_id_] = make_unique<Pipe>();
    auto group_node = make_unique<GroupByNode>(
        move(root_node_), groupingAtts, finalFunction, curr_pipe_id_++, pipes_);
    root_node_ = move(group_node);
  } else if (finalFunction) {
    pipes_[curr_pipe_id_] = make_unique<Pipe>();
    auto sum_node = make_unique<SumNode>(move(root_node_), finalFunction,
                                         curr_pipe_id_++, pipes_);
    root_node_ = move(sum_node);
  }
}

void QueryPlan::BuildProjectNode() {
  if (attsToSelect.size() > 0) {
    if (finalFunction != NULL) {
      attsToSelect.insert(attsToSelect.begin(), "sum");
    }
    pipes_[curr_pipe_id_] = make_unique<Pipe>();
    auto project_node = make_unique<ProjectNode>(move(root_node_), attsToSelect,
                                                 curr_pipe_id_++, pipes_);
    root_node_ = move(project_node);
  }
}

void QueryPlan::BuildDistinctNode() {
  if (distinctAtts) {
    pipes_[curr_pipe_id_] = make_unique<Pipe>();
    auto dup_node = make_unique<DuplicateRemovalNode>(move(root_node_),
                                                      curr_pipe_id_++, pipes_);
    root_node_ = move(dup_node);
  }
}