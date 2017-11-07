
define i32 @testFunc(i32 %inv) {
entry:
  %res = alloca i32
  %inv1 = alloca i32
  store i32 %inv, i32* %inv1
  store i32 0, i32* %res
  %inv2 = load i32, i32* %inv1
  %ifcond = icmp ne i32 %inv2, 0
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  store i32 10, i32* %res
  br label %merge

else:                                             ; preds = %entry
  store i32 2, i32* %res
  br label %merge

merge:                                            ; preds = %else, %then
  %res3 = load i32, i32* %res
  ret i32 %res3
}
