#include "KernelFile.h"
#include "FileTable.h"
#include "bitvector.h"
#include "KernelFs.h"
#include "helperFunctions.h"

BytesCnt KernelFile::relToAbs() {
	return (BytesCnt)((indexTwoNo * indexEntries + fileDataClusterNo) * ClusterSize + relCurPos);
}

KernelFile::KernelFile(DirEntry* dirEntry, char mode) {

	this->dirEntry = dirEntry;
	bool clear = mode == 'w';

	//indexOne = new Index(indexOneNo, INDEX_ONE, true);
	iOne = new IndexOne(dirEntry->firstIndexCluster, clear);

	/*ClusterNo nextClusterNo;
	indexOne->getNextLevelIndex(&nextClusterNo);
	curIndexTwo = new Index(nextClusterNo, INDEX_TWO, true);*/
	ClusterNo iTwoClusterNo = dirEntry->lastClusterNo ? iOne->getEntry(0) : iOne->allocateNext();
	iTwo = new IndexTwo(iTwoClusterNo, clear);
	indexTwoNo = 0;

	//curIndexTwo->getNextLevelIndex(&nextClusterNo);
	//curFileData = new FileData(nextClusterNo);
	ClusterNo dirDataClusterNo = dirEntry->lastClusterNo ? iTwo->getEntry(0) : iTwo->allocateNext();
	curFileData = new FileData(dirDataClusterNo);
	if (curFileData->getClusterNo() == dirEntry->lastClusterNo) curFileData->setSize(dirEntry->lastClusterSize);
	fileDataClusterNo = 0;

	size = dirEntry->fileSize;
	absCurPos = 0;
	relCurPos = 0;

	if (!dirEntry->lastClusterNo) { // Znaci ako je sad ucitan prvi FileData (lastClusterNo je nula) onda postavi to u dirEntry
		// Ovo znaci i da je fajl bez sadrzaja
		dirEntry->lastClusterNo = curFileData->getClusterNo();
		dirEntry->lastClusterSize = 0;
		curFileData->setSize(0); // Fajl je prazan
	}
}

KernelFile::~KernelFile() {
	delete iOne;
	delete iTwo;
	delete curFileData;
	char* filePath = makeFilePath(dirEntry->fileName, dirEntry->ext);
	FileTable::fileTableClose(filePath);
	delete filePath;
}

/*char KernelFile::write(BytesCnt toWrite, char* buffer) {

	BytesCnt bytesWritten = curFileData->write(relCurPos, toWrite, buffer); // Upisi koliko mozes u trenutni data klaster
	relCurPos += (bytesWritten - 1); // Postavljanje trenutne rel pozicije na kraj upisanog sadrzaja
	toWrite -= bytesWritten; // Ukupan broj bajtova koji treba se upise se smanjuje

	while (toWrite > 0) { // Ako jos ima da se pise
		if (fileDataClusterNo == 511 && indexTwoNo == 511) return 1; // Ako je trenutni data klaster poslednji u trenutnom indeksu drugog nivoa
			// I tren tni indeks drugog nivoa je poslednji u indeksu prvog nivoa onda je dostignuta maksimalna velicina fajla GRESKA

			if (++fileDataClusterNo == 512) { // Ako je poslednji data klaster mora se ucita novi indeks drugog nivoa
				ClusterNo next = indexOne->getClusterNo(++indexTwoNo); // Uzmi naredni indeks drugog nivoa iz indeksa prvog nivoa

				if (next == 0) indexOne->getNextLevelIndex(&next); // Ako je u narednom ulazu indeksa prvog nivoa nula mora se alocira novi indeks dva
				curIndexTwo->load(next); // Ucitavanje novog indeksa drugog nivoa

				fileDataClusterNo = 0; // Naredni data klaster se nalazi u nultom ulazu novog indeksa drugog nivoa
			}

		ClusterNo dataNo = curIndexTwo->getClusterNo(fileDataClusterNo); // Trenutni ulaz indeksa drugog nivoa
		if (dataNo == 0) curIndexTwo->getNextLevelIndex(&dataNo); // Ako je nula mora se alocira novi data klaster

		curFileData->load(dataNo); // Ucitavanje novog data klastera

		BytesCnt curWritten = curFileData->write(0, toWrite, buffer + bytesWritten); // Broj bajtova koji se upisuje u ovoj iteraciji
		//pocinje od nule jer je ucitan novi data klaster
		relCurPos = curWritten - 1; // Nova relativna pozicija
		bytesWritten += curWritten;
		toWrite -= curWritten;
	}

	if ((absCurPos = relToAbs()) > size) size = absCurPos + 1;
	
	if (bytesWritten != toWrite) return 2; // Iz nekog razloga nisu upisani svi bajtovi
	return 0;
}*/


