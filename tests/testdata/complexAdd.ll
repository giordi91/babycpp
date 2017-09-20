
define float @complexAdd(float %x, i32 %y) {
entry:
  %y2 = alloca i32
  %x1 = alloca float
  store float %x, float* %x1
  store i32 %y, i32* %y2
  %x3 = load float, float* %x1
  %y4 = load i32, i32* %y2
  %intToFPcast = uitofp i32 %y4 to float
  %addtmp = fadd float %x3, %intToFPcast
  ret float %addtmp
}
