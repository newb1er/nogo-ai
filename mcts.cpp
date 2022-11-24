#include "mcts.h"

typedef struct _NoGoAction {
  std::vector<int> actions;
  size_t currentIndex = 0;

  _NoGoAction() : actions(board::size_x * board::size_y) {
    for (size_t i = 0; i < actions.size(); ++i) actions[i] = i;
    Reset();
  }

  void Shuffle() { std::random_shuffle(actions.begin(), actions.end()); }
  void Reset() {
    currentIndex = 0;
    Shuffle();
  }

  int GetNext() {
    if (currentIndex >= actions.size()) return -1;
    return actions[currentIndex++];
  }
} NoGoAction;

NoGoAction no_go_action;

int MCTS(State& state, int simulation_count = 100) {
  auto node = new Node(&state);

  for (int i = 0; i < simulation_count; ++i) {
    auto leaf = node;

    while (leaf->IsLeaf() == false) leaf = leaf->Selection();
    leaf = leaf->Expansion();
    leaf->Update();
  }

  return node->GetBestAction();
}

Node::Node(State* state) : parent(nullptr), state(state) {}

Node* Node::Selection() const {
  // Select the best child.
  double best_value = kids.at(0)->value / kids.at(0)->visits +
                      sqrt(2 * log(visits) / kids.at(0)->visits);
  Node* best_child = kids.at(0);
  for (size_t i = 0; i < kids.size(); ++i) {
    double value =
        kids.at(i)->value + sqrt(2 * log(visits) / kids.at(i)->visits);
    if (value > best_value) {
      best_value = value;
      best_child = kids.at(i);
    }
  }

  if (best_child == nullptr) {
    // No child was selected. This should never happen.
    std::cerr << "No child was selected." << std::endl;
    assert(false);
  }

  return best_child;
}

Node* Node::Expansion() {
  // Expand all the possible node.
  for (int action = state->GetPossibleAction(); action != -1;
       action = state->GetPossibleAction()) {
    Node* new_node = new Node(state->Clone());
    new_node->parent = this;
    new_node->state->ApplyAction(action);
    kids.push_back(new_node);
  }

  // Select a random child.
  return kids.at(rand() % kids.size());
}

void Node::Update() {
  value = -1 * state->Rollout();
  visits++;

  // Backpropagtion
  Node* node = this;
  while (node->parent != nullptr) {
    node = node->parent;
    node->visits += 1;
    node->value += value;
    value = -value;
  }
}

bool Node::IsLeaf() const { return kids.empty(); }

int Node::GetBestAction() const {
  // Select the best action.
  std::cerr << "root kid size: " << kids.size() << std::endl;
  uint32_t best_visits = 0;
  int index = -1;
  for (size_t i = 0; i < kids.size(); ++i) {
    if (kids.at(i)->visits > best_visits) {
      best_visits = kids.at(i)->visits;
      index = i;
    }
  }

  if (index == -1) {
    // No action was selected. This should never happen.
    std::cerr << "No action was selected." << std::endl;
    assert(false);
    return -1;
  }

  return kids.at(index)->state->GetAction();
}

int State::GetAction() const { return action_; }

NoGoState::NoGoState(board b) : board_(b) {}

double NoGoState::Rollout() {
  auto b_ = board_;
  no_go_action.Reset();
  auto who = board_.info().who_take_turns;

  for (int action = no_go_action.GetNext(); action != -1;
       action = no_go_action.GetNext()) {
    auto after = b_;
    auto move = action::place(action, b_.info().who_take_turns);

    if (move.apply(after) == board::legal) {
      b_ = after;
    }
  }

  // if start with black and black can't make any move at the end, white wins.
  return b_.info().who_take_turns == who ? -1.0f : 1.0f;
}

int NoGoState::GetPossibleAction() {
  if (no_go_action.GetNext() == -1) {
    no_go_action.Reset();
  }

  for (int action = no_go_action.GetNext(); action != -1;
       action = no_go_action.GetNext()) {
    // std::cerr << action << '\n';
    auto after = board_;
    auto move = action::place(action, board_.info().who_take_turns);

    if (move.apply(after) == board::legal) {
      return action;
    }

    action = no_go_action.GetNext();
  }

  return -1;
}

void NoGoState::ApplyAction(const int action) {
  auto move = action::place(action, board_.info().who_take_turns);
  move.apply(board_);
  action_ = action;
}
