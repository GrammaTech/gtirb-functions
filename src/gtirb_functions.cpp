#include "gtirb_functions/gtirb_functions.hpp"
#include <gtirb/Casting.hpp>
#include <gtirb/IR.hpp>

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
    size_t i = 0;
    size_t Length = this->NameSymbols.size();
    this->LongName = this->CanonName->getName();
    this->LongName += " (a.k.a ";
    for (auto& Sym : this->NameSymbols) {
      if (Sym != this->CanonName) {
        this->LongName += Sym->getName();
        if (i + 1 < Length) {
          this->LongName += ", ";
        }
      }
      i += 1;
    }
    this->LongName += ')';
  }
}

Function::CodeBlockSet Function::findExitBlocks(const Module& M,
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
      Node* Succ;
      EdgeLabel Edge_label;
      std::tie(Succ, Edge_label) = Succ_pair;
      if (Edge_label) {
        auto& [Conditional, Direct, Type] = *Edge_label;
        if ((Type == EdgeType::Return) || (Type == EdgeType::Sysret)) {
          ExitBlocks.insert(Block);
          break;
        }
        if ((Type == EdgeType::Call) || (Type == EdgeType::Syscall)) {
          auto Dest = dyn_cast<CodeBlock>(Succ);
          if ((Dest != nullptr) && (Blocks.find(Dest) != Blocks.end())) {
          } else {
            ExitBlocks.insert(Block);
            break;
          }
        }
      }
    }
  }
  return ExitBlocks;
}

std::vector<Function> Function::build_functions(const Context& C,
                                                const Module& Mod) {
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
      auto Block = dyn_cast<CodeBlock>(Node::getByUUID(C, Id));
      if (Block != nullptr) {
        EntryBlocks.insert(Block);
        for (const auto& s : Mod.findSymbols(*Block)) {
          NameSymbols.insert(&s);
        }
      }
    }

    CodeBlockSet FnBlocks;
    auto FnBlockIdIter = BlocksByFn->find(FnId);
    if (FnBlockIdIter != BlocksByFn->end()) {
      auto FnBlockIds = (*FnBlockIdIter).second;
      for (const auto& Id : FnBlockIds) {
        auto Block = dyn_cast<CodeBlock>(Node::getByUUID(C, Id));
        if (Block) {
          FnBlocks.insert(Block);
        }
      }
    }

    const Symbol* CanonName = nullptr;
    auto FnNameIter = FnNames->find(FnId);
    if (FnNameIter != FnNames->end()) {
      auto Id = (*FnNameIter).second;
      CanonName = dyn_cast<Symbol>(Symbol::getByUUID(C, Id));
    }

    CodeBlockSet ExitBlocks;
    if (!std::empty(FnBlocks)) {
      ExitBlocks = findExitBlocks(Mod, FnBlocks);
    }

    Function Fn{FnId,     EntryBlocks, ExitBlocks,
                FnBlocks, NameSymbols, CanonName};
    Fns.push_back(Fn);
  }

  return Fns;
}

} // namespace gtirb
