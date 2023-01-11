#include <algorithm>

#include "mcts.h"

Node::Node() = default;
Node::Node(std::shared_ptr<State> state, size_t depth)
    : visits(0), value(0), depth(depth), state(state) {}

int Node::GetBestAction() const {
  uint32_t best_visits = 0;
  int best_action = -1;

  for (auto& kid : kids) {
    if (kid->visits > best_visits) {
      best_visits = kid->visits;
      best_action = kid->state->GetAction();
    }
  }

  return best_action;
}

bool Node::IsLeaf() const { return kids.empty(); }

MCTSNodePtr Node::FindChild(State& state) {
  MCTSNodePtr selected_node;
  for (auto node : kids) {
    if (*(node->state) == state) {
      selected_node = node;
      break;
    }
  }

  return selected_node;
}
