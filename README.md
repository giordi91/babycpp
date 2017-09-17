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
