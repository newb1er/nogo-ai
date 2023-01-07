#include "mcts.h"

#include <atomic>
#include <cassert>

std::shared_ptr<Node> selection(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> selector(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> expansion(std::weak_ptr<Node>);
double rollout(std::shared_ptr<Node>);
void backpropagation(std::shared_ptr<Node>, double, bool);

void MCTSTree::init(State& state) {
  auto new_state = state.Clone();
  root_list.resize(num_root);

  for (auto& node : root_list) {
    node = std::make_shared<Node>(new_state);
  }

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

int MCTSTree::simulate(State& state, int simulation_count = 100,
                       bool minmax = false) {
  init(state);

#pragma omp parallel for
  for (size_t i = 0; i < num_root; ++i) {
    for (int count = 0; count < simulation_count; ++count) {
      auto leaf = selection(root_list.at(i), minmax);
      leaf = expansion(leaf);

      auto reward = rollout(leaf);
      backpropagation(leaf, reward, minmax);
    }
  }

  auto root = root_list.front();

  // merge root
  for (auto& node : root_list) {
    assert(node->kids.size() == root->kids.size());
    for (size_t i = 0; i < node->kids.size(); ++i) {
      root->kids[i]->visits += node->kids[i]->visits;
      root->kids[i]->value += node->kids[i]->value;
    }
  }

  auto best_action = root->GetBestAction();

  return best_action;
}

std::shared_ptr<Node> selection(std::shared_ptr<Node> node, bool minmax) {
  while (node->IsLeaf() == false) {
    node = selector(node, minmax);
  }

  return node;
}

std::shared_ptr<Node> expansion(std::weak_ptr<Node> n) {
  auto node = n.lock();
  auto possible_actions = node->state->GetPossibleActions();
  // std::random_shuffle(possible_actions.begin(), possible_actions.end());

  /* expand all possible node */
  for (size_t action = 0; action < possible_actions.size(); ++action) {
    auto new_state = node->state->Clone();
    new_state->ApplyAction(possible_actions[action]);

    auto new_node = std::make_shared<Node>(new_state);
    new_node->parent = std::weak_ptr<Node>(node);
    node->kids.push_back(new_node);
  }

  if (node->kids.size() == 0) return node;

  return node->kids.front();
}

double rollout(std::shared_ptr<Node> node) {
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

void backpropagation(std::shared_ptr<Node> node, double value,
                     bool minmax = false) {
  node->visits += 1;
  node->value = value;

  while ((node = node->parent.lock()) != nullptr) {
    if (minmax) value = -value;

    node->visits += 1;
    node->value = value;
  }
}