char KernelFile::write(BytesCnt toWrite, char* buffer) {

	BytesCnt bytesWritten = 0;
	if (relCurPos != ClusterSize) {
		bytesWritten = curFileData->write(relCurPos, toWrite, buffer); // Upisi koliko mozes u trenutni data klaster
		relCurPos += bytesWritten; // Postavljanje trenutne rel pozicije na kraj upisanog sadrzaja
		toWrite -= bytesWritten; // Ukupan broj bajtova koji treba se upise se smanjuje
	}

	while (toWrite > 0) { // Ako jos ima da se pise
		if (fileDataClusterNo == 511 && indexTwoNo == 511) return 1; // Ako je trenutni data klaster poslednji u trenutnom indeksu drugog nivoa
			// I tren tni indeks drugog nivoa je poslednji u indeksu prvog nivoa onda je dostignuta maksimalna velicina fajla GRESKA

		if (++fileDataClusterNo == 512) { // Ako je poslednji data klaster mora se ucita novi indeks drugog nivoa
			ClusterNo next = iOne->getEntry(++indexTwoNo); // Uzmi naredni indeks drugog nivoa iz indeksa prvog nivoa

			if (next == 0) { // Ako je u narednom ulazu indeksa prvog nivoa nula mora se alocira novi indeks dva i posto je novi mora se clearuje
				next = iOne->allocateNext();
				iTwo->load(next, true);
			} else iTwo->load(next); // Ako je vec alociran indeks drugog nivoa onda se samo ucitava, ne mora se clear

			fileDataClusterNo = 0; // Naredni data klaster se nalazi u nultom ulazu novog indeksa drugog nivoa
		}

		ClusterNo dataNo = iTwo->getEntry(fileDataClusterNo); // Trenutni ulaz indeksa drugog nivoa
		if (dataNo == 0) dataNo = iTwo->allocateNext(); // Ako je nula mora se alocira novi data klaster

		BytesCnt curWritten;
		if (toWrite > ClusterSize) {
			KernelFs::getMounted()->writeCluster(dataNo, buffer + bytesWritten);
			curWritten = ClusterSize;
		} else {
			curFileData->load(dataNo); // Ucitavanje novog data klastera
			if (curFileData->getClusterNo() == dirEntry->lastClusterNo) curFileData->setSize(dirEntry->lastClusterSize);
			curWritten = curFileData->write(0, toWrite, buffer + bytesWritten); // Broj bajtova koji se upisuje u ovoj iteraciji
			//pocinje od nule jer je ucitan novi data klaster
		}

		relCurPos = curWritten; // Nova relativna pozicija
		bytesWritten += curWritten;
		toWrite -= curWritten;
	}

	if ((absCurPos = relToAbs()) > size) {
		size = absCurPos;
		dirEntry->fileSize = size;
		// Ovde treba da se jos azurira da je tekuci dataCluster sada poslednji u dirEntry kao i njegov size da se postavi na relCurPos i u dirEntry
		dirEntry->lastClusterNo = curFileData->getClusterNo();
		dirEntry->lastClusterSize = relCurPos;
		//dirEntry->dirty = true;
		curFileData->setSize(relCurPos);
	}

	//if (bytesWritten != toWrite) return 2; // Iz nekog razloga nisu upisani svi bajtovi
	return bytesWritten; // Ovde treba samo return 0
}

