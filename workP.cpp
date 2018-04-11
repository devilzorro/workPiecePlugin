#include "workP.h"
#include "localMQ.h"
#include "json.h"
#include <stdio.h>

FCworkPiece* FCworkPiece::m_workPiece;

FCworkPiece::FCworkPiece()
{
	m_workPiece = this;
	woStatus = true;
	const char *ip = "127.0.0.1";
//	char *port = "7102";
	m_msgQ = new MsgQ();
	m_dealer = new RpcDealerZMQ(ip,"7102");
	localMQStatus = AccessMQ("FCworkPiece","","",1883,300,localMQConnLost,localMQRecv);
	if (localMQStatus == 0)
	{
		printf("\n connect to localMQ sub topic\n");
		addSubTopic("WIS");
	}


	if(pthread_create(&sendLocalMQId,NULL,sendLocalMsgThread,this) != 0)
	{
		printf("create sendLocalMsgThread fail!");
	}
//
	if(pthread_create(&processLocalMsgId,NULL,processLocalMsgThread,this) != 0)
	{
		printf("create processLocalMsgThread fail!");
	}
}

FCworkPiece::~FCworkPiece()
{
	closeChannel();
	destoryChannel();
	woStatus = false;
}

void* FCworkPiece::sendLocalMsgThread(void *arg)
{
	FCworkPiece *ptr = (FCworkPiece *)arg;
	ptr->sendLocalMsgThreadRun();
	pthread_exit(NULL);
}

void* FCworkPiece::processLocalMsgThread(void *arg)
{
	FCworkPiece *ptr = (FCworkPiece *)arg;
	ptr->processLocalMsgThreadRun();
	pthread_exit(NULL);
}

void FCworkPiece::sendLocalMsgThreadRun()
{
	while(woStatus)
	{
		if(!m_msgQ->recvFCmsg.empty())
		{

		}
//		printf("sendLocalMsgThread\n");
		usleep(500000);
	}
}

void FCworkPiece::processLocalMsgThreadRun()
{
	while(woStatus)
	{
//		printf("processLocalMsgThread\n");
		if(!m_msgQ->recvLocalMQmsg.empty())
		{
			printf("process localMQ msg:%s\n",m_msgQ->recvLocalMQmsg[0].c_str());
			vector<string>::iterator it;
			it = m_msgQ->recvLocalMQmsg.begin();
			m_msgQ->newWorkPList = *it;
			m_msgQ->recvLocalMQmsg.erase(m_msgQ->recvLocalMQmsg.begin());
		}
		usleep(500000);
	}
}

void FCworkPiece::localMQRecv(char *msgContent,char *topicName,int topicLen)
{
	string strMsg = msgContent;
	string strTopic = topicName;
	if(strTopic == "WIS")
	{
		printf("localMQRecv:%s\ntopicName:%s\n",msgContent,topicName);
		m_workPiece->m_msgQ->recvLocalMQmsg.push_back(msgContent);
	}
}

void FCworkPiece::localMQConnLost()
{

}

string FCworkPiece::FCService(string servjson)
{
	printf("FCService recv:%s\n",servjson.c_str());
	if(servjson != "")
	{
		Json::Reader reader;
		Json::Value jsonRecv;
		if(reader.parse(servjson,jsonRecv))
		{
			string strWoPequest = jsonRecv["woRequest"].asString();
			if(strWoPequest == "all")
			{
				if(m_msgQ->newWorkPList != "")
				{
					printf("localMQ list content:%s\n",m_msgQ->newWorkPList.c_str());
					return m_msgQ->newWorkPList;
				}
			}
		}
		else
		{
			return "json type false";
		}
	}
	return "null";
}

string FCworkPiece::sendFC(string msgContent)
{
	if(m_dealer)
	{
		string sendResp = m_dealer->SendPluginCmd(keystr,msgContent);
		return sendResp;
	}
	else
	{
		return "";
	}
}

string FCworkPiece::sendlocalMQ(string msgContent)
{
	if(localMQStatus == 0)
	{
		sendLocalMsg(msgContent.c_str(),"");
	}
}
