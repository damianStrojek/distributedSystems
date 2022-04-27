// Damian Strojek 184407
// g++ -std=c++17 lab7.cpp -lpthread
// ./a.out
#include <time.h>       // format czasu
#include <list>         // lista plikow/folderow
#include <iostream>     // cout itd
#include <string>       // string
#include <filesystem>
#include <chrono>       // znow format czasu
#include <iomanip>      // system
#include <sstream>      // stream do system
#include <pthread.h>    // watki
#include <semaphore.h>  // semafor dodatkowo dodany
#include <vector>
#include <unistd.h>
#define SLEEP_TIME 1

namespace fs = std::filesystem;

class Finfo {
public:
    bool isFolder;
    std::string name;
    int64_t length;
    fs::file_time_type mod;
    Finfo *parent;
    virtual ~Finfo() {};
};

class Folder : public Finfo {
public:
    std::list<Finfo*> children;
    Folder(){};
    ~Folder() {
	    for (std::list<Finfo*>::iterator it = children.begin(); it != children.end(); it++) {
            std::cout << "Deleting: " << (*it)->name << "\n";
            delete *it;
	    }
    }
};

template <typename TP>
std::time_t to_time_t(TP tp) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
        + system_clock::now());
    return system_clock::to_time_t(sctp);
}

struct Arguments {
	std::string dir;
	Folder* parent;
	int* counter;
	sem_t* counterSemaphore;
	bool isRoot=0;
};

int sizeOfFolder(Folder* f) {
    int size = 0;
    for (std::list<Finfo*>::iterator it = f->children.begin(); it != f->children.end(); it++) {
        if ((*it)->isFolder == 1)
            size += sizeOfFolder((Folder*)*it);
        else
            size += (*it)->length;
    }
    return size;
}

void makeDashes(int k) {
    for (int i = 0; i < k; i++) std::cout << "-";
}

void writeFiles(Folder* f, int k=0) {
    if (k == 0) {
        for(int i = 0; i < k; i++) std::cout << "   ";
        std::cout << "Name: " << f->name << ", is a folder: " << f-> isFolder <<
        ", length: " << f->length << "B.";

        const std::time_t tf = to_time_t(f->mod);
        std::tm* gmt = gmtime(&tf);
        std::stringstream buffer;
        buffer << std::put_time(gmt, "%A, %d %B %Y %H:%M");
        std::string formattedFileTime = buffer.str();
        std::cout << " Modification date: " << formattedFileTime << "\n";
        k++;
    }
    for (std::list<Finfo*>::iterator it = f->children.begin(); it != f->children.end(); it++) {
        Finfo* child = *it;

        for(int i = 0; i < k; i++) std::cout << "   ";
        std::cout << "Name: " << f->name << ", is a folder: " << f-> isFolder <<
        ", length: " << f->length << "B.";

        const std::time_t tt = to_time_t((child)->mod);
        std::tm* gmt = gmtime(&tt);
        std::stringstream buffer;
        buffer << std::put_time(gmt, "%A, %d %B %Y %H:%M");
        std::string formattedFileTime = buffer.str();
        std::cout << " Modification date: " << formattedFileTime << "\n";

        // wywołanie rekurencyjne
        if (child->isFolder) writeFiles((Folder*)child, k+1);
    }
}

// Dodatek semaforowy
sem_t semaphorT;
bool semaphorTReady = false;

