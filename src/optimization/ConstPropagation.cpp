#include "ConstPropagation.hpp"
#include "logging.hpp"

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute(
    Instruction::OpID op,
    ConstantInt *value1,
    ConstantInt *value2)
{

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op)
    {
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

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value)
{
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr)
    {
        return constant_fp_ptr;
    }
    else
    {
        return nullptr;
    }
}
ConstantInt *cast_constantint(Value *value)
{
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr)
    {
        return constant_int_ptr;
    }
    else
    {
        return nullptr;
    }
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
void ConstFolder::replace_const(Value *value, Function *f){
    std::vector<Instruction *> wait_delete;
    for (auto bb : f->get_basic_blocks()) {
        for (auto instr : bb->get_instructions()){
            //如果这条指令是Binary即运算指令
            //需要用到get_operand 具体参考LightIR文档
            if(instr->isBinary()){
                //TODO
            }
        }
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

void ConstPropagation::run()
{
    // 从这里开始吧！
    //try just propagate compute!
    for (auto f:m_->get_functions()){
    }
}