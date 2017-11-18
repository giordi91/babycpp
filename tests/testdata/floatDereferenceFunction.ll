
define float @testFunc(float* %a) {
entry:
  %res = alloca float
  %a1 = alloca float*
  store float* %a, float** %a1
  %a2 = load float*, float** %a1
  %aDereferenced = load float, float* %a2
  store float %aDereferenced, float* %res
  %res3 = load float, float* %res
  ret float %res3
}
