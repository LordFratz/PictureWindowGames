#include "IncludeMe.h"
#include "FBXExporter.h"

#define DoNotInclude

FBXExporter::FBXExport MyExporter;

void whatever::loadFile(const char * filename)
{
	MyExporter.FBXConvert(filename, "asdasd");
}