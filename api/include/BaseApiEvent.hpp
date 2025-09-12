#pragma once
#include <string>
namespace task_manager {

class BaseApiEvent {
public:
  BaseApiEvent() {}
  virtual ~BaseApiEvent() = default;

  std::string _api = "";
};

} // namespace task_manager
