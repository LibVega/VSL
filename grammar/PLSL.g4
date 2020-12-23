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
    ;

// Shader type statement, for defining new POD types
shaderUserTypeDefinition
    : '@type' name=IDENTIFIER '{' (variableDeclaration ';')+ '}' ';'
    ;

// Shader input or output declaration
shaderInputOutputStatement
    : io=('in'|'out') '(' index=INTEGER_LITERAL ')' variableDeclaration ';'
    ;

// Variable declaration, for globals, type fields, and function locals
variableDeclaration
    : type=IDENTIFIER name=IDENTIFIER ('[' arraySize=INTEGER_LITERAL ']')?
    ;
