/*
 * Herein implement the Server1180 class.  Hide all the essential details.
 * We have to play games with static addresses in order to get this program
 * using a spawned task so it will always read data when it's available.
 * The key to this spawned task is that both accept and read block while
 * waiting for a connection or data to come over the socket.  Both activities
 * break when something happens.
 */

#include "Server1180.h"

Server1180 *Sok;

void doblocked()
{
	Sok->ClientConnect(); // Never leaves.
}

Server1180::Server1180(PacketHandler tHandler, void * ptr, bool showstuff)
{
	handlerObj = ptr;
	handler = tHandler;
	spewcerr = showstuff;
	Sok = this;
	listener = -1;
	connsock = -1;
	numbytes = 0;

	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ // Should never fail.
		if (spewcerr)
			cerr<<"Server listener create Error"<<endl;
		return;
	}

	struct sockaddr_in sin;
	memset((char *) &sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	int one;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char *) &one, sizeof (one));

	// bind can fail if socket is still out there listening.
	// This scenario happens when loading new code without reboot.
	if (bind(listener, (struct sockaddr *) &sin, sizeof(sin)) == -1)
	{
		if (spewcerr)
			cerr<<"Bind failed must still be running from before."<<endl;
		return;
	}

	listen(listener, 1); // Listen for just one connection.

	if ((tid = taskSpawn("SocketComm", 100, 0, 10000, (FUNCPTR) doblocked, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0)) == ERROR)
		if (spewcerr)
			cerr<<"Task creation error do_often Task "<<errno<<endl;
}

// Destructor.  Doubtful that it ever gets called.

Server1180::~Server1180()
{
	taskDelete(tid); // Kill the waiting sockets.
	close(connsock);
	close(listener);
}

// Your application calls this function to get the latest data only once.
// Once read, the data is made unavailable on the assumption you want to
// only get one round of any particular data version.

int Server1180::GetData(char *p)
{
	strncpy(p, data, numbytes);
	p[numbytes]=0;
	int retbytes = numbytes;
	numbytes = 0; // Until we get new data.
	return retbytes;
}

// In case you want to send data back.  Note that you have to suspend
// the task in order to break the block on read().

int Server1180::SendData(char *p)
{
	if (!Connected())
		return 0;
	taskSuspend(tid);
	int writ= write(connsock, p, strlen(p));
	taskResume(tid);
	return writ;
}

// Simplistic test for connected socket.

bool Server1180::Connected()
{
	if (connsock == -1)
		return false;
	return true;
}

// Infinite loop entered with taskSpawn, kind of like a thread.

void Server1180::ClientConnect()
{
	if (spewcerr)
		cerr<<"Client connect top only see this message once"<<endl;

	while (1)
	{
		if (spewcerr)
			cerr<<"while 1 see this every round connsock "<<connsock<<endl;
		if (connsock == -1)
		{
			struct sockaddr remoteaddr;
			int rsize = sizeof(remoteaddr);
			connsock = accept(listener, &remoteaddr, &rsize);
			if (spewcerr)
				cerr<<"Accepted connsock "<<connsock<<endl;
		} else
		{
			int ret = read(connsock, data, BUFLEN);

			if (ret < 0)
				connsock = -1;

			else if (ret == 0) // Other end hung up.
			{
				close(connsock);
				connsock = -1;
				if (spewcerr)
					cerr<<"Hang UP"<<endl;
			}

			else
			{
				data[ret]=0;
				if (spewcerr)
					cerr<<"Got "<<ret<<" bytes: "<<data<<endl;
				numbytes = ret;
				//Got data, if handler is good, handle!
				if (handler != NULL)
				{
					char * p = new char[numbytes];
					if (GetData(p) > 0)
					{
						handler(handlerObj, p);
					}
				}
			}
		}
	}
	return; // Must have succeeded. Never get here.
}
