/*************************************************
 *Filename: main.c
 *Author:   camel
 *Date:     5/8/2013
 *
 *function: 主程序、各个线程
 ************************************************/

#include "user.h"
#include "ipmsg.h"
#include "send_receive.h"
#include "coms.h"
#include "utils.h"
#include <langinfo.h>
#include <locale.h>
#include <pthread.h>
#include <pwd.h>
#include <sys/utsname.h>

void destroyer();

/* 用户输入 */
void *interacter(void *option)
{
		char com[20], *filename;
		int count;

		while (1)
		{
				printf("\nipmsg:");
				fgets(com, sizeof(com), stdin);

				transfStr(com, 1);

				if (!strcmp(com, "list") || !strcmp(com, "ls"))
				{
						pthread_mutex_lock(&usrMutex);
						count = listUsers(NULL, &userList, 2, 1);
						pthread_mutex_unlock(&usrMutex);
				}
				else if (!strcmp(com, "quit") || !strcmp(com, "q"))
				{
						logout();
						destroyer();
						exit(0);
				}
				else if (!strcmp(com, "refresh") || !strcmp(com, "rf"))
				{
						login();
						pthread_mutex_lock(&usrMutex);
						count = listUsers(NULL, &userList, 2, 1);
						pthread_mutex_unlock(&usrMutex);
				}
				else if (!strcmp(com, "talk") || !strcmp(com, "tk"))
				{
						saySth();
				}
				else if (!strcmp(com, "sendfile") || !strcmp(com, "sf"))
				{
			//			selectFiles();
				}
				else if (!strcmp(com, "getfile") || !strcmp(com, "gf"))
				{
			//			recvFiles();
				}
				else if (!strcmp(com, "ceaseSend") || !strcmp(com, "cs"))
				{
			//			ceaseSend();
				}
				else if (!strcmp(com, "help") || !strcmp(com, "h"))
				{
						printf(IMHELP);
				}
		}
}

/* 接受udp数据包 */
void *receiver(void *option)
{
		command *peercom;
		struct sockaddr_in peer;
		int msock = *(int *)option;
		socklen_t len;
		char buf[COMLEN];

		while (1)
		{
				if (recvfrom(msock, buf, sizeof(buf), 0, 
								(struct sockaddr *)&peer, &len) < 0)
						continue;
				
				peercom = (command *)malloc(sizeof(command));
				if (peercom == NULL)
				{
						perror("malloc peercom");
						exit(1);
				}
			
				bzero(peercom, sizeof(command));
			
				if (msgParser(buf, sizeof(buf), peercom) < 0)
				{
						perror("msgParser error");
						exit(1);
				}
				memcpy(&peercom->peer, &peer, sizeof(peercom->peer));

				sem_wait(&waitNonFull);
				pthread_mutex_lock(&msgMutex);

				mList.comTail->next = peercom; /* 把新结点peercom赋值给 */
											   /* 原队尾节点的后继 */
				mList.comTail = peercom; /* 把当前的peercom设置为队尾结点*/
 										 /* 入队操作 */
				sem_post(&waitNonEmpty);
				pthread_mutex_unlock(&msgMutex);
		}
}

