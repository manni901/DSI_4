#include "QueryNode.h"

SelectPipeNode::SelectPipeNode(ParseVector &parse_vector, BitSet &selector,
                               unique_ptr<Schema> schema, QueryNodeP input_node,
                               int pipe_id, PMap<Pipe> &pipes)
    : in_pipe_id_(input_node->OutPipeId()), QueryNode(pipe_id, move(schema)) {
  Record literal;
  cnf_.GrowFromParseTree(parse_vector, selector, schema_.get(), literal);
  children_.push_back(move(input_node));
  rel_op_ = make_unique<SelectPipe>(*pipes[in_pipe_id_].get(), *pipes[out_pipe_id_].get(), cnf_, literal);
  rel_op_->Use_n_Pages(RUN_LEN);
}

void SelectPipeNode::Print() {
  cout << "Select Pipe Operation ";
  cout << "Input pipe ID " << in_pipe_id_ << " ";
  cout << "Output pipe ID " << out_pipe_id_ << " ";
  schema_->Print();
  cout << "\n";
  cnf_.Print();
  cout << "\n";
}

ProjectNode::ProjectNode(QueryNodeP input_node,
                         unordered_set<string> &att_names, int pipe_id, PMap<Pipe> &pipes)
    : in_pipe_id_(input_node->OutPipeId()), QueryNode(pipe_id) {
  vector<Attribute> atts;
  Attribute *input_atts = input_node->GetSchema()->GetAtts();
  int num_input_atts = input_node->GetSchema()->GetNumAtts();

  vector<int> filter_ids;
  for (int i = 0; i < num_input_atts; i++) {
    if (att_names.find(input_atts[i].name) != att_names.end()) {
      filter_ids.emplace_back(i);
      atts.push_back({input_atts[i].name, input_atts[i].myType});
    }
  }

  schema_ = make_unique<Schema>("groupbyschema", atts.size(), atts);
  children_.push_back(move(input_node));
  rel_op_ = make_unique<Project>(*pipes[in_pipe_id_].get(), *pipes[out_pipe_id_].get(), filter_ids, num_input_atts, filter_ids.size());
  rel_op_->Use_n_Pages(RUN_LEN);
}

void ProjectNode::Print() {
  cout << "Project Operation ";
  cout << "Input pipe ID " << in_pipe_id_ << " ";
  cout << "Output pipe ID " << out_pipe_id_ << " ";
  schema_->Print();
  cout << "\n";
}

DuplicateRemovalNode::DuplicateRemovalNode(QueryNodeP input_node, int pipe_id, PMap<Pipe> &pipes)
    : in_pipe_id_(input_node->OutPipeId()), QueryNode(pipe_id) {
  schema_ = make_unique<Schema>(input_node->GetSchema());
  children_.push_back(move(input_node));
  rel_op_ = make_unique<DuplicateRemoval>(*pipes[in_pipe_id_].get(), *pipes[out_pipe_id_].get(), *schema_.get());
  rel_op_->Use_n_Pages(RUN_LEN);
}

void DuplicateRemovalNode::Print() {
  cout << "Duplicate Removal Operation ";
  cout << "Input pipe ID " << in_pipe_id_ << " ";
  cout << "Output pipe ID " << out_pipe_id_ << " ";
  schema_->Print();
  cout << "\n";
}

SelectFileNode::SelectFileNode(ParseVector &parse_vector, BitSet &selector, string table_name,
                               unique_ptr<Schema> schema, int pipe_id, PMap<Pipe> &pipes)
    : QueryNode(pipe_id, move(schema)) {
  Record literal;
  cnf_.GrowFromParseTree(parse_vector, selector, schema_.get(), literal);
  db_file_.Open(string(db_file_prefix + table_name + ".bin").c_str());
  rel_op_ = make_unique<SelectFile>(db_file_, *pipes[out_pipe_id_].get(), cnf_, literal);
  rel_op_->Use_n_Pages(RUN_LEN);
}

