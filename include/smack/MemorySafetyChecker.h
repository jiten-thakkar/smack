//
// This file is distributed under the MIT License. See LICENSE for details.
//
#ifndef MEMORYSAFETYCHECKER_H
#define MEMORYSAFETYCHECKER_H

#include "llvm/Pass.h"
#include "smack/Regions.h"

namespace smack {
  
class MemorySafetyChecker: public llvm::ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  MemorySafetyChecker() : llvm::ModulePass(ID) {}
  
  virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const {
    AU.setPreservesAll();
    AU.addRequired<Regions>();
  }
  virtual bool runOnModule(llvm::Module& m);
};

}

#endif //MEMORYSAFETYCHECKER_H

