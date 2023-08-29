# pragma once
#include <string>
#include <utility>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

class elastic_client
{
  std::string m_elastic_host {};
  std::string m_user_name {};
  std::string m_password {};
  std::string m_index {"mimic_windows_index"};
  bool m_basic_auth {false};

public:
  ~elastic_client() = default;
  auto create_index() -> int;
  auto delete_index() -> int;
  auto get_index() -> int;
  auto exists_index() -> bool;
  auto post_data(const std::string& json_data) -> int;

  elastic_client(std::string elastic_host_in,
                 std::string user_name_in,
                 std::string password_in)
      : m_elastic_host {std::move(elastic_host_in)}
      , m_user_name {std::move(user_name_in)}
      , m_password {std::move(password_in)}
  {
    fmt::print("Elastic Search host: {}\n", m_elastic_host);
    if (!m_user_name.empty() && !m_password.empty()) {
      fmt::print("Username: {}\n", m_user_name);
      fmt::print("Password: {}\n", m_password);
      m_basic_auth = true;
    }

    if (!exists_index()) {
      create_index();
    }
    get_index();
  }
  elastic_client(const elastic_client&) = default;
  elastic_client(elastic_client&&) = delete;
  auto operator=(const elastic_client&) -> elastic_client& = default;
  auto operator=(elastic_client&&) -> elastic_client& = delete;
};
