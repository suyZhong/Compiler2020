#include "ActiveVars.hpp"
#include "ConstPropagation.hpp"
#include "logging.hpp"
#include <unordered_set>

void ActiveVars::run() {
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        } else {
            func_ = func;

            func_->set_instr_name();
            live_in.clear();
            live_out.clear();

            // 在此分析 func_ 的每个bb块的活跃变量，并存储在 live_in live_out 结构内

            //1. init use and def
            LOG_DEBUG << "begin init";
            std::map<BasicBlock *, std::set<Value *>> defB, useB, phiUseB, phiOutB;
            std::set<Value *> argus;
            for (auto arg : func->get_args()) {
                argus.insert(arg);
            }
            for (auto bb : func->get_basic_blocks()) {
                std::set<Value *> tmpDef;
                // for (auto arg : func->get_args()) {
                //     defB[bb].insert(arg);
                //     tmpDef.insert(arg);
                // }
                for (auto instr : bb->get_instructions()) {
                    for (int i = 0; i < instr->get_num_operand(); i++) {
                        auto oper = instr->get_operand(i);
                        if (cast_constantfp(oper) != nullptr || cast_constantint(oper) != nullptr) {
                            continue;
                        }
                        if (tmpDef.find(oper) == tmpDef.end()) {
                            LOG_DEBUG << "find " << oper->get_name();
                            auto opType = oper->get_type();
                            if (!opType->is_function_type() && !opType->is_label_type()) {
                                useB[bb].insert(oper);
                                if (instr->is_phi()) {
                                    phiUseB[bb].insert(oper);
                                    auto oriBB = instr->get_operand(i + 1);
                                    for (auto preBB : bb->get_pre_basic_blocks()) {
                                        if (preBB->get_name() == oriBB->get_name()) {
                                            phiOutB[preBB].insert(oper);
                                        }
                                    }
                                    // phi_use_bb[oper] = instr->get_operand(i + 1);
                                } else {
                                    if (phiUseB[bb].find(oper) != phiUseB[bb].end()) {
                                        phiUseB[bb].erase(oper);
                                    }
                                }
                            }
                        }
                    }

                    if (instr->is_phi() || instr->isBinary() || instr->is_load() || instr->is_call() || instr->is_gep() || instr->is_alloca()) {
                        defB[bb].insert(instr);
                    }
                    tmpDef.insert(instr);
                }
                //2. for (iterate IN and OUT)
                // for (auto ppp = defB[bb].begin(); ppp != defB[bb].end(); ppp++) {
                //     std::cout << "defB " << bb->get_name() << (*ppp)->get_name() << std::endl;
                // }
                // for (auto ppp = useB[bb].begin(); ppp != useB[bb].end(); ppp++) {
                //     std::cout << "useB " << bb->get_name() << (*ppp)->get_name() << std::endl;
                // }
            }
            // no use emm
            // BasicBlock *exitBB;
            // std::unordered_set<BasicBlock *> supposeExitBBs;
            LOG_DEBUG << "begin iteration";
            bool change = true;
            int iteration = 0;
            while (change) {
                change = false;
                LOG_INFO << "it: " << iteration;
                for (auto bb : func->get_basic_blocks()) {
                    // LOG_DEBUG << "bb nums " << func->get_num_basic_blocks();
                    //计算OUT[B]
                    auto tmpIn = live_in[bb];
                    live_in[bb].clear();
                    live_out[bb].clear();
                    for (auto succ_bb : bb->get_succ_basic_blocks()) {
                        for (auto oper = live_in[succ_bb].begin(); oper != live_in[succ_bb].end(); oper++) {
                            //看看是不是在交叉边
                            if (phiUseB[succ_bb].find(*oper) != phiUseB[succ_bb].end() && phiOutB[bb].find(*oper) == phiOutB[bb].end()) {
                                continue;
                            }
                            live_out[bb].insert(*oper);
                        }
                    }
                    //计算IN[B]
                    live_in[bb].insert(useB[bb].begin(), useB[bb].end());
                    for (auto v = live_out[bb].begin(); v != live_out[bb].end(); v++) {
                        if (defB[bb].find(*v) == defB[bb].end()) {
                            live_in[bb].insert(*v);
                        }
                    }
                    for (auto v = live_in[bb].begin(); v != live_in[bb].end(); v++) {
                        if (tmpIn.find(*v) == tmpIn.end()) {
                            change = true;
                        }
                    }
                    for (auto v = tmpIn.begin(); v != tmpIn.end(); v++) {
                        if (live_in[bb].find(*v) == live_in[bb].end()) {
                            change = true;
                        }
                    }
                }
                iteration++;
            }

            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
    return;
}

std::string ActiveVars::print() {
    std::string active_vars;
    active_vars += "{\n";
    active_vars += "\"function\": \"";
    active_vars += func_->get_name();
    active_vars += "\",\n";

    active_vars += "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars += "  \"";
            active_vars += p.first->get_name();
            active_vars += "\": [";
            for (auto &v : p.second) {
                active_vars += "\"%";
                active_vars += v->get_name();
                active_vars += "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    },\n";

    active_vars += "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars += "  \"";
            active_vars += p.first->get_name();
            active_vars += "\": [";
            for (auto &v : p.second) {
                active_vars += "\"%";
                active_vars += v->get_name();
                active_vars += "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}