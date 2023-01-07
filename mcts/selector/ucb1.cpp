#include "../mcts.h"

const double explore_param = 0.5;

std::shared_ptr<Node> selector(std::shared_ptr<Node> node, bool minmax) {
  double revert_ = minmax ? -1.0 : 1.0;
  auto& kids = node->kids;

  double l_explore = sqrt(log(node->visits));
  double best_value = (revert_ * kids.front()->value / kids.front()->visits) +
                      (explore_param * l_explore / sqrt(kids.front()->visits));
  size_t best_child = 0;

  for (size_t kid = 0; kid < kids.size(); ++kid) {
    if (kids.at(kid)->visits == 0) {
      return kids.at(kid);
    }

    double v = (revert_ * kids.at(kid)->value / kids.at(kid)->visits) +
               (l_explore / sqrt(kids.at(kid)->visits));

    if (v > best_value) {
      best_value = v;
      best_child = kid;
    }
  }

  // if (best_child == nullptr) {
  //   // No child was selected. This should never happen.
  //   std::cerr << "No child was selected." << std::endl;
  //   assert(false);
  // }

  return kids.at(best_child);
}