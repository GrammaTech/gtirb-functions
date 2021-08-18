#include <gtirb/AuxDataSchema.hpp>
#include <gtirb/Context.hpp>
#include <gtirb/Module.hpp>
#include <boost/range.hpp>
#include <optional>
#include <unordered_set>
#include <vector>

#ifndef GTIRB_FN_H
#define GTIRB_FN_H

namespace gtirb {

class Function {
  /// A Function is a collection of code blocks, with information
  /// about what the function should be named and which entry and
  /// exit blocks it has.

  using CodeBlockSet = std::unordered_set<const CodeBlock*>;
  using SymbolSet = std::unordered_set<const Symbol *>;



  UUID uuid;
  CodeBlockSet entry_blocks_;
  CodeBlockSet blocks ;
  SymbolSet name_symbols_;
  CodeBlockSet exit_blocks_ ;
  std::string long_name;
  const Symbol * canon_name;

  // helper functions
  void set_name(void);
  static CodeBlockSet makeExitBlocks(const Module & M, const CodeBlockSet & Blocks);

  // Constructor;
  Function(UUID Uuid, 
           CodeBlockSet Entries,
           CodeBlockSet Blocks,
           SymbolSet NameSymbols,
           CodeBlockSet ExitBlocks,
           const Symbol * CanonName = nullptr)
      : uuid(Uuid), entry_blocks_(Entries), 
        blocks(Blocks), name_symbols_(NameSymbols),
        exit_blocks_(ExitBlocks), canon_name(CanonName)
        {set_name();};
  
  public:

  using code_block_iterator = CodeBlockSet::const_iterator;

  using code_block_range  = ::boost::iterator_range<code_block_iterator>;

  code_block_iterator entry_blocks_begin(){
     return entry_blocks_.begin() ;}
  code_block_iterator entry_blocks_end() {
     return entry_blocks_.end(); }
  code_block_range entry_blocks() {
     return  {entry_blocks_.begin(), entry_blocks_.end()};
  }

  code_block_iterator exit_blocks_begin() {
    return exit_blocks_.begin();}
  code_block_iterator exit_blocks_end() {
    return exit_blocks_.end();}

  code_block_range exit_blocks() {
    return {exit_blocks_.begin(), exit_blocks_.end()};}

  code_block_iterator all_blocks_begin() { 
    return blocks.begin(); }
  code_block_iterator all_blocks_end() { 
    return blocks.end(); }
  
  code_block_range all_blocks() {
    return {blocks.begin(), blocks.end()};}

  using symbol_iterator = SymbolSet::const_iterator;
  using symbol_range = ::boost::iterator_range<symbol_iterator>;

  symbol_iterator name_symbols_begin() {
    return name_symbols_.begin();}
  symbol_iterator name_symbols_end() {
    return name_symbols_.end();}

  symbol_range name_symbols(){
    return {name_symbols_.begin(), name_symbols_.end()};}

  const UUID get_uuid() { return uuid; }

  const Symbol * get_name() {return canon_name;}
  const std::string & get_long_name() {return long_name;}


  /// \brief Create all the functions present in a \ref Module
  ///
  /// \param C The current GTIRB \ref Context
  /// \param mod

  /// \return an vector containing the Functions in this module (possibly empty)
  static std::vector<Function> build_functions(const Context& C, const Module& mod);
};

}; // namespace gtirb
#endif
