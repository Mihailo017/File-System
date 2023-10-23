#include "Root.h"
#include "IndexOne.h"
#include "IndexTwo.h"
#include "DirData.h"
#include "helperFunctions.h"

// Inicijalizacija statickih promenljivih
IndexOne* Root::rootIndexOne = nullptr;
IndexTwo* Root::curIndexTwo = nullptr;
DirData* Root::curDirData = nullptr;
ClusterNo Root::indexOneClusterNo = 1;
int Root::numOfFiles = 0;


/*char Root::deleteFile(DirEntry* dirEntry) {
	// Ucitaj indeks prvog nivoa
	Index* fileIndexOne = new Index(dirEntry->firstIndexCluster, INDEX_ONE);
	// Oslobodi, rekurzivno oslobadja sve indekse drugog nivoa i fileData klastere
	fileIndexOne->free(true);
	// Azuriraj sadrzaj deskriptora da je slobodan i posto si promenio sadrzaj mora se postavi i dirty bit
	dirEntry->fileName[0] = 0;
	curDirData->setDirty();

	delete fileIndexOne;

	return 0;
}*/

void Root::ispis() {
	curDirData->ispis();
}
char Root::loadRoot() {
	// Alociraj, ucitaj i ocisti indeks prvog nivoa
	rootIndexOne = new IndexOne(indexOneClusterNo, true);
	if (rootIndexOne == nullptr) return 0;

	// Alociraj, ucitaj i ocisti indeks drugog nivoa
	curIndexTwo = new IndexTwo(rootIndexOne->allocateNext(), true, true);
	if (curIndexTwo == nullptr) return 0;

	// Alociraj, ucitaj i ocisti dir data klaster
	curDirData = new DirData(curIndexTwo->allocateNext(), true);
	if (curDirData == nullptr) return 0;

	return 1;
}

char Root::unloadRoot() {
	delete rootIndexOne;
	delete curIndexTwo;
	delete curDirData;
	return 1;
}

/*int Root::find(char* fname, DirEntry** fileDesc) {
	// fname je u obliku "/imefajl.ext
	if (sizeof(fname) > 14) return -1; // fname je apsolutna putanja pa moze da bude najvise 14 karaktera jer je prvi '/', 8 za ime fajla, jedan za tacku i tri za ekstenziju  i jedan za \0
	if (fileDesc == nullptr) return -2; // U fileDesc se smesta deskritor fajla ako je nadjen zato ne sme da bude nullptr

	// Izdvajanje imena fajla iz aps putanje
	char fileName[8], ext[3];
	getFileNameAndExt(fname, fileName, ext);

	DirEntry* curEntry = nullptr, *firstFree = nullptr;

	for (unsigned short i = 0; i < rootIndexOne->getNumOfEntries(); i++) {

		// Aloceira se trenutni indeks drugog nivoa (klaster se cita iz indeksa prvog)
		ClusterNo curIndexTwoCluster = rootIndexOne->getClusterNo(i);

		if (curIndexTwoCluster == 0) { // Trenutni ulaz u indeksu prvog nivoa je prazan
			if (!firstFree) {
				ClusterNo nextIndexTwo;
				rootIndexOne->getNextLevelIndex(&nextIndexTwo);
				curIndexTwo->load(nextIndexTwo);
				ClusterNo nextDirData;
				curIndexTwo->getNextLevelIndex(&nextDirData);
				curDirData->load(nextDirData);
				firstFree = curDirData->getDirEntryPointer(0); // Njegov prvi ulaz se uzima za slobodan
			}
			break;
		}
		if (curIndexTwoCluster != curIndexTwo->getClusterNo()) curIndexTwo->load(curIndexTwoCluster);

		for (unsigned short j = 0; j < curIndexTwo->getNumOfEntries(); j++) {

			// Alocira se trenutni klaster sa sadrzajem direktorijuma (broj klastera se dobija iz indeksa drugog nivoa)
			ClusterNo curDataCluster = curIndexTwo->getClusterNo(j);
			if (curDataCluster == 0) { // Trenutni ulaz u indeksu drugog nivoa je prazan
				if (!firstFree) { // Ako nije nadjen slobodan ulaz mora se alocira i procita novi data klaster
					ClusterNo nextDirData;
					curIndexTwo->getNextLevelIndex(&nextDirData);
					curDirData->load(nextDirData);
					firstFree = curDirData->getDirEntryPointer(0); // Njegov prvi ulaz se uzima za slobodan
				}
				*fileDesc = firstFree;
				return 1; // Nema potrebe za daljim trazenjem
			}
			if (curDataCluster != curDirData->getClusterNo()) curDirData->load(curDataCluster);

			for (unsigned short k = 0; k < curDirData->getNumOfEntries(); k++) {
				curEntry = curDirData->getDirEntryPointer(k);
				if ((curEntry->fileName[0]) == 0) {
					if (!firstFree) firstFree = curEntry;
					continue;
				}

				bool found = true;

				// Provera ekstenzije prvo zato sto je krace
				for (char c = 0; c < 3; c++) if (curEntry->ext[c] != ext[c]) {
					found = false;
					break;
				}

				// Provera da li se imena iz trenutnog desk fajla i trazenog fajla poklapaju ali samo ako se ekstenzija vec poklapa
				if (found) {
					for (char c = 0; c < 8; c++) if (curEntry->fileName[c] != fileName[c]) {
						found = false;
						break;
					}
				}
				if (found) {
					*fileDesc = curEntry;
					return 0; // Fajl nadjen!
				}
			}
		}
	}

	if (firstFree == nullptr) return 2; // Nema fajla niti slobodnog mesta, direktorijum je pun
	else {
		*fileDesc = firstFree;
		return 1; // Fajl nije nadjen ali ima slobonog mesta u direktorijumu, fileDesc sada pokazuje na prazan ulaz u direktorijum
	}
}*/