/*BytesCnt KernelFile::read(BytesCnt toRead, char* buffer) {

	BytesCnt bytesRead = curFileData->read(relCurPos, toRead, buffer); // Procitaj koliko mozes iz trenutnog klastera
	relCurPos += (bytesRead - 1); // Relativna pozicija na kraj procitanog sadrzaja
	toRead -= bytesRead; // Ukupan brj bajtova koji treba da se procita se smanjuje

	while (toRead > 0) { // Dok jos ima da se cita

		if (fileDataClusterNo == 511 && indexTwoNo == 511) return bytesRead; // Doslo se do kraja fajla, nema vise sadrzaja

		if (fileDataClusterNo == 511) { // Trenutni data klaster je poslednji u ovom indeksu dva, treba se dovuce novi indeks dva i novi data klaster

			ClusterNo next = indexOne->getClusterNo(indexTwoNo + 1); // Ovde ne sme da se poveca indexTwoNo jer naredni mozda ne postoj
			if (next == 0) return bytesRead; // Ako je ulaz slobodan znaci nema vise indeksa nivoa dva, doslo se do kraja fajla
			curIndexTwo->load(next); // Ucitavanje novog indeksa nivoa dva ako ga ima

			indexTwoNo++; // Tek sada sigurno moze da se poveca!
			fileDataClusterNo = 0;
		}

		ClusterNo dataNo = curIndexTwo->getClusterNo(fileDataClusterNo + 1); // Isto kao za indexTwoNo ne sme da se poveca dok sigurno ne znamo da nije zadnji
		if (dataNo == 0) return bytesRead; // Nema narednog data klastera, kraj fajla
		curFileData->load(dataNo); // Ucitavanje novog data klastera

		fileDataClusterNo++; // Tek sada sigurno moze da se poveca!

		BytesCnt curRead = curFileData->read(0, toRead, buffer + bytesRead); // Broj procitanih bajtova u ovoj iteraciji, pocinje od nule jer je dovucen novi data klaster
		relCurPos = curRead - 1; // Jer se staje na kraj procitanog sadrzaja a ne posle zato minus jedan
		bytesRead += curRead;
		toRead -= curRead;
	}

	// Azuriranje apsolutna pozicija na osnovu relativne
	absCurPos = relToAbs();
	return bytesRead; // Read vraca broj procitanih bajtova
}*/

BytesCnt KernelFile::read(BytesCnt toRead, char* buffer) {
	
	if (size == 0) return 0;
	if (toRead > (size - absCurPos)) toRead = size - absCurPos; // Ako je zahtev za citanje veci od velicine fajla cita se do kraja fajla

	BytesCnt bytesRead = 0;
	if (relCurPos != ClusterSize) {
		bytesRead = curFileData->read(relCurPos, toRead, buffer); // Procitaj koliko mozes iz trenutnog klastera
		relCurPos += bytesRead; // Relativna pozicija na kraj procitanog sadrzaja
		toRead -= bytesRead; // Ukupan brj bajtova koji treba da se procita se smanjuje
	}

	while (toRead > 0) { // Dok jos ima da se cita

		if (fileDataClusterNo == 511 && indexTwoNo == 511) {
			absCurPos = relToAbs();
			return bytesRead;
		}// Doslo se do kraja fajla, nema vise sadrzaja

		if (fileDataClusterNo == 511) { // Trenutni data klaster je poslednji u ovom indeksu dva, treba se dovuce novi indeks dva i novi data klaster

			ClusterNo next = iOne->getEntry(indexTwoNo + 1); // Ovde ne sme da se poveca indexTwoNo jer naredni mozda ne postoj
			if (next == 0) {
				absCurPos = relToAbs();
				return bytesRead;
			}// Ako je ulaz slobodan znaci nema vise indeksa nivoa dva, doslo se do kraja fajla
			iTwo->load(next); // Ucitavanje novog indeksa nivoa dva ako ga ima

			indexTwoNo++; // Tek sada sigurno moze da se poveca!
			fileDataClusterNo = 0;
		} else fileDataClusterNo++;

		ClusterNo dataNo = iTwo->getEntry(fileDataClusterNo); // Isto kao za indexTwoNo ne sme da se poveca dok sigurno ne znamo da nije zadnji
		if (dataNo == 0) {
			absCurPos = relToAbs();
			fileDataClusterNo--;
			return bytesRead;
		}// Nema narednog data klastera, kraj fajla
		//fileDataClusterNo++; // Tek sada sigurno moze da se poveca!

		BytesCnt curRead;
		if (toRead > ClusterSize) {
			KernelFs::getMounted()->readCluster(dataNo, buffer + bytesRead);
			curRead = ClusterSize;
		} else {
			// Uporedi dataNo sa ClusterNo poslednjeg i ako su isti postavi size koji pise u dirEntry
			curFileData->load(dataNo); // Ucitavanje novog data klastera

			// Treba da se proveri da li ucitani klaster poslednji u fajlu i ako jeste da se postavi njegov size
			if (curFileData->getClusterNo() == dirEntry->lastClusterNo) curFileData->setSize(dirEntry->lastClusterSize);

			curRead = curFileData->read(0, toRead, buffer + bytesRead); // Broj procitanih bajtova u ovoj iteraciji, pocinje od nule jer je dovucen novi data klaster
		}

		relCurPos = curRead; // Jer se staje na kraj procitanog sadrzaja a ne posle zato minus jedan
		bytesRead += curRead;
		toRead -= curRead;
	}

	// Azuriranje apsolutna pozicija na osnovu relativne
	absCurPos = relToAbs();
	return bytesRead; // Read vraca broj procitanih bajtova
}

