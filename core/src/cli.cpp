#include "core.hpp"
#include "db.hpp"
#include <iostream>
#include <map>
#include <replxx.hxx>
#include <string>
#include <vector>

using namespace task_manager;
using namespace replxx;

int main() {
  // Initialize DB storage
  auto storage = init_storage();
  // Initialize calendar
  Calendar calendar(storage);
  calendar.load_events(); // Load all events from DB

  Replxx repl;

  std::map<std::string, std::string> commands = {{"add", "Add a new event"},
                                                 {"list", "List all events"},
                                                 {"exit", "Exit CLI"}};

  repl.set_completion_callback(
      [&commands](const std::string &text, int & /*context*/) {
        std::vector<replxx::Replxx::Completion> completions;
        for (auto &[name, _] : commands) {
          if (name.find(text) == 0)
            completions.emplace_back(name);
        }
        return completions;
      });

  std::string line;
  while ((line = repl.input("> ")) != "exit") {
    if (line == "list") {
      for (auto &ev : calendar.get_events()) {
        std::cout << "Name: " << ev->get_name() << ", Start: "
                  << std::format("{:%Y-%m-%d %H:%M}", ev->get_start())
                  << ", End: "
                  << std::format("{:%Y-%m-%d %H:%M}", ev->get_end()) << "\n";
      }
    } else if (line == "add") {
      std::string name;
      std::cout << "Event name: ";
      std::getline(std::cin, name);

      auto now = std::chrono::system_clock::now();
      Event ev(name, now, now + std::chrono::hours(1));

      calendar.add_event(ev);
      std::cout << "Event added!\n";
    }
  }

  return 0;
}
