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
    : 'bind' '(' slot=BINDING_SLOT ')' variableDeclaration ';'
    ;

// Shader local statement
shaderLocalStatement
    : 'local' '(' pstage=IDENTIFIER ',' cstage=IDENTIFIER ')' 'flat'? variableDeclaration ';'
    ;

// Shader stage function statement
shaderStageFunction
    : '@' stage=IDENTIFIER '{' '}'
    ;

// Variable declaration, for globals, type fields, and function locals
variableDeclaration
    : type=typeName name=IDENTIFIER ('[' arraySize=(INTEGER_LITERAL|IDENTIFIER) ']')?
    ;

// A type name
typeName
    : basetype=IDENTIFIER ('<' subtype=IDENTIFIER '>')?
    ;
