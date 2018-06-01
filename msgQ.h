#include <string>
#include <string.h>
#include <vector>
#include <map>

using namespace std;

class MsgQ
{
public:
	MsgQ();
	~MsgQ();

	vector<string> recvLocalMQmsg;
	vector<string> sendLocalMQ;
	vector<string> recvFCmsg;
	vector<string> sendFCmsg;
	vector<string> httpMsgs;
	vector<string> downloadCmds;
	vector<string> manualReportList;

	string oldWorkPList;
	string newWorkPList;
	string testList;
	string recvIportListCmd;

	string strUserName;
	string strPw;

	string tmpLoginRet;
	string loginRet;
	string loginStatus;

	string tmpLogoutRet;
	string logoutRet;
	string logoutStatus;

	string downloadStatus;

	string workPDetailResult;
	string workPDetailRequest;

	map<string, string> downloadFiles;
};
