// g++ -std=c++17 a.cpp
// ./a.exe C:\\Users\\Administrator\\Documents\\test
#include <time.h>       // format czasu
#include <list>         // lista plikow/folderow
#include <iostream>     // cout itd
#include <string>       // string
#include <filesystem>
#include <chrono>       // znow format czasu
#include <iomanip>      // system
#include <sstream>      // stream do systemu
#include <windows.h>
#include <stdio.h>
#include <cstring>
#pragma warning(disable : 4996) // problem z data - znowu

class FInfo {
public:
    bool isFolder;
    std::string name;
    int64_t length = 0;
    std::filesystem::file_time_type modificationDate;
    FInfo* parent;
    virtual ~FInfo(){};
};

class Folder : public FInfo {
public:
    std::list<FInfo*> child;
    ~Folder();
};

Folder::~Folder() { 
    for (std::list<FInfo*>::iterator iter = child.begin(); iter != child.end(); iter++){ 
        std::cout << "Deleting: " << (*iter)->name << "\n";
        delete* iter; 
    }
};

int folderSize(Folder* actualFolder) {
    int sum = 0;
    for (std::list<FInfo*>::iterator iter = actualFolder->child.begin(); iter != actualFolder->child.end(); iter++) {
        if ((*iter)->isFolder == 1) sum += folderSize((Folder*)*iter);
        else sum += (*iter)->length;
    }
    return sum;
};

Folder* getFiles(std::string dir, Folder* parent = NULL) {
    char dirArray[1000];
    PROCESS_INFORMATION processInformation;
    STARTUPINFO startupInfo;

    ZeroMemory(&processInformation, sizeof(processInformation));
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);

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

            strcpy(dirArray, ("a.exe " + entry.path().string()).c_str());
            /*
            wchar_t wtext[1000];
            mbstowcs(wtext, dirArray, strlen(dirArray) + 1);
            LPWSTR ptr = wtext;
            */
            if (!CreateProcess(NULL, (LPSTR) dirArray, NULL, NULL, FALSE, CREATE_NEW_CONSOLE,
                NULL, NULL, &startupInfo, &processInformation)) {
                std::cout << "CreateProcess() failed.\n";
                return nullptr;
            }
            std::cout << "Process's number: " << processInformation.dwProcessId << "\n";
            WaitForSingleObject(processInformation.hProcess, INFINITE);
            // Close process and thread handles. 
            CloseHandle(processInformation.hProcess);
            CloseHandle(processInformation.hThread);
        }
    }
    parent->length = folderSize(parent);
    return parent;
};

// template dla formatu daty
template <typename TP> std::time_t to_time_t(TP tp) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
};

void writeFiles(Folder* actualFolder) {
    for (std::list<FInfo*>::iterator iter = actualFolder->child.begin(); iter != actualFolder->child.end(); iter++) {
        FInfo* child = *iter;

        std::cout << "Name: " << child->name << "\n";
        if (child->isFolder) std::cout << "FOLDER\n";
        else std::cout << "FILE\n";

        std::cout << "Length: " << child->length << "B\n";

        const std::time_t tt = to_time_t((child)->modificationDate);
        std::tm* gmt = gmtime(&tt);
        std::stringstream buffer;
        buffer << std::put_time(gmt, "%A, %d %B %Y %H:%M");
        std::string formattedFileTime = buffer.str();
        std::cout << "Modification date: " << formattedFileTime << "\n\n";

        if (child->isFolder) writeFiles((Folder*)child);
    }
};

int main(int argc, char **argv) {
    if (argc == 1) { printf("No directory specified"); return 0; }

    Folder* actualFolder = new Folder;
    std::string str(argv[1]);
    actualFolder = getFiles(str);
    writeFiles(actualFolder);

    std::cout << "Deleting: " << actualFolder->name << "\n";
    delete actualFolder;

    std::cin >> str; // pause
    return 0;
};
