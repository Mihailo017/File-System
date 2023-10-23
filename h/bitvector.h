#pragma once

#include <iostream>
#include "part.h"
#include "fs.h"

#define FREE_WORD 255
#define TAKEN_WORD 0
#define BYTE_ 8

typedef unsigned char VectorWord;


class BitVector {

public:
	static char freeCluster(ClusterNo clusterNo);
	static ClusterNo allocateCluster();
	static char store(); // Ovo mozda ne treba
	static char load(); // I ovo
	static char initialize();

	// Testiranje
	static void ispis();
private:
	BitVector();
	static VectorWord *vector;
	static ClusterNo nextFreeWord;
	static ClusterNo nextFreeBit;
	static ClusterNo bitVectorSize;
	static ClusterNo freeClusters;
	static ClusterNo numOfClusters;
};

