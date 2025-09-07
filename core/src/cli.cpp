#include "calendar.hpp"
#include "core.hpp"
#include "db.hpp"
#include <replxx.hxx>

using namespace task_manager;
using namespace replxx;

struct Command {
  std::string usage;
  std::string description;
};

void print_help(const std::map<std::string, Command> &commands,
                const std::string &cmd = "") {
  if (cmd.empty()) {
    std::cout << "Available commands:\n";
    for (const auto &[name, c] : commands) {
      std::cout << "  " << c.usage << " - " << c.description << "\n";
    }
  } else {
    auto it = commands.find(cmd);
    if (it != commands.end()) {
      std::cout << it->second.usage << " - " << it->second.description << "\n";
    } else {
      std::cout << "Unknown command: " << cmd << "\n";
    }
  }
}

int main() {
  Calendar calendar;
  Replxx rx;
  auto storage = init_storage("task_manager.db");

  const std::map<std::string, Command> commands = {
      {"create_event",
       {"create_event <title>", "Create a new event with the given title"}},
      {"list_events", {"list_events", "List all events"}},
      {"help", {"help [command]", "Show help for commands"}}};

  rx.history_load("history.txt");
  rx.set_max_history_size(1000);

  while (true) {
    calendar.tick();

    char const *cinput = rx.input("task_manager> ");
    if (!cinput)
      break;
    std::string input(cinput);
    if (input.empty())
      continue;

    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    if (cmd == "exit" || cmd == "quit")
      break;

    if (cmd == "help") {
      print_help(commands);
    } else if (cmd == "create_event") {
      std::string title;
      if (!std::getline(iss, title) || title.empty()) {
        std::cout << "Usage: create_event <title>\n";
        continue;
      }
      if (title[0] == ' ')
        title.erase(0, 1);

      auto now = std::chrono::system_clock::now();
      Event ev(title, now, now + std::chrono::hours(1));
      calendar.add_event(ev);
      save_event(ev, storage);

    } else if (cmd == "list_events" || cmd == "ls") {
      auto events = get_all_events(storage);
      for (auto &ev : events) {
        std::cout << ev;
      }
    } else {
      std::cout << "Unknown command\n";
    }

    rx.history_add(input);
  }

  rx.history_save("history.txt");
}
