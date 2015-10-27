#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Obfuscation/Dop.h"
#include "llvm/Transforms/Obfuscation/Utils.h"

using namespace llvm;

namespace {
    struct Dop : public FunctionPass {
        static char ID;
        bool flag;
        Dop() : FunctionPass(ID) {}
        Dop(bool flag) : FunctionPass(ID) {this->flag = flag; Dop();}

        bool runOnFunction(Function &F) override {
            if(toObfuscate(flag,&F,"dop")) {
                errs() << "Hello: ";
                errs().write_escaped(F.getName()) << '\n';

                std::vector worklist;
                for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
                    worklist.push_back(&*I);
                }

// def-use chain for Instruction
                for(std::vector::iterator iter = worklist.begin(); iter != worklist.end(); ++iter){
                    Instruction* instr = *iter;
                    errs() << "def: " <<*instr << "\n";
                    for(Value::use_iterator i = instr->use_begin(), ie = instr->use_end(); i!=ie; ++i){
                        Value *v = *i;
                        Instruction *vi = dyn_cast(*i);
                        errs() << "\t\t" << *vi << "\n";
                    }
                }
            }
        }
    };
}

char Dop::ID = 0;
static RegisterPass<Dop> X("Dop", "Dynamic opaque predicate obfuscation Pass", false, false);

Pass *llvm::createDop() {
  return new Dop();
}

Pass *llvm::createDop(bool flag) {
  return new Dop(flag);
}
