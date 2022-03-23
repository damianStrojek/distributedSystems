#include <time.h>       // format czasu
#include <list>         // lista plikow/folderow
#include <iostream>     // cout itd
#include <string>       // string
#include <filesystem>   // biblioteka dla informacji o plikach itd
#include <chrono>       // znow format czasu
#include <iomanip>      // system
#include <sstream>      // stream do czasu
#pragma warning(disable : 4996) // problem z data - znowu

class FInfo {
public:
    bool isFolder;
    std::string name;
    __int64 length;
    std::filesystem::file_time_type modificationDate;
    FInfo* parent;
    virtual ~FInfo() {};
};

class Folder : public FInfo {
public:
    std::list<FInfo*> child;
    ~Folder();
};

Folder::~Folder() {
    for (std::list<FInfo*>::iterator iter = child.begin(); iter != child.end(); iter++) delete* iter;
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

void wciecie(int level) { for (int i = 0; i < level; i++) std::cout << "   "; };

// template dla formatu daty
template <typename TP> std::time_t to_time_t(TP tp) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now() + system_clock::now());
    return system_clock::to_time_t(sctp);
};

void writeFiles(Folder* actualFolder, int level) {
    // dzieci danego folderu
    for (std::list<FInfo*>::iterator iter = actualFolder->child.begin(); iter != actualFolder->child.end(); iter++) {
        FInfo* child = *iter;

        wciecie(level);
        std::cout << "Name: " << child->name << "\n";

        wciecie(level);
        if (child->isFolder) std::cout << "FOLDER\n";
        else std::cout << "FILE\n";

        wciecie(level);
        std::cout << "Length: " << child->length << "B\n";

        wciecie(level);
        const std::time_t tt = to_time_t((child)->modificationDate);
        std::tm* gmt = gmtime(&tt);
        std::stringstream buffer;
        buffer << std::put_time(gmt, "%A, %d %B %Y %H:%M");
        std::string formattedFileTime = buffer.str();
        std::cout << "Modification date: ";
        std::cout << formattedFileTime << "\n\n";

        if (child->isFolder) writeFiles((Folder*)child, level + 1);
    }
};

int main() {
    Folder* actualFolder = new Folder;
    actualFolder = getFiles("D:\\kali");
    writeFiles(actualFolder, 1);

    delete actualFolder;
    return 0;
};
