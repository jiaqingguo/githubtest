/*************************************************************************
#	 FileName	: server.c
#	 Author		: fengjunhui 
#	 Email		: 18883765905@163.com 
#	 Created	: 2018年12月29日 星期六 13时44分59秒
 ************************************************************************/

#include<stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>

#include "common.h"

sqlite3 *db;  //仅服务器使用

void get_time( char *date)//获得时间
{
	time_t mytime;
	struct tm *mytm;
	mytime=time(NULL);//得到秒数
	mytm=localtime(&mytime);//得到当前的时间
	sprintf(date,"%04d-%02d-%02d  %02d:%02d:%02d",mytm->tm_year+1900,mytm->tm_mon+1,mytm->tm_mday,\
			mytm->tm_hour,mytm->tm_min,mytm->tm_sec);

}
void insert_history(MSG *msg,sqlite3 *db)
{
		char sql[DATALEN]={0};
		char *errmsg;
		char date[64]={0};
	//	char *word=msg->info.name;
		
		get_time(date);//获得当前的日期
		printf("%s\n",date);
		strcpy(msg->username,msg->info.name);

		//sprintf(sql,"insert into historyinfo values('%s','%s','%s');",date,msg->username,msg->recvmsg);
		sprintf(sql,"insert into historyinfo values (\"%s\",\"%s\",\"%s\");",date,msg->username,msg->recvmsg);
		//printf("%s\n",sql);
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
		{
			printf("8888888888\n");
			printf("%s\n",errmsg);
			return;
		}
		else{
			printf("历史记录生成\n");
		}
}

int process_user_or_admin_login_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	//封装sql命令，表中查询用户名和密码－存在－登录成功－发送响应－失败－发送失败响应	
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;

	msg->info.usertype =  msg->usertype;
	strcpy(msg->info.name,msg->username);
	strcpy(msg->info.passwd,msg->passwd);
	
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.name,msg->info.passwd);
	sprintf(sql,"select * from usrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.name,msg->info.passwd);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);		
	}else{
		//printf("----nrow-----%d,ncolumn-----%d.\n",nrow,ncolumn);		
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;	
}

int process_user_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char *errmsg;
	switch(msg->flags)
	{
		case 1:
			//printf("%s",msg->username);
		//	printf("%s",msg->username);
			sprintf(sql,"update usrinfo set name='%s' where name='%s';",msg->info.name,msg->username);
		//	printf("%s",msg->username);
		//	printf("--------华丽的分割线-----");
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
			{
				printf("修改失败\n");
				printf("%s\n",errmsg);
				return;
			}
			else{
				printf("修改成功\n");
				strcpy(msg->recvmsg,"修改成功");
				send(acceptfd,msg,sizeof(MSG),0);
				printf("%s",msg->recvmsg);
				//生成历史记录
				memset(msg->recvmsg, 0, NAMELEN);
				sprintf(msg->recvmsg,"'%s' 修改了工号'%d'的名字为'%s'",msg->info.name,msg->info.no,msg->info.name);
				printf("开始插入历史记录\n");
				insert_history(msg,db);
				printf("'%s' 修改了工号'%d'的名字为'%s'",msg->info.name,msg->info.no,msg->info.name);
			}
		case 2:
			sprintf(sql,"update usrinfo set age='%d' where name='%s'",msg->info.age,msg->info.name);
			printf("%s",msg->username);
			printf("%s",msg->info.name);
		//	sprintf(sql,"select * from usrinfo where name='%s';",msg->info.name);
			if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
			{
				printf("修改失败\n");
				printf("%s\n",errmsg);
				return;
			}
			else{
				printf("修改成功\n");
				strcpy(msg->recvmsg,"修改成功");
				send(acceptfd,msg,sizeof(MSG),0);
				printf("%s",msg->recvmsg);
				//生成历史记录
				memset(msg->recvmsg, 0, NAMELEN);
				sprintf(msg->recvmsg,"'%s' 修改了工号'%d'的年龄为'%d'",msg->info.name,msg->info.no,msg->info.age);
				printf("开始插入历史记录\n");
				insert_history(msg,db);
				printf("'%s' 修改了工号'%d'的年龄为'%d'",msg->info.name,msg->info.no,msg->info.age);
			}
			break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				break;
			case 9:
				break;


	}
}



