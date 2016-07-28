//
// This file is distributed under the MIT License. See LICENSE for details.
//
#include "smack/DekerIDLGenerator.h"
#include "smack/Naming.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/LLVMContext.h"

#include <stack>

//using namespace std;

namespace smack {

using namespace llvm;

bool DekerIDLGenerator::runOnModule(Module& m) {
  BU = &getAnalysis<BUDataStructures>();
  //TD = &getAnalysis<TDDataStructures>();
  errs() << "here \n";
  for (auto& F : m) {
    if (!smack::Naming::isSmackName(F.getName())) {
      errs() << "function " << F.getName() << "\n";
      DSGraph *graph = BU->getDSGraph(F);
      std::vector<DSNode*> argumentNodes;
      //for(Function::arg_iterator I = Function::arg_begin(F), E = arg_end(F); I != E, ++I) {
      for(auto& A : F.getArgumentList()) {
	Value* val = dyn_cast<Value>(&A);
        if(!val || !val->getType()->isPointerTy()) {/*errs() << "got null \n"; */
	  argumentNodes.push_back(NULL);
	} else {
          DSNodeHandle &nodeHandle = graph->getNodeForValue(val);
          DSNode *node = nodeHandle.getNode();
          argumentNodes.push_back(node);
	}
      }
      if(F.getName() == "e") {
        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
          (*I).print(errs());
          errs() << "\n";
        }
      }
      errs() << "size of parameter values " << argumentNodes.size() << "\n";
      for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        Value* pointer = NULL;
	Instruction* inst = NULL;
        if (LoadInst* li = dyn_cast<LoadInst>(&*I)) {
          pointer = li->getPointerOperand();
	  inst = &*I;
        } else if (StoreInst* si = dyn_cast<StoreInst>(&*I)) {
          pointer = si->getPointerOperand();
          inst = &*I;
        }

        if (pointer) {
          //inst->print(errs());
          //errs() << "\n";
          std::stack<int> offsets;
	  if (isa<GetElementPtrInst>(pointer)) {
	    while (GetElementPtrInst* elemPtr = dyn_cast<GetElementPtrInst>(pointer)) {
	      //elemPtr->print(errs());
              //errs() << "\n";
	      if(elemPtr->getNumOperands() < 3) break;
	      ConstantInt* CI = dyn_cast<ConstantInt>(elemPtr->getOperand(2));
              //errs() << CI->getSExtValue() << " \n";
              offsets.push(CI->getSExtValue());
	      pointer = elemPtr->getPointerOperand();
              //pointer = operand->getPointerOperand();
              if(LoadInst* li = dyn_cast<LoadInst>(pointer)) {
                pointer = li->getPointerOperand();
              }
	    }
            //errs() << "element ptr found \n";
            //inst->print(errs());
	  }
          //Function *func = I->getParent()->getParent();

          DSNodeHandle &nodeHandle = graph->getNodeForValue(pointer);
          DSNode *targetNode = nodeHandle.getNode();
          for(std::vector<DSNode*>::iterator it=argumentNodes.begin(); it != argumentNodes.end(); it++) {
            if(*it == targetNode) {
              errs() << "found one\n";
              errs() << "parameter number: " << it - argumentNodes.begin() << "\n";
              if(offsets.size() > 0) {
	        errs() << "offsets: ";
                while(!offsets.empty()) {
                  errs() << offsets.top() << " ";
                  offsets.pop();
	        }
	        errs() << "\n";
	      }
              //inst->print(errs());
              //errs() << "\n";
	      /*MDNode* meta = inst->getMetadata((unsigned)LLVMContext::MD_dbg);
	      DIVariable div(meta);
    DIType dit=div.getType(); 
    DIDerivedType didt=static_cast<DIDerivedType>(dit);
    DICompositeType dict=static_cast<DICompositeType>(didt.getTypeDerivedFrom());
    DIArray dia=dict.getTypeArray(); 
    assert(offset<dia.getNumElements());
    DIType field=static_cast<DIType>(dia.getElement(offset));
    errs()<<"Field'name is "<<field.getName()<<"\n";*/
            }
	  }
        }
        //return ""; 
      }   
    }	
  }
  return false;
}

// Pass ID variable
char DekerIDLGenerator::ID = 0;
}
