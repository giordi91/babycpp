
define i32* @testFunc() {
entry:
  %res = alloca i32*
  store i32* null, i32** %res
  %res1 = load i32*, i32** %res
  ret i32* %res1
}
