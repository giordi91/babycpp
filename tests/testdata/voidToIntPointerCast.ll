
define i32* @testFunc(i8* %b) {
entry:
  %res = alloca i32*
  %b1 = alloca i8*
  store i8* %b, i8** %b1
  %b2 = load i8*, i8** %b1
  %pointerCast = bitcast i8* %b2 to i32*
  store i32* %pointerCast, i32** %res
  %res3 = load i32*, i32** %res
  ret i32* %res3
}
