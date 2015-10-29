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

            LoadInst* dop1p = new LoadInst(dop1, "", false, 4, ii);
            LoadInst* dop2p = new LoadInst(dop2, "", false, 4, ii);
            LoadInst* dop1deref = new LoadInst(dop1p, "", false, 4, ii);
            LoadInst* dop2deref = new LoadInst(dop2p, "", false, 4, ii);

            const Twine & name = "clone";
            ValueToValueMapTy VMap;
            BasicBlock* alterBB = llvm::CloneBasicBlock(obfBB, VMap, name, &F);

            Twine * var3 = new Twine("dopbranch1");
            Value * rvalue = ConstantInt::get(Type::getInt32Ty(F.getContext()), 0);
            preBB->getTerminator()->eraseFromParent();
            ICmpInst * dopbranch1 = new ICmpInst(*preBB, CmpInst::ICMP_SGT , dop1deref, rvalue, *var3);
            BranchInst::Create(obfBB, alterBB, dopbranch1, preBB);

            BasicBlock::iterator splitpt1 = obfBB->begin(),
	                         splitpt2 = alterBB->begin();
            BasicBlock *obfBB2, *alterBB2;
            int num = 2;
	    int n = num;
            for (BasicBlock::iterator e = obfBB->end(); splitpt1 != e && n > 0; ++splitpt1, --n) ;
	    n = num+1;
            for (BasicBlock::iterator e = alterBB->end(); splitpt2 != e && n > 0; ++splitpt2, --n) ;
            Twine *var4 = new Twine("obfBB2");
            obfBB2 = obfBB->splitBasicBlock(splitpt1, *var4);
            Twine *var5 = new Twine("obfBBclone2");
            alterBB2 = alterBB->splitBasicBlock(splitpt2, *var5);

            BasicBlock* dop2BB = BasicBlock::Create(F.getContext(), "dop2BB", &F, obfBB2);
            Twine * var6 = new Twine("dopbranch2");
            Value * rvalue2 = ConstantInt::get(Type::getInt32Ty(F.getContext()), 0);
            dop2BB->getTerminator()->eraseFromParent();
            ICmpInst * dopbranch2 = new ICmpInst(*dop2BB, CmpInst::ICMP_SGT , dop2deref, rvalue, *var3);
            BranchInst::Create(obfBB2, alterBB2, dopbranch2, dop2BB);
            
            obfBB->getTerminator()->eraseFromParent();
            BranchInst::Create(dop2BB, obfBB);
            alterBB->getTerminator()->eraseFromParent();
            BranchInst::Create(dop2BB, alterBB);
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
