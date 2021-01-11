#include "GlobalSubExprDeletion.hpp"
#include "logging.hpp"

// void GlobalSubExprDeletion::initGenKill(Module *m){
//     for (auto func: m->get_functions()){
//         for (auto bb:func->get_basic_blocks()){
//             for(auto instr: bb->get_instructions()){
//                 if(instr->isBinary()){
                    
//                 }
//             }
//         }
//     }
// }

void GlobalSubExprDeletion::run(){
    for (auto func: m_->get_functions()){
        std::vector<Instruction *> wait_delete;
        for (auto bb : func->get_basic_blocks()) {
            for(auto instr:bb->get_instructions()){
                if(instr->isBinary()){
                    auto v0 = instr->get_operand(0);
                    auto v1 = instr->get_operand(1);
                    if(commonExpr_.find({v0,v1}) == commonExpr_.end()){
                        commonExpr_[{v0, v1}] = instr;
                        commonExpr_[{v1, v0}] = instr;
                    } else {
                        auto ans = commonExpr_[{v0, v1}];
                        instr->replace_all_use_with(ans);
                        wait_delete.push_back(instr);
                    }
                }
                if(instr->is_phi()){
                    if(instr->get_num_operand() == 4){
                        if (instr->get_operand(0) == instr->get_operand(2)){
                            auto ans = instr->get_operand(0);
                            instr->replace_all_use_with(ans);
                            wait_delete.push_back(instr);
                        }
                    }
                }
            }
        }
        for (auto instr: wait_delete){
            auto bb = instr->get_parent();
            bb->delete_instr(instr);
        }
    }
}