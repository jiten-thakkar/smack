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

#include <stdio.h>
#include <stack>
#include <unordered_map>
#include <tuple>
#include <unordered_set>
#include <vector>
#include <fstream>

struct Hash {
  size_t operator() (const std::vector<unsigned> &offsets) const {
    size_t hash = 0;
    for(auto it = offsets.begin(); it != offsets.end(); it++) {
      hash = hash*10+*it;
    }
    return hash;
  }
};

struct offSets {
  std::unordered_set<std::vector<unsigned>, Hash> read;
  std::unordered_set<std::vector<unsigned>, Hash> write;
  offSets(std::vector<unsigned> offsets, bool isRead) {
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

void printOffsets(DSNode* node, std::string indentation, std::ofstream *file, std::vector<DSNode*> *visitedNodes) {
          *file << indentation << "Write\n";
          for(DSNode::const_offset_iterator it=node->write_offset_begin(); it!=node->write_offset_end(); it++) {
            errs() << "write: " << *it << " ";
            *file << indentation << *it << " ";
          }
          *file << "\n";
          *file << indentation << "Read\n";
          for(DSNode::const_offset_iterator it=node->read_offset_begin(); it!=node->read_offset_end(); it++) {
            errs() << "read: " << *it << " ";
            *file << indentation << *it << " ";
          }
          *file << "\n";
          //indentation.append("  ");
          visitedNodes->push_back(node);
          for(DSNode::const_edge_iterator ei=node->edge_begin(); ei!=node->edge_end(); ei++) {
            if(std::find(visitedNodes->begin(), visitedNodes->end(), (*ei).second.getNode()) != visitedNodes->end()) continue;
            *file << indentation << "{offset: " << (*ei).first << "\n";
            printOffsets((*ei).second.getNode(), indentation, file, visitedNodes);
            *file << "}\n\n";
          }
}

bool DekerIDLGenerator::runOnModule(Module& m) {
  BU = &getAnalysis<BUDataStructures>();
  //TD = &getAnalysis<TDDataStructures>();
  for (auto& F : m) {
  //errs() << "function name:" << F.getName() << "\n";
    if (!smack::Naming::isSmackName(F.getName()) && F.getName().find("llvm") == std::string::npos) {
      errs() << "Function " << F.getName() << "\n";
      if(F.isDeclaration()) {
        errs() << "just function declaration, skipping \n";
        continue;
      }
      std::unordered_map<int, offSets> projections;
      DSGraph *graph = BU->getDSGraph(F);
      std::vector<DSNode*> argumentNodes;
      int paramNumber=0;
      //FILE *file = fopen (F.getName(), "w+");
      std::ofstream file(F.getName());
      for (Function::arg_iterator A=F.arg_begin(); A!=F.arg_end(); A++, paramNumber++) {
        //fprintf(file, "parameter: "+paramNumber+"\n");
        file << "parameter: " << paramNumber << "\n";
	Value* val = dyn_cast<Value>(&*A);
        if(!val || !val->getType()->isPointerTy()) {/*errs() << "got null \n"; */
	  argumentNodes.push_back(NULL);
	} else {
          DSNodeHandle &nodeHandle = graph->getNodeForValue(val);
          DSNode *node = nodeHandle.getNode();
          argumentNodes.push_back(node);
          std::vector<unsigned int> writeOffsets;
          file << "Write\n";
          for(DSNode::const_offset_iterator it=node->write_offset_begin(); it!=node->write_offset_end(); it++) {
            /*if(projections.find(paramNumber) == projections.end()) {
              projections[paramNumber] = offSets();
            } 
            writeOffsets.push_back(*it);*/
            file << *it << " ";
          }
          //projections[paramNumber].write.insert(writeOffsets);
          file << "\n";
          std::vector<unsigned int> readOffsets;
          file << "Read\n";
          for(DSNode::const_offset_iterator it=node->read_offset_begin(); it!=node->read_offset_end(); it++) {
            /*if(projections.find(paramNumber) == projections.end()) {
              projections[paramNumber] = offSets();
            }
            readOffsets.push_back(*it);*/
            file << *it << " ";
          }
          //projections[paramNumber].read.insert(readOffsets);
          file << "\n";
          std::string indentation("");
          std::vector<DSNode*> visitedNodes;
          visitedNodes.push_back(node);
          //errs() << "num of links: " << node->getNumLinks()  << "\n";
          llvm::PointerType* valPointer = dyn_cast<PointerType>(val->getType());
          if(valPointer->isStructType())
          for(DSNode::const_edge_iterator ei=node->edge_begin(); ei!=node->edge_end(); ei++) {
            if(std::find(visitedNodes.begin(), visitedNodes.end(), (*ei).second.getNode()) != visitedNodes.end()) continue;
            //if((*ei).second) {
              //errs() << "write: : " << (*ei).second.
              //errs() << "parameter " << paramNumber << "offset: " << (*ei).first << "\n";
            //}
            file << indentation << "{\noffset: " << (*ei).first << "\n";
            printOffsets((*ei).second.getNode(), indentation, &file, &visitedNodes);
            file << "}\n\n";
          }
          //file.close();
	}
        file << "\n";
      }
              /*for(auto it = projections.begin(); it != projections.end(); it++) {
                errs() << "Parameter: " << (*it).first << "\n";
                (*it).second.print(errs());
                errs() << "\n";
              }*/
    }
  }
 return false;  
}
// Pass ID variable
char DekerIDLGenerator::ID = 0;
}
