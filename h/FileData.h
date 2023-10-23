#pragma once

#include "Cluster.h"
#include "fs.h"
#include "part.h"



class FileData : public Cluster {

private:

	BytesCnt size;

public:

	FileData(ClusterNo clusterNo, bool clear = false);

	BytesCnt write(BytesCnt start, BytesCnt size, char* buffer);
	BytesCnt read(BytesCnt start, BytesCnt size, char* buffer);

	char load(ClusterNo clusterNo);
	char free() override;

	char setSize(BytesCnt newSize);
	BytesCnt getSize();

};

