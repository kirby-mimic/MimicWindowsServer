#include <iostream>

#include "elastic_client.h"

#include <cpr/cpr.h>
#include <fmt/core.h>

// VERY IMPORTANT, You must define CURL_STATICLIB in CMakeLists.txt

auto elastic_client::create_index() -> int
{
  //  curl - X PUT "localhost:9200/my-index-000001?pretty"
  auto url_string = fmt::format("{}/{}?pretty", m_elastic_host, m_index);
  fmt::print("CreateIndex {}\n", url_string);
  cpr::Response response {};
  if (m_basic_auth) {
    response = cpr::Put(
        cpr::Url {url_string},
        cpr::Authentication {m_user_name, m_password, cpr::AuthMode::BASIC});
  } else {
    response = cpr::Put(cpr::Url {url_string});
  }
  try {
    auto json = nlohmann::json::parse(response.text);
    fmt::print("{}\n", json.dump(2));
  } catch (nlohmann::json::parse_error& ex) {
    fmt::print("CreateIndex parse error at byte {}\n", ex.byte);
  }
  return 0;
}

auto elastic_client::get_index() -> int
{
  //  curl - X PUT "localhost:9200/my-index-000001?pretty"
  auto url_string = fmt::format("{}/{}?pretty", m_elastic_host, m_index);
  fmt::print("GetIndex {}\n", url_string);
  cpr::Response response {};
  if (m_basic_auth) {
    response = cpr::Get(
        cpr::Url {url_string},
        cpr::Authentication {m_user_name, m_password, cpr::AuthMode::BASIC});
  } else {
    response = cpr::Get(cpr::Url {url_string});
  }
  try {
    auto json = nlohmann::json::parse(response.text);
    fmt::print("{}\n", json.dump(2));
  } catch (nlohmann::json::parse_error& ex) {
    fmt::print("GetIndex parse error at byte {}\n", ex.byte);
  }
  return 0;
}

auto elastic_client::delete_index() -> int
{
  auto url_string = fmt::format("{}/{}?pretty", m_elastic_host, m_index);
  fmt::print("DeleteIndex {}\n", url_string);
  cpr::Response response {};
  // curl -X DELETE "localhost:9200/minispy_index?pretty"
  if (m_basic_auth) {
    response = cpr::Delete(
        cpr::Url {url_string},
        cpr::Authentication {m_user_name, m_password, cpr::AuthMode::BASIC});
  } else {
    response = cpr::Delete(cpr::Url {url_string});
  }
  auto json = nlohmann::json::parse(response.text);
  fmt::print("{}\n", json.dump(2));
  return 0;
}

auto elastic_client::exists_index() -> bool
{
  auto url_string = fmt::format("{}/{}?pretty", m_elastic_host, m_index);
  fmt::print("ExistsIndex {}\n", url_string);
  // curl -I "localhost:9200/my-data-stream?pretty"
  cpr::Response response {};
  if (m_basic_auth) {
    response = cpr::Head(
        cpr::Url {url_string},
        cpr::Authentication {m_user_name, m_password, cpr::AuthMode::BASIC});
  } else {
    response = cpr::Head(cpr::Url {url_string});
  }
  fmt::print("Status Code: {}\n", response.status_code);
  fmt::print("Header: {}\n", response.raw_header);
  return (response.status_code == 200);
}

auto elastic_client::post_data(const std::string& json_data) -> int
{
  auto url_string = fmt::format("{}/{}/_doc", m_elastic_host, m_index);
  fmt::print("PostData: {}\n", url_string);
  // curl -I "localhost:9200/my-data-stream?pretty"
  // fmt::print("PostData incoming json {}\n", json_data);
  cpr::Response response {};
  if (m_basic_auth) {
    response = cpr::Post(
        cpr::Url {url_string},
        cpr::Authentication {m_user_name, m_password, cpr::AuthMode::BASIC},
        cpr::Header {{"Content-Type", "application/json"}},
        cpr::Body {json_data});
  } else {
    response = cpr::Post(cpr::Url {url_string},
                  cpr::Header {{"Content-Type", "application/json"}},
                  cpr::Body {json_data});
  }
  #if 1
  fmt::print("Status Code: {}\n", response.status_code);
  fmt::print("Status line: {}\n", response.status_line);
  fmt::print("Reason: {}\n", response.reason);
  fmt::print("error message: {}\n", response.error.message);
  fmt::print("Header: {}\n", response.raw_header);
  fmt::print("Text: {}\n", response.text);
  #endif
  return 0;
}
