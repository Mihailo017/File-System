#include "IndexOne.h"
#include "IndexTwo.h"
#include "bitvector.h"

IndexOne::IndexOne(ClusterNo clusterNo, bool clear) : Cluster(clusterNo, clear, sizeof(ClusterNo)) {
	entries = (ClusterNo*)cluster;
}

char IndexOne::load(ClusterNo clusterNo, bool clear, unsigned long sizeOfEntry) {
	return Cluster::load(clusterNo, clear, sizeOfEntry);
}

ClusterNo IndexOne::getEntry(unsigned short i) {
	// EXCEPTION!!!
	// if (i >= numOfEntries) throw Exception();
	return entries[i];
}

ClusterNo IndexOne::allocateNext() {

	// Nadji prazan ulaz
	unsigned short freeEntry;
	for (freeEntry = 0; freeEntry < numOfEntries && entries[freeEntry] != 0; freeEntry++);

	// Ako nema praznih ulaza (indeks je popunjen) onda EXCEPTION!!!
	if (freeEntry == numOfEntries) {
		return 0; // OVde treba da baci izuzetak
	}

	// EXCEPTION!!!
	ClusterNo nextFreeCluster = BitVector::allocateCluster(); // Ako nema slobodnih klastera na disku, treba da baci izuzetak

	entries[freeEntry] = nextFreeCluster; // Postavi odgovarajuci ulaz
	dirty = true;

	return nextFreeCluster;
}

char IndexOne::free() {
	IndexTwo* temp = nullptr;
	for (unsigned short i = 0; i < numOfEntries; i++) {
		if (entries[i] != 0) {
			if (!temp) temp = new IndexTwo(entries[i]); else temp->load(entries[i]);
			temp->free();
		} 
	}
	delete temp;
	BitVector::freeCluster(clusterNo); // Oslobodi samog sebe
	return 1;
}
