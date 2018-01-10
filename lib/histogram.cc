#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>

#include "prometheus/histogram.h"

namespace prometheus {

Histogram::Histogram(const BucketBoundaries& buckets)
    : bucket_boundaries_(buckets), bucket_counts_(buckets.size() + 1) {
  assert(std::is_sorted(std::begin(bucket_boundaries_),
                        std::end(bucket_boundaries_)));
}

void Histogram::Observe(double value) {
  // TODO: determine bucket list size at which binary search would be faster
  // NOTE: linear search is faster than binary search when array is small
  auto bucket_index = static_cast<std::size_t>(std::distance(
      bucket_boundaries_.begin(),
      std::find_if(bucket_boundaries_.begin(), bucket_boundaries_.end(),
                   [value](double boundary) { return boundary > value; })));
  sum_.Increment(value);
  bucket_counts_[bucket_index].Increment();
}

metric_collect_t Histogram::Collect(label_pair_t* global_labels,
                                    flatbuffers::FlatBufferBuilder* builder) {
  using namespace io::prometheus::client;
  std::vector<flatbuffers::Offset<LabelPair>> labels_vec;
  for (const auto& p : *global_labels) {
    auto name = builder->CreateString(p.first);
    auto help = builder->CreateString(p.second);

    labels_vec.emplace_back(CreateLabelPair(*builder, name, help));
  }
  auto labels = (*builder).CreateVector(labels_vec);

  auto sample_count = std::accumulate(
      bucket_counts_.begin(), bucket_counts_.end(), double{0},
      [](double sum, const Counter& counter) { return sum + counter.Value(); });
  auto sum = sum_.Value();

  std::vector<flatbuffers::Offset<Bucket>> bucket_vec;
  auto cumulative_count = 0ULL;

  for (std::size_t i = 0; i < bucket_counts_.size(); i++) {
    cumulative_count += bucket_counts_[i].Value();

    auto val = (i == bucket_boundaries_.size())
                   ? std::numeric_limits<double>::infinity()
                   : bucket_boundaries_[i];
    bucket_vec.emplace_back(CreateBucket(*builder, cumulative_count, val));
  }
  auto buckets = (*builder).CreateVector(bucket_vec);

  auto histogram = CreateHistogram(*builder, sample_count, sum, buckets);
  auto metric = CreateMetric(*builder, labels, 0, 0, 0, 0, histogram);

  return metric;
}
}
