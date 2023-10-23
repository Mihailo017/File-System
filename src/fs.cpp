#include "fs.h"
#include "File.h"
#include "KernelFs.h"

char FS::mount(Partition* partition) {
	return KernelFs::mount(partition);
}

char FS::unmount() {
	return KernelFs::unmount();
}

char FS::format() {
	return KernelFs::format();
}

FileCnt FS::readRootDir() {
	return KernelFs::readRootDir();
}

char FS::doesExist(char* fname) {
	return KernelFs::doesExist(fname);
}

File* FS::open(char* fname, char mode) {
	File* ret = new File();
	ret->myImpl = KernelFs::open(fname, mode);
	if (!ret->myImpl) {
		delete ret;
		return nullptr;
	}
	return ret;
}

char FS::deleteFile(char* fname) {
	return KernelFs::deleteFile(fname);
}
