#include <gtirb/Context.hpp>
#include <gtirb/IR.hpp>
#include <gtirb/Module.hpp>
#include <gtirb/gtirb.hpp>
#include <gtirb_functions/gtirb_functions.hpp>

int main(void) {
  gtirb::Module::registerAuxDataType<gtirb::schema::FunctionEntries>();
  gtirb::Module::registerAuxDataType<gtirb::schema::FunctionBlocks>();
  gtirb::Module::registerAuxDataType<gtirb::schema::FunctionNames>();
  gtirb::Context Ctx;
  gtirb::IR* IR = gtirb::IR::Create(Ctx);
  gtirb::Module* M = gtirb::Module::Create(Ctx, "");

  auto funs = gtirb::build_functions(Ctx, *M);

  return 0;
}
