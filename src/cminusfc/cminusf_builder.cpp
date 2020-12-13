#include "cminusf_builder.hpp"
#include "logging.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())
#define CONST_INT(num) \
    ConstantInt::get(num, module.get())

// You can define global variables here
// to store state
Value *tmpValue;
float tmpFloat;
int tmpInt;
//定义assign flag使得在assign语句取数时，不对var进行load操作
bool assignFlag;
//传递param类型，（实际可能并不需要）
Type *paramTy;
//如果为真 代表在stmt中遇到了return语句，则ifelse不用前往nextBB
bool retFlag = false;

int brDepth = 0;
//scope has some unique strings

/*
    TODOs
    0.  I solve all the if/else problems but find llvm dont do the same
        !!!!!!!!!!!!maybe need to add retBB!!!!!!!!!
        !!!!!!!!!!!!TODO reconstruct this !!!!!!!!!
        nope, I just add brDepth

*/

/*
    Some Questions:
    1. void函数若有返回值，需要报错还是无视- 我无视了
    2. 
*/

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node) {
    LOG(DEBUG) << "program!";
    //去找各个declarations
    for (auto decl : node.declarations) {
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {
    LOG(DEBUG) << "num!";
    bool slag = true;
    if (node.type == TYPE_INT) {
        LOG(DEBUG) << "push";
        slag = scope.push("@", CONST_INT(node.i_val));
        if (!slag) {
            LOG(DEBUG) << "push fail!";
        }
        //TODO maybe set tmpFloat 0?
        tmpInt = node.i_val;
    } else {
        slag = scope.push("@", CONST_FP(node.f_val));
        tmpFloat = node.f_val;
        if (!slag) {
            LOG(DEBUG) << "push fail!";
        }
    }
    //ignore other situation
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    //这是变量声明；
    LOG(DEBUG) << "varDec!";
    auto TyInt32 = Type::get_int32_type(module.get());
    auto TyFloat = Type::get_float_type(module.get());
    Type *varTy;
    AllocaInst *varAlloca;
    if (node.num != nullptr) {
        bool arrayFLag = true;
        node.num->accept(*this);
        auto arrayLengthValue = scope.find("@");
        LOG(DEBUG) << arrayLengthValue;
        if (!arrayLengthValue->get_type()->is_integer_type()) {
            LOG(DEBUG) << "can rech here?";
            auto neg = scope.find("neg_idx_except");
            std::vector<Value *> nullParams;
            builder->create_call(neg, nullParams);
        } else {
            if (node.type == TYPE_INT) {
                varTy = ArrayType::get_array_type(TyInt32, tmpInt);
            } else {
                varTy = ArrayType::get_array_type(TyFloat, tmpInt);
            }
        }
    } else {
        if (node.type == TYPE_INT) {
            varTy = TyInt32;
        } else {
            varTy = TyFloat;
        }
    }
    LOG(DEBUG) << scope.in_global();
    if (scope.in_global()) {
        scope.push(node.id, GlobalVariable::create(node.id, module.get(), varTy, false, ConstantZero::get(varTy, module.get())));
    } else {
        varAlloca = builder->create_alloca(varTy);
        scope.push(node.id, varAlloca);
    }
    LOG(DEBUG) << "is here finished?";
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    //这是函数声明
    int paraNum = 0;
    LOG(DEBUG) << "funDec!";
    auto TyVoid = Type::get_void_type(module.get());
    auto TyInt32 = Type::get_int32_type(module.get());
    auto TyFloat = Type::get_float_type(module.get());
    Type *funTy;
    if (node.type == TYPE_INT) {
        funTy = TyInt32;
    } else if (node.type == TYPE_FLOAT) {
        funTy = TyFloat;
    }
    //假设不会出错吧
    else {
        funTy = TyVoid;
    }
    std::vector<Type *> funParams;
    for (auto param : node.params) {
        // param->accept(*this);
        // funParams.push_back(paramTy);
        //TODO array pointer
        if (param->isarray) {
            LOG(DEBUG) << param->id;
            if (param->type == TYPE_INT)
                funParams.push_back(Type::get_int32_ptr_type(module.get()));
            else {
                funParams.push_back(Type::get_float_ptr_type(module.get()));
            }
        } else {
            switch (param->type) {
            case TYPE_INT:
                funParams.push_back(TyInt32);
                break;
            case TYPE_FLOAT:
                funParams.push_back(TyFloat);
                break;
            default:
                break;
            }
        }
    }
    auto funType = FunctionType::get(funTy, funParams);
    auto fun = Function::create(funType, node.id, module.get());
    scope.push(node.id, fun);
    scope.enter();
    auto bb = BasicBlock::create(module.get(), "", fun);
    builder->set_insert_point(bb);
    //TODO add args
    //if not may segfault at call
    std::vector<Value *> args; // 获取函数的形参,通过Function中的iterator
    for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++) {
        args.push_back(*arg); // * 号运算符是从迭代器中取出迭代器当前指向的元素
    }
    for (auto param : node.params) {
        param->accept(*this);
        // if (!param->isarray)
        //     builder->create_store(args[paraNum], scope.find(param->id));
        // else {
        //     scope.push(param->id, args[paraNum]);
        // }
        builder->create_store(args[paraNum], scope.find(param->id));
        paraNum++;
    }
    brDepth = 1;
    node.compound_stmt->accept(*this);
    // negBB = BasicBlock::create(module.get(), "negBB", fun);
    // builder->set_insert_point(negBB);
    // builder->create_call(scope.find("neg_idx_except"), {});
    // builder->create_void_ret();
    scope.exit();
}

void CminusfBuilder::visit(ASTParam &node) {
    LOG(DEBUG) << "param";
    if (node.isarray) {
        if (node.type == TYPE_INT) {
            paramTy = Type::get_int32_ptr_type(module.get());
        } else if (node.type == TYPE_FLOAT)
            paramTy = Type::get_float_ptr_type(module.get());
    } else {
        if (node.type == TYPE_INT)
            paramTy = Type::get_int32_type(module.get());
        else if (node.type == TYPE_FLOAT)
            paramTy = Type::get_float_type(module.get());
    }
    scope.push(node.id, builder->create_alloca(paramTy));
    //TODO array
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    LOG(DEBUG) << "compstmt!";
    //把scope放到了funDeclar里面
    //TODO 在这里要加个bb？
    for (auto decl : node.local_declarations) {
        decl->accept(*this);
    }
    for (auto stmt : node.statement_list) {
        stmt->accept(*this);
        if(brDepth==0)
            break;
    }
    //reconTODO: add ret here
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    LOG(DEBUG) << "expression stmt!";
    if (node.expression != nullptr) {
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    LOG_DEBUG << "if!";
    bool isNext = false;
    retFlag = false;
    auto fun = builder->get_insert_block()->get_parent();
    auto trueBB = BasicBlock::create(module.get(), "", fun);
    auto falseBB = BasicBlock::create(module.get(), "", fun);
    BasicBlock *nextBB;
    scope.enter();
    node.expression->accept(*this);
    auto cond = scope.find("@");
    scope.exit();
    //TODO maybe the answer is not i1 ??
    //TODO this func is totally ugly
    //i want to make it elegant
    //type check
    auto condTy = cond->get_type();
    if (condTy->is_float_type()) {
        cond = builder->create_fcmp_gt(cond, ConstantZero::get(Type::get_float_type(module.get()), module.get()));
    } else if (condTy->is_integer_type()) {
        if (static_cast<IntegerType *>(condTy)->get_num_bits() != 1) {
            cond = builder->create_icmp_gt(cond, ConstantZero::get(Type::get_int32_type(module.get()), module.get()));
        }
    }
    builder->create_cond_br(cond, trueBB, falseBB);
    builder->set_insert_point(trueBB);
    LOG_INFO << brDepth;
    brDepth += 1;
    node.if_statement->accept(*this);
    if (!retFlag) {
        if (node.else_statement != nullptr) {
            nextBB = BasicBlock::create(module.get(), "", fun);
            builder->create_br(nextBB);
            isNext = true;
        } else {
            builder->create_br(falseBB);
            brDepth -= 1;
        }
    }
    builder->set_insert_point(falseBB);
    retFlag = false;
    if (node.else_statement != nullptr) {
        node.else_statement->accept(*this);
        if (!retFlag) {
            if (!isNext)
                nextBB = BasicBlock::create(module.get(), "", fun);
            builder->create_br(nextBB);
            isNext = true;
            brDepth -= 1;
        }
    }
    if (isNext) {
        retFlag = false;
        builder->set_insert_point(nextBB);
    }
}

void CminusfBuilder::visit(ASTIterationStmt &node) {
    LOG_DEBUG << "while!";
    retFlag = false;
    auto fun = builder->get_insert_block()->get_parent();
    auto flagBB = BasicBlock::create(module.get(), "", fun);
    auto bodyBB = BasicBlock::create(module.get(), "", fun);
    auto nextBB = BasicBlock::create(module.get(), "", fun);
    builder->create_br(flagBB);
    builder->set_insert_point(flagBB);
    brDepth += 1;
    scope.enter();
    node.expression->accept(*this);
    auto cond = scope.find("@");
    scope.exit();
    auto condTy = cond->get_type();
    if (condTy->is_float_type()) {
        cond = builder->create_fcmp_gt(cond, ConstantZero::get(Type::get_float_type(module.get()), module.get()));
    } else if (condTy->is_integer_type()) {
        if (static_cast<IntegerType *>(condTy)->get_num_bits() != 1) {
            cond = builder->create_icmp_gt(cond, ConstantZero::get(Type::get_int32_type(module.get()), module.get()));
        }
    }
    builder->create_cond_br(cond, bodyBB, nextBB);
    builder->set_insert_point(bodyBB);
    node.statement->accept(*this);
    if (!retFlag){
        brDepth -= 1;
        builder->create_br(flagBB);
    }
    builder->set_insert_point(nextBB);
    retFlag = false;
}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    LOG(DEBUG) << "ret!";
    retFlag = true;
    auto fun = builder->get_insert_block()->get_parent();
    auto funRetTy = fun->get_return_type();
    // auto retBb = BasicBlock::create(module.get(), "", fun);
    // builder->set_insert_point(retBb);
    if (node.expression == nullptr || funRetTy->is_void_type()) {
        builder->create_void_ret();
    } else {
        scope.enter();
        node.expression->accept(*this);
        //TODO type check
        auto retValue = scope.find("@");
        scope.exit();
        if (retValue->get_type()->is_integer_type()) {
            if (static_cast<IntegerType *>(retValue->get_type())->get_num_bits() == 1) {
                retValue = builder->create_zext(retValue, Type::get_int32_type(module.get()));
            }
        }
        if (funRetTy->is_float_type() && retValue->get_type()->is_integer_type()) {
            retValue = builder->create_sitofp(retValue, funRetTy);
        } else if (funRetTy->is_integer_type() && retValue->get_type()->is_float_type()) {
            retValue = builder->create_fptosi(retValue, funRetTy);
        }
        builder->create_ret(retValue);
    }
    brDepth -= 1;
}

void CminusfBuilder::visit(ASTVar &node) {
    bool isPPtr = false;
    Value *var;
    Value *varAlloca;
    Value *indexValue;
    bool assignIndexFlag = assignFlag;//对数组assign语句中若有var，作为记录
    varAlloca = scope.find(node.id);
    isPPtr = varAlloca->get_type()->get_pointer_element_type()->is_pointer_type();
    auto varAllocaTy = varAlloca->get_type();
    //add array read
    LOG_DEBUG << "var";
    if (node.expression != nullptr) {
        scope.enter();
        assignFlag = false;
        node.expression->accept(*this);
        assignFlag = assignIndexFlag;
        indexValue = scope.find("@");
        LOG_ERROR << indexValue->get_name();
        scope.exit();
        std::vector<Value *> idxs;
        //TODO opt
        if (indexValue->get_type()->is_integer_type()) {
            if (static_cast<IntegerType *>(indexValue->get_type())->get_num_bits() == 1)
                indexValue = builder->create_zext(indexValue, Type::get_int32_type(module.get()));
        }
        if (indexValue->get_type()->is_float_type()) {
            indexValue = builder->create_fptosi(indexValue, Type::get_int32_type(module.get()));
        }
        auto fun = builder->get_insert_block()->get_parent();
        auto negBB = BasicBlock::create(module.get(), "", fun);
        auto nextBB = BasicBlock::create(module.get(), "", fun);
        LOG_DEBUG << "wtf here";
        LOG_DEBUG << indexValue->get_name();
        auto negCond = builder->create_icmp_lt(indexValue, ConstantZero::get(Type::get_int32_type(module.get()), module.get()));
        LOG_DEBUG << indexValue->get_name();
        builder->create_cond_br(negCond, negBB, nextBB);
        builder->set_insert_point(negBB);
        builder->create_call(scope.find("neg_idx_except"), {});
        builder->create_br(nextBB);
        builder->set_insert_point(nextBB);
        if (varAllocaTy->get_pointer_element_type()->is_array_type()) {
            idxs.push_back(ConstantZero::get(Type::get_int32_type(module.get()), module.get()));
            idxs.push_back(indexValue);
        } else {
            idxs.push_back(indexValue);
        }
        if(isPPtr){
            varAlloca = builder->create_load(varAlloca);
        }
        varAlloca = builder->create_gep(varAlloca, idxs);
    }
    if (!varAlloca) {
        LOG(WARNING) << "there's no var";
    }
    if (!assignFlag) {
        if (varAllocaTy->get_pointer_element_type()->is_array_type() && node.expression == nullptr) {
            auto Czero = ConstantZero::get(Type::get_int32_type(module.get()), module.get());
            std::vector<Value *> idxs;
            idxs.push_back(Czero);
            idxs.push_back(Czero);
            var = builder->create_gep(varAlloca, idxs);
        } else {
            LOG_DEBUG << "is here?";
            var = builder->create_load(varAlloca);
        }
    }
    scope.push("&", varAlloca);
    scope.push("@", var);
    tmpValue = var;
    //TODO array
}

void CminusfBuilder::visit(ASTAssignExpression &node) {
    Value *varAlloca, *varValue;
    scope.enter();
    assignFlag = true;
    node.var->accept(*this);
    assignFlag = false;
    varAlloca = scope.find("&");
    scope.exit();
    auto varTy = varAlloca->get_type()->get_pointer_element_type();
    scope.enter();
    node.expression->accept(*this);
    varValue = scope.find("@");
    scope.exit();
    if (varValue->get_type()->is_integer_type()) {
        if (static_cast<IntegerType *>(varValue->get_type())->get_num_bits() == 1)
            varValue = builder->create_zext(varValue, Type::get_int32_type(module.get()));
    }
    if (!varTy->is_float_type() && varValue->get_type()->is_float_type()) {
        varValue = builder->create_fptosi(varValue, varTy);
    } else if (!varTy->is_integer_type() && varValue->get_type()->is_integer_type()) {
        varValue = builder->create_sitofp(varValue, varTy);
    }

    builder->create_store(varValue, varAlloca);
    scope.push("@", varValue);
}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    Value *lhs;
    Value *rhs;
    bool isIntOp = true;
    LOG(DEBUG) << "simp expr";
    if (node.additive_expression_r == nullptr) {
        //TODO maybe
    } else {
        //TODO relops
    }
    //TODO type transer
    scope.enter();
    node.additive_expression_l->accept(*this);
    lhs = scope.find("@");
    scope.exit();
    if (node.additive_expression_r != nullptr) {
        scope.enter();
        node.additive_expression_r->accept(*this);
        rhs = scope.find("@");
        scope.exit();
        auto lhsTy = lhs->get_type();
        auto rhsTy = rhs->get_type();
        if (lhsTy->is_integer_type()) {
            if (static_cast<IntegerType *>(lhsTy)->get_num_bits() == 1) {
                lhs = builder->create_zext(lhs, Type::get_int32_type(module.get()));
            }
        }
        if (rhsTy->is_integer_type()) {
            if (static_cast<IntegerType *>(rhsTy)->get_num_bits() == 1) {
                rhs = builder->create_zext(rhs, Type::get_int32_type(module.get()));
            }
        }
        if (lhsTy->is_float_type()) {
            //没有考虑是不是个别的东西（比如指针加）(不需要)
            isIntOp = false;
            if (rhsTy->is_integer_type()) {
                rhs = builder->create_sitofp(rhs, lhsTy);
            }
        } else if (rhsTy->is_float_type()) {
            isIntOp = false;
            if (lhsTy->is_integer_type()) { //实际上不需要判断这个
                lhs = builder->create_sitofp(lhs, rhsTy);
            }
        }
        if (isIntOp) {
            CmpInst *tmp;
            switch (node.op) {
            case OP_LE:
                tmp = builder->create_icmp_le(lhs, rhs);
                break;
            case OP_LT:
                tmp = builder->create_icmp_lt(lhs, rhs);
                break;
            case OP_GT:
                tmp = builder->create_icmp_gt(lhs, rhs);
                break;
            case OP_GE:
                tmp = builder->create_icmp_ge(lhs, rhs);
                break;
            case OP_EQ:
                tmp = builder->create_icmp_eq(lhs, rhs);
                break;
            case OP_NEQ:
                tmp = builder->create_icmp_ne(lhs, rhs);
                break;
            default:
                std::abort();
                break;
            }
            scope.push("@", tmp);
        } else {
            FCmpInst *tmp;
            switch (node.op) {
            case OP_LE:
                tmp = builder->create_fcmp_le(lhs, rhs);
                break;
            case OP_LT:
                tmp = builder->create_fcmp_lt(lhs, rhs);
                break;
            case OP_GT:
                tmp = builder->create_fcmp_gt(lhs, rhs);
                break;
            case OP_GE:
                tmp = builder->create_fcmp_ge(lhs, rhs);
                break;
            case OP_EQ:
                tmp = builder->create_fcmp_eq(lhs, rhs);
                break;
            case OP_NEQ:
                tmp = builder->create_fcmp_ne(lhs, rhs);
                break;
            default:
                std::abort();
                break;
            }
            scope.push("@", tmp);
        }
    } else {
        scope.push("@", lhs);
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    Value *lhs;
    Value *rhs;
    bool opFlag = false;
    bool isIntOp = true;
    LOG(DEBUG) << "addi!";
    if (node.additive_expression != nullptr) {
        opFlag = true;
        scope.enter();
        node.additive_expression->accept(*this);
        lhs = scope.find("@");
        scope.exit();
    }
    scope.enter();
    node.term->accept(*this);
    rhs = scope.find("@");
    scope.exit();

    //TODO zext
    if (opFlag) {
        auto lhsTy = lhs->get_type();
        auto rhsTy = rhs->get_type();
        if (lhsTy->is_integer_type()) {
            if (static_cast<IntegerType *>(lhsTy)->get_num_bits() == 1) {
                lhs = builder->create_zext(lhs, Type::get_int32_type(module.get()));
            }
        }
        if (rhsTy->is_integer_type()) {
            if (static_cast<IntegerType *>(rhsTy)->get_num_bits() == 1) {
                rhs = builder->create_zext(rhs, Type::get_int32_type(module.get()));
            }
        }
        if (lhsTy->is_float_type()) {
            //没有考虑是不是个别的东西（比如指针加）(不需要)
            isIntOp = false;
            if (rhsTy->is_integer_type()) {
                rhs = builder->create_sitofp(rhs, lhsTy);
            }
        } else if (rhsTy->is_float_type()) {
            isIntOp = false;
            if (lhsTy->is_integer_type()) { //实际上不需要判断这个
                lhs = builder->create_sitofp(lhs, rhsTy);
            }
        }
        BinaryInst *tmp;
        if (isIntOp) {
            if (node.op == OP_PLUS)
                tmp = builder->create_iadd(lhs, rhs);
            else if (node.op == OP_MINUS)
                tmp = builder->create_isub(lhs, rhs);
            else {
                std::abort();
            }
        } else {
            if (node.op == OP_PLUS)
                tmp = builder->create_fadd(lhs, rhs);
            else if (node.op == OP_MINUS)
                tmp = builder->create_fsub(lhs, rhs);
            else {
                std::abort();
            }
        }
        scope.push("@", tmp);
    } else {
        scope.push("@", rhs);
    }
}

void CminusfBuilder::visit(ASTTerm &node) {
    // LOG(DEBUG) << "term!";
    Value *lhs;
    Value *rhs;
    bool isIntOp = true;
    bool opFlag = false;
    // if (node.term == nullptr) {
    //     //TODO nothing
    // } else {
    //     opFlag = true;
    // }
    if (node.term != nullptr) {
        opFlag = true;
        scope.enter();
        node.term->accept(*this);
        lhs = scope.find("@");
        scope.exit();
    }
    scope.enter();
    node.factor->accept(*this);
    rhs = scope.find("@");
    scope.exit();

    //TODO type check
    if (opFlag) {
        auto lhsTy = lhs->get_type();
        auto rhsTy = rhs->get_type();
        if (lhsTy->is_integer_type()) {
            if (static_cast<IntegerType *>(lhsTy)->get_num_bits() == 1) {
                lhs = builder->create_zext(lhs, Type::get_int32_type(module.get()));
            }
        }
        if (rhsTy->is_integer_type()) {
            if (static_cast<IntegerType *>(rhsTy)->get_num_bits() == 1) {
                rhs = builder->create_zext(rhs, Type::get_int32_type(module.get()));
            }
        }
        if (lhsTy->is_float_type()) {
            //没有考虑是不是个别的东西（比如指针加）(不需要)
            isIntOp = false;
            if (rhsTy->is_integer_type()) {
                rhs = builder->create_sitofp(rhs, lhsTy);
            }
        } else if (rhsTy->is_float_type()) {
            isIntOp = false;
            if (lhsTy->is_integer_type()) { //实际上不需要判断这个
                lhs = builder->create_sitofp(lhs, rhsTy);
            }
        }
        BinaryInst *tmp;
        if (isIntOp) {
            if (node.op == OP_MUL)
                tmp = builder->create_imul(lhs, rhs);
            else if (node.op == OP_DIV)
                tmp = builder->create_isdiv(lhs, rhs);
            else {
                std::abort();
            }
        } else {
            if (node.op == OP_MUL)
                tmp = builder->create_fmul(lhs, rhs);
            else if (node.op == OP_DIV)
                tmp = builder->create_fdiv(lhs, rhs);
            else {
                std::abort();
            }
        }
        scope.push("@", tmp);
    } else {
        scope.push("@", rhs);
    }
}

void CminusfBuilder::visit(ASTCall &node) {
    LOG(DEBUG) << "call";
    std::vector<Value *> callParams;
    auto callFun = scope.find(node.id);
    int paramIndex = 0;
    //这两行好像用不上
    FunctionType *funTy;
    Type *callFunTy = callFun->get_type();
    if (callFunTy->is_function_type())
        funTy = static_cast<FunctionType *>(callFunTy);
    //TODO complete int or float
    for (auto arg : node.args) {
        scope.enter();
        arg->accept(*this);
        auto argValue = scope.find("@");
        Value *callValue = argValue;
        scope.exit();
        if (argValue->get_type()->is_integer_type()) {
            if (static_cast<IntegerType *>(argValue->get_type())->get_num_bits() == 1)
                callValue = builder->create_zext(argValue, Type::get_int32_type(module.get()));
        }
        auto argTy = funTy->get_param_type(paramIndex);
        if (argTy->is_float_type() && argValue->get_type()->is_integer_type()) {
            callValue = builder->create_sitofp(argValue, argTy);
        } else if (argTy->is_integer_type() && argValue->get_type()->is_float_type()) {
            callValue = builder->create_fptosi(argValue, argTy);
        } else if (argTy->is_pointer_type() && argValue->get_type()->is_array_type()) {
            // auto Czero = ConstantZero::get(Type::get_int32_type(module.get()), module.get());
            // std::vector<Value *> idxs;
            // idxs.push_back(Czero);
            // idxs.push_back(Czero);
            // callValue = builder->create_gep(argValue, idxs);
        }
        callParams.push_back(callValue);
        paramIndex++;
    }
    auto call = builder->create_call(callFun, callParams);
    scope.push("@", call);
}