char KernelFile::seek(BytesCnt newAbsPos) { // newAbsPos == size za trazenje kraja fajla

	if (newAbsPos > size) return 0;

	ClusterNo newFileDataClusaterNo, newIndexTwoNo;
	newFileDataClusaterNo = newAbsPos / ClusterSize; // U kom klasteru za podatke se nalazi nova pozicija
	// (ne broj klastera nego redni broj data klastera u CELOM fajlu)
	newIndexTwoNo = newFileDataClusaterNo / indexEntries; // Redni broj indeksa drugog nivoa u kome se nalazi trazeni data klaster
	// (u kom ulazu indeksa prvog nivoa se nalazi)
	newFileDataClusaterNo %= indexEntries; // Novi redni broj data klastera U INDEKSU DRUGOG NIVOA (u kom ulazu indeksa drugog nivoa se nalazi)

	BytesCnt newRelPos = newAbsPos % ClusterSize; // Pozicija u trazenom data klasteru
	
	if (newIndexTwoNo != indexTwoNo) { // Ako trazeni indeks drugog nivoa nije ucitan, ucitaj ga
		iTwo->load(iOne->getEntry(newIndexTwoNo));
		indexTwoNo = newIndexTwoNo;

		// Ako je ucitan novi indeks drugog nivoa svakako mora se ucita novi data klaster

		curFileData->load(iTwo->getEntry(newFileDataClusaterNo));
		//if (curFileData->getClusterNo() == dirEntry->lastClusterNo) curFileData->setSize(dirEntry->lastClusterSize);
		fileDataClusterNo = newFileDataClusaterNo;

	} else { // Trazeni indeks drugog nivoa je vec ucitan
		if (newFileDataClusaterNo != fileDataClusterNo) { // Ako trazeni data klaster nije ucitan, ucitaj ga
			curFileData->load(iTwo->getEntry(newFileDataClusaterNo));
			//if (curFileData->getClusterNo() == dirEntry->lastClusterNo) curFileData->setSize(dirEntry->lastClusterSize);
			fileDataClusterNo = newFileDataClusaterNo;
		} // Ako jeste nista
	}

	absCurPos = newAbsPos;
	relCurPos = newRelPos;

	if (curFileData->getClusterNo() == dirEntry->lastClusterNo) curFileData->setSize(dirEntry->lastClusterSize);

	return 1; // Uspesno obavljanje
}

