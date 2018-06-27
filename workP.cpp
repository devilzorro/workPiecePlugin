#include "workP.h"
#include "localMQ.h"
#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define HOMEPATH "NUTHOME"
FCworkPiece* FCworkPiece::m_workPiece;

FCworkPiece::FCworkPiece()
{
	m_workPiece = this;
	woStatus = true;
	const char *ip = "127.0.0.1";
	m_strEvo = getenv(HOMEPATH);
//	char *port = "7102";
	decoderPath = m_strEvo + "/lib/libwisDecoder.so";
	void *dl = dlopen(decoderPath.c_str(),RTLD_LAZY);
	m_woDecoder = NULL;
	if (dl != NULL)
	{
		/* code */
		printf("******************dlopen dl nout NULL\n");
		m_woDecoder = (DECODER_PTR1) dlsym(dl, "Startlogic");
		printf("********************test finish\n");
		if (!m_woDecoder)
		{
			/* code */
			printf("***************dl error code:%s\n",dlerror() );
			dlclose(dl);
		}
	}


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
		addSubTopic("WIS");
	}

	//


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

	printf("*****************construct finish!\n");
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
		if (!m_msgQ->manualReportList.empty())
		{
			/* code */
			vector<string>::iterator it;
			it = m_msgQ->manualReportList.begin();
			string tmpRemoteMsg = *it;
			m_msgQ->manualReportList.erase(m_msgQ->manualReportList.begin());
			if (localMQStatus == 0)
			{
				/* code */
				printf("send local MQ msgContent:%s\n",tmpRemoteMsg.c_str());
				sendRemoteMsg(tmpRemoteMsg.c_str(),"WIS",103);
			}
			
		}
		// if(!m_msgQ->recvFCmsg.empty())
		// {

		// }
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
		if (m_msgQ->workPDetailRequest != "")
		{
			/* code */
			string tmpRequest = "equSerialNo=" + m_machineId + "&jobDispatchNo=" + m_msgQ->workPDetailRequest + "&memberID=1";
			m_msgQ->workPDetailResult =  m_httpManager->postStr(m_allListUrl,tmpRequest);
			printf("post cmd:%s\npost url:%s\n",tmpRequest.c_str(),m_allListUrl.c_str());
			m_msgQ->workPDetailRequest = "";
		}
		// if (!m_msgQ->httpMsgs.empty())
		// {
		// 	vector<string>::iterator it;
		// 	it = m_msgQ->httpMsgs.begin();
		// 	string recvMsgContent = *it;
		// 	m_msgQ->httpMsgs.erase(m_msgQ->httpMsgs.begin());
		// 	if (recvMsgContent != "")
		// 	{
		// 		printf("process http msg content:%s\n",recvMsgContent.c_str());
		// 		Json::Reader reader;
		// 		Json::Value jsonRoot;
		// 		Json::Value jsonVal;
		// 		if (reader.parse(recvMsgContent,jsonRoot))
		// 		{
		// 			string rootVal = jsonRoot["woRequest"].asString();
		// 			if (rootVal == "login")
		// 			{
		// 				m_msgQ->loginRet = m_httpManager->loginRequest(m_wisUrl,m_msgQ->strUserName,m_msgQ->strPw,m_machineId,"1");
		// 				if (m_msgQ->loginRet != "")
		// 				{
		// 					// string tmpData = m_msgQ->loginRet;
		// 					// if (tmpData.substr(0,1) == "[")
		// 					// {
		// 					// 	tmpData = tmpData.substr(1,tmpData.size()-1);
		// 					// 	while(tmpData.substr(tmpData.size()-1,1) != "]")
		// 					// 	{
		// 					// 		tmpData = tmpData.substr(0,tmpData.size()-1);
		// 					// 	}
		// 					// 	tmpData = tmpData.substr(0,tmpData.size()-1);
		// 					// 	m_msgQ->loginRet = tmpData;
		// 					// }
		// 					// printf("cut data ret:%s\n",tmpData.c_str());
		// 					//解析登陆信息 判断登陆是否成功
		// 					Json::Reader reader;
		// 					Json::Value loginRoot;
		// 					if (reader.parse(m_msgQ->loginRet,loginRoot))
		// 					{
		// 						if (loginRoot["success"].asBool())
		// 						{
		// 							m_msgQ->loginStatus = "success";
		// 							m_msgQ->logoutRet = "";									/* code */
		// 						}
		// 						else
		// 						{
		// 							m_msgQ->loginStatus = "unlogin";
		// 						}
		// 					}
		// 					else
		// 					{
		// 						printf("login retdata false!\n");
		// 					}
		// 				}
		// 				else
		// 				{
		// 					m_msgQ->loginRet = "请求失败";
		// 				}
		// 			}
		// 			else if (rootVal == "logout")
		// 			{
		// 				if (m_msgQ->loginStatus == "success")
		// 				{
		// 					/* code */
		// 				}
		// 				m_msgQ->logoutRet = m_httpManager->loginRequest(m_wisUrl,m_msgQ->strUserName,m_msgQ->strPw,m_machineId,"0");
		// 				if (m_msgQ->logoutRet != "")
		// 				{
		// 					Json::Reader reader;
		// 					Json::Value logoutRoot;
		// 					if (reader.parse(m_msgQ->logoutRet,logoutRoot))
		// 					{
		// 						if (logoutRoot["success"].asBool() == true)
		// 						{
		// 							m_msgQ->loginStatus = "unlogin";
		// 							m_msgQ->loginRet = "";
		// 							m_msgQ->strUserName = "";
		// 							m_msgQ->strPw = "";
		// 						}
		// 						else if (logoutRoot["success"].asBool() == false)
		// 						{
		// 							/* code */
		// 							m_msgQ->loginStatus = "success";
		// 						}
		// 						else
		// 						{
		// 							m_msgQ->loginStatus = "";
		// 						}
		// 					}
		// 					else
		// 					{
		// 						printf("logout ret data false!\n");
		// 					}
		// 				}
		// 				else
		// 				{
		// 					m_msgQ->logoutRet = "请求失败";
		// 				}
		// 			}
		// 			//刷新工单详情数据
		// 			// else if(rootVal == "refreshData")
		// 			// {
		// 			// }
		// 			//处理下载文件请求
		// 			// else if (rootVal == "download")
		// 			// {
						
		// 			// }
		// 			//手动报工
		// 			// else if (rootVal == "manualReport")
		// 			// {
		// 			// 	/* code */
		// 			// }
		// 			// else if (rootVal == "all")
		// 			// {
		// 				/* code */
		// 				// if ((m_allListUrl != "")&&(m_machineId != ""))
		// 				// {
		// 				// 	/* code */
		// 				// 	m_msgQ->oldWorkPList = m_httpManager->allListRequest(m_allListUrl,m_machineId);
		// 				// 	if (m_msgQ->oldWorkPList == "")
		// 				// 	{
		// 				// 		/* code */
		// 				// 		m_msgQ->oldWorkPList = "请求失败";
		// 				// 	}
		// 				// }
		// 			// }
		// 			else
		// 			{

		// 			}
		// 		}
		// 		else
		// 		{
		// 			printf("http msg type error!\n" );
		// 		}
		// 	}

		// }
		usleep(500000);
	}
}

