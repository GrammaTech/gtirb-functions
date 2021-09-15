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
#include <gtirb/Casting.hpp>
#include <gtirb/Context.hpp>
#include <gtirb/IR.hpp>
#include <gtirb/Module.hpp>
#include <gtirb/Symbol.hpp>
#include <boost/range.hpp>
#include <optional>
#include <type_traits>
#include <unordered_set>
#include <vector>

#ifndef GTIRB_FN_H
#define GTIRB_FN_H

namespace gtirb {

template <class ModuleType> class Function;
std::vector<Function<Module>> build_functions(Context& C, Module& M);
std::vector<Function<const Module>> build_functions(Context& C,
                                                    const Module& M);

/// \class Function<T> serves as a thin wrapper around the function-related
/// information in AuxData (FunctionEntries, FunctionBlocks, FunctionNames).
///
/// A Function is templated on the type of the Module that bore it, i.e.
/// either Module or const Module. Function<Module> provides mutable access to
/// its data, while Function<const Module> provides read-only access to its
/// data.

template <class ModuleType> class Function {

  using is_const_module = std::is_const<ModuleType>;

  friend std::vector<Function<Module>> build_functions(Context& C, Module& M);
  friend std::vector<Function<const Module>> build_functions(Context& C,
                                                             const Module& M);
  template <class Other> friend class Function;

public:
  using CodeBlockType = typename std::conditional_t<is_const_module::value,
                                                    const CodeBlock, CodeBlock>;
  using SymbolType =
      typename std::conditional_t<is_const_module::value, const Symbol, Symbol>;
  using CodeBlockSet = typename std::unordered_set<CodeBlockType*>;
  using SymbolSet = typename std::unordered_set<SymbolType*>;

private:
  UUID Uuid;

  CodeBlockSet EntryBlocks;
  CodeBlockSet ExitBlocks;
  CodeBlockSet AllBlocks;

  SymbolSet NameSymbols;

  SymbolType* CanonName;
  std::string LongName;

  // helper functions

  /// \brief Given the code blocks of a function, return the
  /// subset of blocks that exit the function
  static CodeBlockSet findExitBlocks(ModuleType& M, CodeBlockSet& Blocks) {
    /*
     * Exit blocks are blocks whose outgoing edges are
     * returns or sysrets;
     * calls or syscalls, and the target is not in the function
     */
    auto Cfg = M.getIR()->getCFG();
    CodeBlockSet ExitBlocks;
    for (auto& Block : Blocks) {
      for (auto Succ_pair : cfgSuccessors(Cfg, Block)) {
        auto [Succ, Edge_label] = Succ_pair;
        if (Edge_label) {
          auto& Type = std::get<EdgeType>(*Edge_label);
          if ((Type == EdgeType::Return) || (Type == EdgeType::Sysret)) {
            ExitBlocks.insert(Block);
            break;
          }
          if ((Type == EdgeType::Call) || (Type == EdgeType::Syscall)) {
            auto Dest = dyn_cast<CodeBlockType>(Succ);
            if ((Dest == nullptr) || (Blocks.find(Dest) == Blocks.end())) {
              ExitBlocks.insert(Block);
              break;
            }
          }
        }
      }
    }
    return ExitBlocks;
  }

  void set_name(void) {
    switch (this->NameSymbols.size()) {
    case 0:
      this->LongName = "<unknown>";
      return;
    case 1:
      this->LongName = (*(this->NameSymbols.begin()))->getName();
      return;
    default:
      this->LongName = this->CanonName->getName();
      this->LongName += " (a.k.a ";
      std::vector<SymbolType*> otherNames;
      for (auto& Sym : this->NameSymbols) {
        if (Sym != this->CanonName) {
          otherNames.push_back(Sym);
        }
      }
      size_t i = 0;
      size_t Length = this->NameSymbols.size();
      for (auto& Sym : otherNames) {
        this->LongName += Sym->getName();
        if (i + 1 < Length) {
          this->LongName += ", ";
        }
        i += 1;
      }
      this->LongName += ')';
    }
  }

  // Private constructor;
  Function(const UUID& Uuid_, const CodeBlockSet& Entries,
           const CodeBlockSet& Exits, const CodeBlockSet& Blocks,
           const SymbolSet& Names, Symbol* canonName = nullptr)
      : Uuid(Uuid_), EntryBlocks(Entries), ExitBlocks(Exits), AllBlocks(Blocks),
        NameSymbols(Names), CanonName(canonName) {
    set_name();
  };

  //

  /// \brief Create all the functions present in a \ref Module
  ///
  /// \param C The current GTIRB \ref Context
  /// \param Mod The \class Module

  /// \return an vector containing the Functions in this module, possibly empty
  static std::vector<Function<ModuleType>> build_functions(Context& C,
                                                           ModuleType& Mod) {
    auto* EntriesByFn = Mod.template getAuxData<schema::FunctionEntries>();

    auto* BlocksByFn = Mod.template getAuxData<schema::FunctionBlocks>();

    auto* FnNames = Mod.template getAuxData<schema::FunctionNames>();

    std::vector<Function<ModuleType>> Fns;

    for (const auto& FnEntry : *EntriesByFn) {

      auto& [FnId, FnEntryIds] = FnEntry;
      CodeBlockSet EntryBlocks;
      SymbolSet NameSymbols;

      // Look up the function's entry points and their names
      for (const auto& Id : FnEntryIds) {
        Node* FnBlockNode = Node::getByUUID(C, Id);
        if (CodeBlock* FnBlock = dyn_cast_or_null<CodeBlock>(FnBlockNode)) {
          EntryBlocks.insert(FnBlock);
          for (auto& s : Mod.findSymbols(*FnBlock)) {
            NameSymbols.insert(&s);
          }
        }
      }

      // Look up the function blocks
      CodeBlockSet FnBlocks;
      auto FnBlockIdIter = BlocksByFn->find(FnId);
      if (FnBlockIdIter != BlocksByFn->end()) {
        auto FnBlockIds = (*FnBlockIdIter).second;
        for (const auto& Id : FnBlockIds) {
          if (auto Block =
                  dyn_cast_or_null<CodeBlock>(Node::getByUUID(C, Id))) {
            FnBlocks.insert(Block);
          }
        }
      }

      Symbol* CanonName = nullptr;
      auto FnNameIter = FnNames->find(FnId);
      if (FnNameIter != FnNames->end()) {
        auto Id = (*FnNameIter).second;
        CanonName = dyn_cast<Symbol>(Symbol::getByUUID(C, Id));
      }

      CodeBlockSet ExitBlocks = findExitBlocks(Mod, FnBlocks);

      Function Fn{FnId,     EntryBlocks, ExitBlocks,
                  FnBlocks, NameSymbols, CanonName};
      Fns.push_back(Fn);
    }

    return Fns;
  }

public:
  /// \brief Copy constructor between Function templates
  /// Function<U> is constructable from Function<T> if its
  /// members can be copied
  /// In practice, this allows a Function<const Module> to be
  /// constructed from Function<Module>, but not the other way around.
  template <typename T>
  Function(const Function<T>& F)
      : Uuid(F.Uuid), EntryBlocks(begin(F.EntryBlocks), end(F.EntryBlocks)),
        ExitBlocks(begin(F.ExitBlocks), end(F.ExitBlocks)),
        AllBlocks(begin(F.AllBlocks), end(F.AllBlocks)),
        NameSymbols(begin(F.NameSymbols), end(F.NameSymbols)),
        CanonName(F.CanonName), LongName(F.LongName){};

  /// \section Iterators

  /// \brief Iterators over code blocks, in an arbitrary order
  using code_block_iterator =
      std::conditional_t<is_const_module::value,
                         typename CodeBlockSet::const_iterator,
                         typename CodeBlockSet::iterator>;

  /// \brief Const ranges of code blocks
  using code_block_range = ::boost::iterator_range<code_block_iterator>;

  /// \brief Return an iterator to the first entry block
  code_block_iterator entry_blocks_begin() { return EntryBlocks.begin(); }

  /// \brief Return an iterator to the element after the last entry block
  code_block_iterator entry_blocks_end() { return EntryBlocks.end(); }

  /// \brief Return the entry blocks of the function, as a range
  code_block_range entry_blocks() {
    return {EntryBlocks.begin(), EntryBlocks.end()};
  }

  /// \brief Return an iterator to the first exit block
  code_block_iterator exit_blocks_begin() { return ExitBlocks.begin(); }

  /// \brief Return an iterator to the element after the last exit block
  code_block_iterator exit_blocks_end() { return ExitBlocks.end(); }

  /// \brief Return a range of the exit blocks
  code_block_range exit_blocks() {
    return {ExitBlocks.begin(), ExitBlocks.end()};
  }

  /// \brief Return an iterator to the first code block in the function
  code_block_iterator all_blocks_begin() { return AllBlocks.begin(); }

  /// \brief Return an iterator to the element after the last code block
  code_block_iterator all_blocks_end() { return AllBlocks.end(); }

  code_block_range all_blocks() { return {AllBlocks.begin(), AllBlocks.end()}; }

  /// \brief Iterators over symbols, in arbitrary order
  using symbol_iterator = std::conditional_t<is_const_module::value,
                                             typename SymbolSet::const_iterator,
                                             typename SymbolSet::iterator>;

  /// \brief Ranges over symbols
  using symbol_range = typename ::boost::iterator_range<symbol_iterator>;

  /// A name symbol is any symbol that refers to an entry block of the function

  /// \brief Return an iterator to the first symbol for the function name
  symbol_iterator name_symbols_begin() { return NameSymbols.begin(); }

  /// \brief Return an iterator to the element after the last symbol for the
  /// function name
  symbol_iterator name_symbols_end() { return NameSymbols.end(); }
  symbol_range name_symbols() {
    return {NameSymbols.begin(), NameSymbols.end()};
  }

  /// \brief Returns the name of the function as recorded in AuxData
  SymbolType* getName() { return CanonName; }

  /// \brief Returns a pretty concatenation of the names of the functions,
  /// as a string view
  const std::string& getLongName() { return LongName; }

  /// \brief Returns the UUID of the function
  const UUID& getUUID() { return Uuid; }
};

/// \section Factories for building \class Functions from a \class Module

/// \param C the GTIRB context for the module
/// \param M the Module, either by reference or by constant reference
std::vector<Function<Module>> build_functions(Context& C, Module& M) {
  return Function<Module>::build_functions(C, M);
}

std::vector<Function<const Module>> build_functions(Context& C,
                                                    const Module& M) {
  return Function<const Module>::build_functions(C, M);
}

}; // namespace gtirb
#endif
