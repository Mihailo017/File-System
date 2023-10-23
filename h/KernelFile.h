#pragma once

#include "part.h"
#include "fs.h"
#include "IndexOne.h"
#include "IndexTwo.h"
#include "FileData.h"
#include "DirEntry.h"

const unsigned short indexEntries = 512;

class KernelFile {

	//Index* indexOne, *curIndexTwo;
	DirEntry* dirEntry;
	IndexOne* iOne;
	IndexTwo* iTwo;
	ClusterNo indexTwoNo; // Ne broj klastera indeksa dva vec redni broj u fajlu (redni broj ulaza u klasteru prvog nivoa)
	FileData* curFileData;
	ClusterNo fileDataClusterNo; // Ne broj klastera data klastera vec redni broj u fajlu (redni broj ulaza u klasteru drugog nivoa)
	BytesCnt size, absCurPos, relCurPos; // curentPos se odnosi na apsolutnu poziciju u fajlu, a relCurPos na poziciju u curFileData klasteru

	BytesCnt relToAbs();

public:
	KernelFile(DirEntry* dirEntry, char mode);
	~KernelFile(); //zatvaranje fajla

	char write(BytesCnt toWrite, char* buffer);
	BytesCnt read(BytesCnt toRead, char* buffer);

	char seek(BytesCnt newAbsPos);
	BytesCnt filePos();
	char eof();

	BytesCnt getFileSize();
	char truncate();
};