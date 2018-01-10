#pragma once

#include <vector>
#include "metrics_generated.h"

namespace io {
namespace prometheus {
namespace client {
class MetricFamily;
}
}
}

namespace prometheus {
using builders_t = std::vector<std::shared_ptr<flatbuffers::FlatBufferBuilder>>;
using bld_t = std::shared_ptr<flatbuffers::FlatBufferBuilder>;
inline bld_t make_bld_t() {
  return std::make_shared<flatbuffers::FlatBufferBuilder>();
}
class Collectable {
 public:
  virtual ~Collectable() = default;
  virtual builders_t Collect() = 0;
};
}
