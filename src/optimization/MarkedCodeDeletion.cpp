#include "MarkedCodeDeletion.hpp"
#include "ConstPropagation.hpp"
#include "logging.hpp"

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
        m->get_functions().remove(func);
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
                } else if(!isVital)
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
        if (safe) {
            LOG_DEBUG << "insert safeFunc func " << func->get_name();
            safeFunc.insert(func);
        }
    }
}

bool MarkedCodeDeletion::isDead(Instruction *instr) {
    return deadInstr[bb_].find(instr) != deadInstr[bb_].end();
}

void MarkedCodeDeletion::findAllDep(Instruction *markedInstr) {
    std::vector<Value *> wait4check;
    deadInstr[bb_].erase(markedInstr);
    for (auto oper : markedInstr->get_operands()) {
        if (!oper->get_type()->is_label_type() && !oper->get_type()->is_function_type()) {
            wait4check.push_back(oper);
        }
    }
    while (!wait4check.empty()) {
        auto check = wait4check.back();
        wait4check.pop_back();
        deadInstr[bb_].erase(check);
        auto instr = static_cast<Instruction *>(check);
        for (auto oper : instr->get_operands()) {
            if (isDead(instr) && !oper->get_type()->is_label_type() && !oper->get_type()->is_function_type()) {
                wait4check.push_back(oper);
            }
        }
    }
}

void MarkedCodeDeletion::markUseInstr(Function *f) {
    for (auto bb : f->get_basic_blocks()) {
        for (auto instr : bb->get_instructions()) {
            if (instr->is_call()) {
                auto callee = dynamic_cast<Function *>(instr->get_operand(0));
                LOG_DEBUG << "check func " << callee->get_name();
                if (callee) {
                    findAllDep(instr);
                }
            }
            if (instr->is_ret()) {
                if (instr->get_num_operand() != 0) {
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
        if (func->get_name() == "main") {
            markUseInstr(func);
            break;
        }
    }
    for (auto func : m_->get_functions()) {
        for (auto bb : func->get_basic_blocks()) {
            for (auto instr : deadInstr[bb]) {
                auto dInstr = dynamic_cast<Instruction *>(instr);
                if (dInstr)
                    bb->delete_instr(dInstr);
            }
        }
    }

    return;
}