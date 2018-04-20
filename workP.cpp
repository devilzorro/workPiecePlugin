#include "workP.h"
#include "localMQ.h"
#include "json.h"
#include <stdio.h>
#include <stdlib.h>

#define HOMEPATH "NUTHOME"
FCworkPiece* FCworkPiece::m_workPiece;

FCworkPiece::FCworkPiece()
{
	m_workPiece = this;
	woStatus = true;
	const char *ip = "127.0.0.1";
	m_strEvo = getenv(HOMEPATH);
//	char *port = "7102";
	m_msgQ = new MsgQ();
//	m_dealer = new RpcDealerZMQ(ip,"7102");
	m_ini = new CIni();
	m_httpManager = new HTTPManager();
	//获取machineId地址
	string iportPath = m_strEvo + "/config/moon/iport.ini";
	if(m_ini->OpenFile(iportPath.c_str(),"r") == INI_SUCCESS)
	{
		m_machineId = m_ini->GetStr("iPort","MachineId");
	}
	m_ini->CloseFile();

	//获取wis服务器地址
	string wisPath = m_strEvo + "/config/wisservice.ini";
	if(m_ini->OpenFile(wisPath.c_str(),"r") == INI_SUCCESS)
	{
		string tmpPath = m_ini->GetStr("WisTaskLogin","PostUrl");
		vector<string> vcPath = split(tmpPath,"?");
		m_wisUrl = vcPath[0] + "?";
	}
	m_ini->CloseFile();

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

	printf("machineId:%s\n",m_machineId.c_str());
	printf("url:%s\n",m_wisUrl.c_str());

	string strMd5 = m_httpManager->GetFileMd5("/home/fiyang/nut/config/moon/iport.ini",32);
	printf("file md5 content:%s\n",strMd5.c_str());

	Json::Reader reader;
	Json::Value jsonRecv;

	Json::Value replyRoot;
	Json::Value replyData;
	Json::Value replyArrray;
	string retStr = "";
	if(servjson != "")
	{
		string strBeJson = servjson.substr(0,1);
		if(strBeJson == "{")
		{
			if(reader.parse(servjson,jsonRecv))
			{
				string strWoRequest = jsonRecv["woRequest"].asString();
				if(strWoRequest == "all")
				{
					//发起请求工单列表
					replyRoot["woResponse"] = 0;
				}
				else if(strWoRequest == "all_result")
				{
					//返回请求工单结果
					if(m_msgQ->newWorkPList != "")
					{
						replyRoot["woResponse"] = 1;
						replyRoot["data"] = m_msgQ->newWorkPList;
						retStr = replyRoot.toStyledString();
						printf("localMQ list content:%s\n",m_msgQ->newWorkPList.c_str());
					}
				}
				else if(strWoRequest == "init")
				{
					//初始化请求
					//获取机床序列号
					string iportPath = m_strEvo + "/config/moon/iport.ini";
					printf("iport ini path:%s\n",iportPath.c_str());
					if(m_ini->OpenFile(iportPath.c_str(),"r") == INI_SUCCESS)
					{
						m_machineId = m_ini->GetStr("iPort","MachineId");
					}
					m_ini->CloseFile();



					replyRoot["woResponse"] = 0;
				}
				else if(strWoRequest == "init_result")
				{
					replyRoot["woResponse"] = 1;
				}
				else if(strWoRequest == "login")
				{
					Json::Value jsonData = jsonRecv["data"];
					string strUserName = jsonData["userName"].asString();
					printf("fc recv login data content:%s\n",strUserName.c_str());
					string strSendTest = "login test msg: " + strUserName;
					if(sendlocalMQ(strSendTest.c_str(),"woLogin") != -1)
					{
						replyRoot["woResponse"] = 0;
					}
					else
					{
						replyRoot["woResponse"] = -1;
					}
				}
				else if(strWoRequest == "login_result")
				{

				}
			}
			else
			{
				replyRoot["woResponse"] = -2;
			}
		}
		else
		{
			replyRoot["woResponse"] = -2;
		}
	}
	retStr = replyRoot.toStyledString();
	return retStr;
}

string FCworkPiece::sendFC(string msgContent)
{
//	if(m_dealer)
//	{
//		string sendResp = m_dealer->SendPluginCmd(keystr,msgContent);
//		return sendResp;
//	}
//	else
//	{
//		return "";
//	}
	return "";
}

int FCworkPiece::sendlocalMQ(string msgContent,string topic)
{
	if(localMQStatus == 0)
	{
		return sendLocalMsg(msgContent.c_str(),topic.c_str());
	}
	else
	{
		return -1;
	}
}

vector<string> FCworkPiece::split(string strContent,string mark)
{
	string::size_type pos;
	vector<string> result;
	strContent += mark;
	int size = strContent.size();
	for(int i=0; i<size; i++)
	{
		pos = strContent.find(mark,i);
		if(pos<size)
		{
			string s = strContent.substr(i,pos-i);
			result.push_back(s);
			i = pos+mark.size()-1;
		}
	}
	return result;
}

bool FCworkPiece::isContain(string strContent,string mark)
{
	bool retStatus = false;
	for(int i=0;i<strContent.size();i++)
	{
		if(strContent.find(mark,i) != string::npos)
		{
			retStatus = true;
			break;
		}
	}
	return retStatus;
}