void* getFiles(void* args) {
    Arguments* arguments = (Arguments*) args;
    std::vector <pthread_t*> threads;
    pthread_t* thread;
    Folder* folderRoot;
    
    int counter=0;
    sem_t counterSemaphore;
    sem_init(&counterSemaphore, 0, 1);
    if(arguments->isRoot) sem_init(&semaphorT, 0, 1);
    
    for (const auto& entry : fs::directory_iterator(arguments->dir)) {
        if (arguments->parent == NULL) {
            fs::directory_entry root = fs::directory_entry(arguments->dir);
            arguments->parent = new Folder;
            folderRoot = arguments->parent;
            arguments->parent->isFolder = 1;
            arguments->parent->name = root.path().filename().string();
            arguments->parent->mod = root.last_write_time();
            arguments->parent->parent = NULL;
        }
        if (!entry.is_directory()) {
            Finfo* newFile = new Finfo;
            newFile->isFolder = 0;
            newFile->name = entry.path().filename().string();
            newFile->length = entry.file_size();
            newFile->mod = entry.last_write_time();
            newFile->parent = arguments->parent;

            if(!arguments->isRoot){
            	bool active = 1;
            	while(active){
            		sem_wait(&semaphorT);
            		if(semaphorTReady){
            			active = 0;
            			arguments->parent->children.push_back(newFile);
            			semaphorTReady = 0;
            		}
            		sem_post(&semaphorT);
            	}
            }
            else {
            	arguments->parent->children.push_back(newFile);
            	writeFiles((Folder*) folderRoot);
                std::cout<<"\n-------------------------------------------------\n\n";
                sleep(SLEEP_TIME);
            }
        }
        else {
            Folder* newFile = new Folder;
            newFile->isFolder = 1;
            newFile->name = entry.path().filename().string();
            newFile->mod = entry.last_write_time();
            newFile->parent = arguments->parent;

            if(!arguments->isRoot){
            	bool active = 1;
            	while(active){
            		sem_wait(&semaphorT);
            		if(semaphorTReady){
            			active = 0;
            			arguments->parent->children.push_back(newFile);
            			semaphorTReady = 0;
            		}
            		sem_post(&semaphorT);
            	}
            }
            else {
            	arguments->parent->children.push_back(newFile);
            	writeFiles((Folder*) folderRoot);
                std::cout<<"\n-------------------------------------------------\n\n";
                sleep(SLEEP_TIME);
            }

            Arguments* newArguments = new Arguments;
            newArguments->dir = entry.path().string();
            newArguments->parent = newFile;
            newArguments->counter = &counter;
            newArguments->counterSemaphore = &counterSemaphore;
            thread = new pthread_t;
            pthread_create(&(*thread), NULL, getFiles, (void*)newArguments);
            threads.push_back(thread);
        }
    }

    void* status;
    arguments->parent->length = sizeOfFolder(arguments->parent);
    if(arguments->isRoot) semaphorTReady = 1;

    while(1) {
    	if(counter == threads.size()) break;
    	else {  	
    		if(arguments->isRoot){
    			sem_wait(&semaphorT);
    			if(!semaphorTReady){
    				semaphorTReady = 1;
    				writeFiles((Folder*) folderRoot);
    				std::cout<<"\n-------------------------------------------------\n\n";
                    sleep(SLEEP_TIME);
    			}
    			sem_post(&semaphorT);
    		}    	
    	}
    }

    // Pozbywanie sie resztek
    for(size_t i = 0; i < threads.size(); i++ ){
        pthread_join(*threads[i], &status);
        delete threads[i];
    }
    
    if(arguments->counterSemaphore != nullptr){
	    sem_wait(arguments->counterSemaphore);
	    int* cnt = arguments->counter;
	    (*cnt)++;
	    sem_post(arguments->counterSemaphore);	    
    }
    
    // Pozbycie sie dzieci
    if(arguments->isRoot) sem_destroy(&semaphorT);
    // Pozbycie sie samego siebie
    sem_destroy(&counterSemaphore);
    delete arguments;
    return folderRoot;
}

int main(){
    Folder* f = new Folder;
    Arguments* arguments = new Arguments;
    arguments->parent = NULL;
    arguments->dir = "/home/kali/Desktop/Lab7PR/dictionary1";
    arguments->counterSemaphore = nullptr;
    arguments->isRoot = 1;
    f = (Folder*)getFiles((void*)arguments);

    std::cout << "Deleting: " << f->name << std::endl;
    delete f;
    return 0;
}
