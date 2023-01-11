#pragma once
#include <omp.h>

#include <atomic>
#include <cassert>
#include <chrono>
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
  NoGoState(board b) : State(0.0), board_(b) {}
  NoGoState(const NoGoState& s) : State(0.0), board_(s.board_) {}
  NoGoState(NoGoState&& s) : State(0.0), board_(std::move(s.board_)) {}
  NoGoState& operator=(const NoGoState& s) {
    if (this != &s) {
      board_ = s.board_;
    }

    return *this;
  }
  NoGoState& operator=(NoGoState&& s) {
    if (this != &s) {
      board_ = std::move(s.board_);
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
  Node(std::shared_ptr<State>, size_t);
  int GetBestAction() const;
  bool IsLeaf() const;
  std::shared_ptr<Node> FindChild(State&);

  uint32_t visits;
  double value;
  size_t depth;

  std::weak_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> kids;
  std::shared_ptr<State> state;
};

class MCTSTree {
 public:
  using NodeTable = std::unordered_map<int, std::list<MCTSNodePtr>>;

 public:
  MCTSTree() : init_(false), rave_(false) {}
  MCTSTree(bool using_rave, double rave_bias, unsigned rave_depth,
           std::chrono::time_point<std::chrono::steady_clock> deadline)
      : init_(false),
        rave_(using_rave),
        rave_bias_(rave_bias),
        deadline(deadline) {}
  void reset() { init_ = false; }
  void init(State&);
  void step(const int&);
  void setRave(bool using_rave) { rave_ = using_rave; }
  MCTSNodePtr getRoot() { return root; }

 public:
  void simulate(State&, int, bool);
  Node::NodePtr selection(Node::NodePtr, bool);
  Node::NodePtr expansion(std::weak_ptr<Node>);
  double rollout(Node::NodePtr, bool);
  void backpropagation(Node::NodePtr, double, bool);

 protected:
  NodeTable node_table;

 private:
  bool init_;
  bool rave_;
  double rave_bias_;
  unsigned rave_depth;
  std::chrono::time_point<std::chrono::steady_clock> deadline;
  MCTSNodePtr root;
};

int mcts(std::chrono::milliseconds, int, int, State&, int, int, bool, bool,
         double);