/*char KernelFile::seekEof(BytesCnt newAbsPos) {
	seek(absCurPos);
	/*if (newAbsPos != size || size == 0) return 1; // Moze da se poziva akko je nova pozicija jednaka velicini fajla

	if (size % ClusterSize == 0) { // Posebna verzija metode seek se poziva samo kad je velicina na granici klastera

		ClusterNo newFileDataClusaterNo, newIndexTwoNo;
		newFileDataClusaterNo = newAbsPos / ClusterSize; // U kom klasteru za podatke se nalazi nova pozicija
		// (ne broj klastera nego redni broj data klastera u CELOM fajlu)
		newIndexTwoNo = newFileDataClusaterNo / indexEntries; // Redni broj indeksa drugog nivoa u kome se nalazi trazeni data klaster
		// (u kom ulazu indeksa prvog nivoa se nalazi)
		newFileDataClusaterNo %= indexEntries; // Novi redni broj data klastera U INDEKSU DRUGOG NIVOA (u kom ulazu indeksa drugog nivoa se nalazi)

		BytesCnt newRelPos = newAbsPos % ClusterSize; // Pozicija u trazenom data klasteru (u ovom slucaju ce uvek biti nula)

		if (newIndexTwoNo == indexTwoNo + 1 || newFileDataClusaterNo == fileDataClusterNo + 1) { // Ovde ce uvek da se udje ali neka za svaki slucaj

			if (fileDataClusterNo == 511 && indexTwoNo == 511) return 1; // Ako je trenutni data klaster poslednji u trenutnom indeksu drugog nivoa
				// I trenutni indeks drugog nivoa je poslednji u indeksu prvog nivoa onda je dostignuta maksimalna velicina fajla GRESKA

			if (++fileDataClusterNo == 512) { // Ako je poslednji data klaster mora se ucita novi indeks drugog nivoa
				ClusterNo next = indexOne->getClusterNo(++indexTwoNo); // Uzmi naredni indeks drugog nivoa iz indeksa prvog nivoa

				if (next == 0) indexOne->getNextLevelIndex(&next); // Ako je u narednom ulazu indeksa prvog nivoa nula mora se alocira novi indeks dva
				curIndexTwo->load(next); // Ucitavanje novog indeksa drugog nivoa

				fileDataClusterNo = 0; // Naredni data klaster se nalazi u nultom ulazu novog indeksa drugog nivoa
			}

			ClusterNo dataNo = curIndexTwo->getClusterNo(fileDataClusterNo); // Trenutni ulaz indeksa drugog nivoa
			if (dataNo == 0) curIndexTwo->getNextLevelIndex(&dataNo); // Ako je nula mora se alocira novi data klaster

			curFileData->load(dataNo); // Ucitavanje novog data klastera
		}

		absCurPos = newAbsPos;
		relCurPos = newRelPos;

		return 0;

	} else
		return seek(newAbsPos); // Ako nije na granici okej je obican seek*/
//}

BytesCnt KernelFile::filePos() {
	return absCurPos;
}

char KernelFile::eof() {
	return absCurPos == size;
}

BytesCnt KernelFile::getFileSize() {
	return size;
}

char KernelFile::truncate() {

	size = absCurPos;
	dirEntry->lastClusterNo = curFileData->getClusterNo();
	dirEntry->lastClusterSize = relCurPos;
	//dirEntry->dirty = true;
	curFileData->setSize(relCurPos);

	if (fileDataClusterNo != 511) // Obrisi data nardene data klastere u trenutnom indeksu drugog nivoa
		for (int j = fileDataClusterNo + 1; j < 512; j++) {
			ClusterNo clusterNo = iTwo->getEntry(j);
			if (clusterNo) BitVector::freeCluster(clusterNo);
		}
			//curIndexTwo->freeFileDataCluster(j); // Posto su ulazi tipa file data klasteri nema nista drugo da se radi osim da se proglase slobodnim u 
		// bitVektoru i ulazi u indeks dva postave na nulu

	if (indexTwoNo == 511) return 1; // Ako je ovo poslednji indeks drugog nivoa brisanje je zavrseno

	for (int i = indexTwoNo + 1; i < 512; i++) { // Za sve naredne indekse drugog nivoa

		ClusterNo next = iOne->getEntry(i);
		if (next == 0) return 1; // Nema narednog indeksa drugog nivoa, brisanje je zavrseno

		IndexTwo* temp = new IndexTwo(next); // Ucitaj novi indeks drugog nivoa
			
		for (int j = 0; j < 512; j++) { // Oslobodi mu sve ulaze
			ClusterNo clusterNo = iTwo->getEntry(j);
			if (clusterNo) BitVector::freeCluster(clusterNo);
		}

		delete temp;
	}

	return 1; // Uspesno zavrseno
}
