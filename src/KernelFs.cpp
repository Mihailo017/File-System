#include "KernelFs.h"
#include "FileTable.h"
#include "bitvector.h"
#include "KernelFile.h"

Partition* KernelFs::mounted = nullptr;
bool KernelFs::formatting = false;

// Za sinhornizaciju
HANDLE KernelFs::mutex = CreateSemaphore(NULL, 1, 32, NULL);
HANDLE KernelFs::mountingSem = CreateSemaphore(NULL, 0, 32, NULL);
int KernelFs::mountingWait = 0;
HANDLE KernelFs::formattingSem = CreateSemaphore(NULL, 0, 32, NULL);
int KernelFs::formattingWait = 0;
HANDLE KernelFs::unmountingSem = CreateSemaphore(NULL, 0, 32, NULL);
int KernelFs::unmountingWait = 0;

Partition* KernelFs::getMounted() {
	return mounted;
}

char KernelFs::mount(Partition* partition) {
	waitSem(mutex); 
	if (mounted) {
		mountingWait++;
		signalSem(mutex);
		waitSem(mountingSem);
	}
	mounted = partition;
	signalSem(mutex);
	return 1;
}
char KernelFs::unmount() {
	// Ovde treba blokiranje dok se ne zatvore svi fajlovi
	waitSem(mutex);
	if (FileTable::numOpenFiles()) { // Ako ima otvorenih fajlova
		unmountingWait++;
		signalSem(mutex);
		waitSem(unmountingSem);
	}
	mounted = nullptr;

	// Ovde treba da oslobodi sve koji cekaju na unmount i da resetuje brojac
	ReleaseSemaphore(unmountingSem, unmountingWait, NULL);
	unmountingWait = 0;

	if (mountingWait) {
		mountingWait--;
		signalSem(mountingSem);
	} else
		signalSem(mutex);
	return 1;
}

char KernelFs::format() {
	// Ovde treba blokiranje dok se ne zatvore svi fajlovi

	waitSem(mutex);
	if (FileTable::numOpenFiles()) { // Ako ima otvorenih fajlova
		formattingWait++;
		formatting = true;
		signalSem(mutex);
		waitSem(formattingSem);
	}

	if (!mounted) {
		signalSem(mutex);
		return 0;
	}

	BitVector::initialize();
	BitVector::store();

	Root::loadRoot();
	
	formatting = false;
	// Ovde treba da oslobodi sve koji cekaju na formatiranje i da resetuje brojac
	ReleaseSemaphore(formattingSem, formattingWait, NULL);
	formattingWait = 0;

	signalSem(mutex);

	return 1;
}
FileCnt KernelFs::readRootDir() {
	return Root::getNumOfFiles();
}
char KernelFs::doesExist(char* fname) {
	return Root::findEntry(fname) != nullptr ? 1 : 0;
}
KernelFile* KernelFs::open(char* fname, char mode) {
	// ovde treba greska ako je najavljeno formatiranje particije
	waitSem(mutex);
	if (formatting) {
		signalSem(mutex);
		return nullptr;
	} else {
		signalSem(mutex);
	}
	DirEntry* dirEntry = FileTable::fileTableOpen(fname, mode);
	if (!dirEntry) return nullptr;
	KernelFile* kernelFile = new KernelFile(dirEntry, mode);
	if (!kernelFile) return nullptr;
	if (mode == 'a') kernelFile->seek(kernelFile->getFileSize()); // Postavi kursor na kraj ako je mode 'a'
	return kernelFile;
	// U istoj metodi klase FS povratna vrednost ove funkcije treba se upakuje u objekat File
}

char KernelFs::deleteFile(char* fname) {
	// Mora prvo a se proveri da li je otvoren i da se blokira nit ako jeste
	return Root::deleteFile(fname);
}

char KernelFs::allFilesClosed() {
	waitSem(mutex);
	if (formattingWait) {
		formattingWait--;
		signalSem(formattingSem);
	}
	else if (unmountingWait) {
		unmountingWait--;
		signalSem(unmountingSem);
	} else 
		signalSem(mutex);
	return 1;
}