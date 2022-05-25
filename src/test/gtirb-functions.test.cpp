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
  UUID f1, f2, f3;

  std::map<UUID, std::set<UUID>> fn_blocks;
  std::map<UUID, std::set<UUID>> fn_entries;
  std::map<UUID, UUID> fn_names;
  std::vector<Function<Module>> functions;

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

    std::array<int, 11> inds = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    blocks = make_blocks(C, interval, inds);

    // build the CFG
    addFallthrough(0, 1);
    addBranch(0, 2);
    addFallthrough(1, 2);
    addReturn(2, 3);
    addReturn(4, 5);
    addFallthrough(6, 8);
    addFallthrough(7, 8);
    addBranch(8, 9);
    addFallthrough(8, 10);
    addReturn(9, 11);

    f1 = make_function("f1", {0, 1}, {0, 1, 2});
    f2 = make_function("f2", {4}, {4, 5});
    f3 = make_function("f3", {6, 7}, {6, 7, 8, 9});

    auto sym = Symbol::Create(C, blocks[7], "f4");
    M->addSymbol(sym);

    // write out aux data
    auto tmp_blocks = fn_blocks;
    M->addAuxData<schema::FunctionBlocks>(std::move(tmp_blocks));
    auto tmp_entries = fn_entries;
    M->addAuxData<schema::FunctionEntries>(std::move(tmp_entries));
    auto tmp_names = fn_names;
    M->addAuxData<schema::FunctionNames>(std::move(tmp_names));

    functions = build_functions(C, *M);
  };
};

/// TESTS

TEST_F(TestData, TEST_CONST) {
  static_assert(std::is_same<Function<Module>::code_block_iterator,
                             Function<Module>::CodeBlockSet::iterator>::value);

  static_assert(std::is_same<
                Function<const Module>::code_block_iterator,
                Function<const Module>::CodeBlockSet::const_iterator>::value);

  Function<const Module> f = functions[0];
  // This fails at compile time, since you cannot convert e.g. a const CodeBlock
  // * to a CodeBlock *
  // Function<Module> f2 = f;
  const Module& M2 = *M;
  auto funs2 = build_functions(C, M2);
  static_assert(std::is_same<decltype(funs2),
                             std::vector<Function<const Module>>>::value);
}

TEST_F(TestData, TEST_NAMES) {
  for (auto& fun : functions) {
    const Symbol* name = fun.getName();
    auto& name_str = name->getName();
    auto id = fun.getUUID();
    if (id == f1) {
      EXPECT_TRUE(name_str == "f1");
    } else if (id == f2) {
      EXPECT_TRUE(name_str == "f2");
    }
  }
}

TEST_F(TestData, TEST_LONG_NAMES) {
  for (auto& fun : functions) {
    if (fun.getUUID() == f1) {
      EXPECT_EQ(fun.getLongName(), "f1");
    } else if (fun.getUUID() == f3) {
      EXPECT_EQ(fun.getLongName(), "f3 (a.k.a f4)");
    }
  }
}

TEST_F(TestData, TEST_UUIDS) {
  std::set<UUID> ids;
  for (auto& fun : functions) {
    ids.insert(fun.getUUID());
  }
  std::set<UUID> expected{f1, f2, f3};
  EXPECT_EQ(ids, expected);
}

TEST_F(TestData, TEST_ENTRIES) {
  for (auto& fun : functions) {
    auto entry_iter = fun.entry_blocks_begin();
    ASSERT_NE(entry_iter, fun.entry_blocks_end());
    auto& entry_block = *entry_iter;
    ASSERT_NE(entry_block, nullptr);

    if (fun.getUUID() == f1) {
      std::set<UUID> entries;
      for (auto& entry : fun.entry_blocks()) {
        entries.insert(entry->getUUID());
      }
      EXPECT_EQ(entries,
                (std::set{blocks[1]->getUUID(), blocks[0]->getUUID()}));
    } else if (fun.getUUID() == f2) {
      EXPECT_EQ(entry_block, blocks[4]);
      EXPECT_EQ(std::next(entry_iter), fun.entry_blocks_end());
    }
  }
}

TEST_F(TestData, TEST_EXITS) {
  for (auto& fun : functions) {
    auto exit_iter = fun.exit_blocks_begin();
    ASSERT_NE(exit_iter, fun.exit_blocks_end());
    auto& exit_block = *exit_iter;
    ASSERT_NE(exit_block, nullptr);
    auto id = fun.getUUID();
    if (id == f1) {
      EXPECT_EQ(exit_block, blocks[2]);
      EXPECT_EQ(std::next(exit_iter), fun.exit_blocks_end());

    } else if (id == f2) {
      EXPECT_EQ(exit_block, blocks[4]);
      EXPECT_EQ(std::next(exit_iter), fun.exit_blocks_end());
    } else if (id == f3) {
      std::set<CodeBlock*> exits;
      for (auto& block : fun.exit_blocks()) {
        exits.insert(block);
      }
      EXPECT_EQ(exits, (std::set<CodeBlock*>{blocks[8], blocks[9]}));
    }
  }
}

TEST_F(TestData, TEST_BLOCKS) {
  for (auto& fun : functions) {
    std::set<UUID> fn_ids;
    for (auto& block : fun.all_blocks()) {
      fn_ids.insert(block->getUUID());
    }
    auto id = fun.getUUID();
    if (id == f1) {
      EXPECT_EQ(fn_ids, fn_blocks[f1]);
    } else if (id == f2) {
      EXPECT_EQ(fn_ids, fn_blocks[f2]);
    } else if (id == f3) {
      EXPECT_EQ(fn_ids, fn_blocks[f3]);
    }
  }
}
