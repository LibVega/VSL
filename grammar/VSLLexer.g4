///
/// Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
/// This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
/// the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
///

// This is the ANTLR4 lexer grammar for the Vega Shader Language

lexer grammar VSLLexer;


// Booleans
BOOLEAN_LITERAL : 'true' | 'false' ;

// Keywords
KW_BIND   : 'bind' ;
KW_CONST  : 'const' ;
KW_FLAT   : 'flat' ;
KW_IN     : 'in' ;
KW_LOCAL  : 'local' ;
KW_OUT    : 'out' ;
KW_SHADER : 'shader' ;
KW_TYPE   : '@type' ;

// Number literals
INTEGER_LITERAL
    : '-'? DecimalLiteral [uU]?
    | HexLiteral
    ;
FLOAT_LITERAL
    : '-'? DecimalLiteral ExponentPart
    | '-'? DecimalLiteral '.' DecimalLiteral? ExponentPart?
    ;
fragment DecimalLiteral : DigitChar+ ;
fragment HexLiteral     : '0x' HexDigitChar+ ;
fragment ExponentPart   : [eE] ('-'|'+')? DigitChar+ ;

// Identifiers, valid for variable and type names, and built-ins
IDENTIFIER
    : (AlphaChar|'_') (AlnumChar|'_')*
    | '$' AlphaChar+
    ;

// Punctuation
ATSIGN    : '@' ;
COLON     : ':' ;
COMMA     : ',' ;
LBRACE    : '{' ;
LBRACKET  : '[' ;
LPAREN    : '(' ;
PERIOD    : '.' ;
QMARK     : '?' ;
RBRACE    : '}' ;
RBRACKET  : ']' ;
RPAREN    : ')' ;
SEMICOLON : ';' ;

// Operators
OP_ADD      : '+' ;
OP_ASSIGN   : '=';
OP_ASN_ADD  : '+=' ;
OP_ASN_BAND : '&=' ;
OP_ASN_BOR  : '|=' ;
OP_ASN_BXOR : '^=' ;
OP_ASN_DIV  : '/=' ;
OP_ASN_LSH  : '<<=' ;
OP_ASN_MOD  : '%=' ;
OP_ASN_MUL  : '*=' ;
OP_ASN_RSH  : '>>=' ;
OP_ASN_SUB  : '-=' ;
OP_BAND     : '&' ;
OP_BNOT     : '~' ;
OP_BOR      : '|' ;
OP_BXOR     : '^' ;
OP_DIV      : '/' ;
OP_EQUAL    : '==' ;
OP_GEQUAL   : '>=' ;
OP_GREATER  : '>' ;
OP_LAND     : '&&' ;
OP_LEQUAL   : '<=' ;
OP_LESS     : '<' ;
OP_LNOT     : '!' ;
OP_LOR      : '||' ;
OP_LSHIFT   : '<<' ;
OP_MOD      : '%' ;
OP_MUL      : '*' ;
OP_NEQUAL   : '!=' ;
OP_RSHIFT   : '>>' ;
OP_SUB      : '-' ;

// Whitespace and comments (ignore to hidden channel)
WS
    : [ \t\r\n\u000C]+ -> channel(HIDDEN)
    ;
COMMENT
    : '/*' .*? '*/' -> channel(HIDDEN)
    ;
LINE_COMMENT
    : '//' ~[\r\n]* (('\r'? '\n') | EOF) -> channel(HIDDEN)
    ;

// Character types
fragment AlphaChar    : [a-zA-Z] ;
fragment DigitChar    : [0-9] ;
fragment AlnumChar    : AlphaChar | DigitChar ;
fragment HexDigitChar : [a-fA-F0-9] ;
