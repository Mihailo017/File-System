#pragma once
#include "Cluster.h"
class IndexTwo :
	public Cluster {

private:

	IndexTwo();
	ClusterNo* entries;
	bool isRootIndex;

public:
	IndexTwo(ClusterNo clusterNo, bool clear = false, bool root = false);

	// Preopterecena load metoda zbog sizeOfEntry razlike
	char load(ClusterNo clusterNo, bool clear = false, unsigned long sizeOfEntry = sizeof(ClusterNo)) override;

	// Vraca broj klastera u i-tom ulazu
	ClusterNo getEntry(unsigned short i);

	// Vraca ClusterNo novoalociranog klastera
	ClusterNo allocateNext();

	// Brise sadrzaj Indeksa drugog nivoa
	char free() override;
};

