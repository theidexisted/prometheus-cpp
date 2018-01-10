#pragma once

#include <atomic>

#include "prometheus/collectable.h"
#include "prometheus/metric.h"

#include "metrics_generated.h"

namespace prometheus {

class Gauge : public Metric {
 public:
  static const io::prometheus::client::MetricType metric_type =
      io::prometheus::client::MetricType_GAUGE;

  Gauge();
  Gauge(double);
  void Increment();
  void Increment(double);
  void Decrement();
  void Decrement(double);
  void Set(double);
  void SetToCurrentTime();
  double Value() const;

  metric_collect_t Collect(label_pair_t* global_labels,
                           flatbuffers::FlatBufferBuilder* builder) override;

 private:
  void Change(double);
  std::atomic<double> value_;
};
}
