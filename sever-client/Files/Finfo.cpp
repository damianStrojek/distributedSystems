#include "Finfo.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <sys/types.h>
#include <process.h>
#include <tchar.h>
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
	file->parentName = parentDir->nameS;
	
	return file;
}

void WriteFiles(Finfo* dir, int k) {
	std::cout << dir->nameS << "\n";
	for (int i = 0; i < dir->children->size(); i++) {
		for (int j = 0; j < k; j++)
			std::cout << "  ";

		std::cout << "\n";
		for (int j = 0; j < k; j++)
			std::cout << "  ";

		std::cout << "--";
		if (dir->children->at(i)->isDir)
			WriteFiles(dir->children->at(i), k + 1);
		else
			std::cout << dir->children->at(i)->nameS << "\n";
	}
}