#include <gtirb.hpp>
#include <AuxDataSchema.hpp>
#include <optional>

#ifndef GTIRB_FN_H
#define GTIRB_FN_H

using namespace std;

namespace gtirb{
    class Function {
        /* A thin wrapper around the code blocks and auxillary data
        * that consititute a function in GTIRB.
        * This class does not own any of its own data; 
        * everything is a reference to data in the underlying module
        */
        using CodeBlockSet  = unordered_set<CodeBlock *>;
        private: 
            UUID uuid;
            optional<CodeBlockSet> entry_blocks {};
            optional<CodeBlockSet> blocks {};
            optional<Module::SymbolSet> name_symbols {};
            optional<CodeBlockSet> exit_blocks {};

        public:
        Function(UUID Uuid, optional<CodeBlockSet> Entries, 
            optional<CodeBlockSet> Blocks, optional<SymbolSet> NameSymbols,
            optional<CodeBlockSet> ExitBlocks)
            : uuid(Uuid), entry_blocks(Entries), name_symbols(NameSymbols),
                exit_blocks(ExitBlocks) {};


        Function(UUID Uuid, CodeBlockSet Entries, CodeBlockSetBlocks, SymbolSet NameSymbols):
            Function(Uuid, make_optional(Entries), make_optional(Blocks), make_optional(NameSymbols), optional::nullopt){}; 

        auto & get_entry_blocks(){return entry_blocks;}
        auto & get_exit_blocks(){return exit_blocks;}
        auto & get_blocks(){return *blocks;}
        auto & get_name_symbols(){return *name_symbols}
        UUID get_uuid() {return uuid;}
        std::array<string> get_names(){
            auto arr = array<string>{};
            if (self.name_symbols){
                for (const auto & name_symbol: name_symbols){
                    arr.emplace(name_symbol->second);
                }
            }
            return arr;
        }
        std::string get_name_long(){
            int n_names = std::length(this->name_symbols);
            switch (n_names){
                case 0:
                    return "<unknown>";
                case 1:
                    return this->name_symbols[0].name;
                default:
                    string name = this->name_symbols[0].name;
                    for (auto idx = 1; idx < n_names; i++){
                        name += "(a.k.a ";
                        name += this->name_symbols[i].name;
                        if (idx + 1 < n_names){
                            name += ", ";
                        }
                    }
                    name += ')';
                    return name;

            }
        }

        /// \brief Create the \ref Functions present in a \ref Module
        ///
        /// \param C The current GTIRB \ref Context
        /// \param mod 

        /// \return an array containing the Functions in this module (possibly empty)
        static array<Function> build_functions(const Context & C, const Module & mod){
            //build table of symbols by referent UUID
            auto symbols = std::map<UUID, Module::SymbolSet>{};
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

}
#endif