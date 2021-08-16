#include <Module.hpp>
#include <Context.hpp>
#include <AuxDataSchema.hpp>
#include <optional>
#include <vector>
#include <unordered_set>

#ifndef GTIRB_FN_H
#define GTIRB_FN_H


namespace gtirb{

    class Function {
        /// A Function is a collection of code blocks, with information
        /// about what the function should be named and which entry and 
        /// exit blocks it has.

        using CodeBlockSet  = std::unordered_set<CodeBlock *>;
        using SymbolSet = Module::SymbolSet;

        UUID uuid;
        std::optional<CodeBlockSet> entry_blocks = std::nullopt;
        std::optional<CodeBlockSet> blocks = std::nullopt;
        std::optional<SymbolSet> name_symbols = std::nullopt;
        std::optional<CodeBlockSet> exit_blocks = std::nullopt;
        std::string long_name;
        void set_name(void);

        public:
        Function(UUID Uuid, 
                 std::optional<CodeBlockSet> Entries, 
                 std::optional<CodeBlockSet> Blocks,
                  std::optional<SymbolSet> NameSymbols,
                  std::optional<CodeBlockSet> ExitBlocks
                  ): 
                  uuid(Uuid), 
                  entry_blocks(Entries), 
                  name_symbols(NameSymbols),
                  exit_blocks(ExitBlocks) {set_name()};


        Function( UUID Uuid, 
                  CodeBlockSet Entries, 
                  CodeBlockSet Blocks, 
                  SymbolSet NameSymbols
                  ):
                 uuid(Uuid), 
                 entry_blocks(make_optional(Entries)),
                 blocks(make_optional(Blocks)),
                 name_symbols(make_optional(NameSymbols)),
                 exit_blocks(std::nullopt){set_name()};

        Function(UUID Uuid):
            uuid(Uuid) {set_name()};
        
        std::optional<CodeBlockSet> & get_entry_blocks(){ return entry_blocks; }
        const std::optional<CodeBlockSet> & get_entry_blocks() { return entry_blocks; }

        std::optional<CodeBlockSet> & get_exit_blocks(){ return exit_blocks; }
        const std::optional<CodeBlockSet> & get_exit_blocks(){ return exit_blocks; }
        
        std::optional<CodeBlockSet> & get_all_blocks(){ return blocks; }
        const std::optional<CodeBlockSet> & get_all_blocks(){ return blocks; }
        
        SymbolSet & get_name_symbols(){ return name_symbols }
        const SymbolSet & get_name_symbols() { return name_symbols; }
        
        
        UUID get_uuid() {return uuid;}
        const UUID get_uuid() {return uuid;}
        UUID & get_uuid() {return uuid;}
        const UUID & get_uuid() { return uuid; }

        
        std::vector<string&> get_names(){
            std::vector<string&> arr{};
            if (name_symbols){
                for (const auto & name_symbol: *name_symbols){
                    arr.insert(name_symbol->second);
                }
            }
            return arr;
        }

        /// \brief Get the name of this function, as a string

        std::string get_name(){
        }

        /// \brief Create all the functions present in a \ref Module
        ///
        /// \param C The current GTIRB \ref Context
        /// \param mod 

        /// \return an vector containing the Functions in this module (possibly empty)
        static vector<Function> build_functions(const Context & C, const Module & mod);
    
    }

}
#endif
