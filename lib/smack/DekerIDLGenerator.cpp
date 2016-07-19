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

//using namespace std;

namespace smack {

using namespace llvm;

bool DekerIDLGenerator::runOnModule(Module& m) {
  BU = &getAnalysis<BUDataStructures>();
  errs() << "here \n";
  for (auto& F : m) {
    if (!smack::Naming::isSmackName(F.getName())) {
      errs() << "function " << F.getName() << "\n";
      DSGraph *graph = BU->getDSGraph(F);
      std::vector<DSNode*> argumentNodes;
      //for(Function::arg_iterator I = Function::arg_begin(F), E = arg_end(F); I != E, ++I) {
      for(auto& A : F.getArgumentList()) {
	Value* val = dyn_cast<Value>(&A);
        if(!val || !val->getType()->isPointerTy()) {errs() << "got null \n"; continue;}
        DSNodeHandle &nodeHandle = graph->getNodeForValue(val);
        DSNode *node = nodeHandle.getNode();
        argumentNodes.push_back(node);
      }
      errs() << "size of parameter values " << argumentNodes.size() << "\n";
      for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        Value* pointer = NULL;
        if (LoadInst* li = dyn_cast<LoadInst>(&*I)) {
          pointer = li->getPointerOperand();
        } else if (StoreInst* si = dyn_cast<StoreInst>(&*I)) {
          pointer = si->getPointerOperand();
        }

        if (pointer) {
          //Function *func = I->getParent()->getParent();

          DSNodeHandle &nodeHandle = graph->getNodeForValue(pointer);
          DSNode *targetNode = nodeHandle.getNode();
          for(std::vector<DSNode*>::iterator it=argumentNodes.begin(); it != argumentNodes.end(); it++) {
            if(*it == targetNode) {
              errs() << "found one\n";
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
