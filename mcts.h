#pragma once
#include <cassert>
#include <functional>
#include <map>
#include <vector>

#include "action.h"
#include "board.h"

class State {
 public:
  State() {}
  virtual ~State() {}
  virtual State* Clone() { return this; };
  virtual double Rollout() {}
  virtual int GetPossibleAction() { return -1; }
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
  State* Clone() { return new NoGoState(this->board_); }

  virtual ~NoGoState() {}
  virtual double Rollout() override;
  virtual int GetPossibleAction() override;
  virtual void ApplyAction(const int) override;

  //  private:
  board board_;
};

class Node {
 public:
  Node();
  Node(State*);
  Node* Selection() const;
  Node* Expansion();
  void Update();
  int GetBestAction() const;
  bool IsLeaf() const;

 private:
  std::vector<Node*> kids;
  Node* parent;
  uint32_t visits;
  double value;
  State* state;
};

int MCTS(State&, int);
