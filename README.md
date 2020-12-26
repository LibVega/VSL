# Polaris

[![Build Status](https://travis-ci.com/VegaLib/Polaris.svg?branch=master)](https://travis-ci.com/VegaLib/Polaris)

Polaris is a custom shader language for the [Vega](https://github.com/VegaLib) library. 

Because Vega utilizes a unique binding model and reduced subset of normal GLSL and SPIR-V features, it was decided to implement a custom language that matched this specific use case more directly. The resulting syntax should be familiar and easy to learn for those already versed in GLSL or HLSL.

This repository contains the compiler library, which implements the parsing, compilation, and linking of the shaders, as well as the command line compiler tool.

## Acknowledgements

Many thanks to the authors of the following libraries and tools used in Polaris:

### Premake

* Website: [link](https://github.com/premake/premake-core)
* License: BSD 3-clause ([original](https://github.com/premake/premake-core/blob/master/LICENSE.txt)) ([copy](./license/premake))
* Description: Project generator allowing cross-platform and cross-compiler C++ projects.

### ANTLR4

* Website: [link](https://www.antlr.org/index.html)
* License: BSD 3-clause ([original](https://github.com/antlr/antlr4/blob/master/LICENSE.txt)) ([copy](./license/antlr))
* Description: Lexer/Parser generator used to perform the parsing of Polaris shader files

These third-party tools and libraries are rehosted under their original licenses. The authors of Polaris and the Vega library make no authorship claims.