DirEntry* Root::findEntry(char* filename) {

	// EXCEPTION !!!

	//if (sizeof(fname) > 14) throw exception; // fname je apsolutna putanja pa moze da bude najvise 14 karaktera jer je prvi '/', 8 za ime fajla, jedan za tacku i tri za ekstenziju  i jedan za \0
	//if (fileDesc == nullptr) throw exception; // U fileDesc se smesta deskritor fajla ako je nadjen zato ne sme da bude nullptr

	DirEntry* ret = curDirData->getEntry(filename);
	if (ret == nullptr) {
		IndexTwo* tempIndexTwo = nullptr;
		DirData* tempDirData = nullptr;
		for (unsigned short i = 0; i < rootIndexOne->getNumOfEntries(); i++) {
			if (rootIndexOne->getEntry(i) != 0) {
				//curIndexTwo->load(rootIndexOne->getEntry(i));
				if (!tempIndexTwo) tempIndexTwo = new IndexTwo(rootIndexOne->getEntry(i), false, true); else tempIndexTwo->load(rootIndexOne->getEntry(i));
				for (unsigned short j = 0; j < tempIndexTwo->getNumOfEntries(); j++) {
					if (curIndexTwo->getEntry(j) != 0) {
						//curDirData->load(curIndexTwo->getEntry(j));
						if (!tempDirData) tempDirData = new DirData(tempIndexTwo->getEntry(i)); else tempDirData->load(tempIndexTwo->getEntry(i));
						if (ret = curDirData->getEntry(filename)) break;
					}
				}
			}
			if (ret) {
				delete curIndexTwo;
				delete curDirData;
				curIndexTwo = tempIndexTwo;
				curDirData = tempDirData;
				tempIndexTwo = nullptr;
				tempDirData = nullptr;
				break;
			}
		}
		delete tempIndexTwo;
		delete tempDirData;
	}
	return ret;
}

DirEntry* Root::initDirEntry(char* fname) {
	DirEntry* curEntry = curDirData->allocateNext(fname);
	if (!curEntry) {
		for (unsigned short i = 0; i < rootIndexOne->getNumOfEntries(); i++) {
			if (rootIndexOne->getEntry(i) != 0) {
				curIndexTwo->load(rootIndexOne->getEntry(i));
				for (unsigned short j = 0; j < curIndexTwo->getNumOfEntries(); j++) {
					if (curIndexTwo->getEntry(j) != 0) {
						curDirData->load(curIndexTwo->getEntry(j));
						if (curEntry = curDirData->allocateNext(fname)) break;
					}
				}
			}
			if (curEntry) break;
		}
	}
	if (curEntry) numOfFiles++; // Proveri ovo
	return curEntry;
}

