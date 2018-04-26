#ifndef X3_EXAMPLE_SIMPLE_IMPL_H
#define X3_EXAMPLE_SIMPLE_IMPL_H
#include <portability/x3port.h>
#include <module/cmmacro.h>
#include "fcservplugin.h"
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <pthread.h>
#include "zfinch.h"
#include "msgQ.h"
#include "iniConfig.h"
#include "httpManager.h"

using namespace std;

const char* const keystr = "i5cnc.finch.service.plugins.net.i5wo";

class FCworkPiece : public FCObject
{
		X3BEGIN_CLASS_DECLARE(FCworkPiece,keystr)
		X3DEFINE_INTERFACE_ENTRY(FCObject)
		X3END_CLASS_DECLARE()

public:
	FCworkPiece();
	virtual ~FCworkPiece();

	virtual string FCService(string servjson);

	int sendlocalMQ(string msgContent,string topic);
	string sendFC(string msgContent);

	vector<string> split(string strContent,string mark);
	bool isContain(string strContent,string mark);

	static void localMQRecv(char *msgContent,char *topicName,int topicLen);
	static void localMQConnLost();

	static void * sendLocalMsgThread(void *arg);
	static void * processLocalMsgThread(void *arg);
	static void * processHttpMsgThread(void *arg);
	static void * downloadFilesThread(void *arg);

	virtual void sendLocalMsgThreadRun();
	virtual void processLocalMsgThreadRun();
	virtual void processHttpMsgThreadRun();
	virtual void downloadFilesThreadRun();

	string getEnvPath();

public:
//	RpcDealerZMQ *m_dealer;
	MsgQ *m_msgQ;
	CIni *m_ini;
	HTTPManager *m_httpManager;
	int localMQStatus;
	bool woStatus;
	string m_machineId;
	string m_wisUrl;
	string m_allListUrl;
	string m_refreshUrl;
	string m_strEvo;

private:
	static FCworkPiece* m_workPiece;
	pthread_t sendLocalMQId;
	pthread_t processLocalMsgId;
	pthread_t processHttpMsgId;
	pthread_t downloadFilesId;
};

#endif
;