int process_user_query_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	int i,j;
		sprintf(sql,"select * from usrinfo where name='%s';",msg->info.name);
		if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){

			printf("---get_table failed----%s.\n",errmsg);		
		}else{

			if(nrow == 0){
				strcpy(msg->recvmsg,"failed");
				send(acceptfd,msg,sizeof(MSG),0);
				printf("查找数据不存在\n");
			}else{
				printf("查找成功\n");
				//strcpy(msg->recvmsg,"OK");
				//send(acceptfd,msg,sizeof(MSG),0);
				for(i=0;i<nrow+1;i++)
				{
					for(j=0;j<ncolumn;j++)	
					{
						memset(msg->recvmsg, 0, NAMELEN);
						strcpy(msg->recvmsg,*result++);
						printf("%s, ",msg->recvmsg);
						send(acceptfd,msg,sizeof(MSG),0);

					}
					printf("\n");
				}
			}
		}
		printf("查找结束\n");
		memset(msg->recvmsg, 0, NAMELEN);
		strcpy(msg->recvmsg,"over");
		send(acceptfd,msg,sizeof(MSG),0);


}


int process_admin_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	
	char sql[DATALEN]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	int i,j;
	sprintf(sql,"select * from usrinfo where staffno='%d';",msg->info.no);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		
			printf("---get_table failed----%s.\n",errmsg);	
	}else{
		if(nrow==0)
			{
			strcpy(msg->recvmsg,"数据不存在");
			send(acceptfd,msg,sizeof(MSG),0);
			
			}
		else{

				strcpy(msg->recvmsg,"OK");
				printf("查找到要修改的用户\n");
				send(acceptfd,msg,sizeof(MSG),0);
				//结收要修改的信息
				 recv(acceptfd,msg,sizeof(MSG),0);
				 printf("收到修改指令\n");
				 switch(msg->flags)
             	 {
				 		 case 1:
							sprintf(sql,"update usrinfo set name='%s' where staffno='%d';",msg->info.name,msg->info.no);
							if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
							printf("---get_table failed----%s.\n",errmsg);	
							}else{
								strcpy(msg->recvmsg,"修改成功");
								send(acceptfd,msg,sizeof(MSG),0);//给予响应2
								printf("%s",msg->recvmsg);
								//生成历史记录
								memset(msg->recvmsg, 0, NAMELEN);
								sprintf(msg->recvmsg,"'%s' 修改了工号'%d'的名字为'%s'",msg->info.name,msg->info.no,msg->info.name);
								printf("开始插入历史记录\n");
								insert_history(msg,db);
								printf("'%s' 修改了工号'%d'的名字为'%s'",msg->info.name,msg->info.no,msg->info.name);
							}						
							break;
						 case 2:
							sprintf(sql,"update usrinfo set age='%d' where staffno='%d';",msg->info.age,msg->info.no);
							if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
							printf("---get_table failed----%s.\n",errmsg);	
							}else{
								strcpy(msg->recvmsg,"修改成功");
								send(acceptfd,msg,sizeof(MSG),0);//给予响应2
								printf("%s",msg->recvmsg);
								//生成历史记录
								memset(msg->recvmsg, 0, NAMELEN);
								sprintf(msg->recvmsg,"'%s' 修改了工号'%d'的年龄为'%d'",msg->info.name,msg->info.no,msg->info.age);
								printf("开始插入历史记录\n");
								insert_history(msg,db);
								printf("'%s' 修改了工号'%d'的年龄为'%d'",msg->info.name,msg->info.no,msg->info.age);
							}
							break;
						 case 3:
							break;
						case 4:
							break;
						case 5:
							break;
						case 6:
							break;
						case 7:
							break;
			 	 }

			}

		}
		
	
}


int process_admin_adduser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN]={0};
	char *errmsg;
	printf("结收要添加的用户\n");
	 recv(acceptfd,msg,sizeof(MSG),0);
	 printf("接受成功\n");
//	sprintf(sql,"insert into stu values('%d','%d','%s','%s','%d','%s','%s',%s','%d','%lf')",\
			&msg->info.no,&msg->info.usertype,msg->info.name,msg->info.passwd,&msg->info.age,msg->info.phone,\
			msg->info.addr,msg->info.work,msg->info.date,&msg->info.level,&msg->info.salary);
	sprintf(sql,"insert into usrinfo values(%d,%d,'%s','%s',%d,'%s','%s','%s','%s',%d,%lf);",msg->info.no,msg->info.usertype,msg->info.name,msg->info.passwd,msg->info.age,msg->info.phone,msg->info.addr,msg->info.work,msg->info.date,msg->info.level,msg->info.salary);
    printf("开始添加\n");
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("---exec failed----%s.\n",errmsg);	
	}
	else
	{
		strcpy(msg->recvmsg,"添加成功");
		send(acceptfd,msg,sizeof(MSG),0);//给予响应2
		printf("%s",msg->recvmsg);
		//生成历史记录
		memset(msg->recvmsg, 0, NAMELEN);
		sprintf(msg->recvmsg,"'%s' 添加了工号为'%d'的用户",msg->info.name,msg->info.no);
		printf("开始插入历史记录\n");
		insert_history(msg,db);
		printf("插入历史记录成功\n");
		
	}
	printf("---------华丽的分割线---------\n");
}



