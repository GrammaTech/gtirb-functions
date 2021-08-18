#include "gtirb_functions/gtirb_functions.hpp"
#include <gtirb/gtirb.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <gtest/gtest.h>


using namespace gtirb;
using CodeBlockMap = std::map<int, CodeBlock * >;
using GraphEdge = std::tuple<int, int>; //, EdgeType, ConditionalEdge>;

template<size_t N>
void _generate_subgraph(IR& IR, CodeBlockMap blocks,
                        std::array<GraphEdge, N> edges
                        ) 
{
  auto & graph = IR.getCFG();

  for (const auto & edge: edges)
  {
      auto [src, dst] = edge;
      CodeBlock* b1 = blocks[src];
      CodeBlock * b2 = blocks[dst];
      addVertex(b1, graph);
      addVertex(b2, graph);
      addEdge(b1, b2, graph);
  }
}


template<size_t Num>
CodeBlockMap make_blocks(Context& C, ByteInterval * interval, 
                         std::array<int, Num> indices){
    CodeBlockMap blocks;
    for (auto & i: indices){
        auto * block = CodeBlock::Create(C, i);
        block->setSize(1);
        blocks[i] = block;
        interval->addBlock(i+1, block);
    }
    return blocks;
}

TEST(SUITE0, TEST0) {
  Context C;
  auto* IR = IR::Create(C);
  auto* M = Module::Create(C, "example");
  IR->addModule(M);
  auto* S = Section::Create(C, ".text");
  M->addSection(S);
  auto addr = Addr(0x1000);
  auto* interval = ByteInterval::Create(C, std::make_optional(addr), 0);
  S->addByteInterval(interval);

  std::array<int, 6> inds {0, 1, 2, 3, 10, 11};
  auto blocks = make_blocks(C, interval, inds);
  // build the actual functions
  std::array<GraphEdge, 5> edges {
      GraphEdge{0, 1},//EdgeType::Fallthrough, ConditionalEdge::OnFalse},
      GraphEdge{0, 2}, // EdgeType::Branch, ConditionalEdge::OnTrue},
      GraphEdge{1, 2}, //, EdgeType::Fallthrough, ConditionalEdge::OnFalse},
      GraphEdge{2, 3}, //, EdgeType::Return, ConditionalEdge::OnFalse},
      GraphEdge{10, 11} //, EdgeType::Return, ConditionalEdge::OnFalse}
    }; 

  _generate_subgraph(*IR, blocks, edges);
  auto f1_symbol = Symbol::Create(C, blocks[0], "f1");
  M->addSymbol(f1_symbol);
  std::set<UUID> f1_blocks {blocks[0]->getUUID(),
    blocks[1]->getUUID(), blocks[2]->getUUID()};

  auto f2_symbol = Symbol::Create(C, blocks[10], "f2");
  M->addSymbol(f2_symbol);
  std::set<UUID> f2_blocks {blocks[10]->getUUID()};
  auto gen = boost::uuids::random_generator();
  UUID f1 {gen()};
  UUID f2 {gen()};



  gtirb::schema::FunctionBlocks::Type function_blocks {
      {f1, f1_blocks},
      {f2, f2_blocks}
  };

  schema::FunctionEntries::Type function_entries {
      {f1, {blocks[0]->getUUID()} },
      {f2, {blocks[10]->getUUID()} }
  };

  schema::FunctionNames::Type function_names {
          {f1, f1_symbol->getUUID()}, 
          {f2, f2_symbol->getUUID()}
  };


  M->registerAuxDataType<schema::FunctionEntries>();
  M->registerAuxDataType<schema::FunctionBlocks>();
  M->registerAuxDataType<schema::FunctionNames>();
  M->addAuxData<schema::FunctionBlocks>(std::move(function_blocks));
  M->addAuxData<schema::FunctionEntries>(std::move(function_entries));

  M->addAuxData<schema::FunctionNames>(std::move(function_names));



  auto functions = Function::build_functions(C, *M);
  assert(functions.size() == 2);
  for (auto & fun :functions){
      auto name = fun.get_name()->getName();
      EXPECT_TRUE(name == "f1" || name == "f2");
      if (name == "f1"){
          ASSERT_EQ(fun.get_uuid(), f1);
          for (auto & block: fun.all_blocks()){
              ASSERT_NE(f1_blocks.find(block->getUUID()), f1_blocks.end());
          }
      }
      else{
          ASSERT_EQ(fun.get_uuid(), f2);
          for (auto & block : fun.all_blocks()) {
              ASSERT_NE(f2_blocks.find(block->getUUID()), f2_blocks.end());
          }
      }
  }
}