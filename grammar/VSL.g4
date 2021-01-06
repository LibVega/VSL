///
/// Microsoft Public License (Ms-PL) - Copyright (c) 2020-2021 Sean Moss
/// This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
/// the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
///

// This is the ANTLR4 lexer grammar for the Vega Shader Langauge

parser grammar VSL;
options {
    tokenVocab=VSLLexer;
}


/////
// Top-level file unit
file
    : shaderTypeStatement topLevelStatement* EOF
    ;

// Shader type statement
shaderTypeStatement
    : 'shader' type=IDENTIFIER ';'
    ;

// Shader top level statements
topLevelStatement
    : shaderStructDefinition
    | shaderInputOutputStatement
    | shaderConstantStatement
    | shaderUniformStatement
    | shaderBindingStatement
    | shaderLocalStatement
    | shaderStageFunction
    ;

// Shader struct statement, for defining new POD struct types
shaderStructDefinition
    : '@struct' name=IDENTIFIER '{' (variableDeclaration ';')+ '}' ';'
    ;

// Shader input or output declaration
shaderInputOutputStatement
    : io=('in'|'out') '(' index=INTEGER_LITERAL ')' variableDeclaration ';'
    ;

// Shader constant declaration
shaderConstantStatement
    : 'const' variableDeclaration '=' value=(INTEGER_LITERAL|FLOAT_LITERAL) ';'
    ;

// Shader uniform statement
shaderUniformStatement
    : 'uniform' variableDeclaration ';'
    ;

// Shader binding declaration
shaderBindingStatement
    : 'bind' '(' slot=INTEGER_LITERAL ')' variableDeclaration ';'
    ;

// Shader local statement
shaderLocalStatement
    : 'local' '(' pstage=IDENTIFIER ')' 'flat'? variableDeclaration ';'
    ;

// Shader stage function statement
shaderStageFunction
    : '@' stage=IDENTIFIER statementBlock
    ;


/////
// Statements
statement
    : variableDefinition ';'
    | variableDeclaration ';'
    | assignment ';'
    | ifStatement
    ;
statementBlock
    : '{' statement* '}'
    ;

// Variable declaration, for globals, type fields, and function locals
variableDeclaration
    : baseType=IDENTIFIER ('<' subType=IDENTIFIER '>')? name=IDENTIFIER ('[' arraySize=(INTEGER_LITERAL|IDENTIFIER) ']')?
    ;

// Variable definition (declaration with immediate assignment)
variableDefinition
    : decl=variableDeclaration '=' value=expression
    ;

// Variable assignment
assignment
    : lval=lvalue op=('='|'+='|'-='|'*='|'/='|'%='|'<<='|'>>='|'&='|'|='|'^=') value=expression
    ;

// Lvalue - anything that can occur as the destination of an assignment operation
lvalue
    : name=IDENTIFIER
    | val=lvalue '[' index=expression ']'
    | val=lvalue '.' IDENTIFIER
    ;

// If statement
ifStatement
    : 'if' '(' cond=expression ')' (statement|statementBlock) elifStatement* elseStatement?
    ;
elifStatement
    : 'elif' '(' cond=expression ')' (statement|statementBlock)
    ;
elseStatement
    : 'else' (statement|statementBlock)
    ;

/////
// Expressions
expression
    : atom  # AtomExpr
    // Unary Operators
    | op=('+'|'-') val=expression  # FactorExpr
    | op=('!'|'~') val=expression  # NegateExpr
    // Binary Operators
    | left=expression op=('*'|'/'|'%') right=expression        # MulDivModExpr
    | left=expression op=('+'|'-') right=expression            # AddSubExpr
    | left=expression op=('<<'|'>>') right=expression          # ShiftExpr
    | left=expression op=('<'|'>'|'<='|'>=') right=expression  # RelationalExpr
    | left=expression op=('=='|'!=') right=expression          # EqualityExpr
    | left=expression op=('&'|'|'|'^') right=expression        # BitwiseExpr
    | left=expression op=('&&'|'||') right=expression          # LogicalExpr
    // Ternary (Selector) Operator
    | cond=expression '?' texpr=expression ':' fexpr=expression  # TernaryExpr
    ;

// Atom - smallest indivisible expression type
atom
    : '(' expression ')'                                      # GroupAtom
    | atom '[' index=expression (',' index2=expression)? ']'  # IndexAtom
    | atom '.' IDENTIFIER                                     # MemberAtom
    | functionCall                                            # CallAtom
    | scalarLiteral                                           # LiteralAtom
    | IDENTIFIER                                              # NameAtom
    ;

// Function or constructor call
functionCall
    : name=IDENTIFIER '(' args+=expression (',' args+=expression )* ')'
    ;

// Scalar literal (number or bool)
scalarLiteral
    : INTEGER_LITERAL
    | FLOAT_LITERAL
    | BOOLEAN_LITERAL
    ;
