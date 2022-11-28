#include "../mcts.h"

std::shared_ptr<Node> selector(std::shared_ptr<Node> node, bool minmax) {
  double revert_ = minmax ? -1.0 : 1.0;
  auto& kids = node->kids;

  double best_value = (revert_ * kids.front()->value / kids.front()->visits) +
                      sqrt(2 * log(node->visits) / kids.front()->visits);
  std::shared_ptr<Node> best_child = kids.front();
  for (auto& kid : kids) {
    if (kid->visits == 0) return kid;

    double v = (revert_ * kid->value / kid->visits) +
               sqrt(2 * log(node->visits) / kid->visits);

    if (v > best_value) {
      best_value = v;
      best_child = kid;
    }
  }

  if (best_child == nullptr) {
    // No child was selected. This should never happen.
    std::cerr << "No child was selected." << std::endl;
    assert(false);
  }

  return best_child;
}