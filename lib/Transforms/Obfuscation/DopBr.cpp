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

        // build dependence
        void builddep(Instruction *iuse, std::set<Instruction*> &idep);

        // add dynamic opaque predicates to 
        void addDopBranch(Function &F) {
            BranchInst *ibr;
            BasicBlock *preBB;
            std::set<Instruction *> dep;
            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                ibr = dyn_cast<BranchInst>(bb->getTerminator());
                if (ibr && ibr->isConditional()) {
                    errs() << "find a branch in BB" << "\n";
		    break;
                }
            }
            builddep(ibr, dep);
            
	    errs() << "dep is built." << '\n';
            for (std::set<Instruction *>::iterator i = dep.begin(); i != dep.end(); ++i) {
                errs() << **i << "\n";
            }

            // find the last inst before ibr but not affect it
            preBB = ibr->getParent();
            BasicBlock::iterator offsetinst = ibr;
            for (BasicBlock::iterator j = preBB->begin(); offsetinst != j; --offsetinst) {
	      if (dep.find(offsetinst) == dep.end()) {
                    break;
                }
            }

            // find the local variable for dop
            // the first store instruction (Can be improved!)
            BasicBlock::iterator insertAlloca;
            for (BasicBlock::iterator i = ibr->getParent()->begin(), e = ibr->getParent()->end(); i != e; ++i) {
                unsigned opcode = i->getOpcode();
                if (opcode == Instruction::Store) {
                    insertAlloca = i;
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

            preBB->getInstList().insert(ii, dop1);
            preBB->getInstList().insert(ii, dop2);
            preBB->getInstList().insert(ii, dop1br1);
            preBB->getInstList().insert(ii, dop2br1);
            preBB->getInstList().insert(ii, dop1br2);
            preBB->getInstList().insert(ii, dop2br2);

            // store the variable's address to the dop pointers
            StoreInst* dop1st = new StoreInst(insertAlloca->getOperand(1), dop1, false, ii);
            StoreInst* dop2st = new StoreInst(insertAlloca->getOperand(1), dop2, false, ii);
            StoreInst* dop1br1st = new StoreInst(insertAlloca->getOperand(1), dop1br1, false, ii);
            StoreInst* dop2br1st = new StoreInst(insertAlloca->getOperand(1), dop2br1, false, ii);
            StoreInst* dop1br2st = new StoreInst(insertAlloca->getOperand(1), dop1br2, false, ii);
            StoreInst* dop2br2st = new StoreInst(insertAlloca->getOperand(1), dop2br2, false, ii);

        }
    };
}

// build a set idep, which contains all instructions that iuse depends on
void DopBr::builddep(Instruction *iuse, std::set<Instruction*> &idep) {
    errs() << "Enter builddep" << '\n';
    // for (User::op_iterator opi = iuse->op_begin(), ope = iuse->op_end(); opi != ope; ++opi) {
    // errs() << "Enter for loop" << '\n';
    // Instruction *vi = dyn_cast<Instruction>(*opi);
    Instruction *vi = iuse;
    Instruction *op0, *op1;
    if (vi == NULL) {
        errs() << "No instruction" << '\n';
        return;
    }
    else {
        switch (vi->getOpcode()) {
        case Instruction::Add:
            errs() << "Add" << '\n';
            idep.insert(vi);
            op0 = dyn_cast<Instruction>(vi->getOperand(0));
            op1 = dyn_cast<Instruction>(vi->getOperand(1));
            builddep(op0, idep);
            builddep(op1, idep);
            return;
        case Instruction::Sub:
            errs() << "Sub" << '\n';
            idep.insert(vi);
            op0 = dyn_cast<Instruction>(vi->getOperand(0));
            op1 = dyn_cast<Instruction>(vi->getOperand(1));
            builddep(op0, idep);
            builddep(op1, idep);
            return;
        case Instruction::Mul:
            errs() << "Mul" << '\n';
            idep.insert(vi);
            op0 = dyn_cast<Instruction>(vi->getOperand(0));
            op1 = dyn_cast<Instruction>(vi->getOperand(1));
            builddep(op0, idep);
            builddep(op1, idep);
            return;
        case Instruction::Br:
            errs() << "Br" << '\n';
            idep.insert(vi);
            op0 = dyn_cast<Instruction>(vi->getOperand(0));
            builddep(op0, idep);
            return;
        case Instruction::Load:
        {
            errs() << "Load" << '\n';
            idep.insert(vi);
            BasicBlock::iterator i = vi;
            for (BasicBlock::iterator j = vi->getParent()->end(); i != j; --i) {
                if (i->getOpcode() == Instruction::Store && i->getOperand(1) == vi->getOperand(0)) {
                    break;
                }
            }
            op0 = dyn_cast<Instruction>(i);
            builddep(op0, idep);
            return;
        }
        case Instruction::Store:
            errs() << "Store" << '\n';
            idep.insert(vi);
            op0 = dyn_cast<Instruction>(vi->getOperand(0));
            builddep(op0, idep);
            return;
        case Instruction::Alloca:
            errs() << "Alloca" << '\n';
            idep.insert(vi);
            return;
        case Instruction::ICmp:
            errs() << "ICmp" << '\n';
            idep.insert(vi);
            op0 = dyn_cast<Instruction>(vi->getOperand(0));
            builddep(op0, idep);
            return;
        default:
	    errs() << "Not match" << '\n';
            return;
        }
    }
    // }
}

char DopBr::ID = 0;
static RegisterPass<DopBr> X("DopBr", "Dynamic opaque predicate obfuscation Pass for straight line code");

Pass *llvm::createDopBr() {
    return new DopBr();
}

Pass *llvm::createDopBr(bool flag) {
    return new DopBr(flag);
}