void SelectFileNode::Print() {
  cout << "Select File Operation Output pipe ID " << out_pipe_id_ << " ";
  schema_->Print();
  cout << "\n";
  cnf_.Print();
  cout << "\n";
}

JoinNode::JoinNode(ParseVector &parse_vector, BitSet &selector,
                   unique_ptr<Schema> schema, QueryNodeP left, QueryNodeP right,
                   int out_pipe_id, PMap<Pipe> &pipes)
    : left_pipe_id_(left->OutPipeId()), right_pipe_id_(right->OutPipeId()),
      QueryNode(out_pipe_id, move(schema)) {
  Record literal;
  cnf_.GrowFromParseTree(parse_vector, selector, left->GetSchema(),
                         right->GetSchema(), literal);
  children_.push_back(move(left));
  children_.push_back(move(right));
  rel_op_ = make_unique<Join>(*pipes[left_pipe_id_].get(), *pipes[right_pipe_id_].get(), *pipes[out_pipe_id_].get(), cnf_, literal);
  rel_op_->Use_n_Pages(RUN_LEN);
}

void JoinNode::Print() {
  cout << "Join Operation ";
  cout << "Input pipe ID " << left_pipe_id_ << " ";
  cout << "Input pipe ID " << right_pipe_id_ << " ";
  cout << "Output pipe ID " << out_pipe_id_ << " ";
  schema_->Print();
  cout << "\nJoin CNF:\n";
  cnf_.Print();
  cout << "\n";
}

GroupByNode::GroupByNode(QueryNodeP input_node,
                         unordered_set<string> &grouping_atts,
                         FuncOperator *func, int pipe_id, PMap<Pipe> &pipes)
    : in_pipe_id_(input_node->OutPipeId()), QueryNode(pipe_id) {
  compute_.GrowFromParseTree(func, *input_node->GetSchema());

  vector<Attribute> atts;

  atts.push_back({strdup("sum"), compute_.GetType()});

  Attribute *input_atts = input_node->GetSchema()->GetAtts();
  int num_input_atts = input_node->GetSchema()->GetNumAtts();

  for (int i = 0; i < num_input_atts; i++) {
    if (grouping_atts.find(input_atts[i].name) != grouping_atts.end()) {
      atts.push_back({input_atts[i].name, input_atts[i].myType});
      order_maker_.AddAtt(i, input_atts[i].myType);
    }
  }

  schema_ = make_unique<Schema>("groupbyschema", atts.size(), atts);
  children_.push_back(move(input_node));
  rel_op_ = make_unique<GroupBy>(*pipes[in_pipe_id_].get(), *pipes[out_pipe_id_].get(), order_maker_, compute_);
  rel_op_->Use_n_Pages(RUN_LEN);
}

void GroupByNode::Print() {
  cout << "GroupBy Operation ";
  cout << "Input pipe ID " << in_pipe_id_ << " ";
  cout << "Output pipe ID " << out_pipe_id_ << " ";
  schema_->Print();
  cout << "\n";
  order_maker_.Print();
  cout << "\n";
  compute_.Print();
  cout << "\n";
}

SumNode::SumNode(QueryNodeP input_node, FuncOperator *func, int pipe_id, PMap<Pipe> &pipes)
    : in_pipe_id_(input_node->OutPipeId()), QueryNode(pipe_id) {
  compute_.GrowFromParseTree(func, *input_node->GetSchema());
  Attribute att = {"sum", compute_.GetType()};
  vector<Attribute> atts = {att};
  schema_ = make_unique<Schema>("sumschema", 1, atts);
  children_.push_back(move(input_node));
  rel_op_ = make_unique<Sum>(*pipes[in_pipe_id_].get(), *pipes[out_pipe_id_].get(), compute_);
  rel_op_->Use_n_Pages(RUN_LEN);
}

void SumNode::Print() {
  cout << "Sum Operation ";
  cout << "Input pipe ID " << in_pipe_id_ << " ";
  cout << "Output pipe ID " << out_pipe_id_ << " ";
  schema_->Print();
  cout << "\n";
  compute_.Print();
  cout << "\n";
}