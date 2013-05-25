/*********************************************
 *coms.c
 *
 *
 *
 *主要数据结构、全局变量和包的生成和解析
 ********************************************/

#include "ipmsg.h"
#include "coms.h"
#include "encode.h"

#include <pthread.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <assert.h>

const char allHosts[] = "255.255.255.255";
int msgSock;
int tcpSock;
struct passwd *pwd;
struct utsname sysName;
char workDir[FILENAME];
int utf8;
msgList mList;

pthread_mutex_t msgMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t usrMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t waitNonEmpty, waitNonFull;

/* 生成发送的数据包 */
int msgCreater(char *msg, command *com, size_t msgLen)
{
		int len, tmp;
		char dest[COMLEN]; //是否多余？？

		if ((dest == NULL) || (com == NULL))
				return -1;

		len = snprintf(dest, sizeof(dest), "%d:%d:%s:%s:%d:%s",
						com->version,
						com->packetNo,
						com->senderName,
						com->senderHost,
						com->commandNo,
						com->additional);
		tmp = sizeof(dest) - len - 1;  /* dest[]中剩余空间 */

		if (com->commandNo & IPMSG_FILEATTACHOPT)
		{
				//
		}

		if (utf8)
				u2g(dest, sizeof(dest), msg, msgLen);
		else
				strncpy(msg, dest, msgLen);

		return 0;

}

/* 解析接收到的字符串 */
int msgParser(char *msg, int size, command *com)
{
		char *start;
		char *pos;
		char outStr[COMLEN];
		int min;
		int index = 0, colonCount;

		if ((msg == NULL) || (com == NULL))
				return -1;

		if (utf8)
		{
				g2u(msg, size, outStr, sizeof(outStr));
				start = outStr;
		}
		else 
				start = msg;
		
		
		while (index < 5)
		{
				pos = strchr(start, ':');
				if (pos == NULL)
						break;

				colonCount = 1;
				while (*(pos + 1) == ':')
				{
						pos++;
						colonCount++;
				}
				if (colonCount % 2 == 0)
				{
						start = pos + 1;
						continue;
				}

				switch (index)
				{
						case 0:
							com->version = atoi(start);
							break;
						case 1:
							com->packetNo = atoi(start);
							break;
						case 2:
							min = sizeof(com->senderName) < (pos - start) ?
									sizeof(com->senderName) : (pos - start);
							memcpy(com->senderName, start, min);
							com->senderName[min] = '\0';
							break;
						case 3:
							min = sizeof(com->senderHost) < (pos - start) ?
									sizeof(com->senderHost) : (pos - start);
							memcpy(com->senderHost, start, min);
							com->senderHost[min] = '\0'; 
							break;
						case 4:
							com->commandNo = atoi(start);
							break;
						default:
							break;
				}
				start = pos + 1;
				index++;
		}

		strncpy(com->additional, start, MSGLEN);
		if (com->commandNo & IPMSG_FILEATTACHOPT)
		{
				//
		}

		index++;

		return index;
}

void initCommand(command *com, unsigned int flag)
{
		if (com == NULL)
				return ;
		bzero(com, sizeof(com));
		com->version = 1;
		com->packetNo = (unsigned int)time(NULL);
		strncpy(com->senderName, pwd->pw_name, sizeof(com->senderName));
		strncpy(com->senderHost, sysName.nodename, sizeof(com->senderHost));
		com->commandNo = flag;

}

void deCommand(command *com)
{

}
