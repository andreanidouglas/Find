#pragma once

#include <filesystem>
#include <optional>

namespace Find {

struct CommandArgs {
   private:
    std::optional<std::string_view> m_name;
    std::filesystem::path m_path;
    bool m_to_delete = false;
    bool m_exact = false;

   public:
    static void print_help(std::string& exe_name);
    static auto Parse(int argc, char** argv) -> std::optional<CommandArgs>;

    auto name() -> std::optional<std::string_view>;
    auto path() -> std::filesystem::path;

    [[nodiscard]] auto to_delete() const -> bool
    {
        return m_to_delete;
    }
    [[nodiscard]] auto exact() const
    {
        return m_exact;
    }
};
}  // namespace Find
