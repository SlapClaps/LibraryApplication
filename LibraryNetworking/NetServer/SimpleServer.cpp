#include "SimpleServer.h"

int main()
{
	CustomServer server(60000);
	server.Start();
	server.myLibrary.addMember("Kevin Johansen", "1", "legendo", "hunter123");
	server.myLibrary.addLibrarian("Jonathan Krogstad", "2", "samba", "leker321");
	while (1)
	{
		server.Update(-1, true);
	}
	return 0;
}