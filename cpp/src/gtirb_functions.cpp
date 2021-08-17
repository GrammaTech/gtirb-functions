#include "gtirb_functions.hpp"
#include <Casting.hpp>

namespace gtirb {

void Function::set_name(void) {
  if (!this->name_symbols) {
    this->long_name = "<unknown>";
    return;
  }
  auto nsyms = *(this->name_symbols);
  size_t n_names = nsyms.size();
  switch (n_names) {
  case 0:
    this->long_name = "<unknown>";
  case 1:
    this->long_name = (*(begin(nsyms)))->getName();
  default:
    size_t i = 0;
    for (auto sym = begin(nsyms); sym != end(nsyms); sym++) {
      if (sym == begin(nsyms)) {
        this->long_name = (*(sym))->getName();
        this->long_name += "(a.k.a ";
      } else {
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

static std::vector<Function>
Function::build_functions(const Context& C, const Module& mod) {
  // build table of symbols by referent.UUID
  auto symbols = std::unordered_map<UUID, SymbolSet>{};
  for (const auto & s  : mod.symbols()) {
    auto * ref = s.getReferent<Node>();
    if (ref != nullptr) {
      auto uuid = ref->getUUID();
      if (symbols.find(uuid) == symbols.end()) {
        symbols[uuid] = SymbolSet{};
      }
      symbols[uuid].insert(&s);
    }
  }

  // Load AuxData re: functions
  auto * function_entries =
      mod.getAuxData<schema::FunctionEntries>();

  auto * all_fn_blocks =
      mod.getAuxData<schema::FunctionBlocks>();

  std::vector<Function> functions;

  for (const auto& fn_entry : *function_entries) {
    Function::SymbolSet l_syms = Function::SymbolSet();

    auto id = fn_entry.first;
    auto fn_blocks_uuid_iter = all_fn_blocks->find(id);
    if (fn_blocks_uuid_iter == all_fn_blocks->end()){continue;}
    auto fn_blocks_uuid = (*fn_blocks_uuid_iter).second;
    auto entry_blocks = Function::CodeBlockSet();
    auto fn_blocks = Function::CodeBlockSet();

    for (const auto& id : fn_entry.second) {
      auto block = cast<CodeBlock>(Node::getByUUID(C, id));
      entry_blocks.insert(block);
    }

    for (const auto& b : fn_blocks_uuid) {
      for (const auto& s : symbols[b]) {
        l_syms.insert(s);
      }
      auto block = cast<CodeBlock>(Node::getByUUID(C, b));
      fn_blocks.insert(block);
    }
    functions.emplace_back(fn_entry.first, entry_blocks, fn_blocks, l_syms);
  }

  return functions;
}

} // namespace gtirb
