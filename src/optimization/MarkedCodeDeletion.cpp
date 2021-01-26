#include "MarkedCodeDeletion.hpp"
#include "ConstPropagation.hpp"
#include "logging.hpp"

/* 
 * TODO: 
 * 1. 删除用不了的While循环
 * 2. 考虑一些store变量
 * 
 * 
 * 
 * 
 */

void MarkedCodeDeletion::deleteNoUseFunc(Module *m) {
    std::vector<Function *> wait_delete;
    for (auto func : m->get_functions()) {
        LOG_DEBUG << "try check & delete Dead Funcitnos " << func->get_name();
        if (func->get_basic_blocks().empty()) {
            continue;
        } else {
            if (func->get_use_list().empty() && func->get_name() != "main") {
                // deadFunc.insert(func);
                wait_delete.push_back(func);
            }
        }
    }
    for (auto func : wait_delete) {
        m->delete_function(func);
        // std::list<Function *>::iterator iter;
        // for (iter = funcs.begin(); iter != funcs.end(); iter++) {
        //     if (*iter = func) {
        //         LOG_DEBUG << "delete func" << func->get_name();
        //         break;
        //     }
        // }
        // funcs.erase(iter);
        // funcs.remove(func);
    }
    for (auto func : m->get_functions()) {
        LOG_DEBUG << func->get_name();
    }
}
void MarkedCodeDeletion::deleteNoUseInst(Module *m) {
    for (auto func : m->get_functions()) {
        func->set_instr_name();
        //加入所有的UseList为空的死代码
        for (auto bb : func->get_basic_blocks()) {
            std::vector<Instruction *> wait_delete;
            bb_ = bb;
            for (auto instr : bb->get_instructions()) {
                auto isVital = instr->isTerminator() || instr->is_call() || instr->is_store();
                if (!isVital && instr->get_use_list().empty()) {
                    wait_delete.push_back(instr);
                } else if (!instr->is_void())
                    deadInstr[bb].insert(instr);
            }
            //清除普通死代码
            for (auto instr : wait_delete) {
                bb->delete_instr(instr);
            }
        }
    }
}

void MarkedCodeDeletion::markSafeFunc(Module *m) {
    for (auto func : m->get_functions()) {
        if (func->get_name() == "main") {
            continue;
        }
        //判断这个是否会操作内存，调用函数。
        bool safe = true;
        for (auto bb : func->get_basic_blocks()) {
            for (auto instr : bb->get_instructions()) {
                if (instr->is_call() || instr->is_store()) {
                    safe = false;
                    break;
                }
            }
            if (!safe) {
                break;
            }
        }
        auto funcName = func->get_name();
        if (funcName == "output" || funcName == "outputFloat" || funcName == "neg_idx_except" || funcName == "input")
            safe = false;
        if (safe) {
            LOG_DEBUG << "insert safeFunc func " << func->get_name();
            safeFunc.insert(func);
        }
    }
}

bool MarkedCodeDeletion::isLive(Instruction *instr) {
    return liveInstr.find(instr) != liveInstr.end();
}

void MarkedCodeDeletion::findAllDep(Instruction *markedInstr) {
    std::vector<Value *> wait4check;
    liveInstr.insert(markedInstr);
    for (auto oper : markedInstr->get_operands()) {
        if (!oper->get_type()->is_label_type() && !oper->get_type()->is_function_type()) {
            if (args_.find(oper) == args_.end())
                wait4check.push_back(oper);
        }
    }
    while (!wait4check.empty()) {
        auto check = wait4check.back();
        wait4check.pop_back();
        LOG_DEBUG << "find " << check->get_name() << " dependency";
        // deadInstr[bb_].erase(check);
        liveInstr.insert(check);
        auto instr = static_cast<Instruction *>(check);
        for (auto oper : instr->get_operands()) {
            auto tmpInst = dynamic_cast<Instruction *>(oper);
            if (tmpInst)
                LOG_DEBUG << "op " << oper->get_name();
            if (!isLive(tmpInst) && !oper->get_type()->is_label_type() && !oper->get_type()->is_function_type()) {
                if (args_.find(oper) == args_.end())
                    wait4check.push_back(oper);
            }
        }
    }
}

void MarkedCodeDeletion::markUseInstr(Function *f) {
    for (auto arg : f->get_args()) {
        args_.insert(arg);
    }
    for (auto bb : f->get_basic_blocks()) {
        bb_ = bb;
        for (auto instr : bb->get_instructions()) {
            if (instr->is_call()) {
                auto callee = dynamic_cast<Function *>(instr->get_operand(0));
                LOG_DEBUG << "check func " << callee->get_name();
                if (callee) {
                    if (safeFunc.find(callee) != safeFunc.end()) {
                        LOG_DEBUG << callee->get_name() << " is safe!";
                        deadInstr[bb].insert(instr);
                        continue;
                    } else {
                        findAllDep(instr);
                    }
                }
            }
            if (instr->is_ret()) {
                if (instr->get_num_operand() != 0) {
                    if (args_.find(instr->get_operand(0)) == args_.end())
                        findAllDep(instr);
                }
            }
            if (instr->is_br()) {
                auto cbrInstr = static_cast<BranchInst *>(instr);
                if (cbrInstr->is_cond_br()) {

                    findAllDep(instr);
                }
            }
            if (instr->is_store()) {
                findAllDep(instr);
            }
        }
    }
}

void MarkedCodeDeletion::run() {
    //1. 不会被运行到的代码
    //删除那些非常没用的func
    deleteNoUseFunc(m_);
    //对每个函数都初始化
    deleteNoUseInst(m_);

    //2. 只会影响到无关程序运行结果的变量
    markSafeFunc(m_);
    //找到main函数，把output有关的，ret有关的弄出来
    for (auto func : m_->get_functions()) {
        func_ = func;
        args_.clear();
        markUseInstr(func);
    }
    for (auto func : m_->get_functions()) {
        func_ = func;
        for (auto bb : func->get_basic_blocks()) {
            for (auto instr : deadInstr[bb]) {
                auto dInstr = dynamic_cast<Instruction *>(instr);
                LOG_DEBUG << "delete instr " << dInstr->get_name();
                if (dInstr && liveInstr.find(dInstr) == liveInstr.end())
                    bb->delete_instr(dInstr);
            }
        }
    }

    return;
}