#include "mcts.h"

std::shared_ptr<Node> selection(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> selector(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> expansion(std::shared_ptr<Node>);
double rollout(std::shared_ptr<Node>);
void backpropagation(std::shared_ptr<Node>, double, bool);

int MCTS(State& state, int simulation_count = 100, bool minmax = false) {
  auto new_state = state.Clone();
  auto root = std::make_shared<Node>(new_state);

  while (simulation_count--) {
    auto leaf = expansion(selection(root, minmax));

    backpropagation(leaf, rollout(leaf), minmax);
  }

  return root->GetBestAction();
}

std::shared_ptr<Node> selection(std::shared_ptr<Node> node,
                                bool minmax = false) {
  while (node->IsLeaf() == false) {
    node = selector(node, minmax);
  }

  return node;
}

std::shared_ptr<Node> expansion(std::shared_ptr<Node> node) {
  auto possible_actions = node->state->GetPossibleActions();
  std::random_shuffle(possible_actions.begin(), possible_actions.end());

  /* expand all possible node */
  for (auto& action : possible_actions) {
    auto new_state = node->state->Clone();
    new_state->ApplyAction(action);

    auto new_node = std::make_shared<Node>(new_state);
    new_node->parent = node;
    node->kids.push_back(new_node);
  }

  if (node->kids.empty()) return node;

  return node->kids.front();
}

double rollout(std::shared_ptr<Node> node) {
  auto s = node->state->Clone();

  while (s->IsTerminated() == false) {
    auto possible_actions = s->GetPossibleActions();
    if (possible_actions.size() == 0) break;
    std::random_shuffle(possible_actions.begin(), possible_actions.end());

    s->ApplyAction(possible_actions.front());
  }

  return s->GetReward();
}

void backpropagation(std::shared_ptr<Node> node, double value,
                     bool minmax = false) {
  while (node->parent != nullptr) {
    node->value += value;
    node->visits += 1;

    node = node->parent;
    if (minmax) value = -value;
  }
}