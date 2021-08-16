#include "gtirb_functions.hpp"
#include <gtirb.hpp>

using namespace gtirb;

void _generate_subgraph(Context& C, ByteInterval& interval, int src, int dst,
                        gtirb::EdgeType type) {
  static std::map<int, CodeBlock*> blocks;
  if (!blocks.find(src)) {
    auto* block = CodeBlock::Create(C, src) blocks[src] = block;
    block->setSize(1);
    interval.addBlock(src + 1, block)
  }
  auto b1 = blocks[src];
  if (!blocks.find(dst)) {
    auto* block = CodeBlock::Create(C, dst) blocks[dst] = block;
    block->setSize(1);
    interval.addBlock(dst + 1, block)
  }
  auto b2 = blocks[dst];
  auto label = gtirb::EdgeLabel(gtirb::ConditionalEdge::OnFalse,
                                DirectEdge::IsDirect, type);
  interval.getSection()->getModule()->getIR()->getCFG().m_edges.emplace_back()
}

int test_build_functions(void) {
  Context C;
  auto* IR = IR::Create(C);
  auto* M = Module::Create(C, "example");
  IR->addModule(M);
  auto* S = Section::Create(C, ".text");
  M->addSection(S);
  auto addr = Addr::Addr(0x1000);
  auto* interval = ByteInterval::Create(C, std::make_optional(addr), 0);
  S->addByteInterval(interval);

  // build the actual functions

  auto funcs = Function::build_functions(*M);
