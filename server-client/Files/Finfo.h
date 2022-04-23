#pragma once
#ifndef Finfo_h
#define Finfo_h
#include "Finfo.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include "Finfo.h"
namespace fs = std::filesystem;

class Finfo{
public:
	bool isDir;
	char* name;
	std::string nameS;
	char* path;
	int size;
	Finfo* parent;
	std::string parentName;
	std::vector<Finfo*>* children = new std::vector<Finfo*>;
	friend bool operator== (const Finfo& f1, const Finfo& f2);
};

char* GetNameFromPath(char* path);
char* PathToChar(fs::path toChange);
Finfo* GetFiles(char* filepath, Finfo* parentDir = NULL);
void WriteFiles(Finfo* dir, int k = 0);

#endif