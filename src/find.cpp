#include "find.hpp"
#include <cassert>
#include <fmt/base.h>
#include <fmt/std.h>
#include <iostream>
#include <vector>

namespace Find {
using std::optional;

void CommandArgs::print_help(std::string& exe_name)
{
    const std::filesystem::path exe = exe_name;
    std::cerr << exe.filename().string()
              << " <path> [-n | --name filename] [--delete] [-e | --exact] [-h | --help]\n\n"
              << "find all the files in a given path\n\n"
              << "options:\n"
              << "    -n, --name <filename>      filter list for files that contains filename\n"
              << "    -e, --exact                --name will match filename exactly. if --name is "
                 "not "
                 "specified then becames NOP\n"
              << "    --delete                   delete found files. --name parameter is "
                 "mandatory\n"
              << "    -h, --help                 show this help message\n";
}

auto CommandArgs::Parse(int argc, char** argv) -> optional<CommandArgs>
{
    CommandArgs cmd;
    std::string args_value;
    // fatal error if there is more than 64 args
    assert(argc < 64);

    if (argc < 2) {
        fmt::println("Incorrect number of arguments");
        args_value = std::string(argv[0]);
        CommandArgs::print_help(args_value);
        return {};
    }

    args_value = std::string(argv[0]);
    const std::vector<std::string_view> args(argv + 1, argv + argc);

    if (args[0] == "-h" || args[0] == "--help") {
        CommandArgs::print_help(args_value);
        return {};
    }

    const std::filesystem::path possible_path = std::filesystem::path(args[0]);
    if (possible_path.empty()) {
        fmt::println("Find: path cannot be empty");
        return {};
    }

    if (!std::filesystem::exists(possible_path)) {
        fmt::println("Find: \"{}\" does not exists or you dont have permission", possible_path);
        return {};
    }


    if (!std::filesystem::is_directory(possible_path)) {
        fmt::println("Find: \"{}\" is not a directory", possible_path);
        return {};
    }

    cmd.m_path = possible_path;

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == "-n" || args[i] == "--name") {
            if (args.size() < (i + 2)) {
                fmt::println("Find: paramenter \"-n\" (--name) expects a filename");
                return {};
            }
            cmd.m_name = args[i + 1];
        }

        if (args[i] == "--delete") {
            cmd.m_to_delete = true;
        }

        if (args[i] == "--exact") {
            cmd.m_exact = true;
        }

        if (args[i] == "-h" || args[i] == "--help") {
            CommandArgs::print_help(args_value);
            return {};
        }
    }

    // do not allow --delete without a --name parameter.
    // this is a safety precaution to avoid deleting the entire path, although there might
    // be other ways of doing it.
    if (cmd.m_to_delete && !cmd.m_name.has_value()) {
        fmt::println("Find: Invalid Operation. Delete must have (--name) parameter");
        return {};
    }

    return cmd;
}
auto CommandArgs::path() -> std::filesystem::path
{
    return m_path;
}
auto CommandArgs::name() -> std::optional<std::string_view>
{
    return m_name;
}
};  // namespace Find
