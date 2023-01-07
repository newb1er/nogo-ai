#pragma once
#include <omp.h>

#include <atomic>
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "../action.h"
#include "../board.h"

class Node;

using MCTSNodePtr = std::shared_ptr<Node>;

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

  virtual bool operator==(const State& s) const {
    return action_ == s.GetAction();
  }

  virtual bool operator!=(const State& s) const {
    return action_ != s.GetAction();
  }

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

  virtual bool operator==(const NoGoState& s) const {
    return board_ == (s).board_;
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
  using NodePtr = std::shared_ptr<Node>;

 public:
  Node();
  Node(std::shared_ptr<State>);
  int GetBestAction() const;
  bool IsLeaf() const;
  std::shared_ptr<Node> FindChild(State&);

  uint32_t visits;
  double value;

  std::weak_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> kids;
  std::shared_ptr<State> state;
};

class MCTSTree {
 public:
  MCTSTree() : init_(false) {}
  int simulate(State&, int, bool);
  void reset() { init_ = false; }
  void init(State&);
  void step(const int&);

 protected:
  size_t num_root = 8;

 private:
  bool init_;
  MCTSNodePtr root;
  std::vector<MCTSNodePtr> root_list;
  std::shared_ptr<Node::NodePtr> garbage;
};
