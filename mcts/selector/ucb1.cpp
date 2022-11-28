#include <atomic>
#include <mutex>

#include "../mcts.h"

std::shared_ptr<Node> selector(std::shared_ptr<Node> node, bool minmax) {
  std::mutex mutex;
  double revert_ = minmax ? -1.0 : 1.0;
  auto& kids = node->kids;

  double best_value = (revert_ * kids.front()->value / kids.front()->visits) +
                      sqrt(2 * log(node->visits) / kids.front()->visits);
  size_t best_child = 0;
  std::atomic<bool> found(false);

#pragma omp parallel for
  for (size_t kid = 0; kid < kids.size(); ++kid) {
    if (found.load()) continue;
    if (kids.at(kid)->visits == 0) {
      found.store(true);
      std::lock_guard<std::mutex> lock(mutex);
      best_child = kid;
    }

    double v = (revert_ * kids.at(kid)->value / kids.at(kid)->visits) +
               sqrt(2 * log(node->visits) / kids.at(kid)->visits);

    if (v > best_value) {
      std::lock_guard<std::mutex> lock(mutex);
      if (found.load()) continue;
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