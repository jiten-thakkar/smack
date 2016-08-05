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
#include <unordered_map>
#include <tuple>
#include <unordered_set>
#include <vector>

struct Hash {
  size_t operator() (const std::vector<int> &offsets) const {
    size_t hash = 0;
    for(auto it = offsets.begin(); it != offsets.end(); it++) {
      hash = hash*10+*it;
    }
    return hash;
  }
};

struct offSets {
  std::unordered_set<std::vector<int>, Hash> read;
  std::unordered_set<std::vector<int>, Hash> write;
  offSets(std::vector<int> offsets, bool isRead) {
    if(isRead)
      read.insert(offsets);
    else
      write.insert(offsets);
  }
  offSets(){}
  offSets(const offSets& object):read(object.read), write(object.write){}
  void print(llvm::raw_ostream& ostream) {
    if(!read.empty()) {
    ostream << "Read\n";
    for(auto it = read.begin(); it != read.end(); it++) {
      for(auto vit = (*it).begin(); vit != (*it).end(); vit++) {
        ostream << *vit << " ";
      }
      ostream << "\n";
    }
    }
    if(!write.empty()) {
    ostream << "Write\n";
    for(auto it = write.begin(); it != write.end(); it++) {
      for(auto vit = (*it).begin(); vit != (*it).end(); vit++) {
        ostream << *vit << " ";
      }
      ostream << "\n";
    }
    } 
  }
};

namespace smack {

using namespace llvm;

bool DekerIDLGenerator::runOnModule(Module& m) {
  BU = &getAnalysis<BUDataStructures>();
  //TD = &getAnalysis<TDDataStructures>();
  for (auto& F : m) {
    if (!smack::Naming::isSmackName(F.getName())) {
      errs() << "Function " << F.getName() << "\n";
      std::unordered_map<int, offSets> projections;
      DSGraph *graph = BU->getDSGraph(F);
      std::vector<DSNode*> argumentNodes;
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
      /*if(F.getName() == "e") {
        for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
          (*I).print(errs());
          errs() << "\n";
        }
      }*/
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
          std::vector<int> offsets;
	  if (isa<GetElementPtrInst>(pointer)) {
	    while (GetElementPtrInst* elemPtr = dyn_cast<GetElementPtrInst>(pointer)) {
	      if(elemPtr->getNumOperands() < 3) break;
	      ConstantInt* CI = dyn_cast<ConstantInt>(elemPtr->getOperand(2));
              offsets.push_back(CI->getSExtValue());
	      pointer = elemPtr->getPointerOperand();
              if(LoadInst* li = dyn_cast<LoadInst>(pointer)) {
                pointer = li->getPointerOperand();
              }
	    }
	  }

          DSNodeHandle &nodeHandle = graph->getNodeForValue(pointer);
          DSNode *targetNode = nodeHandle.getNode();
          for(std::vector<DSNode*>::iterator it=argumentNodes.begin(); it != argumentNodes.end(); it++) {
            if(*it == targetNode) {
              int paramNumber = it - argumentNodes.begin();
              std::reverse(offsets.begin(), offsets.end());
              if(projections.find(paramNumber) == projections.end()) {
                if(isa<LoadInst>(inst))
                  projections[paramNumber] = offsets.empty() ? offSets() : offSets(offsets, true);
                else
                  projections[paramNumber] = offsets.empty() ? offSets() : offSets(offsets, false);
              } else if(!offsets.empty()) {
                if(isa<LoadInst>(inst))
                  projections[paramNumber].read.insert(offsets);
	        else
                  projections[paramNumber].write.insert(offsets);
              }
            }
	  }
      }   

    }	
              for(auto it = projections.begin(); it != projections.end(); it++) {
                errs() << "Parameter: " << (*it).first << "\n";
                (*it).second.print(errs());
                errs() << "\n";
              }
  }
}

  return false;
}

// Pass ID variable
char DekerIDLGenerator::ID = 0;
}
