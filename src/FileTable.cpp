#include "FileTable.h"
#include "Root.h"
#include "KernelFs.h"

std::map<std::string, TableEntry> FileTable::fileTable;
HANDLE FileTable::mutex = CreateSemaphore(NULL, 1, 32, NULL);

DirEntry* FileTable::fileTableOpen(char* filename, char mode) {
	/*std::string name(filename);
	if (fileTable.find(name) == fileTable.end()) { // Ovo oznacava da ga nema u tabeli pa mora da ga ucita iz Root
		TableEntry entry;
		entry.mode = mode;

		DirEntry* ret = Root::open(filename, mode);
		if (ret) entry.dirEntry = new DirEntry(ret);
		// EXCEPTION !!!
		else return nullptr;

		fileTable[name] = entry;
		return fileTable[name].dirEntry; // PROVERI OVO !!!
	} else { // Ako ga ima u tabeli tj vec je otvoren, blokiraj se dok ga taj neko ne zatvori
		TableEntry entry = fileTable[name];
		entry.waiting++;
		waitSem(entry.sem);
	}
	return nullptr;*/
	std::string name(filename);
	TableEntry tableEntry;
	waitSem(mutex);
	if (fileTable.find(name) != fileTable.end()) { // Ako je fajl vec u tabeli (otvoren je)
		tableEntry = fileTable[name];
		tableEntry.waiting++;
		fileTable[name] = tableEntry;
		signalSem(mutex);
		waitSem(tableEntry.sem); // Blokiraj se

		// Posto je odgovarajuci ulaz vec u tabeli samo podesi mode i tjt
		tableEntry = fileTable[name];
		tableEntry.mode = mode;
		fileTable[name] = tableEntry;
		//return tableEntry.dirEntry;
	} else { // Fajl nije u tabeli (nije otvoren) mora se napravi novi ulaz u tabelu
		//TableEntry entry; // Napravi novi ulaz
		//tableEntry = entry; // Jer je tableEntry povratna vrednost funkcije
		tableEntry.mode = mode; // Postavi mode, semafori  i brojac se prave u konstruktoru

		DirEntry* ret = Root::open(filename, mode); // Dohvati dirEntry u Root za ovaj fajl (kreirace ga ako ne postoji i mode je 'w')
		if (ret) tableEntry.dirEntry = new DirEntry(ret);
		// EXCEPTION !!!
		else {
			signalSem(mutex);
			return nullptr;
		}

		fileTable[name] = tableEntry; // Ubaci u tabelu
		//return tableEntry.dirEntry; // PROVERI OVO !!!
	}
	signalSem(mutex);
	return tableEntry.dirEntry;
}

char FileTable::fileTableClose(char* filename) {
	std::string name(filename);
	TableEntry tableEntry;
	waitSem(mutex);
	if (fileTable.find(name) != fileTable.end()) // Ako je u tabeli onda je otvoren i treba da se zatvori
		tableEntry = fileTable[name];
	else {
		signalSem(mutex);
		return 0; // Fajl nije u tabeli dakle nije ni bio otvoren
	}

	if (tableEntry.waiting) { // Ako neko ceka da otvori fajl, signaliziraj da moze
		tableEntry.waiting--;
		fileTable[name] = tableEntry;
		// Ovde se ne pusta mutex jer ce da preuzme onaj koji ceka na otvaranje i on ce da signalizira mutex kad bude izlazio iz funkcije
		signalSem(tableEntry.sem);
		return 2; // Izlaz jer nema potrebe da se postavlja ulaz u dir ili brise iz tabele posto ce fajl opet biti otvoren
	}

	// Ako niko ne ceka otvaranje fajla onda postavi ulaz u dir i izbaci iz tabele
	Root::setEntry(tableEntry.dirEntry);
	CloseHandle(tableEntry.sem); // Zatvaranje semafora
	delete tableEntry.dirEntry;
	fileTable.erase(name); // Brisanje iz tabele

	if (!fileTable.size()) KernelFs::allFilesClosed(); // Ako je zatvoren i poslednji fajl signaliziraj ako neko ceka na format ili unmount
	
	signalSem(mutex);
	return 1;
}

int FileTable::numOpenFiles() {
	return fileTable.size();
}
