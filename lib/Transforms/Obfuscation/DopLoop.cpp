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
            BasicBlock *loopBB, *preBB;
            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                if (isloop(bb)) {
                    loopBB = bb;
                    errs() << "find a loop in BB" << "\n";
                } else {
                    errs() << "not a loop" << "\n";
                }
            }

            // search for the BB before the loopBB
            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                Instruction *bbend = bb->getTerminator();
                BranchInst *br = dyn_cast<BranchInst>(bbend);

                if (br && br->isUnconditional() && br->getSuccessor(0) == loopBB) {
                    preBB = bb;
                    break;
                }
                
            }

            // find the local variable for dop
            // the first store instruction (Can be improved!)
            BasicBlock::iterator insertAlloca;
            for (BasicBlock::iterator i = preBB->begin(), e = preBB->end(); i != e; ++i) {
                unsigned opcode = i->getOpcode();
                if (opcode == Instruction::Store) {
                    insertAlloca = i;
                    break;
                }
            }

            // insert allca for the dop pointers
            BasicBlock::iterator ii = std::next(insertAlloca);
            AllocaInst* dop1 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop1");
            AllocaInst* dop2 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop2");
            AllocaInst* dop1br1 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop1br1");
            AllocaInst* dop2br1 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop2br1");
            AllocaInst* dop1br2 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop1br2");
            AllocaInst* dop2br2 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop2br2");

        }

        bool isloop(BasicBlock *bb);
    };
}

// check wether a basic block is a loop body
bool DopLoop::isloop(BasicBlock *bb)
{
    Instruction *bbend = bb->getTerminator();
    BranchInst *br = dyn_cast<BranchInst>(bbend);

    // skip the unconditional jumps
    while (br && br->isUnconditional()) {
        Instruction *next = br->getSuccessor(0)->getTerminator();
        BranchInst *nextbr = dyn_cast<BranchInst>(next);
        br = nextbr ? nextbr : NULL;
    }

    if (br) {
        int succnum = br->getNumSuccessors();
        for (int i = 0; i < succnum; ++i) {
            if (br->getSuccessor(i) == bb)
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
