#include "mcts.h"

#include <omp.h>

#include <mutex>

std::shared_ptr<Node> selection(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> selector(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> expansion(std::shared_ptr<Node>);
double rollout(std::shared_ptr<Node>, int);
void backpropagation(std::shared_ptr<Node>, double, bool);

int MCTS(State& state, int simulation_count = 100, bool minmax = false,
         int num_rollout = 20) {
  auto new_state = state.Clone();
  auto root = std::make_shared<Node>(new_state);

  while (simulation_count--) {
    auto leaf = expansion(selection(root, minmax));

    backpropagation(leaf, rollout(leaf, num_rollout), minmax);
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
  std::mutex mutex;
#pragma omp parallel for
  for (auto& action : possible_actions) {
    auto new_state = node->state->Clone();
    new_state->ApplyAction(action);

    auto new_node = std::make_shared<Node>(new_state);
    new_node->parent = std::weak_ptr<Node>(node);

    {
      std::lock_guard<std::mutex> lock(mutex);
      node->kids.push_back(new_node);
    }
  }

  if (node->kids.empty()) return node;

  return node->kids.front();
}

double rollout(std::shared_ptr<Node> node, int num_rollout = 20) {
  double reward = 0.0;

#pragma omp parallel for
  for (int i = 0; i < num_rollout; ++i) {
    auto s = node->state->Clone();

    while (s->IsTerminated() == false) {
      auto possible_actions = s->GetPossibleActions();
      if (possible_actions.size() == 0) break;
      std::random_shuffle(possible_actions.begin(), possible_actions.end());

      s->ApplyAction(possible_actions.front());
    }

    reward += s->GetReward();
  }

  return reward / num_rollout;
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