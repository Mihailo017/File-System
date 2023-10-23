#pragma once
#include "Cluster.h"

class IndexOne :
	public Cluster {

private:

	IndexOne();
	ClusterNo* entries;

public:
	IndexOne(ClusterNo clusterNo, bool clear = false);

	// Preopterecena load metoda zbog sizeOfEntry razlike
	char load(ClusterNo clusterNo, bool clear = false, unsigned long sizeOfEntry = sizeof(ClusterNo)) override;

	// Vraca broj klastera u i-tom ulazu
	ClusterNo getEntry(unsigned short i);

	// Vraca ClusterNo novoalociranog klastera
	ClusterNo allocateNext();

	// Brise sadrzaj Indeksa prvog nivoa
	char free() override;
};
