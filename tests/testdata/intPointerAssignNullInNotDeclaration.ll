
define i32* @testFunc(i32* %b) {
entry:
  %res = alloca i32*
  %b1 = alloca i32*
  store i32* %b, i32** %b1
  %b2 = load i32*, i32** %b1
  store i32* %b2, i32** %res
  store i32* null, i32** %res
  %res3 = load i32*, i32** %res
  ret i32* %res3
}
