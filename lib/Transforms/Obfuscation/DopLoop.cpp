#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Obfuscation/DopLoop.h"
#include "llvm/Transforms/Obfuscation/Utils.h"

using namespace llvm;

namespace {
    struct DopLoop : public FunctionPass {
        static char ID;
        bool flag;
        DopLoop() : FunctionPass(ID) {}
        DopLoop(bool flag) : FunctionPass(ID) {this->flag = flag; DopLoop();}

        bool runOnFunction(Function &F) override {
            if(toObfuscate(flag,&F,"dopbr")) {
                StringRef *sr = new StringRef("fun");
                if (F.getName().equals(*sr)) {
                    addDopLoop(F);
                    return true;
                }
            }
            return false;
        }
        void addDopLoop(Function &F) {
            errs() << "Set up files for DopLoop" << "\n";
        }
    }
}

char DopLoop::ID = 0;
static RegisterPass<DopLoop> X("DopLoop", "Dynamic opaque predicate obfuscation Pass for straight line code");

Pass *llvm::createDopLoop() {
    return new DopLoop();
}

Pass *llvm::createDopLoop(bool flag) {
    return new DopLoop(flag);
}
