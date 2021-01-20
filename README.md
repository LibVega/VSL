# Vega Shader Langauge

[![Build Status](https://travis-ci.com/LibVega/VSL.svg?branch=master)](https://travis-ci.com/LibVega/VSL)

The Vega Shader Language (VSL) is a custom GPU shading language for the [Vega](https://libvega.dev) library. 

Because Vega utilizes a unique binding model and reduced subset of normal GLSL and SPIR-V features, we've implemented a custom language that matched this specific use case more directly. The resulting syntax should be easy to learn for those already familiar with GLSL or HLSL. 

This repository contains the compiler library, which implements the parsing, compilation, and linking of the shaders, as well as the command line compiler tool. The VSL source is first cross-compiled into equivalent GLSL, and is then compiled with the ShaderC library.

A tutorial and spec guide is coming soon.

## Acknowledgements

Many thanks to the authors of the following libraries and tools used in VSL:

### Premake

* Website: [link](https://github.com/premake/premake-core)
* License: BSD 3-clause ([original](https://github.com/premake/premake-core/blob/master/LICENSE.txt)) ([copy](./license/premake))
* Description: Project generator allowing cross-platform and cross-compiler C++ projects.

### ANTLR4

* Website: [link](https://www.antlr.org/index.html)
* License: BSD 3-clause ([original](https://github.com/antlr/antlr4/blob/master/LICENSE.txt)) ([copy](./license/antlr))
* Description: Lexer/Parser generator used to perform the parsing of shader files

### shaderc

* Website: [link](https://github.com/google/shaderc)
* License: Apache v2 ([original](https://github.com/google/shaderc/blob/main/LICENSE)) ([copy](./license/shaderc))
* Description: Compiles the generated GLSL code into SPIR-V modules

These third-party tools and libraries are rehosted under their original licenses. The authors of the Vega library and VSL make no authorship claims.