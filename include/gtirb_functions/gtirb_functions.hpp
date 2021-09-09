//===- gtirb_functions.hpp -----------------------------------------------*- C++
//-*-===//
//
//  Copyright (C) 2021 GrammaTech, Inc.
//
//  This code is licensed under the MIT license. See the LICENSE file in the
//  project root for license terms.
//
//  This project is sponsored by the Office of Naval Research, One Liberty
//  Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
//  N68335-17-C-0700.  The content of the information does not necessarily
//  reflect the position or policy of the Government and no official
//  endorsement should be inferred.
//
//===----------------------------------------------------------------------===//

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

/// \class Function serves as a thin wrapper around the function-related
/// information in AuxData (FunctionEntries, FunctionBlocks, FunctionNames).

/// A \class Function is read-only; any modifications to the underlying \class
/// Module may invalidate it.

///

class Function {

  using CodeBlockSet = std::unordered_set<CodeBlock*>;
  using SymbolSet = std::unordered_set<Symbol*>;

  UUID Uuid;

  CodeBlockSet EntryBlocks;
  CodeBlockSet ExitBlocks;
  CodeBlockSet AllBlocks;

  SymbolSet NameSymbols;

  Symbol* CanonName;
  std::string LongName;

  // helper functions
  void set_name(void);
  static CodeBlockSet findExitBlocks(Module& M, const CodeBlockSet& Blocks);

  // Constructor;
  Function(const UUID& Uuid_, const CodeBlockSet& Entries,
           const CodeBlockSet& Exits, const CodeBlockSet& Blocks,
           const SymbolSet& Names, Symbol* canonName = nullptr)
      : Uuid(Uuid_), EntryBlocks(Entries), ExitBlocks(Exits), AllBlocks(Blocks),
        NameSymbols(Names), CanonName(canonName) {
    set_name();
  };

public:
  //

  /// \brief Create all the functions present in a \ref Module
  ///
  /// \param C The current GTIRB \ref Context
  /// \param Mod The \class Module

  /// \return an vector containing the Functions in this module, possibly empty
  static std::vector<Function> build_functions(Context& C, Module& Mod);

  /// \section Iterators

  /// \brief Iterators over code blocks, in an arbitrary order
  using code_block_iterator = CodeBlockSet::iterator;
  using const_code_block_iterator = CodeBlockSet::const_iterator;

  /// \brief Const ranges of code blocks
  using code_block_range = ::boost::iterator_range<code_block_iterator>;
  using const_code_block_range =
      ::boost::iterator_range<const_code_block_iterator>;

  /// \brief Return an iterator to the first entry block
  code_block_iterator entry_blocks_begin() { return EntryBlocks.begin(); }
  const_code_block_iterator entry_blocks_begin() const {
    return EntryBlocks.begin();
  }

  /// \brief Return an iterator to the element after the last entry block
  code_block_iterator entry_blocks_end() { return EntryBlocks.end(); }
  const_code_block_iterator entry_blocks_end() const {
    return EntryBlocks.end();
  }

  /// \brief Return a range of the entry blocks
  code_block_range entry_blocks() {
    return {EntryBlocks.begin(), EntryBlocks.end()};
  }
  const_code_block_range entry_blocks() const {
    return {EntryBlocks.begin(), EntryBlocks.end()};
  }

  /// \brief Return an iterator to the first exit block
  code_block_iterator exit_blocks_begin() { return ExitBlocks.begin(); }
  const_code_block_iterator exit_blocks_begin() const {
    return ExitBlocks.begin();
  }
  /// \brief Return an iterator to the element after the last exit block
  code_block_iterator exit_blocks_end() { return ExitBlocks.end(); }
  const_code_block_iterator exit_blocks_end() const { return ExitBlocks.end(); }

  /// \brief Return a range of the exit blocks
  code_block_range exit_blocks() {
    return {ExitBlocks.begin(), ExitBlocks.end()};
  }
  const_code_block_range exit_blocks() const {
    return {ExitBlocks.begin(), ExitBlocks.end()};
  }
  /// \brief Return an iterator to the first code block in the function
  code_block_iterator all_blocks_begin() { return AllBlocks.begin(); }
  const_code_block_iterator all_blocks_begin() const {
    return AllBlocks.begin();
  }
  /// \brief Return an iterator to the element after the last code block
  code_block_iterator all_blocks_end() { return AllBlocks.end(); }
  const_code_block_iterator all_blocks_end() const { return AllBlocks.end(); }
  /// \brief Return a range of the code blocks in the function
  code_block_range all_blocks() { return {AllBlocks.begin(), AllBlocks.end()}; }
  const_code_block_range all_blocks() const {
    return {AllBlocks.begin(), AllBlocks.end()};
  }

  /// \brief Iterators over symbols, in arbitrary order
  using symbol_iterator = SymbolSet::iterator;
  using const_symbol_iterator = SymbolSet::const_iterator;
  /// \brief Ranges over symbols
  using symbol_range = ::boost::iterator_range<symbol_iterator>;
  using const_symbol_range = ::boost::iterator_range<const_symbol_iterator>;

  /// A name symbol is any symbol that refers to an entry block of the function

  /// \brief Return an iterator to the first symbol for the function name
  symbol_iterator name_symbols_begin() { return NameSymbols.begin(); }
  const_symbol_iterator name_symbols_begin() const {
    return NameSymbols.begin();
  }
  /// \brief Return an iterator to the element after the last symbol for the
  /// function name
  symbol_iterator name_symbols_end() { return NameSymbols.end(); }
  const_symbol_iterator name_symbols_end() const { return NameSymbols.end(); }
  /// \brief Return a range over the name symbols of the function
  symbol_range name_symbols() {
    return {NameSymbols.begin(), NameSymbols.end()};
  }
  const_symbol_range name_symbols() const {
    return {NameSymbols.begin(), NameSymbols.end()};
  }

  /// \brief Returns the name of the function as recorded in AuxData
  Symbol* getName() { return CanonName; }
  const Symbol* getName() const { return CanonName; }

  /// \brief Returns a pretty concatenation of the names of the functions,
  /// as a string view
  const std::string& getLongName() { return LongName; }

  /// \brief Returns the UUID of the function
  const UUID& getUUID() { return Uuid; }
};

}; // namespace gtirb
#endif
