// https://github.com/open-telemetry/opentelemetry-cpp/blob/main/examples/grpc/tracer_common.h
// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <cstring>
#include <iostream>
#include <vector>
#include <chrono>
#include <memory>

#include <grpcpp/grpcpp.h>

#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_grpc_metric_exporter_factory.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/aggregation/default_aggregation.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_context_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/provider.h"

using grpc::ClientContext;
using grpc::ServerContext;

namespace
{

auto inline init_metrics(const std::string& collector) -> void
{
  opentelemetry::exporter::otlp::OtlpGrpcMetricExporterOptions metric_opts {};
  metric_opts.endpoint = collector;
  auto exporter =
      opentelemetry::exporter::otlp::OtlpGrpcMetricExporterFactory::Create(
          metric_opts);

  std::string version {"1.2.0"};
  std::string schema {"https://opentelemetry.io/schemas/1.2.0"};

  // Initialize and set the global MeterProvider
  opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions options{};
  options.export_interval_millis = std::chrono::milliseconds(1000);
  options.export_timeout_millis = std::chrono::milliseconds(500);
  std::unique_ptr<opentelemetry::sdk::metrics::MetricReader> reader {
      new opentelemetry::sdk::metrics::PeriodicExportingMetricReader(
          std::move(exporter), options)};
  auto provider = std::shared_ptr<opentelemetry::metrics::MeterProvider>(
      new opentelemetry::sdk::metrics::MeterProvider());
  auto p = std::static_pointer_cast<opentelemetry::sdk::metrics::MeterProvider>(
      provider);
  p->AddMetricReader(std::move(reader));

  opentelemetry::metrics::Provider::SetMeterProvider(provider);
}

auto inline cleanup_metrics() -> void
{
  std::shared_ptr<opentelemetry::metrics::MeterProvider> none;
  opentelemetry::metrics::Provider::SetMeterProvider(none);
}

class GrpcClientCarrier
    : public opentelemetry::context::propagation::TextMapCarrier
{
public:
  GrpcClientCarrier(ClientContext* context_in)
      : context(context_in)
  {
  }
  GrpcClientCarrier() = default;
  virtual opentelemetry::nostd::string_view Get(
      opentelemetry::nostd::string_view /* key */) const noexcept override
  {
    return "";
  }

  virtual void Set(opentelemetry::nostd::string_view key,
                   opentelemetry::nostd::string_view value) noexcept override
  {
    std::cout << " Client ::: Adding " << key << " " << value << "\n";
    context->AddMetadata(std::string(key), std::string(value));
  }

  ClientContext* context;
};

class GrpcServerCarrier
    : public opentelemetry::context::propagation::TextMapCarrier
{
public:
  GrpcServerCarrier(ServerContext* context_in)
      : context(context_in)
  {
  }
  GrpcServerCarrier() = default;
  virtual opentelemetry::nostd::string_view Get(
      opentelemetry::nostd::string_view key) const noexcept override
  {
    auto iter = context->client_metadata().find({key.data(), key.size()});
    if (iter != context->client_metadata().end()) {
      return iter->second.data();
    }
    return "";
  }

  virtual void Set(
      opentelemetry::nostd::string_view /* key */,
      opentelemetry::nostd::string_view /* value */) noexcept override
  {
    // Not required for server
  }

  ServerContext* context;
};

auto inline init_tracer(const std::string& collector) -> void
{
  // https://opentelemetry-cpp.readthedocs.io/en/latest/sdk/GettingStarted.html#exporter
  opentelemetry::exporter::otlp::OtlpGrpcExporterOptions opts {};
  opts.endpoint = collector;
  std::cout << "OTEL_GRPC_EXPORTER_ENDPOINT: " << opts.endpoint << std::endl;

  // Create OTLP exporter instance
  auto exporter =
      opentelemetry::exporter::otlp::OtlpGrpcExporterFactory::Create(opts);
  auto processor =
      opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(
          std::move(exporter));
  // std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
  //     opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(processor));
  std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
      opentelemetry::sdk::trace::TracerProviderFactory::Create(
          std::move(processor),
          opentelemetry::sdk::resource::Resource::Create({
              {"service.name", "server"},
              {"service.namespace", "test"},
          }));
  // Set the global trace provider
  opentelemetry::trace::Provider::SetTracerProvider(provider);

  // set global propagator
  opentelemetry::context::propagation::GlobalTextMapPropagator::
      SetGlobalPropagator(
          opentelemetry::nostd::shared_ptr<
              opentelemetry::context::propagation::TextMapPropagator>(
              new opentelemetry::trace::propagation::HttpTraceContext()));
}

auto inline cleanup_tracer() -> void
{
  std::shared_ptr<opentelemetry::trace::TracerProvider> none;
  opentelemetry::trace::Provider::SetTracerProvider(none);
}

auto inline get_tracer(
    const std::string &tracer_name) -> opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer>
{
  auto provider = opentelemetry::trace::Provider::GetTracerProvider();
  return provider->GetTracer(tracer_name);
}

}  // namespace