#ifndef GLOBALSUBEXPRDELETION_HPP
#define GLOBALSUBEXPRDELETION_HPP

#include "Constant.h"
#include "Instruction.h"
#include "Module.h"
#include "PassManager.hpp"

#include "IRBuilder.h"
#include "Value.h"
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

class GlobalSubExprDeletion : public Pass {
public:
    GlobalSubExprDeletion(Module *m) : Pass(m) {}
    void run() override;
    void initGenKill(Module *m);
    void calcInOut(Module *m);

private:
    std::map<BasicBlock *, std::set<std::pair<Value *, Value *>>> avail_in, avail_out;
};

#endif