#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Transforms/Obfuscation/DopBr.h"
#include "llvm/Transforms/Obfuscation/Utils.h"

using namespace llvm;

namespace {
    struct DopBr : public FunctionPass {
        static char ID;
        bool flag;
        DopBr() : FunctionPass(ID) {}
        DopBr(bool flag) : FunctionPass(ID) {this->flag = flag; DopBr();}

        bool runOnFunction(Function &F) override {
            if(toObfuscate(flag,&F,"dopbr")) {
                StringRef *sr = new StringRef("fun");
                if (F.getName().equals(*sr)) {
                    addDopBranch(F);
                    return true;
                }
            }
            return false;
        }

        // add dynamic opaque predicates to 
        void addDopBranch(Function &F) {
            BranchInst *ibr;
            std::set<Instruction *> dep;
            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                ibr = dyn_cast<BranchInst>(bb->getTerminator());
                if (ibr && ibr->isConditional()) {
                    errs() << "find a branch in BB" << "\n";
                }
            }
            builddep(ibr, dep);
            for (std::set<Instruction *>::iterator i = dep.begin(); i != dep.end(); ++i) {
                errs() << **i << "\n";
            }
        }

        // build dependence
    private:
        void builddep(Instruction *iuse, std::set<Instruction*> &idep);
    };
}


void DopBr::builddep(Instruction *iuse, std::set<Instruction*> &idep) {
    for (User::op_iterator opi = iuse->op_begin(), ope = iuse->op_end(); opi != ope; ++opi) {
        Instruction *vi = dyn_cast<Instruction>(*opi);
        if (vi == NULL) return;
        else {
            switch (vi->getOpcode()) {
            case Instruction::Add:
                idep.insert(vi);
                builddep(dyn_cast<Instruction>(*vi->getOperand(0)), idep);
                builddep(dyn_cast<Instruction>(*vi->getOperand(1)), idep);
                return;
            case Instruction::Sub:
                idep->insert(vi);
                builddep(dyn_cast<Instruction>(*vi->getOperand(0)), idep);
                builddep(dyn_cast<Instruction>(*vi->getOperand(1)), idep);
                return;
            case Instruction::Mul:
                idep->insert(vi);
                builddep(dyn_cast<Instruction>(*vi->getOperand(0)), idep);
                builddep(dyn_cast<Instruction>(*vi->getOperand(1)), idep);
                return;
            case Instruction::Br:
                idep->insert(vi);
                builddep(dyn_cast<Instruction>(*vi->getOperand(0)), idep);
                return;
            case Instruction::Load:
                idep->insert(vi);
                BasicBlock::iterator i = vi;
                for (BasicBlock::iterator j = vi->getParent()->end(); i != j; --i) {
                    if (i->getOpcode() == Instruction::Store && i->getOperand(1) == vi->getOperand(0)) {
                        break;
                    }
                }
                builddep(dyn_cast<Instruction>(*i), idep);
                return;
            case Instruction::Store:
                idep->insert(vi);
                builddep(dyn_cast<Instruction>(*vi->getOperand(1)), idep);
                return;
            case Instruction::Alloca:
                idep->insert(vi);
                return;
            case Instruction::ICmp:
                idep->insert(vi);
                builddep(dyn_cast<Instruction>(*vi->getOperand(0)), idep);
                return;
            }
        }
    }
}

char DopBr::ID = 0;
static RegisterPass<DopBr> X("DopBr", "Dynamic opaque predicate obfuscation Pass for straight line code");

Pass *llvm::createDopBr() {
    return new DopBr();
}

Pass *llvm::createDopBr(bool flag) {
    return new DopBr(flag);
}
