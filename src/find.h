#pragma once

#include <filesystem>
#include <optional>
#include <vector>

namespace Find {

struct CommandArgs {
  std::optional<std::string_view> name = {};
  std::filesystem::path path = {};
  bool to_delete = false;
  bool exact = false;

  void print_help(std::string exe_name);
  static std::optional<CommandArgs> Parse(int argc, char** argv);
};
}  // namespace Find