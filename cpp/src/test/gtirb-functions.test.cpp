#include "gtirb_functions/gtirb_functions.hpp"
#include <gtirb/gtirb.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <gtest/gtest.h>


using namespace gtirb;
using CodeBlockMap = std::map<int, CodeBlock * >;
using GraphEdge = std::tuple<int, int>; 

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

class TestData: public ::testing::Test{

    protected:
    Context C; 
    gtirb::IR * IR;
    gtirb::Module * M;
    gtirb::Section * S;
    gtirb::ByteInterval * interval;
    Symbol * f1_symbol, * f2_symbol; 
    CodeBlockMap blocks;
    std::set<UUID> f1_blocks, f2_blocks;
    UUID f1, f2;

    std::vector<Function> functions;

    TestData(): C(){
         IR = IR::Create(C);
         M = Module::Create(C, "example");
        IR->addModule(M);
         S = Section::Create(C, ".text");
        M->addSection(S);
        auto addr = Addr(0x1000);
        interval = ByteInterval::Create(C, std::make_optional(addr), 0);
        S->addByteInterval(interval);

        std::array<int, 6> inds {0, 1, 2, 3, 10, 11};
        blocks = make_blocks(C, interval, inds);
        // build the actual functions
        std::array<GraphEdge, 5> edges {
            GraphEdge{0, 1},
            GraphEdge{0, 2}, 
            GraphEdge{1, 2}, 
            GraphEdge{2, 3}, 
            GraphEdge{10, 11} 
            }; 

        _generate_subgraph(*IR, blocks, edges);
        f1_symbol = Symbol::Create(C, blocks[0], "f1");
        M->addSymbol(f1_symbol);
        f1_blocks  = {blocks[0]->getUUID(),
            blocks[1]->getUUID(), blocks[2]->getUUID()};

        f2_symbol = Symbol::Create(C, blocks[10], "f2");
        M->addSymbol(f2_symbol);
        f2_blocks  = {blocks[10]->getUUID()};
        auto gen = boost::uuids::random_generator();
        f1 = gen();
        f2 = gen();



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


        functions = Function::build_functions(C, *M);
    };
};

TEST_F(TestData, TEST_SIZE) {
  
  assert(functions.size() == 2);
}
/*
  for (auto & fun :functions){
      auto name = fun.getName()->getName();
      if (name == "f1"){
          ASSERT_EQ(fun.getUUID(), f1);
          ASSERT_EQ(*fun.entry_blocks_begin(), blocks[0]);
          for (auto & block: fun.all_blocks()){
              ASSERT_NE(f1_blocks.find(block->getUUID()), f1_blocks.end());
          }
          ASSERT_EQ(*fun.exit_blocks_begin(), blocks[2]);
      }
      else{
          ASSERT_EQ(fun.getUUID(), f2);
          ASSERT_EQ(*fun.entry_blocks_begin(), blocks[10]);
          ASSERT_EQ(*fun.exit_blocks_begin(), blocks[10]);
          for (auto & block : fun.all_blocks()) {
              ASSERT_NE(f2_blocks.find(block->getUUID()), f2_blocks.end());
          }
          
      }
  }
}*/

TEST_F(TestData, TEST_NAMES){
    for (auto & fun: functions){
         const Symbol * name = fun.getName();
         auto & name_str  = name->getName();
         EXPECT_TRUE(name_str == "f1" || name_str == "f2");
    }
}

TEST_F(TestData, TEST_UUIDS){
    for (auto & fun: functions){
        const UUID & id = fun.getUUID();
        EXPECT_TRUE((id == f1) || (id == f2));
    }
}

TEST_F(TestData, TEST_ENTRIES){
    for (auto & fun: functions){
        auto  entry_iter = fun.entry_blocks_begin();
        ASSERT_NE(entry_iter, fun.entry_blocks_end());
        auto & entry_block = *entry_iter;
        ASSERT_NE(entry_block, nullptr);
        auto expected = fun.getUUID() == f1 ? blocks[0] : blocks[10];
        ASSERT_EQ (entry_block, expected); 
    }
}

TEST_F(TestData, TEST_EXITS){
    for (auto & fun:  functions){
        auto exit_iter = fun.exit_blocks_begin();
        ASSERT_NE(exit_iter, fun.exit_blocks_end());
        auto &  exit_block = *exit_iter;
        ASSERT_NE(exit_block, nullptr);
        auto expected = fun.getUUID() == f1? blocks[2] : blocks[10];
        ASSERT_EQ(exit_block, expected);
    }
}

TEST_F(TestData, TEST_BLOCKS){
    for(auto & fun: functions){
        std::set<const CodeBlock *> fn_blocks;
        for (auto &block: fun.all_blocks()){
            fn_blocks.insert(block);
        }
        auto expected_size = fun.getUUID() == f1 ? 3 : 1;
        ASSERT_EQ(blocks.size(), expected_size);
    }
}

