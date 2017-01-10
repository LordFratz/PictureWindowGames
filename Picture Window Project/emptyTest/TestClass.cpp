#include "TestClass.h"

int main() {
	FBXExporter::FBXExport test;
	test.FBXConvert("../Resources/Box_Idle.fbx","whatever");
	return 1;
}

TestClass::TestClass()
{
}


TestClass::~TestClass()
{
}
