
define float @complexAdd(float %x) {
entry:
  %temp = alloca float
  %x1 = alloca float
  store float %x, float* %x1
  %x2 = load float, float* %x1
  %multmp = fmul float %x2, 2.000000e+00
  store float %multmp, float* %temp
  %x3 = load float, float* %x1
  %subtmp = fsub float %x3, 2.000000e+00
  store float %subtmp, float* %temp
  %temp4 = load float, float* %temp
  ret float %temp4
}
