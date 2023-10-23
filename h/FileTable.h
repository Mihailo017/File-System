#pragma once

#include "DirEntry.h"
#include <map>
#include <iostream>
#include <string>
#include <windows.h>

struct TableEntry {
	DirEntry* dirEntry;
	char mode;
	bool dirty = false; // Sa ovo jos nista nisi uradio
	HANDLE sem = CreateSemaphore(NULL, 0, 32, NULL);
	int waiting = 0;
};

class FileTable {

private:

	static std::map<std::string, TableEntry> fileTable;
	static 	HANDLE mutex;

public:

	static DirEntry* fileTableOpen(char* filename, char mode);
	static char fileTableClose(char* filename);
	static int numOpenFiles();
};