char Root::setEntry(DirEntry* de) {
	char* fileName = makeFilePath(de->fileName, de->ext);
	
	DirEntry* temp = findEntry(fileName);
	if (!temp) return 0; // EXCEPTION !!!

	for (int i = 0; i < 8; i++) temp->fileName[i] = de->fileName[i];
	for (int i = 0; i < 3; i++) temp->ext[i] = de->ext[i];
	temp->ub = 0;
	temp->firstIndexCluster = de->firstIndexCluster;
	temp->fileSize = de->fileSize;
	temp->lastClusterNo = de->lastClusterNo;
	temp->lastClusterSize = de->lastClusterSize;
	for (int i = 0; i < 4; i++) temp->unused[i] = 0;

	curDirData->setDirty();

	delete fileName;
	return 1;
}

FileCnt Root::getNumOfFiles() {
	return numOfFiles;
}

/*int Root::open(char* fname, char mode, DirEntry** fileDesc) {

	DirEntry* curEntry = new DirEntry();
	bool exists = false;

	switch (int r = find(fname, &curEntry)) {
	case 0: exists = true;
		break;
	case 1: exists = false;
		break;
	default: return r; // Kodovi greske isti kao u find funkciji
	}

	// Prema tekstu zadatka
	// r - za citanje, fajl mora da postoji
	// a - za citanje i upis, fajl mora da postoji, kursor se stavlja na kraj
	// w - pravi novi fajl, ako fajl vec postoji brise se
	if ((mode == 'r' || mode == 'a') && !exists) return -3;
	if (mode == 'w' && exists) {
		deleteFile(curEntry);
		exists = false;
	}

	if (!exists) { // Kreiranje fajla ako ne postoji
		// Izdvajanje imena fajla iz aps putanje
		/*getFileNameAndExt(fname, curEntry->fileName, curEntry->ext);
		curEntry->firstIndexCluster = BitVector::allocateCluster();
		curEntry->fileSize = 0;
		//curDirData->setDirty();
		curDirData->initDirEntry(curEntry, fname);
	}

	*fileDesc = curEntry; // Vrati deskriptor fajla preko arg
	return 0; // Fajl otvoren u direktorijumu, ostali poslovi koji treba se zavrse se ne rade ovde vec u kernelfs klasu	
}*/

DirEntry* Root::open(char* fname, char mode) {

	DirEntry* curEntry = findEntry(fname);
	
	// Prema tekstu zadatka
	// r - za citanje, fajl mora da postoji
	// a - za citanje i upis, fajl mora da postoji, kursor se stavlja na kraj
	// w - pravi novi fajl, ako fajl vec postoji brise se
	if ((mode == 'r' || mode == 'a') && !curEntry) return nullptr;
	if (mode == 'w' && curEntry) {
		if (deleteFile(fname)) curEntry = nullptr;
		else return nullptr;
	}

	if (!curEntry) // Kreiranje fajla ako ne postoji
		curEntry = initDirEntry(fname);

	return curEntry; // Fajl otvoren u direktorijumu, ostali poslovi koji treba se zavrse se ne rade ovde vec u kernelfs klasu	
}

char Root::deleteFile(char* fname) {
	int ret = curDirData->removeEntry(fname);
	if (!ret) {
		for (unsigned short i = 0; i < rootIndexOne->getNumOfEntries(); i++) {
			if (rootIndexOne->getEntry(i) != 0) {
				curIndexTwo->load(rootIndexOne->getEntry(i));
				for (unsigned short j = 0; j < curIndexTwo->getNumOfEntries(); j++) {
					if (curIndexTwo->getEntry(j) != 0) {
						curDirData->load(curIndexTwo->getEntry(j));
						if (ret = curDirData->removeEntry(fname)) break;
					}
				}
			}
			if (ret) break;
		}
	}
	if (ret) numOfFiles--; // Proveri ovo
	return ret;
}