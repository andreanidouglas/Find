#include "find.h"

#include <cassert>
#include <iostream>

namespace Find {


	void CommandArgs::print_help(std::string exe_name)
	{
          
            std::filesystem::path exe = exe_name;
            std::cerr
              << exe.filename().string()
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

    std::optional<CommandArgs> CommandArgs::Parse(int argc, char** argv)
    {
            CommandArgs cmd;

            // fatal error if there is more than 64 args
            assert(argc < 64);

            if (argc < 2) {
              std::cerr << "incorrect number of arguments: \n";
              cmd.print_help(argv[0]);
              return {};
            }

            const std::vector<std::string_view> args(argv + 1, argv + argc);

            if (args[0] == "-h" || args[0] == "--help") {
              cmd.print_help(argv[0]);
              return {};
            }

            std::filesystem::path possible_path = args[0];
            if (possible_path.empty()) {
              std::cerr << "find: path cannot be empty\n";
              return {};
            }

            if (!std::filesystem::exists(possible_path)) {
              std::cerr << "find: " << possible_path
                        << " does not exists or you dont have permission\n";
              return {};
            }

            if (!std::filesystem::is_directory(possible_path)) {
              std::cerr << "find: " << possible_path << " is not a directory\n";
              return {};
            }

            cmd.path = possible_path;

            for (size_t i = 1; i < args.size(); i++) {
              if (args[i] == "-n" || args[i] == "--name") {
                if (args.size() < (i + 2)) {
                  std::cerr << "find: paramenter -n (--name) expects a filename\n";
                  return {};
                }
                cmd.name = args[i + 1];
              }

              if (args[i] == "--delete") {
                cmd.to_delete = true;
              }

              if (args[i] == "--exact") {
                cmd.exact = true;
              }

              if (args[i] == "-h" || args[i] == "--help") {
                cmd.print_help(argv[0]);
                return {};
              }
            }

            // do not allow --delete without a --name parameter.
            // this is a safety precaution to avoid deleting the entire path, although there might
            // be other ways of doing it.
            if (cmd.to_delete && !cmd.name.has_value()) {
              std::cerr << "find: invalid operation. delete must have name paramenter";
              return {};
            }

            return cmd;
    }
 };