
define float* @testFunc() {
entry:
  %res = alloca float*
  store float* null, float** %res
  %res1 = load float*, float** %res
  ret float* %res1
}
