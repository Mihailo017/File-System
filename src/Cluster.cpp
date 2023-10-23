#include "Cluster.h"
#include "KernelFs.h"

void Cluster::ispis() {
	for (int i = 0; i < 512; i++) std::cout << ((ClusterNo*)cluster)[i] << ' ';
}

Cluster::Cluster(ClusterNo clusterNo, bool clear, unsigned short sizeOfEntry) {
	cluster = new ClusterWord[ClusterSize]; // ClusterSize = 2KB i tolika je velicina niza cluster jer je to niz bajtova
	dirty = false;
	load(clusterNo, clear, sizeOfEntry);
}

Cluster::~Cluster() {
	store(clusterNo);
	delete cluster;
}

char Cluster::clear() {
	for (int i = 0; i < ClusterSize; cluster[i++] = 0);
	dirty = true;
	return 1;
}

char Cluster::load(ClusterNo clusterNo, bool clear, const unsigned long sizeOfEntry) {

	if (dirty) { // Sacuvaj prvo ako je klaster menjan
		if (KernelFs::getMounted()->writeCluster(this->clusterNo, cluster) == 0) return 0; // Nula znaci da je upis bio neuspesan
		dirty = false;
	}

	this->clusterNo = clusterNo;
	this->sizeOfEntry = sizeOfEntry;
	numOfEntries = ClusterSize / sizeOfEntry;
	
	return clear ? this->clear() : KernelFs::getMounted()->readCluster(clusterNo, cluster);
}

char Cluster::store(ClusterNo clusterNo) {

	if (dirty) {
		return KernelFs::getMounted()->writeCluster(clusterNo, cluster); // 0 - neuspeh, 1 - uspeh
	}
	return 2; // Nije zaprljan, nema razloga da se cuva
}
