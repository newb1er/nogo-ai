#include "mcts.h"

#include <omp.h>

#include <mutex>

// using ActionNodeList = std::vector<std::vector<std::shared_ptr<Node>>>;
// using NodeList = std::vector<std::shared_ptr<Node>>;

// std::shared_ptr<Node> selection(std::shared_ptr<Node>, bool,
// ActionNodeList&); std::shared_ptr<Node> selector(std::shared_ptr<Node>, bool,
// ActionNodeList&);
std::shared_ptr<Node> selection(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> selector(std::shared_ptr<Node>, bool);
std::shared_ptr<Node> expansion(std::shared_ptr<Node>);
double rollout(std::shared_ptr<Node>, int);
void backpropagation(std::shared_ptr<Node>, double, bool);

int MCTS(State& state, int simulation_count = 100, bool minmax = false,
         int num_rollout = 20) {
  auto new_state = state.Clone();
  auto root = std::make_shared<Node>(new_state);

  // ActionNodeList action_nodes(board::size_x * board::size_y);

  while (simulation_count--) {
    auto leaf = expansion(selection(root, minmax));

    backpropagation(leaf, rollout(leaf, num_rollout), minmax);
  }

  return root->GetBestAction();
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

  /* expand all possible node */
  std::mutex mutex;
#pragma omp parallel for
  for (size_t action = 0; action < possible_actions.size(); ++action) {
    auto new_state = node->state->Clone();
    new_state->ApplyAction(possible_actions[action]);

    auto new_node = std::make_shared<Node>(new_state);
    new_node->parent = std::weak_ptr<Node>(node);
    // list.at(action).push_back(new_node);

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