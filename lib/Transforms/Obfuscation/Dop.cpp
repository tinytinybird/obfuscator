#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Obfuscation/Utils.h"

using namespace llvm;

namespace {
    struct Dop : public FunctionPass {
        static char ID;
        Dop() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            if(toObfuscate(flag,&F,"dop")) {
                errs() << "Hello: ";
                errs().write_escaped(F.getName()) << '\n';
                return false;
            }
        }
    };
}

char Dop::ID = 0;
static RegisterPass<Dop> X("Dop", "Dynamic opaque predicate obfuscation Pass", false, false);
