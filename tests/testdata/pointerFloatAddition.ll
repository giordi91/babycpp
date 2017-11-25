
define float @testFunc(float* %data, i32 %index) {
entry:
  %value = alloca float
  %newPtr = alloca float*
  %index2 = alloca i32
  %data1 = alloca float*
  store float* %data, float** %data1
  store i32 %index, i32* %index2
  %data3 = load float*, float** %data1
  %index4 = load i32, i32* %index2
  %pointerShift = getelementptr float, float* %data3, i32 %index4
  store float* %pointerShift, float** %newPtr
  %newPtr5 = load float*, float** %newPtr
  %newPtrDereferenced = load float, float* %newPtr5
  store float %newPtrDereferenced, float* %value
  %value6 = load float, float* %value
  ret float %value6
}
