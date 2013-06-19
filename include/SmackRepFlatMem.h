#ifndef SMACKREPFLATMEM_H
#define SMACKREPFLATMEM_H

#include "SmackRep.h"

namespace smack {

    using llvm::Regex;
    using llvm::SmallVector;
    using llvm::StringRef;
    using namespace std;

    class SmackRepFlatMem : public SmackRep {
    public:
        static const string CURRADDR;

    public:
        SmackRepFlatMem(llvm::DataLayout *td) : SmackRep(td) {}
    };
}

#endif // SMACKREPFLATMEM_H

