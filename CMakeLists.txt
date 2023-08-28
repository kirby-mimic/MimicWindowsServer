cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    MimicWindowsServer
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# enable FetchContent
include(FetchContent)

FetchContent_Declare(protobufs                              # name of the content
  GIT_REPOSITORY https://github.com/kirby-mimic/protobufs.git   # the repository
  GIT_TAG        main                                       # the tag
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(protobufs)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)


find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)
find_package(prometheus-cpp CONFIG REQUIRED)
find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(sentry CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

message(STATUS "Using gRPC ${gRPC_VERSION}")
message(STATUS "Using Protobuf ${Protobuf_VERSION}")
message(STATUS "Using OpenTelemetry-cpp ${opentelemetry-cpp_VERSION}")
message(STATUS "Using sentry-native ${sentry_VERSION}")
message(STATUS "protobufs target include directories: ${protobufs_BINARY_DIR}")

# ---- Declare library ----

# ---- Declare executable ----

add_executable(MimicWindowsServer_exe source/server.cpp
source/elastic_client.cpp)
add_executable(MimicWindowsServer::exe ALIAS MimicWindowsServer_exe)

set_property(TARGET MimicWindowsServer_exe PROPERTY OUTPUT_NAME MimicWindowsServer)

target_compile_features(MimicWindowsServer_exe PRIVATE cxx_std_17)

target_include_directories(MimicWindowsServer_exe PRIVATE ${protobufs_BINARY_DIR}/protos/mimicwindows)

target_link_libraries(MimicWindowsServer_exe PRIVATE
    mimicwindows
    opentelemetry-cpp::trace
    opentelemetry-cpp::otlp_grpc_exporter
    opentelemetry-cpp::ostream_span_exporter
    opentelemetry-cpp::otlp_grpc_metrics_exporter
    fmt::fmt
    nlohmann_json::nlohmann_json
    CLI11::CLI11
    cpr::cpr
)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT MimicWindowsServer_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of MimicWindowsServer"
  )
endif()

include(cmake/dev-mode.cmake)
