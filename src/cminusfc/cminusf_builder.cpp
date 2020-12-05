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
Value *returnValue;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node) {
    LOG(DEBUG) << "program!";
    for (auto decl : node.declarations) {
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {
    LOG(DEBUG) << "num!";
    if (node.type == TYPE_INT) {
        scope.push("intValue",CONST_INT(node.i_val));
    } else {
        scope.push("floatValue",CONST_FP(node.f_val));
    }
    //ignore other situation
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    LOG(DEBUG) << "varDec!";
    bool arrayFlag = false;
    if (node.num != nullptr) {
        bool arrayFLag = true;
        LOG(DEBUG) << "TODO array check";
        node.num->accept(*this);
        int arrayLength = node.num->i_val;
    }
    Type *varTy;
    if (node.type == TYPE_INT){
        varTy = Type::get_int32_type(module.get());
    } else {
        varTy = Type::get_float_type(module.get());
    }
    auto varAlloca = builder->create_alloca(varTy);
    scope.push(node.id, varAlloca);
    LOG(DEBUG) << "is here finished?";
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
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
        param->accept(*this);
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
        // TODO 这里还要考虑是个pointer
    }
    auto funType = FunctionType::get(funTy, funParams);
    auto fun = Function::create(funType, node.id, module.get());
    scope.push(node.id, fun);
    auto bb = BasicBlock::create(module.get(), "", fun);
    builder->set_insert_point(bb);
    node.compound_stmt->accept(*this);
}

void CminusfBuilder::visit(ASTParam &node) {}

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
        node.expression->accept(*this);
        builder->create_ret(returnValue);
    }
}

void CminusfBuilder::visit(ASTVar &node) {}

void CminusfBuilder::visit(ASTAssignExpression &node) {}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    if(node.additive_expression_r ==nullptr){
        //TODO
    } else {
        //TODO relops
    }
    node.additive_expression_l->accept(*this);
    if (node.additive_expression_r != nullptr)
        node.additive_expression_r->accept(*this);
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    LOG(DEBUG) << "addi!";
    if (node.additive_expression == nullptr) {
        //把里面东西全加起来 TODO
    } else {
        //TODO
    }
    if (node.additive_expression != nullptr)
        node.additive_expression->accept(*this);
    node.term->accept(*this);
}

void CminusfBuilder::visit(ASTTerm &node) {
    LOG(DEBUG) << "term!";
    if (node.term == nullptr) {
        //TODO
    } else {
        //TODO
    }
    if (node.term != nullptr)
        node.term->accept(*this);
    node.factor->accept(*this);
}

void CminusfBuilder::visit(ASTCall &node) {
    LOG(DEBUG) << "call";
    std::vector<Value *> callParams;
    auto callFun = scope.find(node.id);
    Type *callFunTy = callFun->get_type();
    auto funTy = static_cast<FunctionType *> (callFunTy);
    //TODO complete int or float
    for (auto arg : node.args) {
        scope.enter();
        arg->accept(*this);
        callParams.push_back(scope.find("intValue"));
        scope.exit();
    }
    auto call = builder->create_call(callFun, callParams);
    scope.push(node.id, call);
}
