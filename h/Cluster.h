#pragma once

#include "part.h"
#include "fs.h"
#include <iostream>


typedef char ClusterWord;

class Cluster {

private:

	Cluster();

protected:

	ClusterWord* cluster;
	ClusterNo clusterNo;
	bool dirty;
	unsigned long numOfEntries, sizeOfEntry;

public:

	// Testiranje
	void ispis();
	// Testiranje

	Cluster(ClusterNo clusterNo, bool clear = false, unsigned short sizeOfEntry = sizeof(char));
	~Cluster();

	unsigned long getNumOfEntries() { return numOfEntries; }
	unsigned long getSizeOfEntry() { return sizeOfEntry; }
	ClusterNo getClusterNo() { return clusterNo; }

	void setDirty() { dirty = true; }

	char clear(); // Popunjava nulama
	virtual char load(ClusterNo clusterNo, bool clear = false, const unsigned long sizeOfEntry = sizeof(ClusterWord)); // Ucitava klaster, ako je dirty onda prvo vraca na disk
	virtual char store(ClusterNo clusterNo); // Vraca na disk, ako nije dirty ne radi nista

	virtual char free() = 0;
};

