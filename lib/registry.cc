#include "prometheus/registry.h"
#include "prometheus/exposer.h"

namespace prometheus {
std::shared_ptr<Registry> Registry::Create(Exposer& exposer) {
  std::shared_ptr<Registry> reg = std::make_shared<Registry>();
  exposer.RegisterCollectable(reg);
  return reg;
}

Family<Counter>& Registry::AddCounter(
    const std::string& name, const std::string& help,
    const std::map<std::string, std::string>& labels) {
  std::lock_guard<std::mutex> lock{mutex_};
  auto counter_family = new Family<Counter>(name, help, labels);
  collectables_.push_back(std::unique_ptr<Collectable>{counter_family});
  return *counter_family;
}

Family<Gauge>& Registry::AddGauge(
    const std::string& name, const std::string& help,
    const std::map<std::string, std::string>& labels) {
  std::lock_guard<std::mutex> lock{mutex_};
  auto gauge_family = new Family<Gauge>(name, help, labels);
  collectables_.push_back(std::unique_ptr<Collectable>{gauge_family});
  return *gauge_family;
}

Family<Histogram>& Registry::AddHistogram(
    const std::string& name, const std::string& help,
    const std::map<std::string, std::string>& labels) {
  std::lock_guard<std::mutex> lock{mutex_};
  auto histogram_family = new Family<Histogram>(name, help, labels);
  collectables_.push_back(std::unique_ptr<Collectable>{histogram_family});
  return *histogram_family;
}

builders_t Registry::Collect() {
  std::lock_guard<std::mutex> lock{mutex_};
  auto results = builders_t{};
  for (auto&& collectable : collectables_) {
    auto metrics = collectable->Collect();
    results.insert(results.end(), metrics.begin(), metrics.end());
  }

  return results;
}
}
