//
// This file is distributed under the MIT License. See LICENSE for details.
//
#ifndef DEKERIDLGENERATOR_H
#define DEKERIDLGENERATOR_H

#include "llvm/Pass.h"
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
#include "dsa/DSNode.h"

namespace smack {

using namespace llvm;
  
class DekerIDLGenerator: public llvm::ModulePass {
private:
BUDataStructures* BU;
TDDataStructures* TD;

public:
  static char ID; // Pass identification, replacement for typeid
  
  DekerIDLGenerator() : llvm::ModulePass(ID) {}
  
  virtual bool runOnModule(llvm::Module& m);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BUDataStructures>();
    AU.setPreservesAll();
  }
};

}

#endif //DEKERIDLGENERATOR_H

