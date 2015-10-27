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

                addDOP(F);

                return true;

                // for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                //     for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
                //         errs().write_escaped(i->getOpcodeName()) << '\n';
                //     }
                // }

            }
            return false;
        }
        void addDOP(Function &F) {
            bool firstAlloca = false;

            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {       
                for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
                    unsigned opcode = i->getOpcode();
                    if (opcode == Instruction::Alloca && firstAlloca == false) {
                        errs() << "def: " << *i << "\n";
                        for(Value::use_iterator ui = i->use_begin(), ie = i->use_end(); ui != ie; ++ui){
                            Value *v = *ui;
                            Instruction *vi = dyn_cast(*ui);
                            errs() << "\t\t" << *vi << "\n";
                        }
                        // errs().write_escaped(i->getOpcodeName()) << '\n';
			// errs().write_escaped(i->getOperand(0)->getName()) << '\n';
                        firstAlloca = true;
			continue;
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
