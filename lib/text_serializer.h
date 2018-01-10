#pragma once

#include <string>
#include <vector>

#include "serializer.h"

namespace prometheus {

class TextSerializer : public Serializer {
 public:
  virtual std::string Serialize(builders_t& builders) override;
};
}
