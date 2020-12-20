///
/// Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
/// This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
/// the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
///

// This is the ANTLR4 lexer grammar for the Polaris shading language

lexer grammar PLSLLexer;


// Keywords
KW_SHADER   : 'shader' ;

// Identifiers, valid for variable and type names, and built-ins
IDENTIFIER
    : (AlphaChar|'_') (AlnumChar|'_')*
    | '$' AlphaChar+
    ;

// Punctuation
SEMICOLON : ';' ;

// Whitespace and comments (ignore to hidden channel)
WS
    : [ \t\r\n\u000C]+ -> skip
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
fragment BinDigitChar : [01] ;
