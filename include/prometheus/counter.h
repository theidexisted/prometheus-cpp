#pragma once

#include <atomic>

#include "metrics_generated.h"

#include "prometheus/gauge.h"
#include "prometheus/metric.h"

namespace prometheus {
class Counter : Metric {
 public:
  static const io::prometheus::client::MetricType metric_type =
      io::prometheus::client::MetricType_COUNTER;

  void Increment();
  void Increment(double);
  double Value() const;

  metric_collect_t Collect(label_pair_t* global_labels,
                           flatbuffers::FlatBufferBuilder* builder) override;

 private:
  Gauge gauge_;
};
}
