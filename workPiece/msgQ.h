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

	string oldWorkPList;
	string newWorkPList;

};
