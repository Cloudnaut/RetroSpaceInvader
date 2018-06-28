#include "FileSystem.hpp"

using namespace akilib::System;

//Definitionen der OS-Abhängigen Funktionen

#ifdef _WIN32 //Alle Windows abhängigen Funktionen

std::string FileSystem::GetExecFilePath()
{
	HMODULE hModule = GetModuleHandle(NULL);
	char FilePath[MAX_PATH];
	GetModuleFileName(hModule, FilePath, MAX_PATH);
	return FilePath;
}

#elif __unix__ //Alle UNIX abhängigen Funktionen

std::string FileSystem::GetExecFilePath()
{
}

#endif

//Allgemein gültige Funktionen

std::string FileSystem::GetExecDirectory()
{
	char Directory[MAX_PATH];
	strcpy_s(Directory, GetExecFilePath().c_str());

	char *pos = strrchr(Directory, '\\');
	if (pos != NULL)
	{
		*pos = '\0'; //this will put the null terminator here. you can also copy to another string if you want
	}

	strcat_s(Directory, "\\");

	return Directory;
}