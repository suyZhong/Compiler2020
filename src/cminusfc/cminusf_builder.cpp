#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())

// You can define global variables here
// to store state

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node) {
    // std::cout << "program" << std::endl;
    for (auto decl : node.declarations) {
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTNum &node) {}

void CminusfBuilder::visit(ASTVarDeclaration &node) {}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    // std::cout << "fundec" << std::endl;
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
    // std::cout << "comp" << std::endl;
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
    if (node.expression != nullptr) {
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {}

void CminusfBuilder::visit(ASTIterationStmt &node) {}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    // std::cout << "ret" << std::endl;
    builder->create_void_ret();
}

void CminusfBuilder::visit(ASTVar &node) {}

void CminusfBuilder::visit(ASTAssignExpression &node) {}

void CminusfBuilder::visit(ASTSimpleExpression &node) {}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    // std::cout << "add" << std::endl;
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
    // std::cout << "term" << std::endl;
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
    // std::cout << "call" << std::endl;
    std::vector<Value *> callParams;
    //TODO complete callpara
    for (auto arg : node.args) {
        arg->accept(*this);
    }
    auto calleeFun = scope.find(node.id);
    auto call = builder->create_call(calleeFun, callParams);
}
