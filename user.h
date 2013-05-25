#ifndef USERS_H
#define USERS_H

#include "coms.h"

typedef struct user
{
		struct sockaddr_in peer;
		char name[NAMELEN];
		char host[NAMELEN];
		char nickname[NAMELEN];
		int inUser; /* 用户是否使用的标志位 */
		int exit; /* 用户是否退出的标志 */
		struct user *next;
}user;

extern user userList; /* 用户链表 */

extern int insertUser(user *uList, user *target);
extern void destroyUsers(user *uList);
extern int listUsers(user **pusers, user *uList, int size, int flag);
extern int unListUsers(user **pusers, int num);
extern int delUser(user *uList, command *peercom);

#endif /* user.h */

