#include "helperFunctions.h"
#include <cstring>

char getFileNameAndExt(char* fname, char* fileName, char* ext) {
	// Popunjava null karakterima na pocetku
	for (int i = 0; i < 9; fileName[i++] = '\0');
	for (int i = 0; i < 4; ext[i++] = '\0');

	int i;

	for (i = 1; fname[i] != '.'; i++) fileName[i - 1] = fname[i];
	for (int j = 0, k = ++i; fname[k] != '\0'; j++, k++) ext[j] = fname[k];

	return 0;
}

char* makeFilePath(char* filename, char* ext) {
	unsigned short nameSize = strlen(filename), extSize = strlen(ext); // Bez NULL znaka
	char* path = new char[nameSize + extSize + 3]; // 3 jer jedan za / jedan za . i jedan za NULL
	strcpy(path, "/");
	strcat(path, filename);
	strcat(path, ".");
	strcat(path, ext);
	return path;
}
