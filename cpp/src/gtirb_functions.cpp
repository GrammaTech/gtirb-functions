#include "gtirb_functions.hpp"

namespace gtirb {

void Function::set_name(void) {
  if (!this->name_symbols) {
    this->long_name = "<unknown>";
    return;
  }
  auto nsyms = *(this->name_symbols);
  int n_names = std::length(nsyms);
  switch (n_names) {
  case 0:
    this->long_name = "<unknown>";
  case 1:
    this->long_name = (*(begin(nsyms)))->getName();
  default:
    for (auto sym = begin(nsyms); sym != end(nsyms); sym++) {
      if (sym == begin(nsyms)) {
        this->long_name = (*(sym))->getName();
        this->long_name += "(a.k.a ";
      } else {
        this->long_name += (*sym)->getName();
        if (sym + 1 < end(nsyms)) {
          name += ", ";
        }
      }
      this->long_name += ')';
    }
  }
}

static std::vector<Function>
Function::build_functions(const Context& C, const Module& mod) const {
  // build table of symbols by referent.UUID
  auto symbols = std::unordered_map<UUID, Module::SymbolSet>{};
  for (const auto& s : mod.symbols()) {
    Node* ref = s.getReferent();
    if (ref != nullptr) {
      auto uuid = ref->getUUID();
      if (!symbols.contains(uuid)) {
        symbols[uuid] = Module::SymbolSet{};
      }
      symbols[uuid].insert(s);
    }
  }

  // Load AuxData re: functions
  schema::FunctionEntries::Type function_entries =
      mod.getAuxData<schema::FunctionEntries>();

  schema::FunctionBlocks::Type all_fn_blocks =
      mod.getAuxData<schema::FunctionBlocks>();

  std::vector<Function> functions();

  for (const auto& fn_entry : function_entries) {
    Module::SymbolSet syms();
    auto id = fn_entry.first;
    auto fn_blocks_uuid = all_fn_blocks[id];
    auto entry_blocks = Function::CodeBlockSet();
    auto fn_blocks = Function::CodeBlockSet();
    for (const auto& id : fn_entry.second) {
      CodeBlock* block = cast<CodeBlock, Node>(Node::getByUUID(C, id));
      entry_blocks.insert(block);
    }
    for (const auto& b : fn_blocks_uuid) {
      for (const auto& s : symbols[b]) {
        syms.insert(s);
      }
      CodeBlock* block = cast<CodeBlock, Node>(Node::getByUUID(C, b));
      fn_blocks.insert(block);
    }
                functions.emplace_back(fn_entry.first, entry_blocks, fn_blocks, syms));
  }
  return functions;
}
} // namespace gtirb
