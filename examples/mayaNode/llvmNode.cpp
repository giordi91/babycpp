#include "llvmNode.h"
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MGlobal.h>

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

  if (codeDirty) {

    std::cout << "code is dirty!!!!" << std::endl;

    babycpp::codegen::Codegenerator gen;

    MString mayaCode = dataBlock.inputValue(code).asString();
    std::string codeData{mayaCode.asChar()};
    if (codeData == "") {
      return MS::kSuccess;
    }

    gen.initFromString(codeData);
    auto p = gen.parser.parseFunction();
    if (p == nullptr) {
      auto error = gen.printDiagnostic();
      std::cout << error << std::endl;
      MGlobal::displayError(MString(error.c_str()));
      return MS::kSuccess;
    }
    // making sure the code is generated
    auto res = p->codegen(&gen);
    if (res == nullptr) {
      auto error = gen.printDiagnostic();
      std::cout << error << std::endl;
      MGlobal::displayError(MString(error.c_str()));
      return MS::kSuccess;
    }

    if (isHandle == true) {
      jit.removeModule(handle);
    }

    handle = jit.addModule(gen.module);
    auto symbol = jit.findSymbol("testFunc");
    customFunction =
        (float (*)(float, float))(intptr_t)llvm::cantFail(symbol.getAddress());
    if (customFunction == nullptr) {
      return MS::kSuccess;
    }

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
}

MStatus LLVMNode::setDependentsDirty(const MPlug &plug, MPlugArray &plugArray) {

  if (plug == code) {
    codeDirty = true;
  }
  return MS::kSuccess;
}
float testFunc(float a, float b) { return a + b; }
