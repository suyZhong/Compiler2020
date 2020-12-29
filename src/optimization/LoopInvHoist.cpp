#include "LoopInvHoist.hpp"
#include "LoopSearch.hpp"
#include "logging.hpp"
#include <algorithm>

/*
 * 想到的某些东西
 * 1. 如果循环体有if/while判断（创建了新的bb），则不可以外提.?
 * 2. 只要在循环体内没有被重新定义过，就可以用
 * 
 * */

bool isUseInLoop(BBset_t *loop, Value *v) {
    for (auto bb = loop->begin(); bb != loop->end(); bb++) {
    }
}

bool isInvHoist(Instruction *inst) {
}

void LoopInvHoist::run() {
    // 先通过LoopSearch获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();

    // 接下来由你来补充啦！

    //目前已知的情报：
    /**
     * loop_searcher.get_loop_base
     * get_inner_loop
     * get_parent_loop
     * get_loops_in_func
     * 
     * 
     */
    //首先根据已知情报应该抽取loop出来
    //就简单的想成，对每个loop做迭代吧，如果这个IHinst不在深层循环里面就给他外提
    /**
     * ！！！需要先对最深层循环开始！！！
     * 那就先弄一个集合存好吧。
     * 用比较垃圾的循环去做，maybe DFS。。？
     * 
     */

    //先弄一个只有大循环的集合
    std::unordered_set<BBset_t *> outLoops;
    std::unordered_set<BBset_t *> inLoops;
    for (auto loop = loop_searcher.begin(); loop != loop_searcher.end(); loop++) {
        if (loop_searcher.get_parent_loop(*loop) == nullptr) {
            outLoops.insert(*loop);
        } else {
            inLoops.insert(*loop);
        }
    }
    //用一个栈，大循环先入栈，小循环后入栈，然后不断出栈实现外移
    std::vector<BBset_t *> DFSStack;
    for (auto loop = outLoops.begin(); loop != outLoops.end(); loop++) {
        bool innerFlag = true;
        DFSStack.push_back(*loop);
        //循环查找，大循环底下可有小循环
        //因为不太了解unorderedset，感觉是无序的
        while (innerFlag) {
            innerFlag = false;
            for (auto iloop = inLoops.begin(); iloop != inLoops.end(); loop++) {
                if (loop_searcher.get_parent_loop(*iloop) == (*loop)) {
                    innerFlag = true;
                    DFSStack.push_back(*iloop);
                    inLoops.erase(iloop);
                }
            }
        }
    }

    while (!DFSStack.empty()) {
        auto loop = DFSStack.back();
        DFSStack.pop_back();
        //现在有了每个loop；对每个loop进行操作
        for (auto bb = loop->begin(); bb != loop->end(); bb++) {
            //现在有了bb；对instr进行操作
            for (auto instr : (*bb)->get_instructions()) {
                //获取operand 看是否被重新赋值，或者出现在判断条件里。
                //这个式子也不可以在深层循环内 （经过预处理 不用管了）

                //现在load和store只会是全局变量操作以及数组操作,直接排除
                if (instr->isTerminator() || instr->is_gep() || instr->is_load() || instr->is_store()) {
                    continue;
                }
                //是Binary指令：：：：
                int opNum = instr->get_num_operand();
                for (int i = 0; i < opNum;i++){
                    auto tmpV = instr->get_operand(i);
                    //开始判断这个tmpV在这个loop里是不是不变的
                    //貌似可以用Phi函数。。？貌似过循环会改变需要phi，所以只要判断在不在phi的左值！
                }
            }
        }
    }
}