#include "IndexTwo.h"
#include "DirData.h"
#include "bitvector.h"

IndexTwo::IndexTwo(ClusterNo clusterNo, bool clear, bool root) : Cluster(clusterNo, clear, sizeof(ClusterNo)) {
	entries = (ClusterNo*)cluster;
	isRootIndex = root;
}

char IndexTwo::load(ClusterNo clusterNo, bool clear, unsigned long sizeOfEntry) {
	return Cluster::load(clusterNo, clear, sizeOfEntry);
}

ClusterNo IndexTwo::getEntry(unsigned short i) {
	// EXCEPTION!!!
	// if (i >= numOfEntries) throw Exception();
	return entries[i];
}

ClusterNo IndexTwo::allocateNext() {

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

char IndexTwo::free() {

	DirData* temp = nullptr;
	for (unsigned short i = 0; i < numOfEntries; i++) {
		ClusterNo entry = entries[i];
		if (entry != 0) {
			if (isRootIndex) { // Ako je ovo indeks drugog nivoa direktorijuma onda ucitaj data indekse i pozovi free za njih
				if (!temp) temp = new DirData(entry); else temp->load(entry);
				temp->free();
			} else // Ako je indeks drugog nivoa za fajl onda se u data indeksima nalazi samo sadrzaj fajla, nema potrebe da se brise samo se proglasi slobodnim
				BitVector::freeCluster(entry);
		}
	}
	delete temp;
	BitVector::freeCluster(clusterNo); // Oslobodi samog sebe
	return 1;
}
