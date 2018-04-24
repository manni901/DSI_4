#include "QueryPlan.h"

QueryPlan::QueryPlan(string stat_file) {
  // Initialize statistics from file.
  stat_.Read(stat_file.c_str());

  // Convert AndList to ParseVector.
  parse_vector_ = ParseUtil::AndListToVector(boolean);

  // Copy relations needed for the query.
  TableList *temp = tables;
  while (temp != NULL) {
    stat_.CopyRel(temp->tableName, temp->aliasAs);
    table_names_.push_back(temp->tableName);
    alias_[temp->tableName] = temp->aliasAs;
    temp = temp->next;
  }

  UpdateJoinOrder();
  BuildJoinNodes();
  BuildGroupByNode();
  BuildProjectNode();
  BuildDistinctNode();
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
      new_schema =
          make_unique<Schema>("catalog", table_name, alias_[table_name]);
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
      make_unique<Schema>("catalog", table_names_[0], alias_[table_names_[0]]);
  auto curr_selector =
      ParseUtil::FilterParseVector(parse_vector_, *schema.get(), selector);
  root_node_ = make_unique<SelectFileNode>(parse_vector_, curr_selector,
                                           move(schema), curr_pipe_id_++);

  for (int i = 1; i < table_names_.size(); i++) {
    schema = make_unique<Schema>("catalog", table_names_[i],
                                 alias_[table_names_[i]]);
    auto curr_selector =
        ParseUtil::FilterParseVector(parse_vector_, *schema.get(), selector);

    auto join_schema =
        make_unique<Schema>(root_node_->GetSchema(), schema.get());
    auto join_selector = ParseUtil::FilterParseVector(
        parse_vector_, *join_schema.get(), selector);

    auto sf_node = make_unique<SelectFileNode>(parse_vector_, curr_selector,
                                               move(schema), curr_pipe_id_++);

    auto join_node =
        make_unique<JoinNode>(parse_vector_, join_selector, move(join_schema),
                              move(root_node_), move(sf_node), curr_pipe_id_++);
    root_node_ = move(join_node);
  }
}

void QueryPlan::BuildGroupByNode() {
  if (groupingAtts) {
    unordered_set<string> group_atts;
    NameList *temp = groupingAtts;
    while (temp != NULL) {
      group_atts.insert(temp->name);
      temp = temp->next;
    }
    if (distinctFunc) {
      auto dup_node =
          make_unique<DuplicateRemovalNode>(move(root_node_), curr_pipe_id_++);
      root_node_ = move(dup_node);
    }
    auto group_node = make_unique<GroupByNode>(move(root_node_), group_atts,
                                               finalFunction, curr_pipe_id_++);
    root_node_ = move(group_node);
  } else if (finalFunction) {
    auto sum_node =
        make_unique<SumNode>(move(root_node_), finalFunction, curr_pipe_id_++);
    root_node_ = move(sum_node);
  }
}

void QueryPlan::BuildProjectNode() {
  if (attsToSelect) {
    unordered_set<string> select_atts;
    if (finalFunction) {
      select_atts.insert("sum");
    }
    NameList *temp = attsToSelect;
    while (temp != NULL) {
      select_atts.insert(temp->name);
      temp = temp->next;
    }
    auto project_node = make_unique<ProjectNode>(move(root_node_), select_atts,
                                                 curr_pipe_id_++);
    root_node_ = move(project_node);
  }
}

void QueryPlan::BuildDistinctNode() {
  if (distinctAtts) {
    auto dup_node =
        make_unique<DuplicateRemovalNode>(move(root_node_), curr_pipe_id_++);
    root_node_ = move(dup_node);
  }
}