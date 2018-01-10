#pragma once

#include <string>
#include <vector>

#include "prometheus/collectable.h"
#include "prometheus/metrics_generated.h"

namespace prometheus {

class Serializer {
 public:
  virtual ~Serializer() = default;
  virtual std::string Serialize(builders_t& builders) = 0;
};
}
