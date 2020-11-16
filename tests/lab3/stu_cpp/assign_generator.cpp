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

  auto mainFun = Function::create(FunctionType::get(Int32Type, {}),
                                  "main", module);
  auto bb = BasicBlock::create(module, "entry", mainFun);
  // BasicBlock的名字在生成中无所谓,但是可以方便阅读
  builder->set_insert_point(bb);


  //全局数组a[10]
  auto *arrayType = ArrayType::get(Int32Type, 10);
  auto initializer = ConstantZero::get(Int32Type, module);
  auto a = GlobalVariable::create("a", module, arrayType, false, initializer);
  // 参数解释：  名字name，所属module，全局变量类型type，参照助教给出的注释给出代码

  auto a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});//获取a[0]地址
  builder->create_store(CONST_INT(10), a0GEP);
  auto a0 = builder->create_load(a0GEP);              // 从a[0]地址 读取a[0]

  auto a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});//获取a[1]地址
  auto mul = builder->create_imul(CONST_INT(2), a0);
  builder->create_store(mul, a1GEP);
  auto a1 = builder->create_load(a1GEP);              // 从a[1]地址 读取a[1]

  auto retAlloca = builder->create_alloca(Int32Type); 
  builder->create_store(a1, retAlloca);
  auto retLoad = builder->create_load(retAlloca); 
  builder->create_ret(retLoad);

  std::cout << module->print();
  delete module;
  return 0;

}