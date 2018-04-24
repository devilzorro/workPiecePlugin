#include <string>
#include <string.h>
#include <vector>

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

	string oldWorkPList;
	string newWorkPList;

	string strUserName;
	string strPw;

	string tmpLoginRet;
	string loginRet;
	string loginStatus;

	string tmpLogoutRet;
	string logoutRet;
	string logoutStatus;
};
