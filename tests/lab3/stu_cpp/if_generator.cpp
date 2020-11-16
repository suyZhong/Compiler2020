/*
int main(){
  float a = 5.555;
  if(a > 1)
    return 233;
  return 0;
}
*/

#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
  auto module = new Module("fun_generator");  // module name是什么无关紧要
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);

  //main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  builder->set_insert_point(bb);
  
  /*float定义*/
  Type *FloatType = Type::get_float_type(module);
  auto aAlloca = builder->create_alloca(FloatType);//a的存放
  builder->create_store(CONST_FP(5.555), aAlloca);

  
  //if
  auto aLoad = builder->create_load(aAlloca);
  auto retAlloca = builder->create_alloca(Int32Type);   // 在内存中分配返回值的位置
  
  auto fcmp = builder->create_fcmp_lt(CONST_FP(1),aLoad);
  auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
  auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
  auto retBB = BasicBlock::create(module, "", mainFun);  // return分支,提前create,以便true分支可以br

  builder->create_cond_br(fcmp, trueBB, falseBB);
  
  //trueBB
  builder->set_insert_point(trueBB);
  builder->create_store(CONST_INT(233),retAlloca);
  builder->create_br(retBB);  // 注意在下一个BB之前要Br一下

  //falseBB
  builder->set_insert_point(falseBB);
  builder->create_store(CONST_INT(0),retAlloca);
  builder->create_br(retBB);

  //retBB
  builder->set_insert_point(retBB);  // ret分支
  auto retLoad = builder->create_load(retAlloca);
  builder->create_ret(retLoad);


  std::cout << module->print();
  delete module;
  return 0;


} 