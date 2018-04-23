#include "msgQ.h"

MsgQ::MsgQ()
{
	oldWorkPList = "";
	newWorkPList = "{\"equSerialNo\": \"T001\",\"data_job\": [{\"time_StartPlan\": \"2017-07-24 09:44:57\",\"time_EndPlan\": \"2017-07-25 09:44:57\",\"jobDispatchRealStartTime\": \"2017-07-24 09:44:57\",\"jobDispatchProjectCount\": 30,\"jobDispatchRealFinishCount\": 0,\"jobDispatchNo\": \"2456\",\"processNo\": \"03\",\"partNo\": \"TS001\",\"status\": \"1\",\"data_technology\": [{\"uniqueID\": \"111\",\"documentName\": \"TS001\",\"type\": \"1\",\"path\": \"www.baidu.com/id=1?\"}, {\"uniqueID\": \"111\",\"documentName\": \"TS001\",\"type\": \"2\",\"path\": \"www.baidu.com/id=1?\"}],\"data_relatedJob\": [{\"relatedJobNo\": \"03\",\"status\": \"1\"}]}]}";

	strUserName = "";
	strPw = "";

	loginRet = "{\"msg\":\"\",\"data\":[{\"memberName\":\"胖虎\",\"memberNo\":\"222\",\"manualJobID\":1,\"uploadID\":1,\"proDeleteID\":1,\"toProgramID\":1,\"downloadID\":1,\"aotoModeID\":1,\"qualityInfoID\":1,\"position\": \"高工\",\"loginTime\": \"2017-02-01 12:12:38\",\"performance\": \"111\",\"department\": \"集成部\",\"path\": \"111\",\"documentName\": \"111\",\"positionLevel\": \"高级\"}],\"success\":true}";

	tmpLoginRet = "";
	loginStatus = "unlogin";



}

MsgQ::~MsgQ()
{

}
