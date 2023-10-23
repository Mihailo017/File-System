#include "DirData.h"
#include "IndexOne.h"
#include "helperFunctions.h"
#include <string>
#include "bitvector.h"

unsigned short DirData::findNextFree() {
	for (int i = 0; i < numOfEntries; i++) if (entries[i].fileName[0] == '\0') return i;
	return dirDataFull;
}

DirData::DirData(ClusterNo clusterNo, bool clear, unsigned short sizeOfEntry) : Cluster(clusterNo, clear, sizeOfEntry) {
	entries = (DirEntry*)cluster;
	nextFree = findNextFree();
}

char DirData::load(ClusterNo clusterNo, bool clear, unsigned long sizeOfEntry) {
	return Cluster::load(clusterNo, clear, sizeOfEntry);
}

DirEntry* DirData::allocateNext(char* filename) {
	nextFree = findNextFree();
	if (nextFree == dirDataFull) return nullptr;
	ClusterNo clusterNo = BitVector::allocateCluster();
	// EXCEPTION !!!
	//if (nextFree == dirDataFull || clusterNo == 0) throw exception

	DirEntry* cur = &(entries[nextFree]);
	getFileNameAndExt(filename, cur->fileName, cur->ext);
	cur->firstIndexCluster = clusterNo;

	dirty = true;
	
	return cur;
}

DirEntry* DirData::getEntry(char* filename) {
	char sfname[9], sext[4];
	getFileNameAndExt(filename, sfname, sext);
	std::string fname(sfname), ext(sext);

	for (int i = 0; i < numOfEntries; i++) {
		if (entries[i].fileName[0] != '\0') {
			std::string curFname(entries[i].fileName), curExt(entries[i].ext);
			if (fname.compare(curFname) == 0 && ext.compare(curExt) == 0) return &(entries[i]);
		}
	}

	return nullptr;
}

char DirData::removeEntry(char* filename) {
	DirEntry* cur = getEntry(filename);
	if (cur == nullptr) return 0;

	IndexOne* temp = new IndexOne(cur->firstIndexCluster);
	temp->free();
	delete temp;

	cur->fileName[0] = '\0';

	return 1;
}

bool DirData::hasNextFree() {
	return nextFree != dirDataFull;
}

char DirData::free() {
	for (int i = 0; i < numOfEntries; i++) {
		if (entries && entries[i].fileName[0] != 0) {
			IndexOne* temp = new IndexOne(entries[i].firstIndexCluster);
			temp->free();
			entries[i].fileName[0] = 0;
		}
	}

	BitVector::freeCluster(clusterNo);
	return 1;
}