/* 处理udp数据包 */
void *processor(void *option)
{
		command *peercom, com;
		int comMode, comOpt, temp;
		int len;
		user *cur;

		initCommand(&com, IPMSG_NOOPERATION);
		while (1)
		{
				sem_wait(&waitNonEmpty);
				pthread_mutex_lock(&msgMutex);

				peercom = mList.comHead.next; /* 出队操作 */
				mList.comHead.next = mList.comHead.next->next;
				if (mList.comHead.next == NULL)
						mList.comTail = &mList.comHead;

				sem_post(&waitNonFull);
				pthread_mutex_unlock(&msgMutex);

				memcpy(&com.peer, &peercom->peer, sizeof(com.peer));

				comMode = GET_MODE(peercom->commandNo);
				comOpt = GET_OPT(peercom->commandNo);

				if (comOpt & IPMSG_SENDCHECKOPT) 
				{
						com.packetNo = (unsigned int)time(NULL);
						snprintf(com.additional, MSGLEN, "%d", 
						          peercom->packetNo);
						com.commandNo = IPMSG_RECVMSG;
						sendMsg(&com); /* 发送确认信号 */
				}

				switch (comMode)
				{
						case IPMSG_SENDMSG:
							if (strlen(peercom->additional) > 0)
							{
									printf("\nGet message from: %s(%s)\n", 
									        peercom->senderName, 
											peercom->senderHost);
									puts(peercom->additional);
							}
							if (comOpt & IPMSG_FILEATTACHOPT)
							{
									//
									//
							}
							break;
						case IPMSG_ANSENTRY:
							cur = (user *)malloc(sizeof(user));
							memcpy(&cur->peer, &peercom->peer, 
									sizeof(cur->peer));
							strncpy(cur->name, peercom->senderName, 
									NAMELEN);
							strncpy(cur->host, peercom->senderHost,
									NAMELEN);
							strncpy(cur->nickname, peercom->additional,
									NAMELEN);
							cur->inUser = 0;
							cur->exit = 0;
							cur->next = NULL;

							pthread_mutex_lock(&usrMutex);
							if (insertUser(&userList, cur) < 0)
									free(cur);
							pthread_mutex_unlock(&usrMutex);
							break;
						case IPMSG_BR_ENTRY:
							com.packetNo = (unsigned int)time(NULL);
							com.commandNo = IPMSG_ANSENTRY;
							strncpy(com.additional, pwd->pw_name, MSGLEN);
							sendMsg(&com);

							cur = (user *)malloc(sizeof(user));
							memcpy(&cur->peer, &peercom->peer, 
									sizeof(cur->peer));
							strncpy(cur->name, peercom->senderName, 
									NAMELEN);
							strncpy(cur->host, peercom->senderHost,
									NAMELEN);
							strncpy(cur->nickname, peercom->additional,
									NAMELEN);
							cur->inUser = 0;
							cur->exit = 0;
							cur->next = NULL;

							pthread_mutex_lock(&usrMutex);
							if (insertUser(&userList, cur) < 0)
									free(cur);
							pthread_mutex_unlock(&usrMutex);
							break;
						case IPMSG_RECVMSG:
							//
							break;
						case IPMSG_RELEASEFILES:
							//
							//
							break;
						case IPMSG_BR_EXIT:
							pthread_mutex_lock(&usrMutex);
							delUser(&userList, peercom);
							pthread_mutex_unlock(&usrMutex);
							break;
						case IPMSG_NOOPERATION:
							//
							break;
						default:
							printf("\nno handle, %x\n", peercom->commandNo);
							break;
				}
				//deCommand(peercom);
				free(peercom);
				peercom = NULL;
		}
}

/* 数据清理 */
void destroyer()
{
		user *curUsr, *preUsr;

		//
		//
		preUsr = &userList;
		pthread_mutex_lock(&usrMutex);
		curUsr = userList.next;
		while (curUsr != NULL)
		{
				if ((curUsr->exit == 1) && (curUsr->inUser == 0))
				{
						preUsr->next = curUsr->next;
						free(curUsr);
				}
				else 
						preUsr = preUsr->next;

				curUsr = preUsr->next;
		}
		pthread_mutex_unlock(&usrMutex);
}

/* 半分钟清理一次 */
void *cleaner(void *option)
{
		while (1)
		{
				sleep(30);
				destroyer();
		}
}

/* 初始化udp和tcp */
void initSvr()
{
		struct sockaddr_in server;
		char targetHost[NAMELEN];
		const int on = 1;

		msgSock = socket(AF_INET, SOCK_DGRAM, 0);
		tcpSock = socket(AF_INET, SOCK_STREAM, 0);

		server.sin_family = AF_INET;
		server.sin_port = htons(IPMSG_DEFAULT_PORT);
		server.sin_addr.s_addr = htonl(INADDR_ANY);

		if (setsockopt(msgSock, SOL_SOCKET, SO_BROADCAST, 
				&on, sizeof(on)) < 0)
		{
				perror("socket option");
				exit(1);
		}

		if (bind(msgSock, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
				perror("udp socket");
				exit(1);
		}

		if (bind(tcpSock, (struct sockaddr *)&server, sizeof(server)) < 0)
		{
				perror("tcp socket");
				exit(1);
		}

		if (listen(tcpSock, 10) < 0)
		{
				perror("tcp listen");
				exit(1);
		}

		printf(IMHELP);
}

int main()
{
		pthread_t procer, recver, iter, fler, cler;
		
		uname(&sysName);
		pwd = getpwuid(getuid());
		getcwd(workDir, sizeof(workDir));

		utf8 =0;
		if (setlocale(LC_CTYPE, ""))
				if (!strcmp(nl_langinfo(CODESET), "UTF-8"))
						utf8 = 1;

		initCommand(&mList.comHead, IPMSG_NOOPERATION);
		//
		mList.comTail = &mList.comHead;
		userList.next = NULL;
		
		sem_init(&waitNonEmpty, 0, 0);
		sem_init(&waitNonFull, 0, MSGLIMIT);

		initSvr();
		
		pthread_create(&procer, NULL, &processor, &msgSock);
		pthread_create(&recver, NULL, &receiver, &msgSock);
		pthread_create(&iter, NULL, &interacter, NULL);
		pthread_create(&cler, NULL, &cleaner, NULL);
		login();

	/*	while(1)
		{
				//
				//
				//
		} */


		pthread_join(procer, NULL);
		pthread_join(recver, NULL);
		pthread_join(iter, NULL);
		pthread_join(cler, NULL);

		return 0;
}
