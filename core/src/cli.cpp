#include "core.hpp"
#include <replxx.hxx>

using namespace task_manager;
using namespace replxx;

void trim_leading_ws(std::string &s) {
  s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
}

int main() {
  try {
    auto storage = init_storage();
    Calendar calendar(storage);

    Replxx repl;
    repl.install_window_change_handler();

    std::map<std::string, std::string> commands = {
        {"add", "Add a new local event"},
        {"list", "List all local and remote events"},
        {"save", "Save local events to the database"},
        {"link_gcal", "Link your Google Calendar account"},
        {"sync", "Manually sync with external calendars"},
        {"help", "Show this help message."},
        {"exit", "Exit the application."}};

    repl.set_completion_callback([&commands](const std::string &context,
                                             int & /*contextLen*/) {
      std::vector<Replxx::Completion> completions;
      for (auto const &[cmd, desc] : commands) {
        if (cmd.rfind(context, 0) == 0) { // check if cmd starts with context
          completions.emplace_back(cmd.c_str());
        }
      }
      return completions;
    });

    while (true) {
      char const *cinput{nullptr};

      do {
        cinput = repl.input("task_manager> ");
      } while (cinput != nullptr && std::string(cinput).empty());

      if (cinput == nullptr) { // Handle Ctrl+D (EOF)
        break;
      }

      std::string line(cinput);

      std::istringstream iss(line);
      std::string cmd;
      iss >> cmd;

      if (cmd == "exit") {
        break;
      } else if (cmd == "list" || cmd == "ls") {
        std::cout << "--- All Events ---\n";
        if (calendar.get_events().empty()) {
          std::cout << "No events found. Use 'add' to create one.\n";
        } else {
          std::cout << calendar;
        }
        std::cout << "------------------\n";

      } else if (cmd == "add") {
        std::string name;
        std::getline(iss, name);
        trim_leading_ws(name);

        // If name is empty, it means the user just typed "add"
        if (name.empty()) {
          char const *name_input = repl.input("  event name> ");
          if (name_input == nullptr) {
            std::cout << "\nAdd operation cancelled.\n";
            continue;
          }
          name = name_input;
        }

        // Ensure we have a name before proceeding
        if (name.empty()) {
          std::cout << "Event name cannot be empty. Add operation cancelled.\n";
          continue;
        }

        auto now = std::chrono::system_clock::now();
        Event ev(name, now, now + std::chrono::hours(1));

        if (calendar.create_event(ev)) {
          std::cout << "Event '" << name << "' added successfully!\n";
        } else {
          std::cout << "Failed to add event.\n";
        }

      } else if (cmd == "remove" || cmd == "rm") {
        std::string id;
        std::getline(iss, id);
        trim_leading_ws(id);

        // If name is empty, it means the user just typed "remove"
        if (id.empty()) {
          char const *id_input = repl.input("  event id> ");
          if (id_input == nullptr) {
            std::cout << "\nRemove operation cancelled.\n";
            continue;
          }
          id = id_input;
        }

        // Ensure we have a id before proceeding
        if (id.empty()) {
          std::cout
              << "Event id cannot be empty. Remove operation cancelled.\n";
          continue;
        }

        if (calendar.remove_event_by_id(
                static_cast<uint32_t>(std::stoul(id)))) {
          std::cout << "Event with id '" << id << "' removed successfully!\n";
        } else {
          std::cout << "Failed to remove event.\n";
        }
      } else if (cmd == "update") {
        uint32_t id = 0;
        std::string name;
        std::string desc;

        // Parse remaining input
        std::string token;
        while (iss >> token) {
          if (token == "id" && (iss >> token)) {
            id = static_cast<uint32_t>(std::stoul(token));
          } else if (token == "name" && std::getline(iss, token)) {
            trim_leading_ws(token);
            name = token;
          } else if (token == "desc" && std::getline(iss, token)) {
            trim_leading_ws(token);
            desc = token;
          }
        }

        if (id == 0) {
          std::cout << "Please specify a valid event id. Update cancelled.\n";
          continue;
        }

        if (name.empty() && desc.empty()) {
          std::cout << "Nothing to update. Provide 'name' and/or 'desc'.\n";
          continue;
        }

        if (calendar.update_event_by_id(id, name, desc)) {
          std::cout << "Event " << id << " updated successfully.\n";
        } else {
          std::cout << "Failed to update event. Event with id " << id
                    << " not found.\n";
        }
      } else if (cmd == "link_gcal") {
        calendar.link_google_account();
      } else if (cmd == "sync") {
        calendar.sync_external_events();
      } else if (cmd == "help") {
        std::cout << "Available commands:\n";
        // Find the longest command name for alignment
        size_t max_len = 0;
        for (auto const &[cmd_name, _] : commands) {
          if (cmd_name.length() > max_len) {
            max_len = cmd_name.length();
          }
        }
        // Print formatted help
        for (auto const &[cmd_name, desc] : commands) {
          std::cout << "  " << std::left << std::setw(max_len + 2) << cmd_name
                    << desc << "\n";
        }
      } else {
        std::cout << "Unknown command: '" << cmd
                  << "'. Type 'help' for a list of commands.\n";
      }
    }

  } catch (const std::exception &e) {
    std::cerr << "An unhandled exception occurred: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "An unknown exception occurred." << std::endl;
    return 1;
  }

  std::cout << "Exiting.\n";
  return 0;
}
