
define void @testFunc() {
entry:
  %ptr = alloca i32*
  %calltmp = call i8* @malloc(i32 4)
  %pointerCast = bitcast i8* %calltmp to i32*
  store i32* %pointerCast, i32** %ptr
  %ptrDereferenced = load i32*, i32** %ptr
  store i32 15, i32* %ptrDereferenced
  ret void
}
