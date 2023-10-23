#pragma once

#include "Cluster.h"
#include "DirEntry.h"

#define dirDataFull 512

class DirData : public Cluster {

private:
	DirData();
	DirEntry* entries;
	unsigned short nextFree;

	unsigned short findNextFree();
public:

	 DirData(ClusterNo clusterNo, bool clear = false, unsigned short sizeOfEntry = sizeof(DirEntry));

	 char load(ClusterNo clusterNo, bool clear = false, unsigned long sizeOfEntry = sizeof(DirEntry)) override;

	 DirEntry* allocateNext(char* filename);
	 DirEntry* getEntry(char* filename);
	 char removeEntry(char* filename);

	 bool hasNextFree();

	 char free() override;
};

