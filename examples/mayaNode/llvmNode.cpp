#include "llvmNode.h"
#include <maya/MFnNumericAttribute.h>

MTypeId LLVMNode::typeId(0x80011);

MObject LLVMNode::inputA;
MObject LLVMNode::inputB;
MObject LLVMNode::output;
MObject LLVMNode::code;

// Class

void *LLVMNode::creator() { return new LLVMNode(); }

MStatus LLVMNode::initialize() {
  MFnNumericAttribute numFn;
  inputA = numFn.create("inputA", "ina", MFnNumericData::kFloat);
  numFn.setKeyable(true);
  numFn.setStorable(true);
  numFn.setWritable(true);
  addAttribute(inputA);

  inputB = numFn.create("inputB", "inb", MFnNumericData::kFloat);
  numFn.setKeyable(true);
  numFn.setStorable(true);
  numFn.setWritable(true);
  addAttribute(inputB);

  MFnTypedAttribute typedFn;
  code = typedFn.create("code", "code", MFnData::kString);
  typedFn.setStorable(true);
  addAttribute(code);

  output = numFn.create("output", "out", MFnNumericData::kFloat);
  typedFn.setStorable(false);
  typedFn.setWritable(false);
  addAttribute(output);

  // This is the curve output attribute

  attributeAffects(inputA, output);
  attributeAffects(inputB, output);
  attributeAffects(code, output);

  return MS::kSuccess;
}
MStatus LLVMNode::compute(const MPlug &plug, MDataBlock &dataBlock) {

  if (!initialized) {
    // create the jit
  }
  if (codeDirty) {

    std::cout << "code is dirty!!!!" << std::endl;

    if (isHandle == true) {
      jit.removeModule(handle);
    }
    babycpp::codegen::Codegenerator gen;

    MString mayaCode = dataBlock.inputValue(code).asString();
    std::string codeData{mayaCode.asChar()};
    gen.initFromString(codeData);
    auto p = gen.parser.parseFunction();
    // making sure the code is generated
    p->codegen(&gen);

    handle = jit.addModule(gen.module);
    auto symbol = jit.findSymbol("testFunc");
    customFunction =
        (float (*)(float, float))(intptr_t)llvm::cantFail(symbol.getAddress());
    codeDirty = false;
    isHandle = true;
  }

  if (customFunction != nullptr) {
    float a = dataBlock.inputValue(inputA).asFloat();
    float b = dataBlock.inputValue(inputB).asFloat();
    dataBlock.outputValue(output).setFloat(customFunction(a, b));
  } else {
    dataBlock.outputValue(output).setFloat(1.0f);
  }

  dataBlock.outputValue(output).setClean();
  return MS::kSuccess;
  /*
          if (plug==output)
          {
                  



                  //MStatus
                  MStatus stat;


                  //Point array for the curve
                  MPointArray pointArray ;

                  //Get data from inputs
                  MDataHandle degreeH = dataBlock.inputValue(degree);
                  int degreeValue = degreeH.asInt();

                  MDataHandle tmH = dataBlock.inputValue(transformMatrix);
                  MMatrix tm = tmH.asMatrix();


                  MArrayDataHandle inputMatrixH =
     dataBlock.inputArrayValue(inputMatrix); inputMatrixH.jumpToArrayElement(0);
                  //Loop to get matrix data and convert in points

                  for (int unsigned
     i=0;i<inputMatrixH.elementCount();i++,inputMatrixH.next())
                  {
                          




                          MMatrix currentMatrix =
     inputMatrixH.inputValue(&stat).asMatrix() ;
                          



                          //Compensate the locator matrix
                          



                          MMatrix fixedMatrix = currentMatrix*tm.inverse();
                          MPoint matrixP
     (fixedMatrix[3][0],fixedMatrix[3][1],fixedMatrix[3][2]);
                          pointArray.append(matrixP);
                          



                  }
                  



          MFnNurbsCurve curveFn;
          MFnNurbsCurveData curveDataFn;
          MObject curveData= curveDataFn.create();

          curveFn.createWithEditPoints(pointArray,degreeValue,MFnNurbsCurve::kOpen,0,0,0,curveData,&stat);
          



          MDataHandle outputH = dataBlock.outputValue(output);
          outputH.set(curveData);
          outputH.setClean();

          }


          return MS::kSuccess;
          */
}

MStatus LLVMNode::setDependentsDirty(const MPlug &plug, MPlugArray &plugArray) {

  if (plug == code) {
    codeDirty = true;
  }
  return MS::kSuccess;
}
float testFunc(float a, float b) { return a + b; }
