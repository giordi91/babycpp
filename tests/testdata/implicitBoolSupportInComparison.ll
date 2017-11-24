
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
  %cmptmp = icmp ult i32 %a3, %b4
  %ifcond = icmp ne i1 %cmptmp, false
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  store i32 1, i32* %res
  br label %merge

else:                                             ; preds = %entry
  store i32 2, i32* %res
  br label %merge

merge:                                            ; preds = %else, %then
  %res5 = load i32, i32* %res
  ret i32 %res5
}
