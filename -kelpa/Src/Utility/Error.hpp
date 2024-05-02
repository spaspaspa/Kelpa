/** 
 * 		@Path	Kelpa/Src/Utility/Error.hpp
 * 		@Brief	Provide error handling
 * 		@Dependency	None
 * 		@Since  2024/04/23
 * 		@Version 1st
 **/
 
#ifndef __KELPA_UTILITY_ERROR_HPP__
#define __KELPA_UTILITY_ERROR_HPP__
#include <cstring>								/* imports ./ { 
	std::strerror 
}*/
#include <windows.h>  
#include <DbgHelp.h>  							/* imports ./ { 
	WindowsAPIs... 
}*/
#include <iostream>  							/* imports ./ { 
	./stdio_s.h/ { sprintf_s }, 
	extern std::cout, std::cerr	
}*/
#include <format>								/* imports ./ { 
	std::format
}*/
#include <ranges>								/* imports ./ { 
	std::views::reverse
}*/
#include <vector>  								/* imports ./ { 
	std::vector 
}*/
#include <cerrno>								/* imports ./ { 
	#define errono 
}*/
#include <source_location>						/* imports ./ { 
	std::source_location::current() 
}*/


#define Assert__(__PREDICATE__, __DESCRIPTION__) if(!(__PREDICATE__)) [[unlikely]] { \
	std::cerr << std::format( "\nAssert\n"									\
	"predicate:\t\t{}				\n"										\
	"description:\t\t{}			\n"											\
	"current stack frame:\t\t{}	\n"											\
	"file:\t\t\t{}					\n"										\
	"function:\t\t{}				\n"										\
	"line:\t\t\t{}					\n"										\
	"system error:\t\t{}			\n",									\
	#__PREDICATE__,														\
	__DESCRIPTION__,													\
	__builtin_return_address(0),										\
	std::source_location::current().file_name(),						\
	std::source_location::current().function_name(),					\
	std::source_location::current().line(),								\
	std::strerror(errno));												\
	Kelpa::Utility::PrintCallStack();									\
	std::quick_exit(EXIT_FAILURE);										\
}

namespace Kelpa {
namespace Utility {
namespace Detail {
BOOL CALLBACK SymEnumSymbolsProc(const char *symbolName, ULONG64 symbolAddress, ULONG symbolSize, void *userData) {  
    std::vector<std::string>* symbols = reinterpret_cast<std::vector<std::string> *>(userData);  
    (* symbols).emplace_back(symbolName);  
    return TRUE;   
}  	
}	

void WSAError() {  
	CHAR 		Buffer[256] {};  
  
    if (FormatMessageA(  
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,  
        nullptr,  
        WSAGetLastError(),  
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  
        Buffer,  
        sizeof Buffer,  
        nullptr))   
        std::fprintf(stderr, "error: %s", Buffer);
    else   
        std::fprintf(stderr, "unknown error");
}    
void PrintCallStack() {  
    HANDLE 		process = 	GetCurrentProcess();  
    HANDLE 		thread = 	GetCurrentThread();  
    CONTEXT 	context;  
    ZeroMemory(&context, sizeof(CONTEXT));  
    context.ContextFlags = 	CONTEXT_FULL;  
    RtlCaptureContext(&context);  
  
    SymInitialize(process, NULL, TRUE);  
  
    STACKFRAME64 stackFrame = 		{};  
    stackFrame.AddrPC.Offset = 		context.Rip;  
    stackFrame.AddrPC.Mode = 		AddrModeFlat;  
    stackFrame.AddrStack.Offset = 	context.Rsp;  
    stackFrame.AddrStack.Mode = 	AddrModeFlat;  
    stackFrame.AddrFrame.Offset = 	context.Rbp;  
    stackFrame.AddrFrame.Mode = 	AddrModeFlat;  
  
    std::vector<std::string> symbols;  
    while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, thread, &stackFrame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) {  
        DWORD64 displacement = 0;  
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];  
        PSYMBOL_INFO pSymbol = 		(PSYMBOL_INFO)buffer;  
        pSymbol->SizeOfStruct = 	sizeof(SYMBOL_INFO);  
        pSymbol->MaxNameLen =		 MAX_SYM_NAME;  
  
        if (SymFromAddr(process, stackFrame.AddrPC.Offset, &displacement, pSymbol)) {  
            symbols.emplace_back(pSymbol->Name);  
        } else {  
            char address[32];  
            sprintf_s(address, sizeof(address), "0x%llX", stackFrame.AddrPC.Offset);  
            symbols.emplace_back(address);  
        }  
    }  
  
    for (auto& sym: symbols | std::views::reverse) std::cout << sym << "\n";  
  
    SymCleanup(process);  
}  
	
	
	
}
}


#endif
