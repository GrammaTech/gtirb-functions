  #include "gtirb_functions.hpp"
  
  namespace gtirb{
        
        
        static array<Function> build_functions(const Context & C, const Module & mod){
            //build table of symbols by referent.UUID
            auto symbols = std::unordered_map<UUID, Module::SymbolSet>{};
            for (const auto & s : mod.symbols()){
                Node * ref = s.getReferent();
                if (ref != nullptr){
                    auto uuid = ref->getUUID();
                    if (!symbols.contains(uuid)){
                        symbols[uuid] = Module::SymbolSet{};
                    }
                    symbols[uuid].insert(s);
                }
            }

            // Load AuxData re: functions
            schema::FunctionEntries::Type function_entries = mod.getAuxData<schema::FunctionEntries>();
            schema::FunctionBlocks::Type all_fn_blocks = mod.getAuxData<schema::FunctionBlocks>();
            
            auto functions = array<Function>();

            for (const auto & fn_entry: function_entries){
                auto syms = Module::SymbolSet();
                auto id = fn_entry.first;
                auto fn_blocks_uuid = all_fn_blocks[id];
                auto entry_blocks = CodeBlockSet{};
                auto fn_blocks = CodeBlockSet{};
                for(const auto & id: fn_entry.second){
                    CodeBlock * block = cast<CodeBlock, Node>(Node::getByUUID(C, id));
                    entry_blocks.insert(block);
                }
                for (const auto& b : fn_blocks_uuid){
                    for (const auto &  s: symbols[b]){
                        syms.insert(s);
                    }
                    CodeBlock * block = cast<CodeBlock, Node>(Node::getByUUID(C, b));
                    fn_blocks.insert(block);
                }
                functions.emplace(Function(uuid,entry_blocks, fn_blocks, syms, option::nullopt));
            }
            return functions;
        }

  }