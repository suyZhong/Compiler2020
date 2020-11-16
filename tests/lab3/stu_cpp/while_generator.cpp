/*
int main(){
  int a;
  int i;
  a = 10;
  i = 0;
  while(i < 10){
    i = i + 1;
    a = a + i;
  }
  return a;
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

int main(){

  auto module = new Module("Cminus code");  // module name是什么无关紧要
  auto builder = new IRBuilder(nullptr, module);
  Type *Int32Type = Type::get_int32_type(module);

  //main函数
  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb);

  //a,i定义,赋值
  
  auto aAlloca = builder->create_alloca(Int32Type);//a的存放
  builder->create_store(CONST_INT(10), aAlloca);//a的赋值

  auto iAlloca = builder->create_alloca(Int32Type);//i的存放
  builder->create_store(CONST_INT(0), iAlloca);//i的赋值
  
  //while
  auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
  auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
  auto whileBB = BasicBlock::create(module, "", mainFun);
  builder->create_br(whileBB);

  //whileBB
  builder->set_insert_point(whileBB);
  auto iLoad = builder->create_load(iAlloca);
  auto icmp = builder->create_icmp_lt(iLoad,CONST_INT(10));
  builder->create_cond_br(icmp, trueBB, falseBB);

  //trueBB
  builder->set_insert_point(trueBB);
  //load i
  iLoad = builder->create_load(iAlloca);
  //i = i + 1
  auto addi = builder->create_iadd(iLoad, CONST_INT(1));
  //store i
  builder->create_store(addi,iAlloca);
  //load a
  auto aLoad = builder->create_load(aAlloca);
  //a = a + i
  auto adda = builder->create_iadd(aLoad, addi);
  //store a
  builder->create_store(adda,aAlloca);

  builder->create_br(whileBB); //无条件跳转
  
  //falseBB
  builder->set_insert_point(falseBB);
  aLoad = builder->create_load(aAlloca);
  builder->create_ret(aLoad);

  std::cout << module->print();
  delete module;
  return 0;

}