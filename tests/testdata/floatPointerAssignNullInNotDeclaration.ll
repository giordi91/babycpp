
define float* @testFunc(float* %b) {
entry:
  %res = alloca float*
  %b1 = alloca float*
  store float* %b, float** %b1
  %b2 = load float*, float** %b1
  store float* %b2, float** %res
  store float* null, float** %res
  %res3 = load float*, float** %res
  ret float* %res3
}
