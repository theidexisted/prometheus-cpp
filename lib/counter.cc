#include "prometheus/counter.h"

using namespace io::prometheus::client;
namespace prometheus {

void Counter::Increment() { gauge_.Increment(); }

void Counter::Increment(double val) { gauge_.Increment(val); }

double Counter::Value() const { return gauge_.Value(); }

metric_collect_t Counter::Collect(label_pair_t* global_labels,
                                  flatbuffers::FlatBufferBuilder* builder) {
  std::vector<flatbuffers::Offset<LabelPair>> labels_vec;
  for (const auto& p : *global_labels) {
    auto name = builder->CreateString(p.first);
    auto help = builder->CreateString(p.second);

    labels_vec.emplace_back(CreateLabelPair(*builder, name, help));
  }
  auto labels = (*builder).CreateVector(labels_vec);

  auto counter = CreateCounter(*builder, Value());
  auto metric = CreateMetric(*builder, labels, 0, counter);

  return metric;
}
}
