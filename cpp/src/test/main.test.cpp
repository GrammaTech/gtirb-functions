#include <gtirb/gtirb.hpp>
#include <gtest/gtest.h>

int main(int argc, char* argv[]) {
  gtirb::Module::registerAuxDataType<gtirb::schema::FunctionEntries>();
  gtirb::Module::registerAuxDataType<gtirb::schema::FunctionBlocks>();
  gtirb::Module::registerAuxDataType<gtirb::schema::FunctionNames>();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
