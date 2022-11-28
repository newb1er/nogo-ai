#include "mcts.h"

std::vector<int> NoGoState::GetPossibleActions() {
  auto b_ = board_;
  std::vector<int> actions;
  actions.reserve(board::size_x * board::size_y);

  for (int i = 0; i < board::size_x * board::size_y; ++i) {
    auto after = b_;
    auto move = action::place(i, b_.info().who_take_turns);
    if (move.apply(after) == board::legal) actions.push_back(i);
  }

  return actions;
}

void NoGoState::ApplyAction(const int action) {
  auto move = action::place(action, board_.info().who_take_turns);
  if (move.apply(board_) != board::legal) {
    std::cerr << "Illegal action: " << action << std::endl;
    assert(false);
  }

  action_ = action;
  reward_ = -reward_;
}