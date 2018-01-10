#include <chrono>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <prometheus/exposer.h>
#include <prometheus/marker.h>
#include <prometheus/registry.h>

using namespace prometheus;

auto registry = Registry::Create(Exposer::GetInstance());

auto& histogram_family = BuildHistogram()
                             .Name("function_call_latency")
                             .Help("Time cost in the functinon, in us")
                             .Labels({{"label", "value"}})
                             .Register(*registry);
auto& histogram1 = histogram_family.Add(
    {{"name", "foo"}},
    Histogram::BucketBoundaries{1, 10, 50, 100, 1000, 5000, 10000, 50000, 100000, 500000, 10000000});

auto& histogram2 = histogram_family.Add(
    {{"name", "bar"}},
    Histogram::BucketBoundaries{1, 10, 50, 100, 1000, 5000, 10000, 50000, 100000, 500000, 10000000});

int main(int argc, char** argv) {
  std::vector<std::thread> thrs;

  thrs.emplace_back(std::thread([]() {
    for (;;) {
      prometheus::Marker marker(histogram1);
      std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100));
    }
  }));

  thrs.emplace_back(std::thread([]() {
    for (;;) {
      prometheus::Marker marker(histogram2);
      std::this_thread::sleep_for(std::chrono::microseconds(rand() % 100));
    }

  }));

  for (auto& t : thrs) {
    t.join();
  }

  return 0;
}
