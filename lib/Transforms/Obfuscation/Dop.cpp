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
                StringRef *sr = new StringRef("fun");
                if (F.getName().equals(*sr)) {
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
            }
            return false;
        }
        void addDOP(Function &F) {
            bool firstStore = true;
            Value *covar;
            BasicBlock *preBB, *postBB, *obfBB;
            BasicBlock::iterator preBBend, obfBBend, insertAlloca;

            for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {
                for (BasicBlock::iterator i = bb->begin(), e = bb->end(); i != e; ++i) {
                    unsigned opcode = i->getOpcode();
                    if (opcode == Instruction::Store) {
                        if (firstStore == true) {
                            errs() << "The first store: " << *i << "\n";
                            errs() << *i->getOperand(1)  << "\n";
                            covar = i->getOperand(1);
                            insertAlloca = i;
                            preBBend = std::next(i);
                            // for(Value::use_iterator ui = i->use_begin(), ie = i->use_end(); ui != ie; ++ui){
                            //   Value *v = *ui;
                            //   Instruction *vi = dyn_cast<Instruction>(*ui);
                            //   errs() << "\t\t" << *vi << "\n";
                            // }
                            // errs().write_escaped(i->getOpcodeName()) << '\n';
                            // errs().write_escaped(i->getOperand(0)->getName()) << '\n';
                            firstStore = false;
                            continue;
                        } else {
                            if (i->getOperand(1) == covar) {
                                obfBBend = i;
                                errs() << "    " << *i << "\n";
                            }
                        }
                    }
                }
            }
	    Function::iterator bb = F.begin();
            preBB = bb;
            Twine *var1 = new Twine("obfBB");
            obfBB = bb->splitBasicBlock(preBBend, *var1);
            Twine *var2 = new Twine("postBB");
            postBB = obfBB->splitBasicBlock(obfBBend, *var2);

            BasicBlock::iterator ii = std::next(insertAlloca);
            AllocaInst* dop1 = new AllocaInst(Type::getInt32PtrTy(getGlobalContext()), 0, 4, "dop1");
            AllocaInst* dop2 = new AllocaInst(Type::getInt32PtrTy(F.getContext()), 0, 4, "dop2");
            preBB->getInstList().insert(ii, dop1);
            preBB->getInstList().insert(ii, dop2);

            StoreInst* dop1st = new StoreInst(insertAlloca->getOperand(1), dop1, false, ii);
            StoreInst* dop2st = new StoreInst(insertAlloca->getOperand(1), dop2, false, ii);

            LoadInst* dop1ld = new LoadInst(dop1st, "", ii);
            LoadInst* dop2ld = new LoadInst(dop2st, "", ii);
            LoadInst* dop1ld = new LoadInst(dop1st, "", ii);
            LoadInst* dop2ld = new LoadInst(dop2st, "", ii);


            Twine * var3 = new Twine("dopbrach1");
            Value * rvalue = ConstantInt::get(Type::getInt32Ty(F.getContext()), 0);
            ICmpInst * dopbranch1 = new ICmpInst(*originalBB, CmpInst::ICMP_SGT , LHS, rvalue, *var3);
            BranchInst::Create(originalBBpart2, alteredBB, (Value *)condition2, originalBB);
        }
    };
}

char Dop::ID = 0;
static RegisterPass<Dop> X("Dop", "Dynamic opaque predicate obfuscation Pass");

Pass *llvm::createDop() {
  return new Dop();
}

Pass *llvm::createDop(bool flag) {
  return new Dop(flag);
}
