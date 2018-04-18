/*
 * httpManager.cpp
 *
 *  Created on: 2018-4-18
 *      Author: fiyang
 */
#include "httpManager.h"
#include <curl/curl.h>
#include <curl/easy.h>

HTTPManager::HTTPManager()
{

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
    str.append(ptr, size*nmemb);
    string* p = (string*)userdata;
    *p = str;
    printf("curl recv str : %s\n",str.c_str());
    return size*nmemb;
}

string HTTPManager::getStr(string strUrl)
{
	return "";
}