int process_admin_deluser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	printf("查看要删除的用户\n");
	char sql[DATALEN]={0};
	char *errmsg;

	sprintf(sql,"delete from usrinfo where staffno='%d';",msg->info.no);

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK)
	{
		printf("---exec failed----%s.\n",errmsg);	
		strcpy(msg->recvmsg,"删除用户不存在");
		send(acceptfd,msg,sizeof(MSG),0);//给予响应2
		printf("%s",msg->recvmsg);
	}
	else
	  {//bug  要删除的不存在也会 进入
		strcpy(msg->recvmsg,"删除成功");
		send(acceptfd,msg,sizeof(MSG),0);//给予响应2
		printf("%s",msg->recvmsg);
		//生成历史记录
		memset(msg->recvmsg, 0, NAMELEN);
		sprintf(msg->recvmsg,"'%s' 删除了工号为'%d'的用户",msg->info.name,msg->info.no);
		printf("开始插入历史记录\n");
		insert_history(msg,db);
		printf("插入历史记录成功\n");


	}

}


int process_admin_query_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	int i,j;
	switch(msg->flags)
	{
	case 1:
		printf("开始查找指定用户\n");
		sprintf(sql,"select * from usrinfo where name='%s';",msg->info.name);
		if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){

			printf("---get_table failed----%s.\n",errmsg);		
		}else{

			if(nrow == 0){
				strcpy(msg->recvmsg,"failed");
				send(acceptfd,msg,sizeof(MSG),0);
				printf("查找数据不存在\n");
			}else{
				printf("查找成功\n");
				strcpy(msg->recvmsg,"OK");
				send(acceptfd,msg,sizeof(MSG),0);
				for(i=0;i<nrow+1;i++)
				{
					for(j=0;j<ncolumn;j++)	
					{
						memset(msg->recvmsg, 0, NAMELEN);
						strcpy(msg->recvmsg,*result++);
						printf("%s,",msg->recvmsg);
						send(acceptfd,msg,sizeof(MSG),0);
					}
					printf("\n");
				}
				/*
				memset(msg->recvmsg, 0, NAMELEN);
				strcpy(msg->recvmsg,"over");
				printf("查找结束\n");
				send(acceptfd,msg,sizeof(MSG),0);
				*/
			}
		}
		memset(msg->recvmsg, 0, NAMELEN);
		strcpy(msg->recvmsg,"over");
		printf("查找结束\n");
		send(acceptfd,msg,sizeof(MSG),0);
		printf("-------\n");
		break;
	case 2:
		sprintf(sql,"select * from usrinfo;");
		if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){

			printf("---get_table failed----%s.\n",errmsg);		
		}else{

			if(nrow == 0){
				strcpy(msg->recvmsg,"failed");
				send(acceptfd,msg,sizeof(MSG),0);
				printf("查找数据不存在\n");
			}else{
				printf("查找成功\n");
				strcpy(msg->recvmsg,"OK");
				send(acceptfd,msg,sizeof(MSG),0);
				for(i=0;i<nrow+1;i++)
				{
					for(j=0;j<ncolumn;j++)	
					{
						memset(msg->recvmsg, 0, NAMELEN);
						strcpy(msg->recvmsg,*result++);
						printf("%s, ",msg->recvmsg);
						send(acceptfd,msg,sizeof(MSG),0);

					}
					printf("\n");
				}
			}
		}
		printf("查找结束\n");
		memset(msg->recvmsg, 0, NAMELEN);
		strcpy(msg->recvmsg,"over");
		send(acceptfd,msg,sizeof(MSG),0);

		break;
	}
}

int process_admin_history_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN]={0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	int i,j;

		sprintf(sql,"select * from historyinfo;");
		if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){

			printf("---get_table failed----%s.\n",errmsg);		
		}else{

			if(nrow == 0){
				strcpy(msg->recvmsg,"暂无历史记录");
				send(acceptfd,msg,sizeof(MSG),0);
				printf("暂无历史记录\n");
			}else{
				printf("查找成功\n");
				strcpy(msg->recvmsg,"OK");
				send(acceptfd,msg,sizeof(MSG),0);
				for(i=0;i<nrow+1;i++)
				{
					for(j=0;j<ncolumn;j++)	
					{
						memset(msg->recvmsg, 0, NAMELEN);
						strcpy(msg->recvmsg,*result++);
						printf("%s, ",msg->recvmsg);
						send(acceptfd,msg,sizeof(MSG),0);

					}
					printf("\n");
				}
			}
		}
		printf("查找结束\n");
		memset(msg->recvmsg, 0, NAMELEN);
		strcpy(msg->recvmsg,"over");
		send(acceptfd,msg,sizeof(MSG),0);
}


