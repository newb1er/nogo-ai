#include "mcts.h"

#include <cassert>

std::shared_ptr<Node> selection(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> selector(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> expansion(std::shared_ptr<Node>);
double rollout(std::shared_ptr<Node>);
void backpropagation(std::shared_ptr<Node>, double, bool);

void MCTSTree::init(State& state) {
  auto new_state = state.Clone();
  root = std::make_shared<Node>(new_state);
  init_ = true;
}

void MCTSTree::step(const int& action) {
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
  // init(state);

  if (init_ == false) {
    init(state);
  }

  while (simulation_count--) {
    auto leaf = expansion(selection(root, minmax));

    backpropagation(leaf, rollout(leaf), minmax);
  }

  auto best_action = root->GetBestAction();

  step(best_action);

  return best_action;
}

std::shared_ptr<Node> selection(std::shared_ptr<Node> node, bool minmax) {
  while (node->IsLeaf() == false) {
    node = selector(node, minmax);
  }

  return node;
}

std::shared_ptr<Node> expansion(std::shared_ptr<Node> node) {
  auto possible_actions = node->state->GetPossibleActions();
  std::random_shuffle(possible_actions.begin(), possible_actions.end());
  node->kids.reserve(possible_actions.size());

  /* expand all possible node */
  for (size_t action = 0; action < possible_actions.size(); ++action) {
    auto new_state = node->state->Clone();
    new_state->ApplyAction(possible_actions[action]);

    auto new_node = std::make_shared<Node>(new_state);
    new_node->parent = std::weak_ptr<Node>(node);
    // list.at(action).push_back(new_node);

    node->kids.push_back(new_node);
  }

  if (node->kids.empty()) return node;

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
  node->value = value;
  node->visits += 1;

  while (node->parent.lock() != nullptr) {
    node = node->parent.lock();
    if (minmax) value = -value;

    node->value += value;
    node->visits += 1;
  }
}