#pragma once

#include <utility>
#include <vector>
#include "metrics_generated.h"

namespace prometheus {
using label_pair_t = std::vector<std::pair<std::string, std::string>>;
using metric_collect_t = flatbuffers::Offset<io::prometheus::client::Metric>;

class Metric {
 public:
  virtual ~Metric() = default;
  virtual metric_collect_t Collect(label_pair_t* global_labels,
                                   flatbuffers::FlatBufferBuilder* builder) = 0;
};
}
