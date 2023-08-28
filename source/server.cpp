#include <iostream>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

// https://github.com/open-telemetry/opentelemetry-cpp/blob/32cd66b62333e84aa8e92a4447e0aa667b6735e5/examples/otlp/README.md#additional-notes-regarding-abseil-library
// grpcpp/grpcpp.h should be included before any other api headers to avoid
// error c2872: 'internal': ambiguous symbol in abseil
#include <fmt/core.h>
#include <grpcpp/grpcpp.h>
#include <nlohmann/json.hpp>

#include "elastic_client.h"
#include "opentelemetry/trace/context.h"
#include "opentelemetry/trace/semantic_conventions.h"
#include "opentelemetry/trace/span_context_kv_iterable_view.h"
#include "mimicwindows.grpc.pb.h"
#include "mimicwindows.pb.h"

#include "tracer_common.h"

class mimicwindows_service final : public mimicwindows::MimicService::Service
{
  elastic_client* m_ec {};

  grpc::Status Update(grpc::ServerContext* context,
                         grpc::ServerReader<mimicwindows::Request>* reader,
                         [[maybe_unused]] mimicwindows::Response* response) override
  {
    // Create a SpanOptions object and set the kind to Server to inform OpenTel.
    opentelemetry::trace::StartSpanOptions options;
    options.kind = opentelemetry::trace::SpanKind::kServer;

    // extract context from grpc metadata
    GrpcServerCarrier carrier(context);

    auto prop = opentelemetry::context::propagation::GlobalTextMapPropagator::
        GetGlobalPropagator();
    auto current_ctx = opentelemetry::context::RuntimeContext::GetCurrent();
    auto new_context = prop->Extract(carrier, current_ctx);
    options.parent = opentelemetry::trace::GetSpan(new_context)->GetContext();

    std::string span_name = "MimicService/MimicWindowsUpdate";
    auto span = get_tracer("grpc")->StartSpan(
        span_name,
        {{opentelemetry::trace::SemanticConventions::kRpcSystem, "grpc"},
         {opentelemetry::trace::SemanticConventions::kRpcService, "MimicWindowsService"},
         {opentelemetry::trace::SemanticConventions::kRpcMethod, "SpyUpdate"},
         {opentelemetry::trace::SemanticConventions::kRpcGrpcStatusCode, 0}},
        options);
    auto scope = get_tracer("grpc")->WithActiveSpan(span);

    std::string json_string {};
    google::protobuf::util::JsonPrintOptions opts {};
    opts.always_print_primitive_fields = true;
    opts.preserve_proto_field_names = true;
    mimicwindows::Request request {};
    while (reader->Read(&request)) {
      google::protobuf::util::MessageToJsonString(request, &json_string, opts);
      m_ec->post_data(json_string);
      break;
    }
    // Make sure to end your spans!
    span->End();
    return grpc::Status::OK;
  }

public:
  mimicwindows_service(const mimicwindows_service&) = default;
  mimicwindows_service(mimicwindows_service&&) = delete;
  mimicwindows_service& operator=(const mimicwindows_service&) = default;
  mimicwindows_service& operator=(mimicwindows_service&&) = delete;
  mimicwindows_service(std::string& elastic_host_in,
                     std::string& user_name_in,
                     std::string& password_in)
    : m_ec(new elastic_client(elastic_host_in, user_name_in, password_in))
  {
  }

  ~mimicwindows_service()  { delete m_ec; }
};

void run_server(std::string& elastic_host,
               std::string& user_name,
               std::string& password)
{
  std::string server_address {"[::]:50055"};
  mimicwindows_service service(elastic_host, user_name, password);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  fmt::print("Server listening on  {}\n", server_address);
  server->Wait();
}

auto main(int argc, char** argv) -> int
{
  CLI::App app {"MiniSpy gRPC Server"};
  app.allow_windows_style_options();

  std::string collector = "http://localhost:4317";
  std::string elasticsearch = "http://localhost:9200";
  std::string user_name {};
  std::string password {};

  auto *tmp = std::getenv("ES_SERVER_URLS");
  if (tmp != nullptr) {
    elasticsearch = std::string(tmp);
  }

  tmp = std::getenv("JAEGERCOLLECTOR_URL");
  if (tmp != nullptr) {
    collector = std::string(tmp);
  }

  app.add_option(
      "-e,--elasticsearch",
      elasticsearch,
      "Address of ElasticSearch host. Example: http://10.0.0.1:9200");
  app.add_option(
      "-j,--jaeger",
      collector,
      "Address of Jaeger otlp collector. Example: http://10.0.0.1:4317");
  app.add_option("-u,--username", user_name, "ElasticSearch user name");
  app.add_option("-p,--password", password, "ElasticSearch password");

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return app.exit(e);
  }
  init_tracer(collector);
  init_metrics(collector);
  run_server(elasticsearch, user_name, password);
  cleanup_metrics();
  cleanup_tracer();
  return 0;
}