
#include <filesystem>
#include <iostream>
#include <fmt/base.h>

#include "find.hpp"


#ifdef WINDOWS
# include <Shobjidl.h>
# include <Windows.h>
#endif

void delete_file(const std::filesystem::path& file)
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
    // TODO(andreanidouglas): use GTK or QT filesystem api to delete the file to recycle bin
    std::filesystem::remove(file);
#endif
}

auto match_filename(std::string_view what, std::string_view where, bool exact) -> bool
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

auto main(int argc, char** argv) -> int
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

    std::error_code ec{};

    const std::filesystem::path p(cmd.path());
    const std::filesystem::recursive_directory_iterator iter(p, ec);

    if (ec.value() == 0) {
        for (const auto& entry : iter) {
            ec.clear();
            if (cmd.name().has_value()) {
                auto name = *cmd.name();
                if (match_filename(entry.path().filename().string(), name, cmd.exact())) {
                    if (cmd.to_delete()) {
                        delete_file(entry);
                        fmt::println("[x] {}", entry.path().string());
                    } else {
                        fmt::println("{}", entry.path().string());
                    }
                }

            } else {
                fmt::println("{}", entry.path().string());
            }
        }

    } else {
        std::cout << "ERROR: " << ec.value() << " " << ec.category().name() << ": " << ec.message()
                  << "\n";
    }

#ifdef WINDOWS
    CoUninitialize();
#endif
    return EXIT_SUCCESS;
}
