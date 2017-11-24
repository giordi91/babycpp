
define i32* @testFunc(float* %b) {
entry:
  %res = alloca i32*
  %b1 = alloca float*
  store float* %b, float** %b1
  %b2 = load float*, float** %b1
  %pointerCast = bitcast float* %b2 to i32*
  store i32* %pointerCast, i32** %res
  %res3 = load i32*, i32** %res
  ret i32* %res3
}
