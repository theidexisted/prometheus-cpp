#include "text_serializer.h"
#include <cmath>
#include <iostream>
#include <sstream>

namespace prometheus {

using namespace io::prometheus::client;

namespace {

// Write a double as a string, with proper formatting for infinity and NaN
std::string ToString(double v) {
  if (std::isnan(v)) {
    return "Nan";
  }
  if (std::isinf(v)) {
    return (v < 0 ? "-Inf" : "+Inf");
  }
  return std::to_string(v);
}

const std::string& EscapeLabelValue(const std::string& value,
                                    std::string* tmp) {
  bool copy = false;
  for (size_t i = 0; i < value.size(); ++i) {
    auto c = value[i];
    if (c == '\\' || c == '"' || c == '\n') {
      if (!copy) {
        tmp->reserve(value.size() + 1);
        tmp->assign(value, 0, i);
        copy = true;
      }
      if (c == '\\') {
        tmp->append("\\\\");
      } else if (c == '"') {
        tmp->append("\\\"");
      } else {
        tmp->append("\\\n");
      }
    } else if (copy) {
      tmp->push_back(c);
    }
  }
  return copy ? *tmp : value;
}

// Write a line header: metric name and labels
void WriteHead(std::ostream& out, const MetricFamily* family,
               const Metric* metric, const std::string& suffix = "",
               const std::string& extraLabelName = "",
               const std::string& extraLabelValue = "") {
  out << family->name()->str() << suffix;
  if (metric->label()->size() != 0 || !extraLabelName.empty()) {
    out << "{";
    const char* prefix = "";
    std::string tmp;

    auto label = metric->label();

    for (unsigned int i = 0; i < label->size(); ++i) {
      auto lp = label->Get(i);
      out << prefix << lp->name()->str() << "=\""
          << EscapeLabelValue(lp->value()->str(), &tmp) << "\"";
      prefix = ",";
    }

    /*
for (auto& lp : metric.label()) {
  out << prefix << lp.name() << "=\"" << EscapeLabelValue(lp.value(), &tmp)
      << "\"";
  prefix = ",";
}
    */
    if (!extraLabelName.empty()) {
      out << prefix << extraLabelName << "=\""
          << EscapeLabelValue(extraLabelValue, &tmp) << "\"";
    }
    out << "}";
  }
  out << " ";
}

// Write a line trailer: timestamp
void WriteTail(std::ostream& out, const Metric* metric) {
  if (metric->timestamp_ms() != 0) {
    out << " " << metric->timestamp_ms();
  }
  out << "\n";
}

void SerializeCounter(std::ostream& out, const MetricFamily* family,
                      const Metric* metric) {
  WriteHead(out, family, metric);
  out << ToString(metric->counter()->value());
  WriteTail(out, metric);
}

void SerializeGauge(std::ostream& out, const MetricFamily* family,
                    const Metric* metric) {
  WriteHead(out, family, metric);
  out << ToString(metric->gauge()->value());
  WriteTail(out, metric);
}

void SerializeSummary(std::ostream& out, const MetricFamily* family,
                      const Metric* metric) {
  const auto& sum = metric->summary();
  WriteHead(out, family, metric, "_count");
  out << sum->sample_count();
  WriteTail(out, metric);

  WriteHead(out, family, metric, "_sum");
  out << ToString(sum->sample_sum());
  WriteTail(out, metric);

  auto quantiles = sum->quantile();
  for (unsigned int i = 0; i < quantiles->size(); ++i) {
    WriteHead(out, family, metric, "_quantile", "quantile",
              ToString(quantiles->Get(i)->quantile()));
    out << ToString(quantiles->Get(i)->value());
    WriteTail(out, metric);
  }

  /*
	auto& sum = metric.summary();
	WriteHead(out, family, metric, "_count");
	out << sum.sample_count();
	WriteTail(out, metric);

	WriteHead(out, family, metric, "_sum");
	out << ToString(sum.sample_sum());
	WriteTail(out, metric);

	for (auto& q : sum.quantile()) {
	WriteHead(out, family, metric, "_quantile", "quantile",
			ToString(q.quantile()));
	out << ToString(q.value());
	WriteTail(out, metric);
	}
	*/
}

void SerializeUntyped(std::ostream& out, const MetricFamily* family,
                      const Metric* metric) {
  WriteHead(out, family, metric);
  out << ToString(metric->untyped()->value());
  WriteTail(out, metric);
}

void SerializeHistogram(std::ostream& out, const MetricFamily* family,
                        const Metric* metric) {
  const auto& hist = metric->histogram();
  WriteHead(out, family, metric, "_count");
  out << hist->sample_count();
  WriteTail(out, metric);

  WriteHead(out, family, metric, "_sum");
  out << ToString(hist->sample_sum());
  WriteTail(out, metric);

  double last = -std::numeric_limits<double>::infinity();
  auto bucket = hist->bucket();
  for (unsigned int i = 0; i < bucket->size(); ++i) {
    auto b = bucket->Get(i);
    WriteHead(out, family, metric, "_bucket", "le", ToString(b->upper_bound()));
    last = b->upper_bound();
    out << b->cumulative_count();
    WriteTail(out, metric);
  }
  /*
  for (auto& b : hist->bucket()) {
    WriteHead(out, family, metric, "_bucket", "le", ToString(b.upper_bound()));
    last = b.upper_bound();
    out << b.cumulative_count();
    WriteTail(out, metric);
  }
  */

  if (last != std::numeric_limits<double>::infinity()) {
    WriteHead(out, family, metric, "_bucket", "le", "+Inf");
    out << hist->sample_count();
    WriteTail(out, metric);
  }
}

void SerializeFamily(std::ostream& out, const MetricFamily* family) {
  if (!family->help()->str().empty()) {
    out << "# HELP " << family->name()->str() << " " << family->help()->str()
        << "\n";
  }

  auto metrics = family->metric();
  switch (family->type()) {
    case MetricType_COUNTER:
      out << "# TYPE " << family->name()->str() << " counter\n";
      for (unsigned int i = 0; i < metrics->size(); ++i) {
        SerializeCounter(out, family, metrics->Get(i));
      }
      break;

    case MetricType_GAUGE:
      out << "# TYPE " << family->name()->str() << " gauge\n";
      for (unsigned int i = 0; i < metrics->size(); ++i) {
        SerializeGauge(out, family, metrics->Get(i));
      }
      break;

    case MetricType_SUMMARY:
      out << "# TYPE " << family->name()->str() << " summary\n";
      for (unsigned int i = 0; i < metrics->size(); ++i) {
        SerializeSummary(out, family, metrics->Get(i));
      }
      break;

    case MetricType_UNTYPED:
      out << "# TYPE " << family->name()->str() << " untyped\n";
      for (unsigned int i = 0; i < metrics->size(); ++i) {
        SerializeUntyped(out, family, metrics->Get(i));
      }
      break;

    case MetricType_HISTOGRAM:
      out << "# TYPE " << family->name()->str() << " histogram\n";
      for (unsigned int i = 0; i < metrics->size(); ++i) {
        SerializeHistogram(out, family, metrics->Get(i));
      }
      break;

    default:
      break;
  }
}
}

std::string TextSerializer::Serialize(builders_t& builders) {
  std::ostringstream ss;
  for (auto& family : builders) {
    SerializeFamily(ss, GetMetricFamily(family->GetBufferPointer()));
  }
  return ss.str();
}
}
