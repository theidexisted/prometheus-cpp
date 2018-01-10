#include <ctime>

#include "prometheus/gauge.h"

namespace prometheus {
Gauge::Gauge() : value_{0} {}

Gauge::Gauge(double value) : value_{value} {}

void Gauge::Increment() { Increment(1.0); }
void Gauge::Increment(double value) {
  if (value < 0.0) {
    return;
  }
  Change(value);
}

void Gauge::Decrement() { Decrement(1.0); }

void Gauge::Decrement(double value) {
  if (value < 0.0) {
    return;
  }
  Change(-1.0 * value);
}

void Gauge::Set(double value) { value_.store(value); }

void Gauge::Change(double value) {
  auto current = value_.load();
  while (!value_.compare_exchange_weak(current, current + value))
    ;
}

void Gauge::SetToCurrentTime() {
  auto time = std::time(nullptr);
  Set(static_cast<double>(time));
}

double Gauge::Value() const { return value_; }

metric_collect_t Gauge::Collect(label_pair_t* global_labels,
                                flatbuffers::FlatBufferBuilder* builder) {
  using namespace io::prometheus::client;
  std::vector<flatbuffers::Offset<LabelPair>> labels_vec;
  for (const auto& p : *global_labels) {
    auto name = builder->CreateString(p.first);
    auto help = builder->CreateString(p.second);

    labels_vec.emplace_back(CreateLabelPair(*builder, name, help));
  }
  auto labels = (*builder).CreateVector(labels_vec);

  auto gauge = CreateGauge(*builder, Value());
  auto metric = CreateMetric(*builder, labels, gauge);

  return metric;
}
/*
io::prometheus::client::Metric Gauge::Collect() {
  io::prometheus::client::Metric metric;
  auto gauge = metric.mutable_gauge();
  gauge->set_value(Value());
  return metric;
}
*/
}
