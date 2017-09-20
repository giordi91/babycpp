
define float @test(float %x) {
entry:
  %x1 = alloca float
  store float %x, float* %x1
  %x2 = load float, float* %x1
  %addtmp = fadd float %x2, 1.000000e+00
  ret float %addtmp
}
