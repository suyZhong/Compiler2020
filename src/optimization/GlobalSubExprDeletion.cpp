#include "GlobalSubExprDeletion.hpp"
#include "logging.hpp"

void GlobalSubExprDeletion::initGenKill(Module *m){
    for (auto func: m->get_functions()){
        for (auto bb:func->get_basic_blocks()){
            for(auto instr: bb->get_instructions()){
                if(instr->isBinary()){
                    
                }
            }
        }
    }
}