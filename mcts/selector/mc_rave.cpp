#include <atomic>
#include <mutex>
#include <tuple>

#include "../mcts.h"

using ActionNodeList = std::vector<std::vector<std::shared_ptr<Node>>>;
using NodeList = std::vector<std::shared_ptr<Node>>;

std::tuple<double, int> amaf(const std::shared_ptr<Node>& node,
                             const NodeList& list) {
  double amaf_value = 0.0;
  int amaf_visits = 0;

  for (auto& kid : list) {
    auto n_ = kid;
    while (!(n_->parent.expired()) && n_->parent.lock() != nullptr) {
      if (n_->parent.lock() == node) {
        amaf_value += kid->value;
        amaf_visits += kid->visits;
        break;
      }
      n_ = n_->parent.lock();
    };
  }

  return std::tie(amaf_value, amaf_visits);
}

std::shared_ptr<Node> selector(std::shared_ptr<Node> node, bool minmax,
                               ActionNodeList& list) {
  double revert_ = minmax ? -1.0 : 1.0;
  double bias = 0;
  //   double beta = 0.5;
  auto& kids = node->kids;

  double amaf_value;
  int amaf_visits;
  double ucb_value = (revert_ * kids.front()->value / kids.front()->visits);
  std::tie(amaf_value, amaf_visits) =
      amaf(kids.front(), list.at(kids.front()->state->GetAction()));
  double rave_value = (revert_ * amaf_value / amaf_visits);
  double exploration_value = sqrt(2 * log(node->visits) / kids.front()->visits);
  double beta = (double)amaf_visits /
                (amaf_visits + kids.front()->visits +
                 4 * bias * bias * amaf_visits * kids.front()->visits);

  double best_value =
      ((1 - beta) * ucb_value + beta * rave_value) + exploration_value;
  size_t best_child = 0;

  for (size_t kid = 1; kid < kids.size(); ++kid) {
    if (kids.at(kid)->visits == 0) {
      return kids.at(kid);
    }

    ucb_value = (revert_ * kids.at(kid)->value / kids.at(kid)->visits);
    std::tie(amaf_value, amaf_visits) =
        amaf(kids.front(), list.at(kids.front()->state->GetAction()));
    rave_value = (revert_ * amaf_value / amaf_visits);
    exploration_value = sqrt(2 * log(node->visits) / kids.at(kid)->visits);
    beta = (double)amaf_visits /
           (amaf_visits + kids.at(kid)->visits +
            4 * bias * bias * amaf_visits * kids.at(kid)->visits);

    double v = ((1 - beta) * ucb_value + beta * rave_value) + exploration_value;

    if (v > best_value) {
      best_value = v;
      best_child = kid;
    }
  }

  return kids.at(best_child);
}