void FCworkPiece::downloadFilesThreadRun()
{
	while(woStatus)
	{
		if (!m_msgQ->downloadCmds.empty())
		{
			vector<string>::iterator it;
			it = m_msgQ->downloadCmds.begin();
			string downloadCmd = *it;
			m_msgQ->downloadCmds.erase(m_msgQ->downloadCmds.begin());
			if (downloadCmd != "")
			{
				/* code */
				m_msgQ->downloadStatus = "start";

				Json::Reader reader;
				Json::Value valueRoot;
				Json::Value arrayVal;
				if (reader.parse(downloadCmd,valueRoot))
				{
					/* code */
					arrayVal = valueRoot["data"]["downloadList"];
					if (arrayVal.size() >= 1)
					{
						/* code */
						for (int i = 0; i < arrayVal.size(); ++i)
						{
							/* code */
							Json::Value arrayObj = arrayVal[i];
							string fileName = arrayObj["fileName"].asString();
							string downloadUrl = arrayObj["url"].asString();
							string md5 = arrayObj["md5"].asString();
							string tmpPath = m_strEvo.substr(0,m_strEvo.size()-3);
							string storePath = tmpPath + "program/" + fileName;
							// printf("file store path:%s\n", storePath.c_str());
							// printf("download url : %s\n",downloadUrl.c_str());
							//downloadFile方法目前未添加md5校验,暂不检测方法返回值
							bool downloadRet = m_httpManager->getDownloadFile(downloadUrl,storePath);
							if (downloadRet)
							{
								/* code */
								string fileMd5 = m_httpManager->GetFileMd5(storePath.c_str(),32);
								if (fileMd5 != md5)
								{
									/* code */
									m_msgQ->downloadStatus = fileName + "下载失败";
									printf("download file error msg:%s\n",m_msgQ->downloadStatus.c_str());
									break;
								}
							}
							else
							{
								m_msgQ->downloadStatus = fileName + "下载失败";
								break;
							}
							if (i == arrayVal.size()-1)
							{
								/* code */
								printf("*************give downloadStatus end!\n");
								m_msgQ->downloadStatus = "end";
							}
						}
						
					}
				}
				else
				{
					printf("downloadCmd json type false!\n");
				}
			}
			else
			{
			}
		}
		else
		{
			// m_msgQ->downloadStatus = "end";
		}
		usleep(500000);
	}
}

