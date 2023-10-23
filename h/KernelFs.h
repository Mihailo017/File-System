#pragma once

#include "part.h"
#include "fs.h"
#include "Root.h"
#include<windows.h>

#define signalSem(x) ReleaseSemaphore(x,1,NULL)
#define waitSem(x) WaitForSingleObject(x,INFINITE)

class KernelFile;

class KernelFs {
private:
	KernelFs();
	static Partition* mounted;
	static bool formatting;
	static HANDLE mutex;
	static HANDLE mountingSem;
	static int mountingWait;
	static HANDLE formattingSem;
	static int formattingWait;
	static HANDLE unmountingSem;
	static int unmountingWait;
public:

	static Partition* getMounted();

	static char mount(Partition* partition); //montira particiju
	// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha

	static char unmount(); //demontira particiju
	// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha

	static char format(); //formatira particiju;
	// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha

	static FileCnt readRootDir();
	// vraca -1 u slucaju neuspeha ili broj fajlova u slucaju uspeha

	static char doesExist(char* fname); //argument je naziv fajla sa
	//apsolutnom putanjom

	static KernelFile* open(char* fname, char mode);
	static char deleteFile(char* fname);

	static char allFilesClosed();
};

