# babycpp
Custom implemented language which is basically a small, **SMALL** subset of c++, just another learning project.

## Grammar
The main struggle was to be able to generate a formal definition of the rules driving the parsing process, here
below the rules:

## Production rules
First of all some nomeclature:
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


# Static typing

The way I decided to handle static typing is with a bottom up approach. Each statement is evaluated indipendently and whatever reference might be needed in the current statement must be defined in any previous statement. 

Starting from the bottom, types will be compared between operation taking a left and right hand side, implicit conversion will be added when necessary and the type of the operator AST node will be tagged with the resulting type of the operation. Right now, the language supports only integers and floats which simplify life a lot.
Here is a simple example. We have a simple statement of type y* (x + 2), the AST looks something like the following:

##

![alt text](https://github.com/giordi91/babycpp/blob/master/images/graph1.png "Basic AST: step 1")

##
The nodes are color coded based on the type, each node has a type field, most of the AST after parsing is "data less", meaning that we don't know what type the variables are, the only known nodes are the VariableAST nodes that comes from a declaration, for example:
```c++
int x = 20;
```
The following will boil down to a VariableAST node, whith a value pointing to the right hand side, which is a NumberAST node or can be an expression, the datatype is known at parse time and gets set accordingly. Type is also know for NumberAST nodes and function arguments. 
