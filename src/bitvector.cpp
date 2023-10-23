#include "bitvector.h"
#include "KernelFs.h"

// Obrisi kad zavrsis testiranje
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

VectorWord* BitVector::vector = nullptr;
ClusterNo BitVector::nextFreeWord = 0;
ClusterNo BitVector::nextFreeBit = 0;
ClusterNo BitVector::bitVectorSize = 0;
ClusterNo BitVector::freeClusters = 0;
ClusterNo BitVector::numOfClusters = 0;

//extern Partition* mounted;

char BitVector::initialize() {

	Partition* p;
	if (!(p = KernelFs::getMounted())) return 0; // Nema montirane particije

	numOfClusters = p->getNumOfClusters();
	freeClusters = numOfClusters - 2; // Broj clustera minus dva za BitVector i Root 
	//bitVectorSize = numOfClusters / BYTE;
	bitVectorSize = ClusterSize;
	vector = new VectorWord[bitVectorSize];

	for (ClusterNo i = 1; i < bitVectorSize; vector[i++] = FREE_WORD); // Na pocetku svi slobodni
	vector[0] = 252; // BitVector i Roos nisu slobodni zato 252 = 11111100b
	nextFreeWord = 0;
	nextFreeBit = 2; // O je BitVector a 1 je Root*/

	return 1; // Uspesno izvrsavanje
}

char BitVector::freeCluster(ClusterNo clusterNo) {

	ClusterNo offset = clusterNo % BYTE_, word = clusterNo / BYTE_;
	char mask = 1 << offset, check = ~mask;
	if ((vector[word] & check) == FREE_WORD || clusterNo >= numOfClusters) return 0; // Proverava da li je klaster stvarno zauzet i da li clusterNo u opsegu
	vector[word] |= mask;
	freeClusters++;
	nextFreeWord = word; // Sledeca slobodna rec se postavlja na ovu
	nextFreeBit = offset; // Analogno za bit

	return 1;
}

ClusterNo BitVector::allocateCluster() {

	if (freeClusters == 0) return 0; // Nulu je zauzeo sam bitVektor

	freeClusters--;
	char mask = ~(1 << nextFreeBit);
	vector[nextFreeWord] &= mask; // Postavljanje odgovarajuceg bita na nulu

	ClusterNo ret = nextFreeWord * BYTE_ + nextFreeBit; // Broj klastera koji se alocira

	/*Sada treba da se postave nextFreeBit i nextFreeWord na odgovarajuce vrednosti*/

	++nextFreeBit %= BYTE_; // Povecanje za 1;

	if (!freeClusters) return ret; // Ako nema slobodnih ne mora ni da se trazi

	while (vector[nextFreeWord] == TAKEN_WORD) nextFreeWord = (nextFreeWord + 1) % bitVectorSize; // Trazi se prva sledeca rec koja nije skroz popunjena
	while ((unsigned char)(vector[nextFreeWord] | (~(1 << nextFreeBit))) != FREE_WORD) ++nextFreeBit %= BYTE_; // Trazi se najmladji bit u reci koji je jedinica(slobodan) tj
		// Or operacija sa 1 pomeren za toliko mesta ulevo pa komplement da daje razlicito od sve jedinice(255)

	return ret;
}

char BitVector::store() {
	return KernelFs::getMounted()->writeCluster(0, (char*)vector);
}

char BitVector::load() {
	return KernelFs::getMounted()->readCluster(0, (char*)vector);
}

void BitVector::ispis() {
	for (int i = 0; i < bitVectorSize; i++) {
		//std::cout << (unsigned short)(vector[i]) << ' ';
		printf(BYTE_TO_BINARY_PATTERN" ", BYTE_TO_BINARY(vector[i]));
	}
}