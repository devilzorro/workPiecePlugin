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
	bool downloadFile(string downloadUrl,string storePath,string strMd5);
	string GetFileMd5(char *path, int md5_len);

private:
	void ByteToHexStr(const unsigned char* source, char* dest, int sourceLen);
};

#endif /* HTTPMANAGER_H_ */
