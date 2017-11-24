
define float* @testFunc(i32* %b) {
entry:
  %res = alloca float*
  %b1 = alloca i32*
  store i32* %b, i32** %b1
  %b2 = load i32*, i32** %b1
  %pointerCast = bitcast i32* %b2 to float*
  store float* %pointerCast, float** %res
  %res3 = load float*, float** %res
  ret float* %res3
}
