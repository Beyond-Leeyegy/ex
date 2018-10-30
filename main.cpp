#include "main.hpp"
#include "linked_list.h"
#include "client_structs.h"
#include "network.h"
#include "def.h"
#include  "game.hpp"
#include "utils.h"
#include "data_protocal.h"
#include <pthread.h>
#include <sys/select.h>
#include "meta_data.h"
struct arg {
	sPCLIENT_DATA client1;
	sPCLIENT_DATA client2;
};
typedef struct arg * arg_thread;

int g_ProcessLife = TRUE;			//决定了程序是否循环运行——服务器是否一致运行
SOCKET g_SOCK = INVALID_SOCKET;    // 服务器基本套接字
sPCLIENT_DATA g_ClientList = NULL; // 客户数据链表
int g_TotalClient = 0;				//总连接者数

void ProcessLoop();						//程序监听主体,基于Select机制的异步IO
void* start_game_thread(void * arg);	//开始一个游戏线程
void DestroySignal(int sigNum);			//设定g_ProcessLife为假
void InitSignal();						//Signal机制设定

int main(){
	InitSignal();
	ProcessLoop(); // keep listening and starting new game thread for appropriare clients
	return 0;
}

//int main(){
//	Game game(NULL,NULL);
//	game.set_state(GameStart);
//	game.set_state(GameSelect);
//	game.set_state(GameSplash);
//	game.set_state(GameRun);
//	game.update_auto_data_on_run();
//        byte packet[dMAX_SOCK_BUFF];
//       log("debug info ::start PutGameRunFrame\r\n") ;
//    	int frame_num = 1;
//        int pos = PutGameRunFrame(packet,frame_num);
//
//	printf("youare ehre");
//	return 0;
//}


void* start_game_thread(void * arg) {
	arg_thread args = (arg_thread)arg;
	Game game(args->client1, args->client2);//纯粹的界面相关
	//friend包在game初始化的时候立即发出
	
	game.init();//客户端来做
	game.run();	//服务器端来做——服务器端接收客户端数据，逻辑判断、处理，发送相应数据回去
	game.over();//客户端来做
}

void DestroySignal(int sigNum) {
	g_ProcessLife = FALSE;
}

void InitSignal() {

	/*
	处理SIGPIPE
	原因：SIGPIPE会强制关闭进程而不给出任何提示，这样程序员无从确定信息，为了避免这种情况，需要进行设置
	触发SIGPIPE的场景：比如尝试向已经关闭的套接字中写数据
	*/
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	act.sa_flags &= ~SA_RESETHAND;
	sigaction(SIGPIPE, &act, NULL);

	/*
	当发生强制结束服务器运行的Signal时候，调用正常结束服务器运行函数的Signal的设置
	*/
	signal(SIGINT, DestroySignal);
	signal(SIGKILL, DestroySignal);
	signal(SIGQUIT, DestroySignal);
	signal(SIGTERM, DestroySignal);  //这里似乎与linux现在的定义不大相同。
}

