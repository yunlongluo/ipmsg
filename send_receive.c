/*************************************************
*send_receive.c
*
*
*
*消息、文件（夹）的发送和接收 用户登录和退出
*************************************************/

#include "send_receive.h"
#include "ipmsg.h"
#include "coms.h"
#include "user.h"
#include "utils.h"

#include <pthread.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <arpa/inet.h>

/* 发送UDP包 */
void *sendMsg(command *com)
{
		char buf[COMLEN];
		int len, temp;

		msgCreater(buf, com, sizeof(buf));
		len = strlen(buf);

		if (com->commandNo & IPMSG_FILEATTACHOPT)
		{
				//
		}
		else
				sendto(msgSock, buf, len+1, 0, (struct sockaddr *)&com->peer
						, sizeof(com->peer));
}

void saySth()
{
		command com;
		user *cur;
		user *pusers[10];
		char buf[100];
		char *tips = "Please input user NO:";
		int count, who;

		printf("*Talking mode.\n"
				"*Continue enter to send message.\n"
				"*Ctrl+D to quit.\n");

		pthread_mutex_lock(&usrMutex);
		count = listUsers(pusers, &userList, 
							sizeof(pusers) / sizeof(pusers[0]), 0);
		pthread_mutex_unlock(&usrMutex);

		who = inputNo(1, count, tips);
		if (who > 0)
		{
				cur = pusers[who - 1];
				initCommand(&com, IPMSG_SENDMSG|IPMSG_SENDCHECKOPT);
				memcpy(&com.peer, &cur->peer, sizeof(com.peer));

				while (1)
				{
					printf("talking with %s:", cur->name);
					if (fgets(buf, sizeof(buf), stdin) == NULL)
							break;
					if (buf[0] == '\n')
					{
							printf("message can't null.\n");
							continue;
					}
					memcpy(com.additional, buf, sizeof(buf));
					com.packetNo = (unsigned int)time(NULL);
					sendMsg(&com);
					printf("Message sent.\n");
				}
		}
		puts("\nEnd conversation");

		pthread_mutex_lock(&usrMutex);
		unListUsers(pusers, count);
		pthread_mutex_unlock(&usrMutex);
}

void login()
{
		command com;

		initCommand(&com, IPMSG_BR_ENTRY);

		com.peer.sin_family = AF_INET;
		com.peer.sin_port = htons(IPMSG_DEFAULT_PORT);

		if (inet_pton(AF_INET, allHosts, &com.peer.sin_addr) < 0)
				printf("login: ip error\n");

		strncpy(com.additional, pwd->pw_name, MSGLEN);

		sendMsg(&com);
}

void logout()
{
		command com;

		initCommand(&com, IPMSG_BR_EXIT);

		com.peer.sin_family = AF_INET;
		com.peer.sin_port = htons(IPMSG_DEFAULT_PORT);

		if (inet_pton(AF_INET, allHosts, &com.peer.sin_addr) < 0)
				printf("logout: error.\n");

		strncpy(com.additional, pwd->pw_name, MSGLEN);

		sendMsg(&com);
		printf("bye!\n");
}
