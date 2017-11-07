
define float @debug() {
entry:
  %x1 = alloca i32
  %x = alloca i32
  br i1 true, label %then, label %else

then:                                             ; preds = %entry
  store i32 2, i32* %x
  br label %merge

else:                                             ; preds = %entry
  store i32 4, i32* %x1
  br label %merge

merge:                                            ; preds = %else, %then
}
