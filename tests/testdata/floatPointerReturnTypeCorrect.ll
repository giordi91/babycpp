
define float* @testFunc(float* %a) {
entry:
  %res = alloca float*
  %a1 = alloca float*
  store float* %a, float** %a1
  %a2 = load float*, float** %a1
  store float* %a2, float** %res
  %res3 = load float*, float** %res
  ret float* %res3
}
