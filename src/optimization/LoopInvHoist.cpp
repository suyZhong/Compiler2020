#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

/*
 * 想到的某些东西
 * 1. 如果循环体有if/while判断（创建了新的bb），则不可以外提.?
 * 2. 只要在循环体内没有被重新定义过，就可以用
 * 
 * */

void LoopInvHoist::run()
{
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
    //怎么抽取，innerloop咋用还没想好 先睡吧
    for(auto f:m_->get_functions()){
        for(auto loop:loop_searcher.get_loops_in_func(f)){
            auto loopBase = loop_searcher.get_loop_base(loop);
        }
    }
}