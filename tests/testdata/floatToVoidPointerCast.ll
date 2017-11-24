
define i8* @testFunc(float* %b) {
entry:
  %res = alloca i8*
  %b1 = alloca float*
  store float* %b, float** %b1
  %b2 = load float*, float** %b1
  %pointerCast = bitcast float* %b2 to i8*
  store i8* %pointerCast, i8** %res
  %res3 = load i8*, i8** %res
  ret i8* %res3
}
