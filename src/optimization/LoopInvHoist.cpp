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

BasicBlock *findNextExit(BasicBlock *bb, int depth) {
    if (depth == 1 && bb->get_pre_basic_blocks().size() == 2) {
        return bb;
    }
    if (bb->get_pre_basic_blocks().size() == 2) {
        depth -= 1;
    }
    if (bb->get_succ_basic_blocks().size() == 2) {
        depth += 1;
    } else if (bb->get_succ_basic_blocks().size() == 0){
        return nullptr;
    }
    for (auto nextbb : bb->get_succ_basic_blocks()) {
        BasicBlock *exit;
        exit = findNextExit(nextbb, depth);
        if (exit != nullptr) {
            return exit;
        }
    }
    return nullptr;
}

void LoopInvHoist::run() {
    // 先通过LoopSearch获取循环的相关信息
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();

    // 接下来由你来补充啦！
    LOG_DEBUG << "first-name";
    m_->set_print_name();
    int debug = 10;
    //首先根据已知情报应该抽取loop出来
    //就简单的想成，对每个loop做迭代吧，如果这个IHinst不在深层循环里面就给他外提

    //先弄一个只有大循环的集合
    LOG_DEBUG << "big-loops";

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
    LOG_DEBUG << "DFS stack";
    std::vector<BBset_t *> DFSStack;
    std::vector<BBset_t *> supportStack;
    int dfsDepth;
    LOG_DEBUG << "size of outLoops and inLoops" << outLoops.size() << inLoops.size();
    for (auto loop = outLoops.begin(); loop != outLoops.end(); loop++) {
        LOG_DEBUG << "try DFS inner loops";
        bool innerFlag = true;
        dfsDepth = 1;
        DFSStack.push_back(*loop);
        supportStack.push_back(*loop);

        while (dfsDepth) {
            innerFlag = false;
            for (auto tmp = inLoops.begin(); tmp != inLoops.end(); tmp++) {
                if (loop_searcher.get_parent_loop(*tmp) == supportStack.back()) {
                    // LOG_DEBUG << "how many times inserat? " << dfsDepth;
                    DFSStack.push_back(*tmp);
                    supportStack.push_back(*tmp);
                    dfsDepth += 1;
                    innerFlag = true;
                    break;
                }
            }
            if (!innerFlag) {
                dfsDepth -= 1;
                supportStack.pop_back();
            } else {
                inLoops.erase(supportStack.back());
            }
        }
    }
    LOG_INFO << "size of DFSStack is " << DFSStack.size();
    LOG_DEBUG << "begin calc loops";
    while (!DFSStack.empty()) {
        auto loop = DFSStack.back();
        DFSStack.pop_back();
        BasicBlock *base = loop_searcher.get_loop_base(loop);

        //先提取phi
        //若某指令的右值里有phi，也不行
        std::unordered_set<std::string> UnInvHoist;
        //定义一个flag去迭代判断是否该沿着UnInv的使用者顺藤摸瓜
        //get uselist 貌似是全局的，不可以这么做，老老实实写循环吧
        // bool flag = true;

        //TODO考虑if。？
        //循环整个loop，遍历所有instr，把被使用过的（phi）都加入
        LOG_DEBUG << "make uninvhoist";
        for (auto instr : base->get_instructions()) {
            if (instr->is_phi()) {
                LOG_INFO << "unInvHoist insert " << instr->get_name();
                UnInvHoist.insert(instr->get_name());
            }
        }
        for (auto bb = loop->begin(); bb != loop->end(); bb++) {
            //如果这个bb在内层循环里，那没必要去再试一次
            if (std::count(DFSStack.begin(), DFSStack.end(), (loop_searcher.get_inner_loop(*bb)))) {
                continue;
            }
            for (auto instr : (*bb)->get_instructions()) {
                if (instr->is_phi() || instr->is_fcmp() || instr->is_cmp()) {
                    LOG_INFO << "unInvHoist insert " << instr->get_name();
                    UnInvHoist.insert(instr->get_name());
                } else if (instr->isTerminator() || instr->is_gep() || instr->is_load() || instr->is_store()) {
                    continue;
                }
                //actually i'm not sure if these went right.
                int opNum = instr->get_num_operand();
                for (int i = 0; i < opNum; i++) {
                    auto tmpV = instr->get_operand(i);
                    if (UnInvHoist.find(tmpV->get_name()) != UnInvHoist.end()) {
                        UnInvHoist.insert(instr->get_name());
                        break;
                    }
                }
            }
        }

        //现在有了每个正规的loop；对每个loop进行操作

        std::vector<BasicBlock *> availBBs;
        availBBs.push_back(base);

        LOG_DEBUG << "make availBBs for check and motion";
        LOG_INFO << "base bb is " << base->get_name();
        debug = 0;
        //must ignore the if/else
        while (1) {
            std::list<BasicBlock *> succBBs;
            auto bb = availBBs.back();
            LOG_INFO << bb->get_name();
            succBBs = bb->get_succ_basic_blocks();
            LOG_INFO << "succs size is " << succBBs.size();
            BasicBlock *nextbb;
            if (loop->find(succBBs.front()) != loop->end()) {
                nextbb = succBBs.front();
            } else {
                nextbb = succBBs.back();
            }
            LOG_DEBUG << "nexbb is " << nextbb->get_name();
            int brDepth = 0;
            if (succBBs.size() != 1 && bb->get_pre_basic_blocks().size() == 1) {
                LOG_DEBUG << "ignore if/else";
                nextbb = findNextExit(bb, 0);
            } else if (succBBs.size() == 2 && bb->get_pre_basic_blocks().size() == 2) {
                for (BasicBlock *nextbb : bb->get_succ_basic_blocks()) {
                    if (std::count(DFSStack.begin(), DFSStack.end(), (loop_searcher.get_inner_loop(nextbb)))) {
                        LOG_DEBUG << "jump iterations";
                        break;
                    }
                }
            }
            LOG_DEBUG << "the " << debug++ << " bb";
            if (nextbb == base) {
                break;
            }
            if (availBBs.back() != nextbb)
                availBBs.push_back(nextbb);
        }
        LOG_DEBUG << "begin preparation";
        //应当前移的位置bb
        BasicBlock *bb4move;
        for (auto bb : base->get_pre_basic_blocks()) {
            if (loop->find(bb) == loop->end()) {
                bb4move = bb;
                break;
            }
        }
        auto termiInstr = bb4move->get_terminator();
        bb4move->delete_instr(termiInstr);
        LOG_DEBUG << "begin motion";

        LOG_INFO << "size of UnInvHoist: " << UnInvHoist.size();
        std::vector<Instruction *> wait_move;
        for (auto bb = availBBs.begin(); bb != availBBs.end(); bb++) {
            //现在有了bb；对instr进行操作
            LOG_INFO << "bb name: " << (*bb)->get_name();
            debug = 1;
            for (auto instr : (*bb)->get_instructions()) {
                //获取operand 看是否被重新赋值，或者出现在判断条件里。
                //这个式子也不可以在深层循环内 （经过预处理 不用管了）
                //现在load和store只会是全局变量操作以及数组操作,直接排除
                bool motionFlag = false;
                int i;
                if (instr->is_phi() || instr->isTerminator() || instr->is_gep() || instr->is_load() || instr->is_store()) {
                    continue;
                }
                //是Binary指令：：：：
                if (instr->isBinary()) {
                    motionFlag = true;
                    int opNum = instr->get_num_operand();
                    for (i = 0; i < opNum; i++) {
                        auto tmpV = instr->get_operand(i);
                        LOG_DEBUG << "instr" << instr->get_name() << " op " << i << " " << tmpV->get_name();
                        //开始判断这个tmpV在这个loop里是不是不变的
                        //貌似可以用Phi函数。。？貌似过循环会改变需要phi，所以只要判断在不在phi的左值！
                        //创建个set，把phi左值，phi左值的所有引用的inst都不可以用。
                        if (UnInvHoist.find(tmpV->get_name()) != UnInvHoist.end()) {
                            motionFlag = false;
                        }
                    }
                    //前移到bb4move
                    if (motionFlag) {
                        wait_move.push_back(instr);
                        bb4move->add_instruction(instr);
                        // (*bb)->delete_instr(instr);
                    }
                }
            }
            for(auto instr : wait_move){
                (*bb)->delete_instr(instr);
            }
        }
        bb4move->add_instruction(termiInstr);
    }
}