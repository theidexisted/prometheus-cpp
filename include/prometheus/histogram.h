#pragma once

#include <vector>

#include "prometheus/counter.h"

#include "metrics_generated.h"

namespace prometheus {
class Histogram : public Metric {
 public:
  using BucketBoundaries = std::vector<double>;

  static const io::prometheus::client::MetricType metric_type =
      io::prometheus::client::MetricType_HISTOGRAM;

  Histogram(const BucketBoundaries& buckets);

  void Observe(double value);

  metric_collect_t Collect(label_pair_t* global_labels,
                           flatbuffers::FlatBufferBuilder* builder) override;

 private:
  const BucketBoundaries bucket_boundaries_;
  std::vector<Counter> bucket_counts_;
  Counter sum_;
};
}
