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
  size_t operator() (const std::vector<unsigned int> &offsets) const {
    size_t hash = 0;
    for(auto it = offsets.begin(); it != offsets.end(); it++) {
      hash = hash*10+*it;
    }
    return hash;
  }
};

struct offSets {
  std::unordered_set<std::vector<unsigned int>, Hash> read;
  std::unordered_set<std::vector<unsigned int>, Hash> write;
  offSets(std::vector<unsigned int> offsets, bool isRead) {
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
  //errs() << "function name:" << F.getName() << "\n";
    if (!smack::Naming::isSmackName(F.getName())) {
      errs() << "Function " << F.getName() << "\n";
      std::unordered_map<int, offSets> projections;
      DSGraph *graph = BU->getDSGraph(F);
      std::vector<DSNode*> argumentNodes;
      int paramNumber=0;
      for (Function::arg_iterator A=F.arg_begin(); A!=F.arg_end(); A++, paramNumber++) {
	Value* val = dyn_cast<Value>(&*A);
        if(!val || !val->getType()->isPointerTy()) {/*errs() << "got null \n"; */
	  argumentNodes.push_back(NULL);
	} else {
          DSNodeHandle &nodeHandle = graph->getNodeForValue(val);
          DSNode *node = nodeHandle.getNode();
          argumentNodes.push_back(node);
          std::vector<unsigned int> writeOffsets;
          for(DSNode::const_offset_iterator it=node->write_offset_begin(); it!=node->write_offset_end(); it++) {
            if(projections.find(paramNumber) == projections.end()) {
              projections[paramNumber] = offSets();
            } 
            writeOffsets.push_back(*it);
          }
          projections[paramNumber].write.insert(writeOffsets);
          
          std::vector<unsigned int> readOffsets;
          for(DSNode::const_offset_iterator it=node->read_offset_begin(); it!=node->read_offset_end(); it++) {
            if(projections.find(paramNumber) == projections.end()) {
              projections[paramNumber] = offSets();
            }
            readOffsets.push_back(*it);
          }
          projections[paramNumber].read.insert(readOffsets);
          for(DSNode::const_edge_iterator ei=node->edge_begin(); ei!=node->edge_end(); ei++) {
            //if((*ei).second) {
              errs() << "offset: " << (*ei).first << "\n";
            //}
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
