#include <gtirb/AuxDataSchema.hpp>
#include <gtirb/Context.hpp>
#include <gtirb/Module.hpp>
#include <boost/range.hpp>
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
  using SymbolSet = std::unordered_set<const Symbol*>;

  UUID uuid;

  CodeBlockSet EntryBlocks;
  CodeBlockSet blocks;
  CodeBlockSet ExitBlocks;
  SymbolSet NameSymbols;

  const Symbol* CanonName;
  std::string long_name;

  // helper functions
  void set_name(void);
  static CodeBlockSet makeExitBlocks(const Module& M,
                                     const CodeBlockSet& Blocks);

  // Constructor;
  Function(UUID Uuid, CodeBlockSet Entries, CodeBlockSet Blocks,
           SymbolSet Names, CodeBlockSet ExitBlocks_,
           const Symbol* canonName = nullptr)
      : uuid(Uuid), EntryBlocks(Entries), blocks(Blocks),
        ExitBlocks(ExitBlocks_), NameSymbols(Names), CanonName(canonName) {
    set_name();
  };

public:
  // Factory

  /// \brief Create all the functions present in a \ref Module
  ///
  /// \param C The current GTIRB \ref Context
  /// \param mod

  /// \return an vector containing the Functions in this module (possibly empty)
  static std::vector<Function> build_functions(const Context& C,
                                               const Module& mod);

  /// \section Iterators

  using code_block_iterator = CodeBlockSet::const_iterator;

  using code_block_range = ::boost::iterator_range<code_block_iterator>;

  code_block_iterator entry_blocks_begin() { return EntryBlocks.begin(); }
  code_block_iterator entry_blocks_end() { return EntryBlocks.end(); }
  code_block_range entry_blocks() {
    return {EntryBlocks.begin(), EntryBlocks.end()};
  }

  code_block_iterator exit_blocks_begin() { return ExitBlocks.begin(); }
  code_block_iterator exit_blocks_end() { return ExitBlocks.end(); }

  code_block_range exit_blocks() {
    return {ExitBlocks.begin(), ExitBlocks.end()};
  }

  code_block_iterator all_blocks_begin() { return blocks.begin(); }
  code_block_iterator all_blocks_end() { return blocks.end(); }

  code_block_range all_blocks() { return {blocks.begin(), blocks.end()}; }

  using symbol_iterator = SymbolSet::const_iterator;
  using symbol_range = ::boost::iterator_range<symbol_iterator>;

  symbol_iterator name_symbols_begin() { return NameSymbols.begin(); }
  symbol_iterator name_symbols_end() { return NameSymbols.end(); }

  symbol_range name_symbols() {
    return {NameSymbols.begin(), NameSymbols.end()};
  }

  const UUID& getUUID() { return uuid; }

  const Symbol* getName() { return CanonName; }
  const std::string& get_long_name() { return long_name; }
};

}; // namespace gtirb
#endif
