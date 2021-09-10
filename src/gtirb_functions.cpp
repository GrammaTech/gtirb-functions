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

#include "gtirb_functions/gtirb_functions.hpp"
#include <gtirb/Casting.hpp>
#include <gtirb/IR.hpp>
#include <gtirb/Symbol.hpp>

namespace gtirb {

void Function::set_name(void) {
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
    std::vector<Symbol*> otherNames;
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

Function::CodeBlockSet Function::findExitBlocks(Module& M,
                                                const CodeBlockSet& Blocks) {
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
          auto Dest = dyn_cast<CodeBlock>(Succ);
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

std::vector<Function> Function::build_functions(Context& C, Module& Mod) {
  // build table of symbols by referent.UUID
  // Load AuxData re: functions
  auto* EntriesByFn = Mod.getAuxData<schema::FunctionEntries>();

  auto* BlocksByFn = Mod.getAuxData<schema::FunctionBlocks>();

  auto* FnNames = Mod.getAuxData<schema::FunctionNames>();

  std::vector<Function> Fns;

  for (const auto& FnEntry : *EntriesByFn) {

    auto& [FnId, FnEntryIds] = FnEntry;
    CodeBlockSet EntryBlocks;
    SymbolSet NameSymbols;

    for (const auto& Id : FnEntryIds) {
      Node* FnBlockNode = Node::getByUUID(C, Id);
      if (CodeBlock* FnBlock = dyn_cast_or_null<CodeBlock>(FnBlockNode)) {
        EntryBlocks.insert(FnBlock);
        for (auto& s : Mod.findSymbols(*FnBlock)) {
          NameSymbols.insert(&s);
        }
      }
    }

    CodeBlockSet FnBlocks;
    auto FnBlockIdIter = BlocksByFn->find(FnId);
    if (FnBlockIdIter != BlocksByFn->end()) {
      auto FnBlockIds = (*FnBlockIdIter).second;
      for (const auto& Id : FnBlockIds) {
        if (auto Block = dyn_cast_or_null<CodeBlock>(Node::getByUUID(C, Id))) {
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

} // namespace gtirb
