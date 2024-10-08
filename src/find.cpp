#include "find.hpp"
#include <cassert>
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
        std::cerr << "incorrect number of arguments: \n";
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
    std::cout << possible_path;
    if (possible_path.empty()) {
        std::cerr << "find: path cannot be empty\n";
        return {};
    }

    if (!std::filesystem::exists(possible_path)) {
        std::cerr << "find: " << possible_path << " does not exists or you dont have permission\n";
        return {};
    }

    if (!std::filesystem::is_directory(possible_path)) {
        std::cerr << "find: " << possible_path << " is not a directory\n";
        return {};
    }

    cmd.m_path = possible_path;

    for (size_t i = 1; i < args.size(); i++) {
        if (args[i] == "-n" || args[i] == "--name") {
            if (args.size() < (i + 2)) {
                std::cerr << "find: paramenter -n (--name) expects a filename\n";
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
        std::cerr << "find: invalid operation. delete must have name paramenter";
        return {};
    }

    return cmd;
}
auto CommandArgs::path() ->  std::filesystem::path
{
    return m_path;
}
auto CommandArgs::name() -> std::optional<std::string_view>
{
    return m_name;
}
};  // namespace Find
