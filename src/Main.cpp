#include <cassert>
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

#ifdef WINDOWS
# include <Shobjidl.h>
# include <Windows.h>
#endif

namespace Find {

struct CommandArgs {
  std::optional<std::string_view> name = {};
  std::filesystem::path path = {};
  bool to_delete = false;
  bool exact = false;

  void print_help(std::string exe_name)
  {
    std::filesystem::path exe = exe_name;
    std::cerr
      << exe.filename().string()
      << " <path> [-n | --name filename] [--delete] [-e | --exact] [-h | --help]\n\n"
      << "find all the files in a given path\n\n"
      << "options:\n"
      << "    -n, --name <filename>      filter list for files that contains filename\n"
      << "    -e, --exact                --name will match filename exactly. if --name is not "
         "specified then becames NOP\n"
      << "    --delete                   delete found files. --name parameter is mandatory\n"
      << "    -h, --help                 show this help message\n";
  }

  static std::optional<CommandArgs> Parse(int argc, char** argv)
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
      std::cerr << "find: " << possible_path << " does not exists or you dont have permission\n";
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
    // this is a safety precaution to avoid deleting the entire path, although there might be other
    // ways of doing it.
    if (cmd.to_delete && !cmd.name.has_value()) {
      std::cerr << "find: invalid operation. delete must have name paramenter";
      return {};
    }

    return cmd;
  }
};
}  // namespace Find

void delete_file(std::filesystem::path file)
{
#ifdef WINDOWS
  // use win32 api to delete the file sending it to recyclebin
  // TODO: collect all files to delete and do it in one operation

  IFileOperation* pfo;
  wchar_t path[1024] = { 0 };
  MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, file.string().c_str(),
                      (size_t)file.string().size(), path, 1024);
  IShellItem* pSI;
  SHCreateItemFromParsingName(path, NULL, IID_PPV_ARGS(&pSI));

  HRESULT hr = CoCreateInstance(__uuidof(FileOperation), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));
  if (SUCCEEDED(hr)) {
    pfo->DeleteItem(pSI, NULL);
    pfo->PerformOperations();
    pfo->Release();
  }

#else
  // use c++ stl to remove file
  // TODO: use GTK or QT filesystem api to delete the file to recycle bin
  std::filesystem::remove(file);
#endif
}

static bool match_filename(std::string_view what, std::string_view where, bool exact)
{
  // if exact is true. we need to make sure both strings match
  if (exact) {
    return what.compare(where) == 0;
  }

  // otherwise we can use substring finding, to see if the string is contained.
  else {
    return what.find(where) != std::string::npos;
  }
}

int main(int argc, char** argv)
{
  Find::CommandArgs cmd;
  const auto& args = Find::CommandArgs::Parse(argc, argv);
  if (!args.has_value()) {
    return EXIT_FAILURE;
  }
  cmd = args.value();

#ifdef WINDOWS
  HRESULT hr1 = CoInitialize(NULL);
  if (!SUCCEEDED(hr1))
    return EXIT_FAILURE;
#endif

  try {
    // recursive_directory_iterator can fail if you dont have access to the file.
    for (const auto& entry : std::filesystem::recursive_directory_iterator(cmd.path)) {
      if (cmd.name.has_value()) {
        auto name = cmd.name.value();
        if (match_filename(entry.path().filename().string(), name, cmd.exact)) {
          if (cmd.to_delete) {
            // std::filesystem::remove(entry);

            delete_file(entry);
            std::cout << "[X] " << entry.path().string() << "\n";
          } else {
            std::cout << entry.path().string() << "\n";
          }
        }
      } else {
        std::cout << entry.path().string() << "\n";
      }
    }
  }
  // windows stl apparently does not implements the standard, so a lot of errors shows as unknown.
  catch (std::filesystem::filesystem_error e) {
    std::cerr << "could not perform operation: " << e.code().message()
              << ". path: " << e.path1().string() << std::endl;
#ifdef WINDOWS
    CoUninitialize();
#endif
    return EXIT_FAILURE;
  }

#ifdef WINDOWS
  CoUninitialize();
#endif
  return EXIT_SUCCESS;
}