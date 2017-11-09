
define i32 @testFunc(i32 %a, i32 %b) {
entry:
  %res = alloca i32
  %b2 = alloca i32
  %a1 = alloca i32
  store i32 %a, i32* %a1
  store i32 %b, i32* %b2
  store i32 0, i32* %res
  %a3 = load i32, i32* %a1
  %b4 = load i32, i32* %b2
  %multmp = mul i32 3, %b4
  %subtmp = sub i32 %a3, %multmp
  %ifcond = icmp ne i32 %subtmp, 0
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  %a5 = load i32, i32* %a1
  %addtmp = add i32 %a5, 10
  store i32 %addtmp, i32* %res
  br label %merge

else:                                             ; preds = %entry
  %b6 = load i32, i32* %b2
  %subtmp7 = sub i32 2, %b6
  store i32 %subtmp7, i32* %res
  br label %merge

merge:                                            ; preds = %else, %then
  %res8 = load i32, i32* %res
  ret i32 %res8
}
