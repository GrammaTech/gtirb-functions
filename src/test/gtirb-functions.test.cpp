#include "gtirb_functions/gtirb_functions.hpp"
#include <gtirb/gtirb.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <gtest/gtest.h>

using namespace gtirb;
using CodeBlockMap = std::map<int, CodeBlock*>;
using GraphEdge = std::tuple<int, int, EdgeType, ConditionalEdge>;

template <size_t N>
void generate_subgraph(IR* IR, CodeBlockMap blocks,
                       std::array<GraphEdge, N> edges) {
  auto& graph = IR->getCFG();
  for (const auto& edge : edges) {
    auto& [src, dst, edge_type, cond] = edge;
    CodeBlock* b1 = blocks[src];
    CodeBlock* b2 = blocks[dst];
    addVertex(b1, graph);
    addVertex(b2, graph);
    auto const opt_edgedesc = gtirb::addEdge(b1, b2, graph);
    if (opt_edgedesc) {
      EdgeLabel prop{std::tuple{cond, DirectEdge::IsDirect, edge_type}};
      graph[*opt_edgedesc] = prop;
    }
  }
}

template <size_t Num>
CodeBlockMap make_blocks(Context& C, ByteInterval* interval,
                         std::array<int, Num> indices) {
  CodeBlockMap blocks;
  for (auto& i : indices) {
    auto* block = CodeBlock::Create(C, i);
    block->setSize(1);
    blocks[i] = block;
    interval->addBlock(i + 1, block);
  }
  return blocks;
}

class TestData : public ::testing::Test {

protected:
  Context C;
  gtirb::IR* IR;
  gtirb::Module* M;
  gtirb::Section* S;
  gtirb::ByteInterval* interval;
  Symbol *f1_symbol, *f2_symbol;
  CodeBlockMap blocks;
  std::set<gtirb::UUID> f1_blocks, f2_blocks;
  UUID f1, f2;

  std::map<UUID, std::set<UUID>> fn_blocks;
  std::map<UUID, std::set<UUID>> fn_entries;
  std::map<UUID, UUID> fn_names;
  std::vector<Function> functions;

  void setupIR() {
    IR = IR::Create(C);
    M = Module::Create(C, "example");
    IR->addModule(M);
    S = Section::Create(C, ".text");
    M->addSection(S);
    auto addr = Addr(0x1000);
    interval = ByteInterval::Create(C, std::make_optional(addr), 0);
    S->addByteInterval(interval);
  }

  UUID make_function(const std::string& Name, std::vector<int> Entries,
                     std::vector<int> Blocks) {
    auto id = boost::uuids::random_generator()();
    for (auto& entry : Entries) {
      fn_entries[id].insert(blocks[entry]->getUUID());
    }
    for (auto& b : Blocks) {
      fn_blocks[id].insert(blocks[b]->getUUID());
    }
    auto sym = Symbol::Create(C, blocks[Entries[0]], Name);
    M->addSymbol(sym);
    fn_names[id] = sym->getUUID();
    return id;
  }
  void addEdge(int src, int dst, EdgeType edge_type, ConditionalEdge cond) {
    auto& graph = IR->getCFG();
    CodeBlock* b1 = blocks[src];
    CodeBlock* b2 = blocks[dst];
    addVertex(b1, graph);
    addVertex(b2, graph);
    auto const opt_edgedesc = gtirb::addEdge(b1, b2, graph);
    if (opt_edgedesc) {
      EdgeLabel prop{std::tuple{cond, DirectEdge::IsDirect, edge_type}};
      graph[*opt_edgedesc] = prop;
    }
  }

  void addFallthrough(int src, int dst) {
    addEdge(src, dst, EdgeType::Fallthrough, ConditionalEdge::OnFalse);
  }

  void addBranch(int src, int dst) {
    addEdge(src, dst, EdgeType::Branch, ConditionalEdge::OnTrue);
  }

  void addReturn(int src, int dst) {
    addEdge(src, dst, EdgeType::Return, ConditionalEdge::OnFalse);
  }

  TestData() : C() {
    setupIR();

    std::array<int, 6> inds{0, 1, 2, 3, 4, 5};
    blocks = make_blocks(C, interval, inds);

    // build the CFG
    addFallthrough(0, 1);
    addBranch(0, 2);
    addFallthrough(1, 2);
    addReturn(2, 3);
    addReturn(4, 5);

    f1 = make_function("f1", {0}, {0, 1, 2});
    f2 = make_function("f2", {4}, {4, 5});

    // write out aux data
    auto tmp_blocks = fn_blocks;
    M->addAuxData<schema::FunctionBlocks>(std::move(tmp_blocks));
    auto tmp_entries = fn_entries;
    M->addAuxData<schema::FunctionEntries>(std::move(tmp_entries));
    auto tmp_names = fn_names;
    M->addAuxData<schema::FunctionNames>(std::move(tmp_names));

    functions = Function::build_functions(C, *M);
  };
};

/// TESTS

TEST_F(TestData, TEST_SIZE) { EXPECT_EQ(functions.size(), 2); }

TEST_F(TestData, TEST_NAMES) {
  for (auto& fun : functions) {
    const Symbol* name = fun.getName();
    auto& name_str = name->getName();
    if (fun.getUUID() == f1) {
      EXPECT_TRUE(name_str == "f1");
    } else {
      EXPECT_TRUE(name_str == "f2");
    }
  }
}

TEST_F(TestData, TEST_UUIDS) {
  std::set<UUID> ids;
  for (auto& fun : functions) {
    ids.insert(fun.getUUID());
  }
  std::set<UUID> expected{f1, f2};
  EXPECT_EQ(ids, expected);
}

TEST_F(TestData, TEST_ENTRIES) {
  for (auto& fun : functions) {
    auto entry_iter = fun.entry_blocks_begin();
    ASSERT_NE(entry_iter, fun.entry_blocks_end());
    auto& entry_block = *entry_iter;
    ASSERT_NE(entry_block, nullptr);
    auto expected = fun.getUUID() == f1 ? blocks[0] : blocks[4];
    EXPECT_EQ(entry_block, expected);
    EXPECT_EQ(std::next(entry_iter), fun.entry_blocks_end());
  }
}

TEST_F(TestData, TEST_EXITS) {
  for (auto& fun : functions) {
    auto exit_iter = fun.exit_blocks_begin();
    ASSERT_NE(exit_iter, fun.exit_blocks_end());
    auto& exit_block = *exit_iter;
    ASSERT_NE(exit_block, nullptr);
    auto expected = fun.getUUID() == f1 ? blocks[2] : blocks[4];
    EXPECT_EQ(exit_block, expected);
    EXPECT_EQ(std::next(exit_iter), fun.exit_blocks_end());
  }
}

TEST_F(TestData, TEST_BLOCKS) {
  for (auto& fun : functions) {
    std::set<UUID> fn_ids;
    for (auto& block : fun.all_blocks()) {
      fn_ids.insert(block->getUUID());
    }
    auto expected = fun.getUUID() == f1 ? fn_blocks[f1] : fn_blocks[f2];
    EXPECT_EQ(fn_ids, expected);
  }
}
