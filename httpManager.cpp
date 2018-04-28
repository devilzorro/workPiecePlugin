/*
 * httpManager.cpp
 *
 *  Created on: 2018-4-18
 *      Author: fiyang
 */
#include "httpManager.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <openssl/md5.h>
#include <string.h>
#include <algorithm>
#include <vector>

HTTPManager* HTTPManager::m_http;

HTTPManager::HTTPManager()
{
	m_http = this;
	storeHttpRecv = "";
	recvStatus = "";
}

HTTPManager::~HTTPManager()
{

}

size_t HTTPManager::write_data(char *buffer,size_t size,size_t nitems,void *outstream)
{
    int written = fwrite(buffer,size,nitems,(FILE*)outstream);
    return written;
}

size_t HTTPManager::write_str_call_back(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	string str;
	string storeMsgContent;
	str.append(ptr, size*nmemb);
	string* p = (string*)userdata;
	*p = str;
	printf("curl recv str : %s\n",str.c_str());
	if (m_http->recvStatus == "start")
	{
		/* code */
		if ((m_http->storeHttpRecv == "")&& (str.substr(0,1) == "{"))
		{
		/* code */
			m_http->storeHttpRecv = str;
		}
		else if((m_http->storeHttpRecv != ""))
		{
			m_http->storeHttpRecv = m_http->storeHttpRecv + str;
		}
	}
	else if (m_http->recvStatus == "end")
	{
		/* code */
	}
	

	return size*nmemb;
}

string HTTPManager::getStr(string strUrl)
{
	string strret = "";
	CURL* pCurl = curl_easy_init();
	if(pCurl != NULL)
	{
		curl_easy_setopt(pCurl,CURLOPT_WRITEDATA,(void*)&strret);
		//curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30 );
		curl_easy_setopt(pCurl,CURLOPT_WRITEFUNCTION,write_str_call_back);
		curl_easy_setopt(pCurl, CURLOPT_MAXREDIRS, 5);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(pCurl,CURLOPT_CONNECTTIMEOUT,10);
		curl_easy_setopt(pCurl,CURLOPT_TIMEOUT,10);
		curl_easy_setopt(pCurl,CURLOPT_URL,strUrl.c_str());

		recvStatus = "start";
		if(curl_easy_perform(pCurl) == CURLE_OK)
		{
			recvStatus = "end";
			strret = storeHttpRecv;
			storeHttpRecv = "";
			curl_easy_cleanup(pCurl);
			printf("curl download finish!\n");
		}
		else
		{
			recvStatus = "end";
			storeHttpRecv = "";
			curl_easy_cleanup(pCurl);
			printf("curl download fail!\n");
		}
	}
	else
	{
		printf("curl init fail!\n");
	}
	return strret;
}

string HTTPManager::postStr(string strUrl,string strCmd)
{
	string strRet = "";
	string storeMsgContent = "";
	CURL* pCurl = curl_easy_init();
	if (pCurl != NULL)
	{
		/* code */
		curl_easy_setopt(pCurl,CURLOPT_URL,strUrl.c_str());
		curl_easy_setopt(pCurl,CURLOPT_POSTFIELDS,strCmd.c_str());
		curl_easy_setopt(pCurl,CURLOPT_WRITEFUNCTION,write_str_call_back);
		curl_easy_setopt(pCurl,CURLOPT_WRITEDATA,(void*)&strRet);
		curl_easy_setopt(pCurl,CURLOPT_POST,1);
		curl_easy_setopt(pCurl,CURLOPT_VERBOSE,1);
		curl_easy_setopt(pCurl,CURLOPT_HEADER,1);
		curl_easy_setopt(pCurl,CURLOPT_FOLLOWLOCATION,1);
		curl_easy_setopt(pCurl,CURLOPT_CONNECTTIMEOUT,10);
		curl_easy_setopt(pCurl,CURLOPT_TIMEOUT,10);

		recvStatus = "start";
		if(curl_easy_perform(pCurl) == CURLE_OK)
		{
			recvStatus = "end";
			strRet = storeHttpRecv;
			storeHttpRecv = "";
			printf("http post final recv%s\n",strRet.c_str());
			// if ((storeMsgContent == "")&& (strRet.substr(0,1) == "{")&&(strRet.substr(strRet.length()-1,1) != "}"))
			// {
			// 	/* code */
			// 	storeMsgContent = strRet;
			// }
			// else if((storeMsgContent != "")&&(strRet.substr(0,1) != "{")&&(strRet.substr(strRet.length()-1,1) == "}"))
			// {
			// 	storeMsgContent = storeMsgContent + strRet;
			// }
			curl_easy_cleanup(pCurl);
			printf("curl post!\n");
		}
		else
		{
			recvStatus = "end";
			storeHttpRecv = "";
			curl_easy_cleanup(pCurl);
			printf("curl post fail!\n");
			// strRet = "请求失败";
		}
	}

	if ((storeMsgContent != "")&&(storeMsgContent.substr(storeMsgContent.length()-1,1) == "}"))
	{
		return storeMsgContent;
	}
	else
	{
		return strRet;
	}

	return strRet;
}

