#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Finfo.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <process.h>
#include <tchar.h>
#include <cstdio>
#include <cstdlib>
#pragma comment(lib, "Ws2_32.lib")
namespace fs = std::filesystem;

bool operator==(const Finfo& f1, const Finfo& f2){
	return (f1.parentName == f2.parentName);
}

char* GetNameFromPath(char* path){
	char* newPath = (char*)calloc(strlen(path) + 1, sizeof(char));
	strcpy(newPath, path);
	char* str = newPath;
	char* pch;
	char* xn = NULL;
	pch = strtok(str, "/ \ \\ ");
	while (pch != NULL){
		xn = pch;
		pch = strtok(NULL, " / \ \\ ");
	}
	return xn;
}

char* PathToChar(fs::path toChange){
	std::string str = toChange.string();
	char* cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str());
	return cstr;
}

Finfo* GetFiles(char* filepath, Finfo* parentDir){
	Finfo* file = new Finfo;
	file->isDir = fs::is_directory(filepath);
	file->size = fs::file_size(filepath);
	file->path = filepath;
	file->name = GetNameFromPath(filepath);
	file->nameS = GetNameFromPath(filepath);
	file->parent = parentDir;
	
	if (parentDir == NULL)
		file->parentName = "null";

	if(file->isDir)
		std::cout << "[" << _getpid() << "] " << file->name  << "\n\n";

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
			std::cout << GetNameFromPath(PathToChar(entry.path())) << "\n";
		}
	}
	return file;
}

void WriteFiles(Finfo* dir, int k){
	std::cout << dir->nameS << "\n";
	for (int i = 0; i < dir->children->size(); i++){
		for (int j = 0; j < k; j++)
			std::cout << "  ";

		std::cout << "\n";
		for (int j = 0; j < k; j++)
			std::cout << "  ";

		std::cout << "--";
		if (dir->children->at(i)->isDir)
			WriteFiles(dir->children->at(i), k+1);
		else
			std::cout << dir->children->at(i)->nameS << "\n";
	}
}

void __cdecl Server(void* Args){
	std::vector<Finfo*>* dataFiles = (std::vector<Finfo*>*) Args;
	fd_set master, read_fds;
	struct sockaddr_in myaddr, remoteaddr;
	int fdmax;							// maksymalny numer deskryptora pliku
	int listener;						// deskryptor gniazda nas³uchuj¹cego
	int newfd;							// nowozaakceptowany deskryptor gniazda
	char buf[256];						// bufor na dane pochodz¹ce od klienta
	int nbytes;
	bool yes = true;					// dla setsockopt() SO_REUSEADDR, patrz ni¿ej
	int addrlen;
	int i, j;
	Finfo* data = new Finfo;

	WSADATA wsaData;

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR)
		printf("Initialization error.\n");

	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// utwórz gniazdo nas³uchuj¹ceo
	if ((listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&yes,
		sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(27015);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr*)&myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	// listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}

	FD_SET(listener, &master);
	fdmax = listener;

	for (;;) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener, (struct sockaddr*)&remoteaddr,
						&addrlen)) == -1) {
					}
					else {
						FD_SET(newfd, &master);
						if (newfd > fdmax) {
							fdmax = newfd;
						}
					}
				}
				else {
					if ((nbytes = recv(i, (char*)data, sizeof(Finfo), 0)) <= 0) {
						FD_CLR(i, &master); 
					}
					else {
						Finfo* tmp = new Finfo;
						memcpy((void*)tmp, (void*)data, sizeof(Finfo));
						dataFiles->push_back(tmp);
					}
				}
			}
		}
	}
}

Finfo* CompleteFilesFromServer(std::vector<Finfo*> data){
	std::vector<Finfo*> tree;

	for (int i = 0; i < data.size(); i++){
		Finfo* tmp = new Finfo;
		memcpy((void*)tmp, (void*)data[i], sizeof(Finfo));
		tmp->name = NULL;
		tmp->parent = NULL;
		tmp->path = NULL;
		if(tmp->parentName!="null")
			tmp->children = new std::vector<Finfo*>;
		tree.push_back(tmp);
	}

	Finfo* root = new Finfo;
	for (int i = 0; i < data.size(); i++){
		if (tree[i]->parentName == "null"){
			root = tree[i];
			break;
		}
	}

	Finfo* actual;
	for (int i = 0; i < tree.size(); i++){
		actual = tree[i];
		for (int j = 0; j < tree.size(); j++){
			if (actual->nameS == tree[j]->parentName)
				actual->children->push_back(tree[j]);
		}
	}
	return root;
}