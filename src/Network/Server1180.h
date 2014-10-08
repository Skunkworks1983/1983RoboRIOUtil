/* Here is an encapsulation of the logic for collecting data over
 * a Server at port 1180 from a client out there somewhere.
 * Somewhere could be the driver station, where this code was
 * tested, an accessory computer at the driver station, or on
 * the robot itself.  The stated intention is to perform vision
 * processing on a computer more powerful than the cRio, and pass
 * the results into the main robot code.  The drive to make this
 * code be the server is to make use of the roborealm builtin 
 * client code.  So I should have read the roborealm document
 * earlier.  Live and learn.  
 * TFD February 2012
 */

#ifndef SERVER1180_H
#define SERVER1180_H 1

#include "WPILib.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUFLEN 512
#define PORT 1180

typedef void (*PacketHandler)(void *ptr, char *data);
class Server1180
{

public:
	Server1180 (PacketHandler handler, void * inst, bool=false); // Constructs listener socket and so on.
	~Server1180 (); // if argument is true then cerr calls will spew.

	int SendData (char *); // Send null-terminated string preferably < 512 bytes.
	int GetData (char *); // Return number of bytes copied into pointer, 
	// zero for for nothing new.
	bool Connected (); // Return connection status in case you're interested.

	void ClientConnect(); // Listen for client if we don't have one yet. 
	// Has to be public so taskSpawn will work.
private:
	PacketHandler handler;
	void * handlerObj;
	bool spewcerr; // Set in constructor to spew info to cerr.
	int listener; // Our listening socket.
	int connsock; // Our connected client.  Just one.
	char data[BUFLEN+1]; // Leave space for a null, always.
	int numbytes; // Number of bytes to copy in when GetData called.
	int tid; // Spawned task id, needed to suspend when writing.
};

#endif
