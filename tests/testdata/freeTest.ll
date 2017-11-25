
define void @freeWrap(float* %ptr) {
entry:
  %toFree = alloca i8*
  %ptr1 = alloca float*
  store float* %ptr, float** %ptr1
  %ptr2 = load float*, float** %ptr1
  %pointerCast = bitcast float* %ptr2 to i8*
  store i8* %pointerCast, i8** %toFree
  %toFree3 = load i8*, i8** %toFree
  call void @free(i8* %toFree3)
  ret void
}
