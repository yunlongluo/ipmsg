#ifndef COMMAND_H
#define COMMAND_H

#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#define NAMELEN     50
#define MSGLEN      1000
#define COMLEN      1500
#define RECFRG      1448
#define HSIZE       10
#define FILENAME    128
#define MSGLIMIT    100
#define HL_HEADERSIZE 4
#define HL_FILESIZE  9
#define HL_FILETYPE  1
#define HL_1416      11
#define CAPACITY     50

#define IMHELP \
		"COMMANDS: help(h) list(ls) talk(tk) sendfile(sf)\n"\
		"COMMANDS: getfile(gf) refresh(rf) ceasesend (cs) quit(q)\n"

typedef struct command 
{
		unsigned int version;
		unsigned int packetNo;
		char         senderName[NAMELEN];
		char         senderHost[NAMELEN];
		unsigned int commandNo;
		char         additional[MSGLEN];
		struct sockaddr_in peer;
		struct command *next; /* 链表 */
}command;

typedef struct msgList 
{
		command comHead;
		command *comTail;
}msgList; /* 消息队列 */

extern const char allHosts[];
extern int msgSock;
extern int tcpSock;
extern struct passwd *pwd;
extern struct utsname sysName;
extern char workDir[FILENAME];
extern int utf8;

extern msgList mList;

extern pthread_mutex_t msgMutex;
extern pthread_mutex_t usrMutex;
extern sem_t waitNonEmpty, waitNonFull;

extern int msgParser(char *msg, int size, command *com);
extern int msgCreater(char *msg, command *com, size_t msgLen);
extern void initCommand(command *com, unsigned int flag);
extern void deCommand(command *com);

#endif /* command.h */
