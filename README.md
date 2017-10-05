# babycpp
Custom implemented language which is basically a small, **SMALL** subset of c++, just another learning project.

![alt text](https://github.com/giordi91/babycpp/blob/master/images/repl.gif "")

##### Table of Contents
* [How To] (#how-to)
* [Roadmap](#roadmap)  
* [Supported Compilers](#compilers)
* [Grammar](#grammar)
* [Production Rules](#production-rules)
* [Static typing](#static-typing)

## How to
At current state of development only the REPL is usable, although not tested. What you can do is pretty much what you see in the gif. Automatic type casting should be working although I suggest to first try with same datatypes, meaning all ints or all floats.
There are several options listed in the main CMakeLists.txt, you can disable test builds and other things. By default everything is set to ON, mainly for development easy of mind.

```cmake
option(BUILD_TESTS "Whether or not to build the tests" ON)
option(BUILD_JIT   "Whether or not to build the jit engine" ON)
option(BUILD_REPL  "Whether or not to build the interpreter" ON)
```

To compile:

####Linux
```bash
mkdir build
cd build
cmake -- . -DCMAKE_CXX_COMPILER=clang++
make -j
```

####Windows
```bash
mkdir build
cd build
cmake -- . -G"Visual Studio 15 2017 Win64"
```
The only major dependency as a library is LLVM, no extra tools/projects from the llvm family are needed. You can follow the instruction to compile LLVM from here:
https://llvm.org/docs/GettingStarted.html

## Roadmap

This project is still in development and in early stage. The current state of things is a basic REPL able to define and call functions. Although the main goal is not the REPL itself, which is a nice toy and useful for testing. The main goal is to get a compiler in the standard sense, which will allow me to compile to object file and link with a regular c++ generated object files. Here below the thing, I am going to focus next:

* Error handling: the current system has no error handling, just a null return and a std::cout, the plan is to put in place a basic system to handle warnings and errors with some context attached to it
* Compiler executable: get the actual compiler executable going using the core library written.
* Simple integration in Autodesk Maya: as a possible use case I want to integrate the compiler in a Maya node and use to jit on the fly and execute code in the Maya graph, really similar to what fabric engine is doing.

## Supported Compilers
The project is being cross developed on a Linux and Windows enviroment. 
The compiler used are the following:
* Ubuntu : Clang 6 from trunk / Some testing done in GCC 7.1
* Windows: Visual Studio 2017 15.3.3


## Grammar
The main struggle was to be able to generate a formal definition of the rules driving the parsing process, here
below the rules:

## Production Rules
First of all some nomenclature:
* Statement: by statement I mean a complete "sentence" a piece of code that makes sense and is valid from start to end
             in a single blob.
* Symbols:
  * () = just simple grouping
  * {} = repetition, at least one
  * [] = optionality
  * |  = or
  * "" = literarl ascii values

**statement** =  (extern|definition|expression) ";"

**extern** = "extern" function_protype

**definition** = datatype identifier ["=" expression] |
                 datatype function_definition

**function_definition** = function_prototype "{"{expressions;} "}"

**function_prototype** = datatype identifier "("[ {datatype identifier "," }] ")" 

**expression** =  primary [{operator expression}] | function_call

**primary** = number|identifier| parentheses_expression

**parentheses_expression** = "(" expression ")"

**function_call** = identifier "(" [{identifier} ","}] ")"

## Static typing

The way I decided to handle static typing is a bottom-up approach. Each statement is evaluated independently and whatever reference might be needed in the current statement must be defined in any previous statement. 

Starting from the bottom, types will be compared between operation taking a left and right-hand side, an implicit conversion will be added when necessary and the type of the operator AST node will be tagged with the resulting type of the operation. Right now, the language supports only integers and floats which simplify life a lot.
Here is a simple example. We have a simple statement of type y* (x + 2), the AST looks something like the following:
##

![alt text](https://github.com/giordi91/babycpp/blob/master/images/graph1.png "Basic AST: step 1")

##
The nodes are color-coded based on the type, each node has a type field, most of the AST after parsing is "data-less", meaning that we don't know what type the variables are, the only known nodes are the VariableAST nodes that come from a declaration, for example:
```c++
int x = 20;
```
The above will boil down to a VariableAST node, with a value pointing to the right-hand side, which is a NumberAST node or can be an expression, the datatype is known at parse time and gets set accordingly. Type is also known for NumberAST nodes and function arguments. 

The code generation starts from the top and works its way down recursively, arrived at the leaves it starts generating code and working its way up. In this case, the algorithm will go all the way down to the + binary node. 
It will call the code generation for both left and right-hand side. The right-hand side is simple, the type is known. 
The left-hand side has no known type and needs to look it up. If the variable is defined in the context, either from the previous definition or function argument, it will extract the type and set the AST node type to the corresponding one.

In this case, the variable x is of type float, both branches of the binary operator emitted code now is just a matter of performing the addition, before doing so, the code generation function of the binary operator will check that both types are 
equals, if not, it will perform the necessary casting, where the simple rule is, int gets cast to float.

##

![alt text](https://github.com/giordi91/babycpp/blob/master/images/graph2.png "Basic AST: step 2")

##

Now that both branches have the same type we know the resulting type of the operation, we emit the code and tag the binary operator node with the same type.

##

![alt text](https://github.com/giordi91/babycpp/blob/master/images/graph3.png "Basic AST: step 3")

##

At this point we start to see that the resulting datatypes start to "bubble up" the AST,  we move up one step, we have a product operation, we have a left-hand side of type int, and right hand side of type float, the binary operator will emit code for both branches and cast the int to a float:

##

![alt text](https://github.com/giordi91/babycpp/blob/master/images/graph4.png "Basic AST: step 4")

##

Finally we compute the result and set the resulting datatype:

##

![alt text](https://github.com/giordi91/babycpp/blob/master/images/graph5.png "Basic AST: step 5")

##

Here we evaluated correctly the statement. Finally in the case of a function, when we hit a return statement, the resulting type will have "bubbled up" until the return type, and we can easily perform a check against the known required return type and act accordingly.