bool HTTPManager::downloadFile(string downloadUrl,string storePath,string strMd5)
{
	CURL* pCurl = curl_easy_init();
	if(pCurl != NULL)
	{
		FILE *pfile = fopen(storePath.c_str(),"wb");
		curl_easy_setopt(pCurl,CURLOPT_WRITEDATA,(void*)pfile);
		//curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 30 );
		curl_easy_setopt(pCurl,CURLOPT_WRITEFUNCTION,write_data);
		curl_easy_setopt(pCurl, CURLOPT_MAXREDIRS, 5);
		curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(pCurl,CURLOPT_URL,downloadUrl.c_str());
		if(curl_easy_perform(pCurl) == CURLE_OK)
		{
			fclose(pfile);
			curl_easy_cleanup(pCurl);
			printf("curl download finish!\n");
		}
		else
		{
			fclose(pfile);
			curl_easy_cleanup(pCurl);
			printf("curl download fail!\n");
			return false;
		}
	}
	else
	{
		printf("curl init fail!\n");
		return false;
	}
	return true;
}

string HTTPManager::GetFileMd5(char *path, int md5_len)
{
	FILE *fp;
	#ifdef WIN32
	fopen_s(&fp,path, "rb");
	#else
	fp = fopen(path,"rb");
	#endif
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	unsigned char digest[32] = { 0 };
	char file_md5[35];
	string strfile_md5;
	//int i;

	if (fp == NULL)
	{
		fprintf(stderr, "fopen %s failed\n", path);
		fclose(fp);
		return "";
	}

	MD5_Init(&mdContext);
	while ((bytes = fread(data, 1, 1024, fp)) != 0)
	{
		MD5_Update(&mdContext, data, bytes);
	}
	MD5_Final(digest, &mdContext);

	memset(file_md5, '\0', 35);

	if (md5_len == 16)
	{
		ByteToHexStr(digest, file_md5, 32);
		strfile_md5 = file_md5;
		strfile_md5 = strfile_md5.substr(0, 16);
		transform(strfile_md5.begin(), strfile_md5.end(), strfile_md5.begin(), (int (*)(int))tolower);

	}
	else if (md5_len == 32)
	{
		ByteToHexStr(digest, file_md5, 32);
		strfile_md5 = file_md5;
		strfile_md5 = strfile_md5.substr(0, 32);
		transform(strfile_md5.begin(), strfile_md5.end(), strfile_md5.begin(), (int (*)(int))tolower);
	}
	else
	{
		fclose(fp);
		return "";
	}

	fclose(fp);
	return strfile_md5;
}

string HTTPManager::loginRequest(string strUrl,string strUserName,string strPW,string strMachineId,string logStatus)
{
	string strRet = "";
	string requestContent = "memID=" + strUserName + "&memPass=" + strPW
			+ "&equSerialNo=" + strMachineId + "&loginStatus=" + logStatus;
	printf("post url content:%s,post params:%s\n",strUrl.c_str(),requestContent.c_str());
	strRet = postStr(strUrl,requestContent);
	return strRet;
}

string HTTPManager::allListRequest(string strUrl,string strMachineId)
{
	string strRet = "";
	string requestContent = "equSerialNo=" + strMachineId;
	printf("post url content:%s,post params:%s\n",strUrl.c_str(),requestContent.c_str());
	strRet = postStr(strUrl,requestContent);
	return strRet;
}

string HTTPManager::woDetailRequest(string strUrl,string strMachineId,string jobNum,string memId)
{
	string strRet = "";
	string requestContent = "equSerialNo=" + strMachineId + "&jobDispatchNo=" + jobNum + "&memberID=" + memId;
	printf("post url content:%s,post params:%s\n",strUrl.c_str(),requestContent.c_str());
	strRet = postStr(strUrl,requestContent);
	return strRet;
}

void HTTPManager::ByteToHexStr(const unsigned char* source, char* dest, int sourceLen)
{
	short i;
	unsigned char highByte, lowByte;

	for (i = 0; i < sourceLen; i++)
	{
		highByte = source[i] >> 4;
		lowByte = source[i] & 0x0f;

		highByte += 0x30;

		if (highByte > 0x39)
			dest[i * 2] = highByte + 0x07;
		else
			dest[i * 2] = highByte;

		lowByte += 0x30;
		if (lowByte > 0x39)
			dest[i * 2 + 1] = lowByte + 0x07;
		else
			dest[i * 2 + 1] = lowByte;
	}
}
