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

//scope has some unique strings

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
        LOG(DEBUG) << "TODO array check";
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
    varAlloca = builder->create_alloca(varTy);
    scope.push(node.id, varAlloca);
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
        builder->create_store(args[paraNum], scope.find(param->id));
        paraNum++;
    }

    node.compound_stmt->accept(*this);
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
    }
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    LOG(DEBUG) << "expression stmt!";
    if (node.expression != nullptr) {
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    bool isNext = false;
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
    builder->create_cond_br(cond, trueBB, falseBB);
    builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);
    if (!retFlag && node.else_statement !=nullptr)
    {
        nextBB = BasicBlock::create(module.get(), "", fun);
        builder->create_br(nextBB);
        isNext = true;
    } else if (node.else_statement == nullptr) {
        builder->create_br(falseBB);
    }
    retFlag = false;
        builder->set_insert_point(falseBB);
    if (node.else_statement != nullptr) {
        node.else_statement->accept(*this);
        if (!retFlag) {
            if (!isNext)
                nextBB = BasicBlock::create(module.get(), "", fun);
            builder->create_br(nextBB);
            isNext = true;
        }
    }
    retFlag = false;
    if (isNext)
        builder->set_insert_point(nextBB);
}

void CminusfBuilder::visit(ASTIterationStmt &node) {}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    LOG(DEBUG) << "ret!";
    retFlag = true;
    // auto fun = builder->get_insert_block()->get_parent();
    // auto retBb = BasicBlock::create(module.get(), "", fun);
    // builder->set_insert_point(retBb);
    if (node.expression == nullptr) {
        builder->create_void_ret();
    } else {
        scope.enter();
        node.expression->accept(*this);
        LOG(DEBUG) << "run here????";
        LOG(DEBUG) << scope.find("@")->get_type();
        builder->create_ret(scope.find("@"));
        scope.exit();
    }
}

void CminusfBuilder::visit(ASTVar &node) {
    Value *var;
    Value *varAlloca;
    varAlloca = scope.find(node.id);
    if (!varAlloca) {
        LOG(WARNING) << "there's no var";
    }
    if (!assignFlag) {
        var = builder->create_load(varAlloca);
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
    if (!varTy->is_float_type() && varValue->get_type()->is_float_type()) {
        LOG(DEBUG) << "ohmyzsss";
        varValue = builder->create_fptosi(varValue, varTy);
    } else if (!varTy->is_integer_type() && varValue->get_type()->is_integer_type()) {
        varValue = builder->create_sitofp(varValue, varTy);
    }

    builder->create_store(varValue, varAlloca);
}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    Value *lhs;
    Value *rhs;
    bool opFlag = false;
    bool isIntOp = true;
    LOG(DEBUG) << "simp expr";
    if (node.additive_expression_r == nullptr) {
        //TODO maybe
    } else {
        //TODO relops must
    }
    scope.enter();
    node.additive_expression_l->accept(*this);
    lhs = scope.find("@");
    scope.exit();
    if (node.additive_expression_r != nullptr) {
        scope.enter();
        node.additive_expression_r->accept(*this);
        rhs = scope.find("@");
        scope.exit();
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
    // if (node.additive_expression == nullptr) {
    //     //nothing TODO
    // } else {
    //     //TODO maybe
    //     opFlag = true;
    // }
    if (node.additive_expression != nullptr) {
        opFlag = true;
        scope.enter();
        node.additive_expression->accept(*this);
        //TODO type check
        // LOG(DEBUG) << "run here?";
        lhs = scope.find("@");
        scope.exit();
    }
    scope.enter();
    node.term->accept(*this);
    rhs = scope.find("@");
    scope.exit();

    //TODO fptosi
    if (opFlag) {
        isIntOp = true;
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
    LOG(DEBUG) << "term!";
    Value *lhs;
    Value *rhs;
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
        bool isIntOp = true;
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

    //这两行好像用不上
    // Type *callFunTy = callFun->get_type();
    // auto funTy = static_cast<FunctionType *>(callFunTy);
    //
    //TODO complete int or float
    for (auto arg : node.args) {
        scope.enter();
        arg->accept(*this);
        callParams.push_back(scope.find("@"));
        scope.exit();
    }
    LOG(DEBUG) << "setbreakpoint";
    auto call = builder->create_call(callFun, callParams);
    scope.push("@", call);
}