int process_client_quit_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);

}


int process_client_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			process_user_or_admin_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			process_user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			process_user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			process_admin_modify_request(acceptfd,msg);
			break;

		case ADMIN_ADDUSER:
			process_admin_adduser_request(acceptfd,msg);
			break;

		case ADMIN_DELUSER:
			process_admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			process_admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			process_admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			process_client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}

}


int main(int argc, const char *argv[])
{
	//socket->填充->绑定->监听->等待连接->数据交互->关闭 
	int sockfd;
	int acceptfd;
	ssize_t recvbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);

	MSG msg;
	//thread_data_t tid_data;
	char *errmsg;

	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ //华清远见创客学院         嵌入式物联网方向讲师
		printf("create historyinfo table success.\n");
	}

	//创建网络通信的套接字
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if(sockfd == -1){
		perror("socket failed.\n");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd); 

	
	/*优化4： 允许绑定地址快速重用 */
	int b_reuse = 1;
	setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));
	
	//填充网络结构体
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
//	serveraddr.sin_port   = htons(atoi(argv[2]));
//	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
	serveraddr.sin_port   = htons(5001);
	serveraddr.sin_addr.s_addr = inet_addr("192.168.1.106");


	//绑定网络套接字和网络结构体
	if(bind(sockfd, (const struct sockaddr *)&serveraddr,addrlen) == -1){
		printf("bind failed.\n");
		exit(-1);
	}

	//监听套接字，将主动套接字转化为被动套接字
	if(listen(sockfd,10) == -1){
		printf("listen failed.\n");
		exit(-1);
	}

	//定义一张表
	fd_set readfds,tempfds;
	//清空表
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	//添加要监听的事件
	FD_SET(sockfd,&readfds);
	int nfds = sockfd;
	int retval;
	int i = 0;

#if 0 //添加线程控制部分
	pthread_t thread[N];
	int tid = 0;
#endif

	while(1){
		tempfds = readfds;
		//记得重新添加
		retval =select(nfds + 1, &tempfds, NULL,NULL,NULL);
		//判断是否是集合里关注的事件
		for(i = 0;i < nfds + 1; i ++){
			if(FD_ISSET(i,&tempfds)){
				if(i == sockfd){
					//数据交互 
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&cli_len);
					if(acceptfd == -1){
						printf("acceptfd failed.\n");
						exit(-1);
					}
					printf("ip : %s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);
					nfds = nfds > acceptfd ? nfds : acceptfd;
				}else{
					recvbytes = recv(i,&msg,sizeof(msg),0);//接受登录请求
					printf("msg.type :%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){
						printf("peer shutdown.\n");
						close(i);
						FD_CLR(i, &readfds);  //删除集合中的i
					}else{
						process_client_request(i,&msg);
					}
				}
			}
		}
	}
	close(sockfd);

	return 0;
}







#if 0
					//tid_data.acceptfd = acceptfd;   //暂时不使用这种方式
					//tid_data.state	  = 1;
					//tid_data.thread   = thread[tid++];	
					//pthread_create(&tid_data.thread, NULL,client_request_handler,(void *)&tid_data);
#endif 

#if 0
void *client_request_handler(void * args)
{
	thread_data_t *tiddata= (thread_data_t *)args;

	MSG msg;
	int recvbytes;
	printf("tiddata->acceptfd :%d.\n",tiddata->acceptfd);

	while(1){  //可以写到线程里--晚上的作业---- UDP聊天室
		//recv 
		memset(msg,sizeof(msg),0);
		recvbytes = recv(tiddata->acceptfd,&msg,sizeof(msg),0);
		if(recvbytes == -1){
			printf("recv failed.\n");
			close(tiddata->acceptfd);
			pthread_exit(0);
		}else if(recvbytes == 0){
			printf("peer shutdown.\n");
			pthread_exit(0);
		}else{
			printf("msg.recvmsg :%s.\n",msg.recvmsg);
			strcat(buf,"*-*");
			send(tiddata->acceptfd,&msg,sizeof(msg),0);
		}
	}

}

#endif 