void ProcessLoop() {

	sPCLIENT_DATA tmp1, tmp2;//用于调度算法
	arg_thread arg = (arg_thread)malloc(sizeof(struct arg));


	/*
		服务器套接字初始化
	*/
	g_SOCK = InitServerSock(dSERVER_PORT, dMAX_LISTEN); // listening 
	if (g_SOCK == INVALID_SOCKET) {
		log("[ERROR::INVALID_SOCKET]in ProcessLoop\r\n\r\n\r\n");
		return;
	}

	//fd_Set 
	fd_set read_set;
	fd_set write_set;
	fd_set exc_set;
	struct timeval tv;
	SOCKET nfds;
	
	tv.tv_sec = 0;
	tv.tv_usec = 0;


	///*
	//	testing
	//*/
	//bool test_is_ready = false;

	sPCLIENT_DATA client, next_client;

	while ((g_ProcessLife))
	{

		/*
			fd_set init
		*/
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_ZERO(&exc_set);

		/*
			fd_set bind g_SOCK
		*/
		FD_SET(g_SOCK, &read_set);
		nfds = g_SOCK;


		/*
			nfds 
		*/
		LIST_WHILE(g_ClientList, client, next_client, m_next);
		if (client->m_scok > nfds)
			nfds = client->m_scok;

		/*
			fd_Set bind client_socks
		*/
		if (!client->in_Game) {
			FD_SET(client->m_scok, &read_set);
			FD_SET(client->m_scok, &write_set);
			FD_SET(client->m_scok, &exc_set);
		}
		LIST_WHILEEND(g_ClientList, client, next_client);

		/*
			select
		*/
		if (select(nfds + 1, &read_set, &write_set, &exc_set, &tv) < -1) {
			log("[ERROR::SELECT FAIL]IN ProcessLoop\r\n\r\n\r\n");
			continue;
		}

		/*
			new request to connect
		*/
		if (FD_ISSET(g_SOCK, &read_set))
			AcceptNewClient(g_SOCK);


		LIST_WHILE(g_ClientList, client, next_client, m_next);
//		log("[deug]\r\n");
		if (client->in_Game)
			continue;
//		log("[conti]\r\n");
		/*
			exception handling
		*/
		if (FD_ISSET(client->m_scok, &exc_set)) {
			log("[Error Handling::disconnect]:%s  IN ProcessLoop\r\n", client->m_IP);
			DisconnectClient(client);
			LIST_SKIP(client, next_client);
		}

		/*
			no response over 1 min
		*/
		if (client->m_lastRecvTime + 60000 <= timeGetTime()) {
			log("[WARNING::disconnect]:%s over 1 minute no repsonse IN ProcessLoop\r\n\r\n", client->m_IP);
			DisconnectClient(client);
			LIST_SKIP(client, next_client);
		}

		/*
			try recv
		*/
		if (FD_ISSET(client->m_scok, &read_set)) {
			log("[FEEL LIKE TO READ\r\n]");
			if (!RecvFromClient(client)) {
				log("[ERROR::disconnect]:%s bad recv got minus return value IN ProcessLoop\r\n\r\n\r\n", client->m_IP);
				DisconnectClient(client);
				LIST_SKIP(client, next_client);
			}
		}
		//log("out of reading\r\n");
		/*
			try handle recved data
		*/
		//Game tmp(NULL,NULL);
		//log("try handle\r\n");
		if (client->m_recvSize) {
			if (!onHandle_CONTROL_CENTER_FOR_IS_READY(client))
			{
				log("[ERROR:BAD_DATA_TRANSFER]:% IN ProcessLoop\r\n\r\n\r\n", client->m_IP);
				DisconnectClient(client);
				LIST_SKIP(client, next_client);
			}
			/*test_is_ready = true;*/
			log("[HANDLE_ONE data GIAGRAM OUT\r\n:%d]:%s", client->m_scok, client->m_IP);
		}
		////test
		//if (test_is_ready) {
		//	log("pos1\r\n");
		//}
		//log("end in d-c-loop\r\n");
		LIST_WHILEEND(g_ClientList, client, next_client);


		//log("out of d-c-loop\r\n");

		/*
			try send
		*/
		LIST_WHILE(g_ClientList, client, next_client, m_next);
		if (client->in_Game)
			continue;
		////test
		//if (test_is_ready) {
		//	log("pos3");
		//}
		//log("try send\r\n");
		if (client->m_sendSize && FD_ISSET(client->m_scok, &write_set)) {
		

			//log("acctually wanna send\r\n");

			if (FlushSendBuff(client) < 0) {
				DisconnectClient(client);
				LIST_SKIP(client, next_client);
			}

		}
		//log("end in send-loop\r\n");
		LIST_WHILEEND(g_ClientList, client, next_client);
	
	
		//log("out of send-liio\r\n");

		////test
		//if (test_is_ready) {
		//	log("pos6");
		//}


		int count = 0;

		/*
			dispatch two clients to start a game
		*/
		LIST_WHILE(g_ClientList, client, next_client, m_next);
		////test
		//if (test_is_ready) {
		//	log("pos7");
		//}
//		log("[DEBUG]i am the entry of tmp1 int\r\n");
		if (!client->in_Game && client->is_Ready) {
			if (count == 0) {
				tmp1 = client;
			//	log("[DEBUG]I am in the tmp1 init\r\n");
			}
			else if (count == 1) {
				log("just comming into count == 1 \r\n");
				tmp2 = client;
				log("[debug::%d::%d]\r\n\r\n\r\n", tmp1->m_scok, tmp2->m_scok);
				arg->client1 = tmp1;
				arg->client2 = tmp2;

				pthread_t t1;
				
				//have to finish here!!!
				arg->client1->in_Game = 1;
				arg->client2->in_Game = 1;

				pthread_create(&t1, NULL, start_game_thread, arg);
			}
			count = (count + 1) % 2;
		}
		LIST_WHILEEND(g_ClientList, client, next_client);
		//if (test_is_ready) {
		//	log("pos10");
		//}
	}

}
