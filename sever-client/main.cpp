#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <processthreadsapi.h>
#include "Finfo.h"
namespace fs = std::filesystem;

int main() {
	std::vector<Finfo*> data;
	HANDLE hThread = (HANDLE)_beginthread(Server, 0, (void*)&data);
	Sleep(500);
	char folderPath[] = "C:\\Users\\Administrator\\Documents\\test";
	Finfo* folder = GetFiles(folderPath);
	Sleep(5000);
	TerminateThread(hThread, (DWORD)0);
	data.push_back(folder);
	Finfo* root = CompleteFilesFromServer(data);
	WriteFiles(root);

	return 0;
}