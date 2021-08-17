#include <gtirb/AuxDataSchema.hpp>
#include <gtirb/Context.hpp>
#include <gtirb/Module.hpp>
#include <optional>
#include <unordered_set>
#include <vector>

#ifndef GTIRB_FN_H
#define GTIRB_FN_H

namespace gtirb {

class Function {
  /// A Function is a collection of code blocks, with information
  /// about what the function should be named and which entry and
  /// exit blocks it has.

  using CodeBlockSet = std::unordered_set<const CodeBlock*>;
  using SymbolSet = std::unordered_set<const Symbol *>;

  UUID uuid;
  std::optional<CodeBlockSet> entry_blocks = std::nullopt;
  std::optional<CodeBlockSet> blocks = std::nullopt;
  std::optional<SymbolSet> name_symbols = std::nullopt;
  std::optional<CodeBlockSet> exit_blocks = std::nullopt;
  std::string long_name;
  void set_name(void);

public:
  Function(UUID Uuid, std::optional<CodeBlockSet> Entries,
           std::optional<CodeBlockSet> Blocks,
           std::optional<SymbolSet> NameSymbols,
           std::optional<CodeBlockSet> ExitBlocks)
      : uuid(Uuid), entry_blocks(Entries), blocks(Blocks), name_symbols(NameSymbols),
        exit_blocks(ExitBlocks){set_name();};

  Function(UUID Uuid, CodeBlockSet Entries, CodeBlockSet Blocks,
           SymbolSet NameSymbols)
      : uuid(Uuid), entry_blocks(make_optional(Entries)),
        blocks(make_optional(Blocks)), name_symbols(make_optional(NameSymbols)),
        exit_blocks(std::nullopt){set_name();};

  Function(UUID Uuid) : uuid(Uuid){set_name();};

  std::optional<CodeBlockSet>& get_entry_blocks() { return entry_blocks; }
  const std::optional<CodeBlockSet>& get_const_entry_blocks() { return entry_blocks; }

  std::optional<CodeBlockSet>& get_exit_blocks() { return exit_blocks; }
  const std::optional<CodeBlockSet>& get_const_exit_blocks() { return exit_blocks; }

  std::optional<CodeBlockSet>& get_all_blocks() { return blocks; }
  const std::optional<CodeBlockSet>& get_const_all_blocks() { return blocks; }

  std::optional<SymbolSet>& get_name_symbols() { return name_symbols; }
  const std::optional<SymbolSet>& get_const_name_symbols() { return name_symbols; }

  UUID get_uuid() { return uuid; }
  const UUID get_const_uuid() { return uuid; }

  std::vector<std::string> get_names() {
    std::vector<std::string> arr{};
    if (name_symbols) {
      for (const auto& name_symbol : *name_symbols) {
        auto name = name_symbol->getName();
        arr.emplace_back(name);
      }
    }
    return arr;
  }

  /// \brief Get the name of this function, as a string

  std::string get_name();

  /// \brief Create all the functions present in a \ref Module
  ///
  /// \param C The current GTIRB \ref Context
  /// \param mod

  /// \return an vector containing the Functions in this module (possibly empty)
  static std::vector<Function> build_functions(const Context& C, const Module& mod);
};

}; // namespace gtirb
#endif
