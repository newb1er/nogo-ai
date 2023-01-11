#include <algorithm>
#include <atomic>
#include <list>
#include <mutex>
#include <random>
#include <tuple>

#include "../mcts.h"

using NodeList = std::list<std::shared_ptr<Node>>;

std::tuple<double, int> amaf(const NodeList& list) {
  double amaf_value = 0.0;
  int amaf_visits = 0;

  for (auto& node : list) {
    amaf_value += node->value;
    amaf_visits += node->visits;
  }

  return std::tie(amaf_value, amaf_visits);
}

Node::NodePtr selector(Node::NodePtr node, double bias, unsigned rave_depth,
                       bool minmax, MCTSTree::NodeTable& table) {
  std::vector<size_t> order(node->kids.size());
  std::generate(order.begin(), order.end(), [n = 0]() mutable { return n++; });
  std::shuffle(order.begin(), order.end(),
               std::mt19937(std::random_device()()));

  double revert_ = minmax ? -1.0 : 1.0;
  auto kids = node->kids;
  double l_explore = sqrt(2 * log(node->visits));

  auto n = kids.front();
  auto node_list = table[n->state->GetAction()];
  NodeList list;
  std::copy_if(node_list.begin(), node_list.end(), std::back_inserter(list),
               [node, rave_depth](Node::NodePtr& n_) {
                 return n_->depth > node->depth &&
                        n_->depth < (node->depth + rave_depth);
               });
  auto [amaf_value, amaf_visits] = amaf(list);

  double rave_value = (revert_ * amaf_value / (double)amaf_visits);

  double ucb_value = 0;
  double exploration_value = 0;
  double beta = 1;
  if (n->visits != 0) {
    ucb_value = (revert_ * n->value / n->visits);
    exploration_value = l_explore / sqrt(n->visits);

    beta = (double)amaf_visits / (amaf_visits + n->visits +
                                  4 * bias * bias * amaf_visits * n->visits);
  }

  double best_value =
      ((1 - beta) * ucb_value + beta * rave_value) + exploration_value;

  size_t best_child = 0;

  for (size_t kid : order) {
    // for (size_t kid = 1; kid < kids.size(); ++kid) {
    if (kids.at(kid)->visits == 0) {
      return kids.at(kid);
    }

    n = kids.at(kid);
    node_list = table[n->state->GetAction()];
    list.clear();
    std::copy_if(node_list.begin(), node_list.end(), std::back_inserter(list),
                 [node, rave_depth](Node::NodePtr& n_) {
                   return n_->depth > node->depth &&
                          n_->depth < (node->depth + rave_depth);
                 });
    std::tie(amaf_value, amaf_visits) = amaf(list);

    if (amaf_visits == 0) return n;
    rave_value = (revert_ * amaf_value / amaf_visits);

    if (n->visits == 0) {
      ucb_value = 0;
      exploration_value = 0;
      beta = 1;
    } else {
      ucb_value = (revert_ * kids.at(kid)->value / kids.at(kid)->visits);
      exploration_value = l_explore / sqrt(n->visits);
      beta = (double)amaf_visits / (amaf_visits + n->visits +
                                    4 * bias * bias * amaf_visits * n->visits);
    }

    double v = ((1 - beta) * ucb_value + beta * rave_value) + exploration_value;

    if (v > best_value) {
      best_value = v;
      best_child = kid;
    }
  }

  return kids.at(best_child);
}