void FCworkPiece::localMQRecv(char *msgContent,char *topicName,int topicLen)
{
	string strMsg = msgContent;
	string strTopic = topicName;
	printf("localMQRecv:%s\ntopicName:%s\n",msgContent,topicName);
	if(strTopic == "WIS")
	{
		m_workPiece->m_msgQ->recvIportListCmd = msgContent;
		// m_workPiece->m_msgQ->recvLocalMQmsg.push_back(msgContent);
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

void FCworkPiece::getWoContent(const int &retStatus,const char *woContent)
{
	if (retStatus == 0)
	{
		printf("***************get wo content:%s\n",woContent );
		m_workPiece->m_msgQ->newWorkPList = woContent;
	}
	else
	{
		printf("error status:%d errror content%s\n",retStatus,woContent );
	}
}

string FCworkPiece::FCService(string servjson)
{
	printf("FCService recv:%s\n",servjson.c_str());

	// if (m_woDecoder != NULL)
	// {
	// 	/* code */
	// 	printf("***************test m_woDecoder\n");
		
	// }

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
					// if (m_msgQ->loginStatus == "success")
					// {
					// 	/* code */
						
					// }
					// else
					// {
					// 	replyRoot["woResponse"] = 0;
					// }
					replyRoot["woResponse"] = 0;
						// sendlocalMQ("requestWisList","WISpad");
						// m_msgQ->httpMsgs.push_back(servjson);
					printf("recv recvIportListCmd content:%s\n", m_msgQ->recvIportListCmd.c_str());
					// if (m_msgQ->recvIportListCmd != "")
					// {
						/* code */
						if (m_woDecoder != NULL)
						{
							/* code */
							// char *testData = "{\"FCode\": \"10012\",\"URL\": \"http://10.10.60.117:8080/download/78d1850f81684c108a59216521df51f1.txt\",\"MD5\": \"e27d8e0e391c5c465b7c7f557e3969a8\"}";
							if (m_msgQ->recvIportListCmd != "")
							{
								/* code */
								char* listCmd = (char *)m_msgQ->recvIportListCmd.c_str();
								(*m_woDecoder)(listCmd,&getWoContent);
							}
							
						}
					// }
				}
				else if(strWoRequest == "all_result")
				{
					//返回请求工单结果
					// if(m_msgQ->loginStatus == "success")
					// {
						
					// }
					// else
					// {
					// 	replyRoot["woResponse"] = 1;
					// 	replyRoot["data"] = "wis用户未登录";
					// }
					if(m_msgQ->newWorkPList != "")
					{
						replyRoot["woResponse"] = 1;
						replyRoot["data"] = m_msgQ->newWorkPList;
						// retStr = replyRoot.toStyledString();
						// printf("localMQ list content:%s\n",m_msgQ->oldWorkPList.c_str());
					}
					else
					{
						replyRoot["woResponse"] = 0;
					}
					
				}
				//初始化请求
				else if(strWoRequest == "init")
				{
					replyRoot["woResponse"] = 0;
					m_msgQ->manualReportList.clear();
					m_msgQ->workPDetailResult = "";
					m_msgQ->workPDetailRequest = "";
				}
				else if(strWoRequest == "init_result")
				{
					replyRoot["woResponse"] = 1;
				}
				//下载文件请求
				else if(strWoRequest == "download")
				{
					replyRoot["woResponse"] = 0;
					m_msgQ->downloadCmds.push_back(servjson);
				}
				else if (strWoRequest == "download_result")
				{
					/* code */
					if (m_msgQ->downloadStatus == "end")
					{
						/* code */
						replyRoot["woResponse"] = 1;
						replyRoot["data"] = "下载成功！";
						m_msgQ->downloadStatus = "";
					}
					else if ((m_msgQ->downloadStatus != "end")&&(m_msgQ->downloadStatus != "start")&&(m_msgQ->downloadStatus != ""))
					{
						/* code */
						replyRoot["woResponse"] = -1;
						replyRoot["data"] = m_msgQ->downloadStatus;
					}
					else
					{
						replyRoot["woResponse"] = 0;
					}
				}
				//请求工单详情
				else if (strWoRequest == "detail")
				{
					/* code */
					Json::Value jsonData = jsonRecv["data"];
					m_msgQ->workPDetailRequest = jsonData["jobDispatchNo"].asString();
					replyRoot["woResponse"] = 0;
				}
				else if (strWoRequest == "detail_result")
				{
					/* code */
					if (m_msgQ->workPDetailResult != "")
					{
						/* code */
						replyRoot["woResponse"] = 1;
						replyRoot["data"] = m_msgQ->workPDetailResult;
						m_msgQ->workPDetailResult = "";
					}
					else
					{
						replyRoot["woResponse"] = 0;
					}
				}
				//手动报工请求
				else if (strWoRequest == "manualReport")
				{
					/* code */
					m_msgQ->manualReportList.push_back(jsonRecv["data"].asString());
					replyRoot["woResponse"] = 0;
				}
				else if (strWoRequest == "manualReport_result")
				{
					/* code */
					if (m_msgQ->manualReportList.empty())
					{
						/* code */
						replyRoot["woResponse"] = 1;
					} 
					else
					{
						replyRoot["woResponse"] = 0;
					}
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
					// Json::Value jsonData = jsonRecv["data"];
					// m_msgQ->strUserName = jsonData["userName"].asString();
					// m_msgQ->strPw = jsonData["passWord"].asString();
					// printf("fc recv logout data content:%s\n",m_msgQ->strUserName.c_str());
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
