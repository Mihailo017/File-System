#include "FileData.h"
#include "KernelFs.h"


FileData::FileData(ClusterNo clusterNo, bool clear) : Cluster(clusterNo, clear) {
	size = ClusterSize;
}

BytesCnt FileData::write(BytesCnt start, BytesCnt size, char* buffer) {
	// EXCEPTION!!!
	if (start > this->size) return 0; // Ovde treba da baci izuzetak

	// Upis
	unsigned short i;
	for (i = 0; i < (ClusterSize - start) && i < size; i++)
		cluster[start + i] = buffer[i];

	if (start + i > this->size) this->size = start + i;

	dirty = true;

	return i; // Vraca broj uspesno upisanih bajtova
}

BytesCnt FileData::read(BytesCnt start, BytesCnt size, char* buffer) {
	// EXCEPTION!!!
	if (start >= this->size) return 0; // Ovde treba da baci izuzetak

	// Citanje
	unsigned short i;
	for (i = 0; i < size && (i < this->size - start); i++)
		buffer[i] = cluster[start + i];

	return i; // Vraca broj uspesno procitanih bajtova
}

char FileData::load(ClusterNo clusterNo) {
	if (dirty) { // Sacuvaj prvo ako je klaster menjan
		if (KernelFs::getMounted()->writeCluster(this->clusterNo, cluster) == 0) return 0; // Nula znaci da je upis bio neuspesan
		dirty = false;
	}

	this->clusterNo = clusterNo;
	this->sizeOfEntry = sizeof(char);
	numOfEntries = ClusterSize;
	size = ClusterSize;

	return KernelFs::getMounted()->readCluster(clusterNo, cluster);
}

char FileData::free() { // IndexTwo je taj koji ga oslobadja u bitVector
	return 1; 
}

char FileData::setSize(BytesCnt newSize) {
	size = newSize;
	return 1;
}

BytesCnt FileData::getSize() {
	return size;
}