/* This file is part of LeagueSkinChanger by b3akers, licensed under the MIT license:
*
* MIT License
*
* Copyright (c) b3akers 2020
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/
//#include <iostream>
#include <Windows.h>
//#include <vector>
#include <thread>
//#include <chrono>
#include <tlhelp32.h>
//#include <psapi.h>

uint32_t find_process(std::wstring name)
{
	uint32_t process;

	HANDLE process_snap;
	PROCESSENTRY32 pe32;
	process_snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	pe32.dwSize = sizeof(PROCESSENTRY32);

	Process32First(process_snap, &pe32);

	if (pe32.szExeFile == name)
		process = pe32.th32ProcessID;

	else {
		while (Process32Next(process_snap, &pe32) & pe32.szExeFile != name)
		{
			if (pe32.szExeFile == name)
				break;
		}

		process = pe32.th32ProcessID;
	}

	CloseHandle(process_snap);

	return process;
}

//using namespace std::chrono_literals;

bool inject(uint32_t pid)
{
	TCHAR current_dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, current_dir);

	auto dll_path = std::wstring(current_dir) + L"\\skin_changer2.dll";

	auto handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

	FILETIME ft;
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);

	FILETIME create, exit, kernel, user;
	GetProcessTimes(handle, &create, &exit, &kernel, &user);

	//auto delta = 10 - static_cast< int32_t >( ( *reinterpret_cast< uint64_t* >( &ft ) - *reinterpret_cast< uint64_t* >( &create.dwLowDateTime ) ) / 10000000U );
	//if ( delta > 0 )
		//std::this_thread::sleep_for( std::chrono::seconds( delta ) );
	// POSSIBLE FIX FOR CRASHES

	auto dll_path_remote = VirtualAllocEx(handle, nullptr, (dll_path.size() + 1) * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	WriteProcessMemory(handle, dll_path_remote, dll_path.data(), (dll_path.size() + 1) * sizeof(wchar_t), nullptr);

	auto thread = CreateRemoteThread(handle, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(LoadLibrary(L"kernel32.dll"), "LoadLibraryW")), dll_path_remote, 0, nullptr);

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
	VirtualFreeEx(handle, dll_path_remote, 0, MEM_RELEASE);
	CloseHandle(handle);
	//printf( "[+] Injected successfully!\n" );
	return true;
}

int main()
{
	//printf("[+] Looking for league of legends processes...\n");

	auto league_processes = find_process(L"League of Legends.exe");

	inject(league_processes);

	//while (true) {
		//std::this_thread::sleep_for(9s);
	//}

	return 0;
}
