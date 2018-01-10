#pragma once

#include <chrono>
#include "histogram.h"

namespace prometheus {
class Marker {
 public:
  Marker(Histogram& histogram)
      : time_point_(std::chrono::steady_clock::now()), histogram_(histogram) {}

  ~Marker() {
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::steady_clock::now() - time_point_).count();
    histogram_.Observe(diff);
  }

 private:
  std::chrono::steady_clock::time_point time_point_;
  Histogram& histogram_;
};
}  // namespace prometheus
