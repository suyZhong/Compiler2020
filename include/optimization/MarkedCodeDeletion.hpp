#ifndef MARKEDCODEDELETION_HPP
#define MARKEDCODEDELETION_HPP

#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "IRBuilder.h"
#include "Value.h"
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class MarkedCodeDeletion : public Pass {
public:
    MarkedCodeDeletion(Module *m) : Pass(m) {}
    void run() override;
    void findAllDep(Instruction *instr);
    bool isLive(Instruction *instr);
    void deleteNoUseFunc(Module *m);
    void deleteNoUseInst(Module *m);
    void markSafeFunc(Module *m);
    void markUseInstr(Function *f);

private:
    Function *func_;
    BasicBlock *bb_;
    std::map<BasicBlock *, std::set<Value *>> deadInstr;
    std::set<Value *> liveInstr;
    std::unordered_set<Value *> safeFunc;
};

#endif