/*************************************************
*user.c
*
*
*
*用户添加、删除和显示
*************************************************/

#include "user.h"
#include <arpa/inet.h>

user userList;

int insertUser(user *uList, user *target)
{
		user *pre, *cur;
		int compName, compHost, compAddr;

		if ((uList == NULL) || (target == NULL))
				return -1;
		if (target->peer.sin_addr.s_addr == htonl(INADDR_ANY))
				return -1;

		pre = uList;
		cur = uList->next;
		/* 链表的排列顺序是ASCII由小到大 */
		while (cur != NULL)
		{
				compName = strncmp(cur->name, target->name, 
									sizeof(cur->name));
				compHost = strncmp(cur->host, target->host,
									sizeof(cur->host));
				compAddr = memcmp(&cur->peer, &target->peer,
									sizeof(cur->peer));

				if (compName > 0)
						break;
				/* 是否己经存在 */
				if ((compName == 0) && (compHost == 0) && (compAddr == 0))
				{
						if (cur->exit == 1)
								cur->exit = 0;
						return -1;
				}

				pre = cur;
				cur = cur->next;
		}
		target->next = pre->next; /* 链表插入 */
		pre->next = target;

		return 0;
}

void destroyUser(user *uList)
{
		user *cur;

		if (uList == NULL)
				return ;

		while ((cur = uList->next) != NULL)
		{
				uList->next = cur->next;
				free(cur);
		}
}

int listUsers(user **pusers, user *uList, int size, int flag)
{
		user *cur;
		int order = 0;
		const char name[] = "Name";
		const char host[] = "Hostname";
		const char ip[] = "Ip";

		if ((pusers == NULL) && (flag == 0))
				return 0;

		if (flag == 0)
				bzero(pusers, sizeof(user *) * size);

		printf("\n*****************************************************\n");
		printf("   %-22s%-25s%-15s\n", name, host, ip);

		cur = uList->next;
		/* 遍历所有用户 */
		while ((cur != NULL) && (size > 1))
		{
				if (cur->exit == 0)
				{
						printf("%-3d%-22s%-25s%-15s\n", ++order, cur->name,
								cur->host, inet_ntoa(cur->peer.sin_addr));

						if (flag == 0) /* 选择用户时使用 */
						{
								cur->inUser = 1;
								*pusers++ = cur;
								size--;
						}
				}
				cur = cur->next;
		}
		printf("******************************************************\n");

		if (order == 0)
				printf("\nNo other users found.\nPlease refresh(rf) or "
							"list(ls)\n");
		return order;

}

int unListUsers(user **pusers, int num)
{
		user *cur;
		int tmp = 0;

		if (pusers == NULL)
				return -1;

		while (num-- > 0)
		{
				cur = *pusers++;
				if (cur != NULL)
				{
						cur->inUser = 0; /* 未使用的用户 */
						tmp++;
				}
		}
		return tmp;
}

int delUser(user *uList, command *peercom)
{
		user *cur;
		int compName, compHost, compAddr;

		if (uList == NULL)
				return -1;
		cur = uList->next;

		while (cur != NULL)
		{
				compName = strncmp(cur->name, peercom->senderName,
									sizeof(cur->name));
				compHost = strncmp(cur->host, peercom->senderHost,
									sizeof(cur->host));
				compAddr = memcmp(&cur->peer, &peercom->peer,
									sizeof(cur->peer));

				if (compName > 0)
						return -1;

				if ((compName == 0) && (compHost == 0) && (compAddr == 0))
				{
						cur->exit = 1;
						break;
				}
				
				cur = cur->next;
		}
		return 0;
}
