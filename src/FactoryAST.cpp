#include "FactoryAST.h"


namespace babycpp {
    namespace memory {


        using allocVariableAST =

        codegen::VariableExprAST * FactoryAST::allocateASTNode<
            codegen::VariableExprAST, std::string &, codegen::ExprAST *, int>();
    } // namespace memory
} // namespace babycpp
