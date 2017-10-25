
#pragma once
#include <maya/MPxNode.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTypeId.h>
#include <jit.h>


//Class

class LLVMNode : public MPxNode
{
public:
		static void* creator();

 
		static MStatus initialize();
		MStatus compute(const MPlug& plug,MDataBlock& dataBlock) override;
		MStatus setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) override;
public:
	static MObject inputA;
	static MObject inputB;
	static MObject output;
	static MObject code;
	static MTypeId typeId;
	bool codeDirty = true;
	bool initialized = false;
	float(*customFunction)(float, float) = nullptr;
	
	babycpp::jit::BabycppJIT jit;
	babycpp::jit::BabycppJIT::ModuleHandle handle;
	bool isHandle = false;
};