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

  while (simulation_count--) {
    auto leaf = node;

    while (leaf->IsLeaf() == false) leaf = leaf->Selection();
    leaf = leaf->Expansion();
    leaf->Update();
  }

  return node->GetBestAction();
}

Node::Node(State* state) : value(0), parent(nullptr), state(state) {}

Node* Node::Selection() const {
  // Select the best child.

  /* UCB1 */
  double best_value = (-1 * kids.front()->value / kids.front()->visits) +
                      sqrt(2 * log(visits) / kids.front()->visits);
  Node* best_child = kids.front();
  for (auto& kid : kids) {
    if (kid->visits == 0) return kid;

    double v =
        (-1 * kid->value / kid->visits) + sqrt(2 * log(visits) / kid->visits);

    if (v > best_value) {
      best_value = v;
      best_child = kid;
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
  std::vector<int> actions = state->GetPossibleActions();

  // Expand all the possible node.
  for (int action : actions) {
    Node* new_node = new Node(state->Clone());
    new_node->parent = this;
    new_node->state->ApplyAction(action);
    kids.push_back(new_node);
  }

  if (kids.empty()) return this;

  // Select a random child.
  return kids.front();
}

void Node::Update() {
  value = state->Rollout();
  visits++;

  auto v = -value;

  // Backpropagtion
  Node* node = this;
  while (node->parent != nullptr) {
    node = node->parent;
    node->visits += 1;
    node->value += v;
    v = -v;
  }
}

bool Node::IsLeaf() const { return kids.empty(); }

int Node::GetBestAction() const {
  // Select the best action.
  // std::cerr << "root kid size: " << kids.size() << std::endl;
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
    // std::cerr << "No action was selected." << std::endl;
    return -1;
  }

  return kids.at(index)->state->GetAction();
}

int State::GetAction() const { return action_; }

NoGoState::NoGoState(board b) : board_(b) {}

double NoGoState::Rollout() {
  auto b_ = board_;
  auto who = board_.info().who_take_turns;

  int action = 0;

  while (action != -1) {
    no_go_action.Reset();
    for (action = no_go_action.GetNext(); action != -1;
         action = no_go_action.GetNext()) {
      auto after = b_;
      auto move = action::place(action, b_.info().who_take_turns);

      if (move.apply(after) == board::legal) {
        b_ = after;
        break;
      }
    }
  }

  // if start with black and black can't make any move at the end, white wins.
  return b_.info().who_take_turns == who ? -1.0f : 1.0f;
}

std::vector<int> NoGoState::GetPossibleActions() {
  no_go_action.Reset();

  int action;
  std::vector<int> actions;
  actions.reserve(no_go_action.actions.size());

  while ((action = no_go_action.GetNext()) != -1) {
    auto b_ = board_;
    auto move = action::place(action, b_.info().who_take_turns);
    if (move.apply(b_) == board::legal) actions.push_back(action);
  }

  return actions;
}

void NoGoState::ApplyAction(const int action) {
  auto move = action::place(action, board_.info().who_take_turns);
  if (move.apply(board_) != board::legal) {
    std::cerr << "action should be legal\n";
    assert(false);
  }
  action_ = action;
}
