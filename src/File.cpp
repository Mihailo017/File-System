#include "File.h"
#include "KernelFile.h"

File::File() {
	myImpl = nullptr;
}

File::~File() {
	delete myImpl;
}
char File::write(BytesCnt size, char* buffer) {
	return myImpl->KernelFile::write(size, buffer);
}

BytesCnt File::read(BytesCnt size, char* buffer) {
	return myImpl->KernelFile::read(size, buffer);
}

char File::seek(BytesCnt pos) {
	return myImpl->KernelFile::seek(pos);
}

BytesCnt File::filePos() {
	return myImpl->KernelFile::filePos();
}

char File::eof() {
	return myImpl->KernelFile::eof();
}

BytesCnt File::getFileSize() {
	return myImpl->KernelFile::getFileSize();
}

char File::truncate() {
	return myImpl->KernelFile::truncate();
}
