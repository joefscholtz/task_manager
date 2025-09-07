#include "core.hpp"
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

  const std::map<std::string, Command> commands = {
      {"create_event",
       {"create_event <title>", "Create a new event with the given title"}},
      {"list_events", {"list_events", "List all events"}},
      {"help", {"help [command]", "Show help for commands"}}};

  // REPLXX autocomplete
  rx.set_completion_callback(
      [&commands](std::string const &text, int & /*context*/) {
        std::vector<Replxx::Completion> completions;
        for (auto &[name, c] : commands) {
          if (name.find(text) == 0) {
            completions.emplace_back(name);
          }
        }
        return completions;
      });

  rx.history_load("history.txt"); // save/restore history
  rx.set_max_history_size(1000);

  while (true) {
    calendar.tick();

    char const *cinput = rx.input("task_manager> ");
    if (cinput == nullptr)
      break; // EOF / Ctrl+D
    std::string input(cinput);

    if (input == "exit" || input == "quit")
      break;

    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    if (cmd == "help") {
      std::string arg;
      iss >> arg; // optional command
      print_help(commands, arg);
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
    } else if (cmd == "list_events" || cmd == "ls") {
      std::cout << calendar;
    } else {
      std::cout << "Unknown command\n";
    }

    rx.history_add(input);
  }

  rx.history_save("history.txt");
  return 0;
}
