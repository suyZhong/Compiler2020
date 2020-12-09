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
bool assignFlag;
Type *paramTy;

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
            funParams.push_back(Type::get_pointer_type(TyInt32));
        } else {
            switch (param->type) {
            case TYPE_VOID:
                funParams.push_back(TyVoid);
                break;
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
        paraNum++;
    }
    auto funType = FunctionType::get(funTy, funParams);
    auto fun = Function::create(funType, node.id, module.get());
    scope.push(node.id, fun);
    auto bb = BasicBlock::create(module.get(), "", fun);
    builder->set_insert_point(bb);
    //TODO add args
    //if not may segfault at call
    for (auto param : node.params) {
        param->accept(*this);
    }
    
    node.compound_stmt->accept(*this);
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
    LOG(DEBUG) << "alloca?";
    scope.push(node.id, builder->create_alloca(paramTy));
    LOG(DEBUG) << "asd?";
    //TODO array
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    LOG(DEBUG) << "compstmt!";
    scope.enter();
    //TODO 在这里要加个bb？
    for (auto decl : node.local_declarations) {
        decl->accept(*this);
    }
    for (auto stmt : node.statement_list) {
        stmt->accept(*this);
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    LOG(DEBUG) << "expression stmt!";
    if (node.expression != nullptr) {
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {}

void CminusfBuilder::visit(ASTIterationStmt &node) {}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    LOG(DEBUG) << "ret!";
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
    if (node.additive_expression_r == nullptr) {
        //TODO maybe
    } else {
        //TODO relops must
    }
    node.additive_expression_l->accept(*this);
    if (node.additive_expression_r != nullptr) {
        node.additive_expression_r->accept(*this);
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    Value *lhs;
    Value *rhs;
    bool opFlag = false;
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
        bool isIntOp = true;
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
    auto call = builder->create_call(callFun, callParams);
    scope.push(node.id, call);
}
