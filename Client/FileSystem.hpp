#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>

#ifdef _WIN32
#include <Windows.h>
#elif  __unix__
//IncludePath
#endif

namespace akilib
{
	namespace System
	{

		class FileSystem
		{
		public:
			static std::string GetExecFilePath(); //Gibt den kompletten Programmpfad zurück
			static std::string GetExecDirectory(); //Gibt den Ordnerpfad in dem sich das Program befindet zurück
		};

	}
}


#endif //FILESYSTEM_HPP