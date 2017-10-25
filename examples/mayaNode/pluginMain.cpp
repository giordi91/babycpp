#include "llvmNode.h"

#include <maya/MFnPlugin.h>


MStatus initializePlugin( MObject obj )
{ 
	MStatus stat;

	MFnPlugin fnPlugin( obj, "Marco Giordano", "1.0", "Any");


	stat = fnPlugin.registerNode( "LLVMNode",
								  LLVMNode::typeId,
								  &LLVMNode::creator,
								  &LLVMNode::initialize
								  );

	

	if( stat != MS::kSuccess )
		stat.perror( "could not register the faceRigNode node" );


	return stat;
}


MStatus uninitializePlugin(MObject object)

{
	MFnPlugin pluginFn;
	pluginFn.deregisterNode(MTypeId(0x80011));

	return MS::kSuccess; 

}