#pragma once

#include "fs.h"
#include "part.h"
#include "DirEntry.h"

class IndexOne;
class IndexTwo;
class DirData;

class Root {

	Root();

	//static Index* rootIndexOne;
	//static Index* curIndexTwo;
	static IndexOne* rootIndexOne;
	static IndexTwo* curIndexTwo;
	static DirData* curDirData;
	static ClusterNo indexOneClusterNo;
	static int numOfFiles;

public:

	// Testiranje
	//static char deleteFile(DirEntry* dirEntry);
	static void ispis();
	// Testiranje

	static char loadRoot(); // Inicijalizacija jer nema konstruktora, ovo treba da se pozove iz format metode KernelFS
	static char unloadRoot();

	//static int find(char* fname, DirEntry** fileDesc); // Nalazenje fajla sa trazenim imenom
	static DirEntry* findEntry(char* filename);
	static DirEntry* initDirEntry(char* fname);
	static char setEntry(DirEntry* dirEntry);

	static FileCnt getNumOfFiles();

	static DirEntry* open(char* fname, char mode);
	static char deleteFile(char* fname);
};

