/**
 * Framework for NoGo and similar games (C++ 11)
 * agent.h: Define the behavior of variants of the player
 *
 * Author: Theory of Computer Games
 *         Computer Games and Intelligence (CGI) Lab, NYCU, Taiwan
 *         https://cgilab.nctu.edu.tw/
 */

#pragma once
#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>

#include "action.h"
#include "board.h"
#include "mcts/mcts.h"

class agent {
 public:
  agent(const std::string& args = "") {
    std::stringstream ss("name=unknown role=unknown " + args);
    for (std::string pair; ss >> pair;) {
      std::string key = pair.substr(0, pair.find('='));
      std::string value = pair.substr(pair.find('=') + 1);
      meta[key] = {value};
    }
  }
  virtual ~agent() {}
  virtual void open_episode(const std::string& flag = "") {}
  virtual void close_episode(const std::string& flag = "") {}
  virtual action take_action(const board& b) { return action(); }
  virtual bool check_for_win(const board& b) { return false; }
  virtual void notify_action(const action& a) {}

 public:
  virtual std::string property(const std::string& key) const {
    return meta.at(key);
  }
  virtual void notify(const std::string& msg) {
    meta[msg.substr(0, msg.find('='))] = {msg.substr(msg.find('=') + 1)};
  }
  virtual std::string name() const { return property("name"); }
  virtual std::string role() const { return property("role"); }

 protected:
  typedef std::string key;
  struct value {
    std::string value;
    operator std::string() const { return value; }
    template <typename numeric,
              typename = typename std::enable_if<
                  std::is_arithmetic<numeric>::value, numeric>::type>
    operator numeric() const {
      return numeric(std::stod(value));
    }
  };
  std::map<key, value> meta;
};

/**
 * base agent for agents with randomness
 */
class random_agent : public agent {
 public:
  random_agent(const std::string& args = "") : agent(args) {
    if (meta.find("seed") != meta.end()) engine.seed(int(meta["seed"]));
  }
  virtual ~random_agent() {}

 protected:
  std::default_random_engine engine;
};

/**
 * random player for both side
 * put a legal piece randomly
 */
class player : public random_agent {
 public:
  player(const std::string& args = "")
      : random_agent("name=random role=unknown " + args),
        space(board::size_x * board::size_y),
        who(board::empty) {
    if (name().find_first_of("[]():; ") != std::string::npos)
      throw std::invalid_argument("invalid name: " + name());
    if (role() == "black") who = board::black;
    if (role() == "white") who = board::white;
    if (who == board::empty)
      throw std::invalid_argument("invalid role: " + role());
    for (size_t i = 0; i < space.size(); i++) space[i] = action::place(i, who);
  }

  virtual action take_action(const board& state) {
    std::shuffle(space.begin(), space.end(), engine);
    for (const action::place& move : space) {
      board after = state;
      if (move.apply(after) == board::legal) return move;
    }
    return action();
  }

 private:
  std::vector<action::place> space;
  board::piece_type who;
};

class MCTSAgent : public agent {
 public:
  MCTSAgent(const std::string& args = "")
      : agent("name=MCTSAgent role=unknown " + args) {
    if (meta.find("T") != meta.end()) {
      simulation_count = (int(meta["T"]));
    }
    if (role() == "black") who = board::black;
    if (role() == "white") who = board::white;
    if (who == board::empty)
      throw std::invalid_argument("invalid role: " + role());
  }

  virtual void open_episode(const std::string& flag = "") { root_init = false; }

  virtual action take_action(const board& state) {
    NoGoState no_go_state(state);
    if (root_init == false) {
      CreateRootNode(no_go_state);
      root_init = true;
    } else {
      root = root->FindChild(no_go_state);
    }

    int act = MCTS(root, simulation_count, true);
    if (act == -1) return action();
    return action::place(act, who);
  }

  virtual void close_episode(const std::string& flag = "") {
    root_init = false;
  }

  virtual void notify_action(const action& a) {}

 private:
  int simulation_count = 100;
  board::piece_type who;

  MCTSNodePtr root;
  bool root_init = false;
};

agent* make_agent(const std::string& args = "") {
  std::string type;
  auto first_space = args.find(" ");
  auto first_equal = args.find("=");

  if (first_space > first_equal) {
    type = "random";
  } else {
    type = args.substr(0, first_space);
  }

  std::stringstream ss(args);
  for (std::string pair; ss >> pair;) {
    std::string key = pair.substr(0, pair.find('='));
    std::string value = pair.substr(pair.find('=') + 1);
    if (key == "search") type = value;
  }

  std::transform(type.begin(), type.end(), type.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (type == "random") {
    return new player(args);
  } else if (type == "mcts") {
    return new MCTSAgent(args);
  }
  return nullptr;
}