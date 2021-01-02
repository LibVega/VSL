@rem
@rem Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
@rem This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
@rem the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
@rem

@rem This is the Window batch script for generating the premake project.

@echo off

start /D "./tools" /W /B premake5.exe --file=..\vsl.project vs2019
