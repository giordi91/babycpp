
define i32 @testFunc(i32* %data, i32 %index) {
entry:
  %value = alloca i32
  %newPtr = alloca i32*
  %index2 = alloca i32
  %data1 = alloca i32*
  store i32* %data, i32** %data1
  store i32 %index, i32* %index2
  %data3 = load i32*, i32** %data1
  %index4 = load i32, i32* %index2
  %pointerShift = getelementptr i32, i32* %data3, i32 %index4
  store i32* %pointerShift, i32** %newPtr
  %newPtr5 = load i32*, i32** %newPtr
  %newPtrDereferenced = load i32, i32* %newPtr5
  store i32 %newPtrDereferenced, i32* %value
  %value6 = load i32, i32* %value
  ret i32 %value6
}
