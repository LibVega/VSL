///
/// Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
/// This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
/// the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
///

// This is the ANTLR4 lexer grammar for the Polaris shading language

parser grammar PLSL;
options {
    tokenVocab=PLSLLexer;
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
    : shaderUserTypeDefinition
    | shaderInputOutputStatement
    | shaderConstantStatement
    | shaderBindingStatement
    | shaderLocalStatement
    | shaderStageFunction
    ;

// Shader type statement, for defining new POD types
shaderUserTypeDefinition
    : '@type' name=IDENTIFIER '{' (variableDeclaration ';')+ '}' ';'
    ;

// Shader input or output declaration
shaderInputOutputStatement
    : io=('in'|'out') '(' index=INTEGER_LITERAL ')' variableDeclaration ';'
    ;

// Shader constant declaration
shaderConstantStatement
    : 'const' variableDeclaration '=' value=(INTEGER_LITERAL|FLOAT_LITERAL) ';'
    ;

// Shader binding declaration
shaderBindingStatement
    : 'bind' '(' slot=INTEGER_LITERAL ')' variableDeclaration ';'
    ;

// Shader local statement
shaderLocalStatement
    : 'local' '(' pstage=IDENTIFIER ',' cstage=IDENTIFIER ')' 'flat'? variableDeclaration ';'
    ;

// Shader stage function statement
shaderStageFunction
    : '@' stage=IDENTIFIER '{' statement* '}'
    ;


/////
// Statements
statement
    : variableDefinition ';'
    | variableDeclaration ';'
    | assignment ';'
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
    | val=lvalue '.' SWIZZLE
    | val=lvalue '.' IDENTIFIER
    ;


/////
// Expressions
expression
    : atom  # AtomExpr
    // Unary Operators
    | val=lvalue op=('++'|'--')  # PostfixExpr
    | op=('++'|'--') val=lvalue  # PrefixExpr
    | op=('+'|'-') val=lvalue    # FactorExpr
    | op=('!'|'~') val=lvalue    # NegateExpr
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
    | atom '.' SWIZZLE                                        # SwizzleAtom
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
