
define i32 @testFunc(i32 %a) {
entry:
  %i = alloca i32
  %x = alloca i32
  %a1 = alloca i32
  store i32 %a, i32* %a1
  store i32 0, i32* %x
  store i32 0, i32* %i
  %i2 = load i32, i32* %i
  %a3 = load i32, i32* %a1
  %cmptmp = icmp ult i32 %i2, %a3
  br i1 %cmptmp, label %loop, label %afterloop

loop:                                             ; preds = %loop, %entry
  %x4 = load i32, i32* %x
  %i5 = load i32, i32* %i
  %addtmp = add i32 %x4, %i5
  store i32 %addtmp, i32* %x
  %i6 = load i32, i32* %i
  %addtmp7 = add i32 %i6, 1
  store i32 %addtmp7, i32* %i
  %i8 = load i32, i32* %i
  %a9 = load i32, i32* %a1
  %cmptmp10 = icmp ult i32 %i8, %a9
  br i1 %cmptmp10, label %loop, label %afterloop

afterloop:                                        ; preds = %loop, %entry
  %x11 = load i32, i32* %x
  ret i32 %x11
}
