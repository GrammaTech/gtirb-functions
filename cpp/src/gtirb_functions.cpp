#include "gtirb_functions/gtirb_functions.hpp"
#include <gtirb/Casting.hpp>
#include <gtirb/IR.hpp>

namespace gtirb {

void Function::set_name(void) {
  SymbolSet nsyms = this->name_symbols_;
  size_t n_names = nsyms.size();
  switch (n_names) {
  case 0:
    this->long_name = "<unknown>";
    return;
  case 1:
    this->long_name = (*(begin(nsyms)))->getName();
    return;
  default:
    size_t i = 0;
    this->long_name = this->canon_name->getName();
    this->long_name += " (a.k.a "; 
    for (auto sym = begin(nsyms); sym != end(nsyms); sym++) {
      if (*sym != this->canon_name) {
        this->long_name += (*sym)->getName();
        if (i + 1 < n_names) {
          this->long_name += ", ";
        }
      }
      i +=1;
    }
    this->long_name += ')';
  }
}

Function::CodeBlockSet Function::makeExitBlocks(const Module & M,
 const CodeBlockSet & Blocks){
  /* 
   * Exit blocks are blocks whose outgoing edges are 
     * returns or sysrets;
     * calls or syscalls, and the target is not in the function
  */
  auto cfg = M.getIR()->getCFG();
  CodeBlockSet exit_blocks;
  for (auto & block: Blocks) {
    for (auto succ_pair : cfgSuccessors(cfg, block)){
      Node * succ;
      EdgeLabel edge_label;
      std::tie(succ, edge_label) = succ_pair;
        if (edge_label) {
          auto & [conditional, direct, type] = *edge_label;
          if ((type == EdgeType::Return) || (type == EdgeType::Sysret)) {
            exit_blocks.insert(block);
            break;
          }
          if ((type == EdgeType::Call) || (type == EdgeType::Syscall)){
            auto dest = dyn_cast<CodeBlock>(succ);
            if ((dest != nullptr) 
            && (Blocks.find(dest) != Blocks.end()) )
            { }
            else{
              exit_blocks.insert(block);
              break;
            }
          }
        }
      }
    }
  return exit_blocks;
  }

std::vector<Function>
Function::build_functions(const Context& C, const Module& mod) {
  // build table of symbols by referent.UUID
  // Load AuxData re: functions
  auto * function_entries =
      mod.getAuxData<schema::FunctionEntries>();

  auto * all_fn_blocks =
      mod.getAuxData<schema::FunctionBlocks>();

  auto * all_names = mod.getAuxData<schema::FunctionNames>();

  std::vector<Function> functions;

  for (const auto& fn_entry : *function_entries) {
    
    auto & [fn_id, fn_entry_ids] = fn_entry;
    UUID fnid2 = fn_id;
    CodeBlockSet entry_blocks;
    SymbolSet name_symbols;

    for (const auto& entry_id : fn_entry_ids) {
      auto block = dyn_cast<CodeBlock>(Node::getByUUID(C, entry_id));
      if (block != nullptr){
        entry_blocks.insert(block);
        for (const auto& s : mod.findSymbols(*block)) {
          name_symbols.insert(&s);
        }
      }
    }


    CodeBlockSet fn_blocks ;
    auto fn_blocks_uuid_iter = all_fn_blocks->find(fn_id);
    if (fn_blocks_uuid_iter != all_fn_blocks->end()){
      auto fn_blocks_by_uuid = (*fn_blocks_uuid_iter).second;
      for (const auto& b : fn_blocks_by_uuid) {
        auto block = dyn_cast<CodeBlock>(Node::getByUUID(C, b));
        if (block){
          fn_blocks.insert(block);
        }
      }
    }

    const Symbol * canon_name = nullptr;
    auto fn_name_iter = all_names->find(fn_id);
    if (fn_name_iter != all_names->end()) {
      auto id = (*fn_name_iter).second;
      canon_name = dyn_cast<Symbol>(Symbol::getByUUID(C, id)); 
    }

    CodeBlockSet exit_blocks;
    if (!std::empty(fn_blocks)){
      exit_blocks = makeExitBlocks(mod, fn_blocks);
    }

    Function fun 
    { fnid2, 
      entry_blocks, 
      fn_blocks, 
      name_symbols,
      exit_blocks, 
      canon_name
    };
    functions.push_back(fun);
  }

  return functions;
}

} // namespace gtirb
