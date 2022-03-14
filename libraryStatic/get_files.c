#include <list>         // lista plikow/folderow
#include <iostream>     // cout itd
#include <string>       // string
#include <filesystem>   // biblioteka dla informacji o plikach itd
#pragma warning(disable : 4996) // problem z data - znowu

#include "folder.h"

int folderSize(Folder* actualFolder) {
    int sum = 0;
    for (std::list<FInfo*>::iterator iter = actualFolder->child.begin(); iter != actualFolder->child.end(); iter++) {
        if ((*iter)->isFolder == 1) {
            sum += folderSize((Folder*)*iter);
        }
        else {
            sum += (*iter)->length;
        }
    }
    return sum;
};

Folder* getFiles(std::string dir, Folder* parent = NULL) {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (parent == NULL) {
            std::filesystem::directory_entry root = std::filesystem::directory_entry(dir);
            parent = new Folder;
            parent->isFolder = true;
            parent->name = root.path().filename().string();
            parent->modificationDate = root.last_write_time();
            parent->parent = NULL;
        }
        if (!entry.is_directory()) {
            FInfo* newFile = new FInfo;
            newFile->isFolder = false;
            newFile->name = entry.path().filename().string();
            newFile->length = entry.file_size();
            newFile->modificationDate = entry.last_write_time();
            newFile->parent = parent;
            parent->child.push_back(newFile);
        }
        else {
            Folder* newFile = new Folder;
            newFile->isFolder = 1;
            newFile->name = entry.path().filename().string();
            newFile->modificationDate = entry.last_write_time();
            newFile->parent = parent;
            parent->child.push_back(newFile);
            getFiles(entry.path().string(), newFile);
        }
    }
    parent->length = folderSize(parent);
    return parent;
};