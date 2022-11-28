#pragma once
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "../action.h"
#include "../board.h"

class State {
 public:
  State() : action_(-1), reward_(0), terminated_(false) {}
  State(double init_reward)
      : action_(-1), reward_(init_reward), terminated_(false) {}
  virtual ~State() {}
  virtual std::shared_ptr<State> Clone() {
    return std::shared_ptr<State>(this);
  };

  virtual std::vector<int> GetPossibleActions() { return std::vector<int>(); }
  virtual void ApplyAction(const int action) {}
  int GetAction() const { return action_; }
  double GetReward() const { return reward_; }
  bool IsTerminated() const { return terminated_; }

 protected:
  int action_;
  double reward_;
  bool terminated_;
};

class NoGoState : public State {
 public:
  NoGoState(board b) : State(-1.0), board_(b) {}
  NoGoState(const NoGoState& s) : State(-1.0), board_(s.board_) {}
  NoGoState(NoGoState&& s) : State(-1.0), board_(std::move(s.board_)) {}
  NoGoState& operator=(const NoGoState& s) {
    if (this != &s) {
      board_ = s.board_;
      reward_ = -1.0;
    }

    return *this;
  }
  NoGoState& operator=(NoGoState&& s) {
    if (this != &s) {
      board_ = std::move(s.board_);
      reward_ = -1.0;
    }

    return *this;
  }
  std::shared_ptr<State> Clone() {
    return std::make_shared<NoGoState>(this->board_);
  }
  virtual ~NoGoState() {}

  virtual std::vector<int> GetPossibleActions() override;
  virtual void ApplyAction(const int) override;

 private:
  board board_;
};

class Node {
 public:
  Node();
  Node(std::shared_ptr<State>);
  int GetBestAction() const;
  bool IsLeaf() const;

  uint32_t visits;
  double value;

  std::weak_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> kids;
  std::shared_ptr<State> state;
};

int MCTS(State&, int, bool, int);
