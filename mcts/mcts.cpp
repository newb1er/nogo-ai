#include "mcts.h"

#include <atomic>
#include <cassert>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());

Node::NodePtr selector(Node::NodePtr, bool);
Node::NodePtr selector(Node::NodePtr, double, bool, MCTSTree::NodeTable&);

// std::shared_ptr<Node> selection(Node::NodePtr, bool);
// std::shared_ptr<Node> expansion(std::weak_ptr<Node>);
// double rollout(std::shared_ptr<Node>);
// void backpropagation(std::shared_ptr<Node>, double, bool);

void MCTSTree::init(State& state) {
  auto new_state = state.Clone();
  root = std::make_shared<Node>(new_state, 0);
  init_ = true;
}

void MCTSTree::step(const int& action) {
  return;
  if (init_ == false) return;

  for (auto& kid : root->kids) {
    if (kid->state->GetAction() == action) {
      root = kid;
      break;
    }
  }
}

void MCTSTree::simulate(State& state, int simulation_count = 100,
                        bool minmax = false) {
  init(state);

  for (int count = 0; count < simulation_count; ++count) {
    auto leaf = selection(root, minmax);
    leaf = expansion(leaf);

    auto reward = rollout(leaf);
    backpropagation(leaf, reward, minmax);
  }
}

Node::NodePtr MCTSTree::selection(Node::NodePtr node, bool minmax) {
  while (node->IsLeaf() == false) {
    if (rave_) {
      node = selector(node, rave_bias_, minmax, node_table);
    } else {
      node = selector(node, minmax);
    }
  }

  return node;
}

Node::NodePtr MCTSTree::expansion(std::weak_ptr<Node> n) {
  auto node = n.lock();
  auto possible_actions = node->state->GetPossibleActions();
  // std::random_shuffle(possible_actions.begin(), possible_actions.end());

  /* expand all possible node */
  for (auto& action : possible_actions) {
    auto new_state = node->state->Clone();
    new_state->ApplyAction(action);

    auto new_node = std::make_shared<Node>(new_state, node->depth + 1);
    node_table[action].push_back(new_node);
    new_node->parent = std::weak_ptr<Node>(node);
    node->kids.push_back(new_node);
  }

  if (node->kids.size() == 0) return node;

  return node->kids.front();
}

double MCTSTree::rollout(Node::NodePtr node) {
  double reward = 0.0;

  auto s = node->state->Clone();
  while (s->IsTerminated() == false) {
    auto possible_actions = s->GetPossibleActions();
    if (possible_actions.size() == 0) break;
    std::random_shuffle(possible_actions.begin(), possible_actions.end());

    s->ApplyAction(possible_actions.front());
  }

  reward += s->GetReward();

  return reward;
}

void MCTSTree::backpropagation(Node::NodePtr node, double value,
                               bool minmax = false) {
  node->visits += 1;
  node->value = value;

  while ((node = node->parent.lock()) != nullptr) {
    if (minmax) value = -value;

    node->visits += 1;
    node->value = value;
  }
}

int mcts(State& state, int num_tree = 1, int simulation_count = 100,
         bool minmax = false, bool rave = false, double rave_bias = 10.0) {
  std::vector<MCTSTree> tree_list;

  for (int i = 0; i < num_tree; ++i) {
    tree_list.emplace_back(rave, rave_bias);
  }

#pragma omp parallel
  {
#pragma omp for
    for (int i = 0; i < num_tree; ++i) {
      tree_list[i].simulate(state, simulation_count, minmax);
    }
  }

  auto headTree = tree_list.front();
  auto headTreeRoot = headTree.getRoot();

  // merge tree
  for (auto& tree : tree_list) {
    auto root = tree.getRoot();
    for (size_t i = 0; i < root->kids.size(); ++i) {
      headTreeRoot->kids[i]->visits += root->kids[i]->visits;
      headTreeRoot->kids[i]->value += root->kids[i]->value;
    }
  }

  auto best_action = headTreeRoot->GetBestAction();

  return best_action;
}