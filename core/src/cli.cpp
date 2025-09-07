#include "core.hpp"
#include <replxx.hxx>

using namespace task_manager;
using namespace replxx;

int main() {
  Calendar calendar;
  Replxx rx;

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

    if (cmd == "create_event") {
      std::string title;
      if (!std::getline(iss, title) || title.empty()) {
        std::cout << "Usage: create_event <title>\n";
        continue; // or return
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
}
