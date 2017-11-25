
define i32* @testFunc() {
entry:
  %ptr = alloca i32*
  %calltmp = call i8* @malloc(i32 20)
  %pointerCast = bitcast i8* %calltmp to i32*
  store i32* %pointerCast, i32** %ptr
  %ptr1 = load i32*, i32** %ptr
  ret i32* %ptr1
}
