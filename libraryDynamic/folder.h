#pragma once
#include <iostream>     // cout itd
#include <string>       // string
#include <filesystem>   // biblioteka dla informacji o plikach itd
#include <chrono>       // znow format czasu
#include <iomanip>      // system
#include <sstream>      // stream do czasu

class FInfo {
    public:
        bool isFolder;
        std::string name;
        __int64_t length;
        std::filesystem::file_time_type modificationDate;
        FInfo* parent;
        virtual ~FInfo() {};
};

class Folder : public FInfo {
    public:
        std::list<FInfo*> child;
        ~Folder();
};

inline Folder::~Folder() {
    for (std::list<FInfo*>::iterator iter = child.begin(); iter != child.end(); iter++) delete* iter;
};