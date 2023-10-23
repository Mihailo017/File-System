#pragma once

#include "part.h"
#include "fs.h"


struct DirEntry {
	char fileName[8] = "\0      ";
	char ext[3] = "";
	char ub = 0; // Ne koristi se, upisati nulu
	ClusterNo firstIndexCluster = 0; // ClusterNo u kome se nalazi indeks prvog nivoa fajla
	BytesCnt fileSize = 0; // U bajtovima
	ClusterNo lastClusterNo = 0;
	BytesCnt lastClusterSize = 0;
	//bool dirty = false;
	char unused[4] = {0}; // Slobodno za koriscenje

	DirEntry(DirEntry* de) {
		for (int i = 0; i < 8; i++) fileName[i] = de->fileName[i];
		for (int i = 0; i < 3; i++) ext[i] = de->ext[i];
		ub = 0;
		firstIndexCluster = de->firstIndexCluster;
		fileSize = de->fileSize;
		lastClusterNo = de->lastClusterNo;
		lastClusterSize = de->lastClusterSize;
		for (int i = 0; i < 4; i++) unused[i] = 0;
	}
};