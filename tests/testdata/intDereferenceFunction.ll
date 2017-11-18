
define i32 @testFunc(i32* %a, i32* %b) {
entry:
  %bValue = alloca i32
  %aValue = alloca i32
  %res = alloca i32
  %b2 = alloca i32*
  %a1 = alloca i32*
  store i32* %a, i32** %a1
  store i32* %b, i32** %b2
  store i32 0, i32* %res
  %a3 = load i32*, i32** %a1
  %aDereferenced = load i32, i32* %a3
  store i32 %aDereferenced, i32* %aValue
  %b4 = load i32*, i32** %b2
  %bDereferenced = load i32, i32* %b4
  store i32 %bDereferenced, i32* %bValue
  %aValue5 = load i32, i32* %aValue
  %bValue6 = load i32, i32* %bValue
  %addtmp = add i32 %aValue5, %bValue6
  store i32 %addtmp, i32* %res
  %res7 = load i32, i32* %res
  ret i32 %res7
}
