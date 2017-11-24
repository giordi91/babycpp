
define i32 @testFunc(i32 %a, i32 %b, i32 %c, i32 %k) {
entry:
  %x10 = alloca i32
  %x = alloca i32
  %res = alloca i32
  %k4 = alloca i32
  %c3 = alloca i32
  %b2 = alloca i32
  %a1 = alloca i32
  store i32 %a, i32* %a1
  store i32 %b, i32* %b2
  store i32 %c, i32* %c3
  store i32 %k, i32* %k4
  store i32 0, i32* %res
  %a5 = load i32, i32* %a1
  %multmp = mul i32 %a5, 2
  %ifcond = icmp ne i32 %multmp, 0
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  %k6 = load i32, i32* %k4
  %addtmp = add i32 %k6, 1
  store i32 %addtmp, i32* %x
  %a7 = load i32, i32* %a1
  %x8 = load i32, i32* %x
  %addtmp9 = add i32 %a7, %x8
  store i32 %addtmp9, i32* %res
  br label %merge

else:                                             ; preds = %entry
  %c11 = load i32, i32* %c3
  %subtmp = sub i32 %c11, 1
  store i32 %subtmp, i32* %x10
  %x12 = load i32, i32* %x10
  %b13 = load i32, i32* %b2
  %subtmp14 = sub i32 %x12, %b13
  store i32 %subtmp14, i32* %res
  br label %merge

merge:                                            ; preds = %else, %then
  %res15 = load i32, i32* %res
  ret i32 %res15
}
