#include "Finfo.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <process.h>
#include <tchar.h>
#include <vector>
#pragma comment(lib, "Ws2_32.lib")
namespace fs = std::filesystem;

int main(int argc, char* argv[]){
	char* filepath;
	std::string pName;
	if (argc == 2){
		filepath = argv[1];
		pName = argv[2];
	}
	else {
		filepath = argv[1];
		pName = argv[2];
	}

	Finfo* file = new Finfo;
	file->isDir = fs::is_directory(filepath);
	file->size = fs::file_size(filepath);
	file->path = filepath;
	std::string tmp(PathToChar(filepath));
	file->name = GetNameFromPath(filepath);
	file->nameS = GetNameFromPath(filepath);
	file->parentName = pName;
	
	std::cout << "[" << _getpid() << "] " << file->name << "\n\n";
	if (file->isDir){
		for (const auto& entry : fs::directory_iterator(filepath)){
			if (entry.is_directory()){
				STARTUPINFO si;
				PROCESS_INFORMATION pi;
				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&pi, sizeof(pi));
				std::string tmp1 = "C:\\Users\\Administrator\\Desktop\\Laboratorium1\\Files\\Debug\\Files.exe";
				const char* tmp2 = tmp1.c_str();
				char* ppath = _strdup(tmp2);
				std::string arg = " " + (std::string)PathToChar(entry.path()) + " " + file->name;
				const char* argv = arg.c_str();
				char* args = _strdup(argv);
				CreateProcess(ppath, args, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
			}
			else
				file->children->push_back(GetFiles(PathToChar(entry.path()), file));
			std::cout << GetNameFromPath(PathToChar(entry.path())) << std::endl;
		}
	}

	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR)
		printf("Initialization error.\n");

	SOCKET mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mainSocket == INVALID_SOCKET){
		printf("Error creating socket: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in service;
	memset(&service, 0, sizeof(service));
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(27015);

	if (connect(mainSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR){
		printf("Failed to connect.\n");
		WSACleanup();
		return 1;
	}

	int bytesSent = 0, bytesRecv = SOCKET_ERROR;

	for (int i = 0; i < file->children->size(); i++){
		Finfo* tmp = file->children->at(i);
		bytesSent += send(mainSocket, (char*)tmp, sizeof(Finfo), 0);
	}
	file->children->clear();
	bytesSent += send(mainSocket, (char*)file, sizeof(Finfo), 0);
	std::cout << "Sent " << bytesSent << " bytes to root process.\n";
	getchar();
}