#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CFG.h"

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

            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                if (isloop(bb)) {
                    errs() << "find a loop in BB" << "\n";
                } else {
                    errs() << "not a loop" << "\n";
                }
            }

            // insert allca for the dop pointers
            // BasicBlock::iterator ii = std::next(insertAlloca);
            // AllocaInst* dop1 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop1");
            // AllocaInst* dop2 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop2");
            // AllocaInst* dop1br1 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop1br1");
            // AllocaInst* dop2br1 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop2br1");
            // AllocaInst* dop1br2 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop1br2");
            // AllocaInst* dop2br2 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop2br2");

        }

        bool isloop(BasicBlock *bb);
    };
}

// test wether a basic block is a loop
bool DopLoop::isloop(BasicBlock *bb)
{
    Instruction *bbend = bb->getTerminator();
    BranchInst *br = dyn_cast<BranchInst>(bbend);

    // skip the unconditional jumps
    while (br && br->isUnconditonal()) {
        Instruction *next = br->getSuccessor()->getTerminator();
        BranchInst *nextbr = dyn_cast<BranchInst>(next);
        br = nextbr ? nextbr : NULL;
    }

    if (br) {
        BasicBlock *BB = br->getParent;
        for (succ_iterator PI = succ_begin(BB), E = succ_end(BB); PI != E; ++PI) {
            if (*PI == bb)
                return true;
        }
        return false;
    } else
        return false;
}

char DopLoop::ID = 0;
static RegisterPass<DopLoop> X("DopLoop", "Dynamic opaque predicate obfuscation Pass for straight line code");

Pass *llvm::createDopLoop() {
    return new DopLoop();
}

Pass *llvm::createDopLoop(bool flag) {
    return new DopLoop(flag);
}
