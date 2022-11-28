#include "mcts.h"

Node::Node() = default;
Node::Node(std::shared_ptr<State> state) : value(0), state(state) {}

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