
#include <filesystem>
#include <iostream>

#include "find.h"

#ifdef WINDOWS
# include <Shobjidl.h>
# include <Windows.h>
#endif

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