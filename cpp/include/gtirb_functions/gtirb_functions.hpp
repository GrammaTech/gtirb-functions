#include <Module.hpp>
#include <Context.hpp>
#include <AuxDataSchema.hpp>
#include <optional>
#include <array>

#ifndef GTIRB_FN_H
#define GTIRB_FN_H

using namespace std;

namespace gtirb{

    class Function {
        /// A Function is a collection of code blocks, with information
        /// about what the function should be named and which entry and 
        /// exit blocks it has.

        using CodeBlockSet  = unordered_set<CodeBlock *>;
        using SymbolSet = Module::SymbolSet;

        UUID uuid;
        optional<CodeBlockSet> entry_blocks {};
        optional<CodeBlockSet> blocks {};
        optional<SymbolSet> name_symbols {};
        optional<CodeBlockSet> exit_blocks {};
        

        public:
        Function(UUID Uuid, optional<CodeBlockSet> Entries, 
            optional<CodeBlockSet> Blocks, optional<SymbolSet> NameSymbols,
            optional<CodeBlockSet> ExitBlocks)
            : uuid(Uuid), entry_blocks(Entries), name_symbols(NameSymbols),
                exit_blocks(ExitBlocks) {};


        Function( UUID Uuid, CodeBlockSet Entries, 
                  CodeBlockSet Blocks, SymbolSet NameSymbols):
            Function(Uuid, make_optional(Entries), make_optional(Blocks),
            make_optional(NameSymbols), optional::nullopt){}; 
        
        optional<CodeBlockSet> & get_entry_blocks(){ return entry_blocks; }
        const optional<CodeBlockSet> & get_entry_blocks() { return entry_blocks; }

        optional<CodeBlockSet> & get_exit_blocks(){ return exit_blocks; }
        const optional<CodeBlockSet> & get_exit_blocks(){ return exit_blocks; }
        
        optional<CodeBlockSet> & get_all_blocks(){ return blocks; }
        const optional<CodeBlockSet> & get_all_blocks(){ return blocks; }
        
        SymbolSet & get_name_symbols(){ return name_symbols }
        const SymbolSet & get_name_symbols() { return name_symbols; }
        
        
        UUID get_uuid() {return uuid;}
        const UUID get_uuid() {return uuid;}
        UUID & get_uuid() {return uuid;}
        const UUID & get_uuid() { return uuid; }

        
        std::array<string&> get_names(){
            std::array<string&> arr{};
            if (name_symbols){
                for (const auto & name_symbol: *name_symbols){
                    arr.insert(name_symbol->second);
                }
            }
            return arr;
        }

        /// \brief Get the name of this function, as a string

        std::string get_name(){
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

        /// \brief Create all the functions present in a \ref Module
        ///
        /// \param C The current GTIRB \ref Context
        /// \param mod 

        /// \return an array containing the Functions in this module (possibly empty)
        static array<Function> build_functions(const Context & C, const Module & mod);
    
    }

}
#endif