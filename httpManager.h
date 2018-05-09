/*
 * httpManager.h
 *
 *  Created on: 2018-4-18
 *      Author: fiyang
 */

#ifndef HTTPMANAGER_H_
#define HTTPMANAGER_H_

#include <string>
#include <stdio.h>

using namespace std;

class HTTPManager
{
public:
	HTTPManager();
	~HTTPManager();

	static size_t write_data(char *buffer,size_t size,size_t nitems,void *outstream);
	static size_t write_str_call_back(char *ptr, size_t size, size_t nmemb, void *userdata);

	string getStr(string strUrl);
	string postStr(string strUrl,string cmd);
	bool getDownloadFile(string downloadUrl,string storePath);
	bool postDownloadFile(string downloadUrl,string storePath);
	string GetFileMd5(const char *path, int md5_len);

	string loginRequest(string strUrl,string strUserName,string strPassWord,string strMachineId,string logStatus);
	string allListRequest(string strUrl,string strMachineId);
	string woDetailRequest(string strUrl,string strMachineId,string jobNum,string memId);

private:
	void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen);

private:
	static HTTPManager* m_http;
	string storeHttpRecv;
	string recvStatus;
};

#endif /* HTTPMANAGER_H_ */
