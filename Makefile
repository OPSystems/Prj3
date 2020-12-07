all: Client Server DirClient DirServer DiskClient DiskClientRandom DiskServer FileClient FileServer DirectoryClient DirectoryServer

Client: Client.cpp
	g++ Client.cpp -o Client

Server: Server.cpp
	g++ Server.cpp -o Server -lpthread

DirClient: DirClient.cpp
	g++ DirClient.cpp -o DirClient

DirServer: DirServer.cpp
	g++ DirServer.cpp -o DirServer -lpthread

DiskClient: DiskClient.cpp
	g++ DiskClient.cpp -o DiskClient

DiskClientRandom: DiskClientRandom.cpp
	g++ DiskClientRandom.cpp -o DiskClientRandom

DiskServer: DiskServer.cpp
	g++ DiskServer.cpp -o DiskServer -lpthread

FileClient: FileClient.cpp
	g++ FileClient.cpp -o FileClient

FileServer: FileServer.cpp
	g++ FileServer.cpp -o FileServer -lpthread

DirectoryClient: DirectoryClient.cpp
	g++ DirectoryClient.cpp -o DirectoryClient

DirectoryServer: DirectoryServer.cpp
	g++ DirectoryServer.cpp -o DirectoryServer -lpthread