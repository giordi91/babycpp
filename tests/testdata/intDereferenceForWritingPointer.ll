
define i32 @testFunc(i32* %a) {
entry:
  %x = alloca i32
  %a1 = alloca i32*
  store i32* %a, i32** %a1
  %aDereferenced = load i32*, i32** %a1
  store i32 10, i32* %aDereferenced
  store i32 0, i32* %x
  %x2 = load i32, i32* %x
  ret i32 %x2
}
