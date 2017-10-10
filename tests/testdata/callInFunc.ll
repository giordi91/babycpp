
define float @test() {
entry:
  %calltmp = call float @avg(float 2.000000e+00)
  %addtmp = fadd float 2.000000e+00, %calltmp
  ret float %addtmp
}
