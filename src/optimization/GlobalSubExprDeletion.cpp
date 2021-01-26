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

/*
  * 大体想法是
  * 1. 静态单赋值格式下，任何一个变量都不会被再次赋值，意味着所有表达式都是可用的
  * 2. 那么单纯的把表达式丢进map里，每次有新的表达式看看有没有对应关系，有就替换
  * 3. 只是很简单的想法，没有考虑支配者关系，无法处理if/else。
  * 
*/
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