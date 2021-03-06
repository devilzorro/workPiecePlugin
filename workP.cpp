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
		m_wisUrl = vcPath[0] ;
		tmpPath = m_ini->GetStr("WisTaskList","PostUrl");
		vcPath.clear();
		vcPath = split(tmpPath,"?");
		m_allListUrl = vcPath[0];
		tmpPath = m_ini->GetStr("WisWorkArt","PostUrl");
		vcPath.clear();
		vcPath = split(tmpPath,"?");
		m_refreshUrl = vcPath[0];
	}
	m_ini->CloseFile();

	localMQStatus = AccessMQ("FCworkPiece","","",1883,300,localMQConnLost,localMQRecv);
	if (localMQStatus == 0)
	{
		printf("\n connect to localMQ sub topic\n");
		addSubTopic("WISpad");
	}


	if(pthread_create(&sendLocalMQId,NULL,sendLocalMsgThread,this) != 0)
	{
		printf("create sendLocalMsgThread fail!\n");
	}
//
	if(pthread_create(&processLocalMsgId,NULL,processLocalMsgThread,this) != 0)
	{
		printf("create processLocalMsgThread fail!\n");
	}

	if (pthread_create(&processHttpMsgId,NULL,processHttpMsgThread,this) != 0)
	{
		printf("create processHttpMsgThread fail!\n");
	}

	if (pthread_create(&downloadFilesId,NULL,downloadFilesThread,this) != 0)
	{
		printf("create downloadFilesId fail!\n");
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

void* FCworkPiece::processHttpMsgThread(void *arg)
{
	FCworkPiece *ptr = (FCworkPiece *)arg;
	ptr->processHttpMsgThreadRun();
	pthread_exit(NULL);
}

void* FCworkPiece::downloadFilesThread(void *arg)
{
	FCworkPiece *ptr = (FCworkPiece *)arg;
	ptr->downloadFilesThreadRun();
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

void FCworkPiece::processHttpMsgThreadRun()
{
	while(woStatus)
	{
		if (!m_msgQ->httpMsgs.empty())
		{
			vector<string>::iterator it;
			it = m_msgQ->httpMsgs.begin();
			string recvMsgContent = *it;
			m_msgQ->httpMsgs.erase(m_msgQ->httpMsgs.begin());
			if (recvMsgContent != "")
			{
				printf("process http msg content:%s\n",recvMsgContent.c_str());
				Json::Reader reader;
				Json::Value jsonRoot;
				Json::Value jsonVal;
				if (reader.parse(recvMsgContent,jsonRoot))
				{
					string rootVal = jsonRoot["woRequest"].asString();
					if (rootVal == "login")
					{
						m_msgQ->loginRet = m_httpManager->loginRequest(m_wisUrl,m_msgQ->strUserName,m_msgQ->strPw,m_machineId,"1");
						if (m_msgQ->loginRet != "")
						{
							// string tmpData = m_msgQ->loginRet;
							// if (tmpData.substr(0,1) == "[")
							// {
							// 	tmpData = tmpData.substr(1,tmpData.size()-1);
							// 	while(tmpData.substr(tmpData.size()-1,1) != "]")
							// 	{
							// 		tmpData = tmpData.substr(0,tmpData.size()-1);
							// 	}
							// 	tmpData = tmpData.substr(0,tmpData.size()-1);
							// 	m_msgQ->loginRet = tmpData;
							// }
							// printf("cut data ret:%s\n",tmpData.c_str());
							//解析登陆信息 判断登陆是否成功
							Json::Reader reader;
							Json::Value loginRoot;
							if (reader.parse(m_msgQ->loginRet,loginRoot))
							{
								if (loginRoot["success"].asBool())
								{
									m_msgQ->loginStatus = "success";
									m_msgQ->logoutRet = "";									/* code */
								}
								else
								{
									m_msgQ->loginStatus = "unlogin";
								}
							}
							else
							{
								printf("login retdata false!\n");
							}
						}
						else
						{
							m_msgQ->loginRet = "请求失败";
						}
					}
					else if (rootVal == "logout")
					{
						m_msgQ->logoutRet = m_httpManager->loginRequest(m_wisUrl,m_msgQ->strUserName,m_msgQ->strPw,m_machineId,"0");
						if (m_msgQ->logoutRet != "")
						{
							Json::Reader reader;
							Json::Value logoutRoot;
							if (reader.parse(m_msgQ->logoutRet,logoutRoot))
							{
								if (logoutRoot["success"].asBool())
								{
									m_msgQ->loginStatus = "unlogin";
									m_msgQ->loginRet = "";
									m_msgQ->strUserName = "";
									m_msgQ->strPw = "";
								}
								else
								{
									m_msgQ->loginStatus = "success";
								}
							}
							else
							{
								printf("logout ret data false!\n");
							}
						}
						else
						{
							m_msgQ->logoutRet = "请求失败";
						}
					}
					//刷新工单详情数据
					else if(rootVal == "refreshData")
					{
					}
					//处理下载文件请求
					else if (rootVal == "download")
					{
						
					}
					//手动报工
					else if (rootVal == "manualReport")
					{
						/* code */
					}
					else if (rootVal == "all")
					{
						/* code */
						if ((m_allListUrl != "")&&(m_machineId != ""))
						{
							/* code */
							m_msgQ->oldWorkPList = m_httpManager->allListRequest(m_allListUrl,m_machineId);
							if (m_msgQ->oldWorkPList == "")
							{
								/* code */
								m_msgQ->oldWorkPList = "请求失败";
							}
						}
					}
					else
					{

					}
				}
				else
				{
					printf("http msg type error!\n" );
				}
			}

		}
		usleep(500000);
	}
}

void FCworkPiece::downloadFilesThreadRun()
{
	while(woStatus)
	{
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
	else if (strTopic == "WISpad")
	{
		if(msgContent != "requestWisList")
		{
			m_workPiece->m_msgQ->oldWorkPList = msgContent;
		}
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

	Json::Reader reader;
	Json::Value jsonRecv;

	Json::Value replyRoot;
	Json::Value replyData;
	Json::Value replyArrray;

	Json::Value defaultReply;
	defaultReply["woResponse"] = 0;
	string retStr = defaultReply.toStyledString();
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
					if (m_msgQ->loginStatus == "success")
					{
						/* code */
						replyRoot["woResponse"] = 0;
						sendlocalMQ("requestWisList","WISpad");
						m_msgQ->httpMsgs.push_back(servjson);
					}
					else
					{
						replyRoot["woResponse"] = 0;
					}
				}
				else if(strWoRequest == "all_result")
				{
					//返回请求工单结果
					if(m_msgQ->loginStatus == "success")
					{
						if((m_msgQ->oldWorkPList != "")&&(m_msgQ->oldWorkPList != "requestWisList"))
						{
							replyRoot["woResponse"] = 1;
							replyRoot["data"] = m_msgQ->oldWorkPList;
							// retStr = replyRoot.toStyledString();
							// printf("localMQ list content:%s\n",m_msgQ->oldWorkPList.c_str());
						}
						else
						{
							replyRoot["woResponse"] = 0;
						}
					}
					else
					{
						replyRoot["woResponse"] = 1;
						replyRoot["data"] = "wis用户未登录";
					}
					
				}
				//初始化请求
				else if(strWoRequest == "init")
				{
					replyRoot["woResponse"] = 0;
				}
				else if(strWoRequest == "init_result")
				{
					replyRoot["woResponse"] = 1;
				}
				//wis用户登录请求
				else if(strWoRequest == "login")
				{
					m_msgQ->httpMsgs.push_back(servjson);
					Json::Value jsonData = jsonRecv["data"];
					m_msgQ->strUserName = jsonData["userName"].asString();
					m_msgQ->strPw = jsonData["passWord"].asString();
					printf("fc recv login data content:%s\n",m_msgQ->strUserName.c_str());
					if((m_wisUrl != "")&&(m_machineId != "")&&(m_msgQ->strUserName != "")&&(m_msgQ->strPw != ""))
					{
						// m_msgQ->tmpLoginRet = m_httpManager->loginRequest(m_wisUrl,m_msgQ->strUserName,m_msgQ->strPw,m_machineId,"1");
						// printf("login ret content:%s\n",m_msgQ->tmpLoginRet.c_str());
						replyRoot["woResponse"] = 0;
					}
					else
					{
						replyRoot["woResponse"] = -1;
					}
				}
				else if(strWoRequest == "login_result")
				{
					if(m_msgQ->loginRet != "")
					{
						//解析获取到的登录信息更新登录状态字段
						replyRoot["woResponse"] = 1;
						replyRoot["data"] = m_msgQ->loginRet;
						// m_msgQ->loginRet = "";
					}
					else
					{
						replyRoot["woResponse"] = 0;
					}
				}
				//wis用户登出
				else if(strWoRequest == "logout")
				{
					m_msgQ->httpMsgs.push_back(servjson);
					Json::Value jsonData = jsonRecv["data"];
					m_msgQ->strUserName = jsonData["userName"].asString();
					m_msgQ->strPw = jsonData["passWord"].asString();
					printf("fc recv logout data content:%s\n",m_msgQ->strUserName.c_str());
					if((m_wisUrl != "")&&(m_machineId != "")&&(m_msgQ->strUserName != "")&&(m_msgQ->strPw != ""))
					{
						replyRoot["woResponse"] = 0;
					}
					else
					{
						replyRoot["woResponse"] =  -1;
					}
				}
				else if(strWoRequest == "logout_result")
				{
					if (m_msgQ->logoutRet != "")
					{
						replyRoot["woResponse"] = 1;
						replyRoot["data"] = m_msgQ->logoutRet;
						// m_msgQ->logoutRet = "";
					}
					else
					{
						replyRoot["woResponse"] = 0;
					}
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

	string strMd5 = m_httpManager->GetFileMd5("/home/fiyang/nut/config/moon/iport.ini",32);
	printf("file md5 content:%s\n",strMd5.c_str());
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
