#pragma once
#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "action.h"
#include "board.h"

class State {
 public:
  State() {}
  virtual ~State() {}
  virtual std::shared_ptr<State> Clone() {
    return std::shared_ptr<State>(this);
  };
  virtual double Rollout() {}
  virtual std::vector<int> GetPossibleActions() { return std::vector<int>(); }
  virtual void ApplyAction(const int action) {}
  int GetAction() const;

 protected:
  int action_;
};

class NoGoState : public State {
 public:
  NoGoState(board);
  NoGoState(const NoGoState& s) : board_(s.board_) {}
  NoGoState(NoGoState&& s) : board_(std::move(s.board_)) {}
  NoGoState& operator=(const NoGoState& s) {
    std::cerr << "copying NoGoState" << std::endl;
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
  std::shared_ptr<State> Clone() {
    return std::make_shared<NoGoState>(this->board_);
  }

  virtual ~NoGoState() {}
  virtual double Rollout() override;
  virtual std::vector<int> GetPossibleActions() override;
  virtual void ApplyAction(const int) override;

  //  private:
  board board_;
};

class Node {
 public:
  Node();
  Node(std::shared_ptr<State>);
  std::shared_ptr<Node> Selection() const;
  std::shared_ptr<Node> Expansion(std::shared_ptr<Node>);
  void Update(std::shared_ptr<Node>);
  int GetBestAction() const;
  bool IsLeaf() const;

 private:
  uint32_t visits;
  double value;

  std::shared_ptr<Node> parent;
  std::vector<std::shared_ptr<Node>> kids;
  std::shared_ptr<State> state;
};

int MCTS(State&, int);
