#pragma once

#define EXPORT __declspec(dllexport)

extern "C" {
	namespace whatever {
		EXPORT void loadFile(const char* filename);
	}
};