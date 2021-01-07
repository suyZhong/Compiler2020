#include "ConstPropagation.hpp"
#include "logging.hpp"

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute(
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2) {

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op) {
    case Instruction::add:
        return ConstantInt::get(c_value1 + c_value2, module_);

        break;
    case Instruction::sub:
        return ConstantInt::get(c_value1 - c_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(c_value1 * c_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(c_value1 / c_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}
ConstantInt *ConstFolder::compare(
    CmpInst::CmpOp op,
    ConstantInt *value1,
    ConstantInt *value2) {

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op) {
    case CmpInst::GE:
        return ConstantInt::get(c_value1 >= c_value2, module_);
        break;
    case CmpInst::GT:
        return ConstantInt::get(c_value1 > c_value2, module_);
        break;
    case CmpInst::LE:
        return ConstantInt::get(c_value1 <= c_value2, module_);
        break;
    case CmpInst::LT:
        return ConstantInt::get(c_value1 < c_value2, module_);
        break;
    case CmpInst::EQ:
        return ConstantInt::get(c_value1 = c_value2, module_);
        break;
    case CmpInst::NE:
        return ConstantInt::get(c_value1 != c_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}
ConstantFP *ConstFolder::compute(
    Instruction::OpID op,
    ConstantFP *value1,
    ConstantFP *value2) {

    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op) {
    case Instruction::fadd:
        return ConstantFP::get(c_value1 + c_value2, module_);

        break;
    case Instruction::fsub:
        return ConstantFP::get(c_value1 - c_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFP::get(c_value1 * c_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFP::get(c_value1 / c_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}
ConstantInt *ConstFolder::compare(
    FCmpInst::CmpOp op,
    ConstantFP *value1,
    ConstantFP *value2) {

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op) {
    case FCmpInst::GE:
        return ConstantInt::get(c_value1 >= c_value2, module_);
        break;
    case FCmpInst::GT:
        return ConstantInt::get(c_value1 > c_value2, module_);
        break;
    case FCmpInst::LE:
        return ConstantInt::get(c_value1 <= c_value2, module_);
        break;
    case FCmpInst::LT:
        return ConstantInt::get(c_value1 < c_value2, module_);
        break;
    case FCmpInst::EQ:
        return ConstantInt::get(c_value1 = c_value2, module_);
        break;
    case FCmpInst::NE:
        return ConstantInt::get(c_value1 != c_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value) {
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr) {
        return constant_fp_ptr;
    } else {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value) {
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr) {
        return constant_int_ptr;
    } else {
        return nullptr;
    }
}

BasicBlock *ConstFolder::traverseBranch(BasicBlock *bb, int depth) {

    if (depth == 1 && bb->get_pre_basic_blocks().size() == 2) {
        return bb;
    }
    LOG_DEBUG << "rb insert " << bb->get_name();
    if (bb->get_name() == "label27")
        LOG_DEBUG << depth << "qwe" << bb->get_pre_basic_blocks().size();
    redundantBBs.insert(bb);
    if (bb->get_pre_basic_blocks().size() == 2) {
        depth -= 1;
    }
    if (bb->get_name() == "label24")
        LOG_DEBUG << depth << "asd" << bb->get_succ_basic_blocks().size();
    if (bb->get_succ_basic_blocks().size() == 2) {
        depth += 1;
    } else if (bb->get_succ_basic_blocks().size() == 0) {
        return nullptr;
    }
    BasicBlock *exit = nullptr;
    for (auto nextbb : bb->get_succ_basic_blocks()) {
        exit = traverseBranch(nextbb, depth);
        // if (exit != nullptr) {
        //     return exit;
        // }
    }
    return exit;
}
/*
 *  写给队友的一些话：
 *  这个函数用来将所有的常数运算及其所有的表现形式都做一个替换，以便删除
 *  目前的思路是：遍历所有指令->判断是否为运算指令（或icmp-fcmp等）->判断operands是否为常数
 *  ->若是：将其入栈(wait_delete)，并且对每个左值进行替换
 *  ->若不是：continue
 *  ->结束后将栈里的冗余东西都删去（也可以另外再来个函数，不过感觉不方便）
 * 
 * 注意浮点数整数，用
*/
void ConstFolder::replace_const(Function *f) {
    std::vector<Instruction *> wait_delete;
    for (auto bb : f->get_basic_blocks()) {
        std::unordered_map<Value *, Value *> vGlobal;
        std::map<Value *, int *> iGlobal;
        std::map<Value *, float *> fGlobal;
        for (auto instr : bb->get_instructions()) {
            //如果这条指令是Binary即运算指令
            //需要用到get_operand 具体参考LightIR文档
            bool allConst = false;
            if (instr->isBinary()) {
                //TODO
                Value *oper0, *oper1;
                auto opID = instr->get_instr_type();
                oper0 = instr->get_operand(0);
                oper1 = instr->get_operand(1);
                if (oper0->get_type()->is_float_type()) {
                    auto constFP0 = cast_constantfp(oper0);
                    auto constFP1 = cast_constantfp(oper1);
                    allConst = constFP0 != nullptr && constFP1 != nullptr;
                    if (allConst) {
                        auto ans = compute(opID, constFP0, constFP1);
                        instr->replace_all_use_with(ans);
                    }
                    LOG_DEBUG << "instr " << instr->get_name() << "is float " << (allConst ? "const" : "");
                } else if (oper0->get_type()->is_integer_type()) {
                    auto constInt0 = cast_constantint(oper0);
                    auto constInt1 = cast_constantint(oper1);
                    allConst = constInt0 != nullptr && constInt1 != nullptr;
                    if (allConst) {
                        auto ans = compute(opID, constInt0, constInt1);
                        instr->replace_all_use_with(ans);
                    }
                    LOG_DEBUG << "instr " << instr->get_name() << "is int " << (allConst ? "const" : "");
                }
            }
            if (instr->is_fp2si()) {
                Value *oper;
                oper = instr->get_operand(0);
                auto constFP = cast_constantfp(oper);
                allConst = constFP != nullptr;
                if (allConst) {
                    auto ans = ConstantInt::get((int)(constFP->get_value()), module_);
                    LOG_DEBUG << "the fp2si " << instr->get_name() << " from " << constFP->get_value() << "to" << (int)(constFP->get_value());
                    instr->replace_all_use_with(ans);
                }
            }
            if (instr->is_si2fp()) {
                Value *oper;
                oper = instr->get_operand(0);
                auto constInt = cast_constantint(oper);
                allConst = constInt != nullptr;
                if (allConst) {
                    auto ans = ConstantFP::get((float)(constInt->get_value()), module_);
                    instr->replace_all_use_with(ans);
                }
            }
            if (instr->is_cmp()) {
                Value *oper0, *oper1;
                auto cmpInst = static_cast<CmpInst *>(instr);
                auto opID = cmpInst->get_cmp_op();
                oper0 = instr->get_operand(0);
                oper1 = instr->get_operand(1);
                auto constInt0 = cast_constantint(oper0);
                auto constInt1 = cast_constantint(oper1);
                allConst = constInt0 != nullptr && constInt1 != nullptr;
                if (allConst) {
                    auto ans = compare(opID, constInt0, constInt1);
                    instr->replace_all_use_with(ans);
                }
            }
            if (instr->is_fcmp()) {
                Value *oper0, *oper1;
                auto fcmpInst = static_cast<FCmpInst *>(instr);
                auto opID = fcmpInst->get_cmp_op();
                oper0 = instr->get_operand(0);
                oper1 = instr->get_operand(1);
                auto constInt0 = cast_constantfp(oper0);
                auto constInt1 = cast_constantfp(oper1);
                allConst = constInt0 != nullptr && constInt1 != nullptr;
                if (allConst) {
                    auto ans = compare(opID, constInt0, constInt1);
                    instr->replace_all_use_with(ans);
                }
            }
            if (instr->is_zext()) {
                Value *oper;
                oper = instr->get_operand(0);
                auto constInt = cast_constantint(oper);
                allConst = constInt != nullptr;
                if (allConst) {
                    int iValue = constInt->get_value();
                    auto ans = ConstantInt::get(iValue, module_);
                    instr->replace_all_use_with(ans);
                }
            }
            if (instr->is_store()){
                Value *rval, *lval;
                rval = instr->get_operand(0);
                lval = instr->get_operand(1);
                if (rval->get_type()->is_integer_type()) {
                    auto constInt = cast_constantint(rval);
                    if(constInt != nullptr){
                        vGlobal.insert({lval, rval});
                    }
                } else if (rval->get_type()->is_float_type()) {
                    auto constFP = cast_constantfp(rval);
                    if(constFP != nullptr){
                        vGlobal.insert({lval, rval});
                    }
                }
            }
            if(instr->is_load()){
                Value *lval;
                lval = instr->get_operand(0);
                if(vGlobal.find(lval)!=vGlobal.end()){
                    allConst = true;
                    auto value = vGlobal[lval];
                    instr->replace_all_use_with(value);
                }
            }
            if (allConst) {
                wait_delete.push_back(instr);
            }
        }
        //删除需要删除的instr
        for (auto instr : wait_delete) {
            bb->delete_instr(instr);
        }
    }
}

void ConstFolder::clear_redundancy(Function *f) {
    redundantBBs.clear();
    for (auto bb : f->get_basic_blocks()) {
        BasicBlock *nextBB = nullptr;
        std::vector<Instruction *> wait_delete;
        if (redundantBBs.find(bb) != redundantBBs.end()) {
            continue;
        }
        auto term = bb->get_terminator();
        if (term->is_br()) {
            auto brTerm = static_cast<BranchInst *>(term);
            if (brTerm->is_cond_br()) {
                auto flagValue = brTerm->get_operand(0);
                auto flag = cast_constantint(flagValue);
                if (flag != nullptr) {
                    BasicBlock *brBB, *uselessBB;
                    if (flag->get_value()) {
                        brBB = static_cast<BasicBlock *>(brTerm->get_operand(1));
                        uselessBB = static_cast<BasicBlock *>(brTerm->get_operand(2));
                    } else {
                        brBB = static_cast<BasicBlock *>(brTerm->get_operand(2));
                        uselessBB = static_cast<BasicBlock *>(brTerm->get_operand(1));
                    }
                    LOG_INFO << "want to delete " << uselessBB->get_name();
                    nextBB = traverseBranch(uselessBB, 1);
                    bb->delete_instr(brTerm);
                    auto brInst = BranchInst::create_br(brBB, bb);
                }
            }
        }
        if (nextBB != nullptr) {
            // LOG_DEBUG << "branch next bb is " << nextBB->get_name();
            for (auto instr : nextBB->get_instructions()) {
                if (instr->is_phi() && instr->get_num_operand() == 4) {
                    auto oper0 = instr->get_operand(0);
                    auto label0 = instr->get_operand(1);
                    auto oper1 = instr->get_operand(2);
                    auto label1 = instr->get_operand(3);
                    auto bb0 = static_cast<BasicBlock *>(label0);
                    auto bb1 = static_cast<BasicBlock *>(label1);
                    if (redundantBBs.find(bb0) != redundantBBs.end() && redundantBBs.find(bb1) == redundantBBs.end()) {
                        auto ans = oper1;
                        wait_delete.push_back(instr);
                        instr->replace_all_use_with(oper1);
                    }
                    if (redundantBBs.find(bb1) != redundantBBs.end() && redundantBBs.find(bb0) == redundantBBs.end()) {
                        auto ans = oper0;
                        wait_delete.push_back(instr);
                        instr->replace_all_use_with(oper0);
                    }
                }
            }
        }
        for (auto instr : wait_delete) {
            nextBB->delete_instr(instr);
        }
    }
    for (auto bb : redundantBBs) {
        f->remove(bb);
    }
}

/*
 * 1. 使用replace_const先把常量进行替换
 * 然后删去被替换的多余变量
 * 
 * 2. 把必然跳转的if/else只保留一个，即删去bb-label和跳转以及多余的bb？
 *  或者把它变成br而不是cond_br
 *  2.1 个人理解的方法：碰到了cond_br就看operand是否是常数
 * （假设之前的操作已经化简了常数的比较并简化为常数结果）
 *  然后把不前往的bb删去，并把cond_br变为br
 *  
 */

void ConstPropagation::run() {
    // 从这里开始吧！
    //try just propagate compute!
    for (auto f : m_->get_functions()) {
        ConstFolder CF(m_);
        CF.replace_const(f);
        CF.clear_redundancy(f);
    }
}