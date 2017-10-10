
define float @avg(float %x) {
entry:
  %x1 = alloca float
  store float %x, float* %x1
  %x2 = load float, float* %x1
  %multmp = fmul float %x2, 2.000000e+00
  ret float %multmp
}
