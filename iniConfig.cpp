/*
 * iniConfig.cpp
 *
 *  Created on: 2018-4-19
 *      Author: fiyang
 */
#include "iniConfig.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
* 功  能：构造函数
* 参  数：无
* 返回值：无
* 备  注：
******************************************************************************/
CIni::CIni( )
{
 memset( m_szKey,0,sizeof(m_szKey) );
 m_fp = NULL;
}

/******************************************************************************
* 功  能：析构函数
* 参  数：无
* 返回值：无
* 备  注：
******************************************************************************/

CIni::~CIni()
{
 m_Map.clear();
}

/******************************************************************************
* 功  能：打开文件函数
* 参  数：无
* 返回值：
* 备  注：
******************************************************************************/
INI_RES CIni::OpenFile(const char* pathName, const char* type)
{
 string szLine,szMainKey,szLastMainKey,szSubKey;
 char strLine[ CONFIGLEN ] = { 0 };
 KEYMAP mLastMap;
 int  nIndexPos = -1;
 int  nLeftPos = -1;
 int  nRightPos = -1;
    m_fp = fopen(pathName, type);

    if (m_fp == NULL)
    {
  printf( "open inifile %s error!\n",pathName );
        return INI_OPENFILE_ERROR;
    }

 m_Map.clear();

 while( fgets( strLine, CONFIGLEN,m_fp) )
 {
  szLine.assign( strLine );
  //删除字符串中的非必要字符
  nLeftPos = szLine.find("\n" );
  if( string::npos != nLeftPos )
  {
   szLine.erase( nLeftPos,1 );
  }
  nLeftPos = szLine.find("\r" );
  if( string::npos != nLeftPos )
  {
   szLine.erase( nLeftPos,1 );
  }
  //判断是否是主键
  nLeftPos = szLine.find("[");
  nRightPos = szLine.find("]");
  if(  nLeftPos != string::npos && nRightPos != string::npos )
  {
   // szLine.erase( nLeftPos,1 );
   // nRightPos--;
   // szLine.erase( nRightPos,1 );
    nLeftPos++;
   mLastMap.clear();
   szLastMainKey =  szLine.substr(nLeftPos,(nRightPos-nLeftPos)) ;
   m_Map[ szLastMainKey ] = mLastMap;
  // printf("ini main key:%s\n",szLastMainKey.c_str());
  }
  else
  {
   //是否是子键
   if( nIndexPos = szLine.find("="),string::npos != nIndexPos)
   {
    string szSubKey,szSubValue;
    szSubKey = szLine.substr( 0,nIndexPos );
    szSubValue = szLine.substr( nIndexPos+1,szLine.length()-nIndexPos-1);
    szSubKey = cutFront(szSubKey," ");
    szSubKey = cutEnd(szSubKey," ");
    if(szSubValue != ""&&szSubValue != " ")
    {
    	szSubValue = cutFront(szSubValue," ");
    	szSubValue = cutEnd(szSubValue," ");
    }
    mLastMap[szSubKey] = szSubValue ;
   // printf("subkey content:%s***",szSubKey.c_str());
   // printf("subVal content:%s\n",szSubValue.c_str());
    m_Map[ szLastMainKey ] = mLastMap;
   }
   else
   {
    //TODO:不符合ini键值模板的内容 如注释等
   }
  }

 }
 //插入最后一次主键
// m_Map[ szLastMainKey ] = mLastMap;

    return INI_SUCCESS;
}

/******************************************************************************
* 功  能：关闭文件函数
* 参  数：无
* 返回值：
* 备  注：
******************************************************************************/
INI_RES CIni::CloseFile()
{


    if (m_fp != NULL)
    {
        fclose(m_fp);
  m_fp = NULL;
    }

    return INI_SUCCESS;
}

/******************************************************************************
* 功  能：获取[SECTION]下的某一个键值的字符串
* 参  数：
*  char* mAttr  输入参数    主键
*  char* cAttr  输入参数 子键
*  char* value  输出参数 子键键值
* 返回值：
* 备  注：
******************************************************************************/
INI_RES CIni::GetKey(const char* mAttr, const char* cAttr, char* pValue)
{
 KEYMAP mKey = m_Map[mAttr];

// map<string,string>::iterator it;
// it = mKey.begin();
// while(it != mKey.end())
// {
// 	 printf("getKey key:%s\n",it->first.c_str());
// 	 printf("getKey val:%s\n",it->second.c_str());
// 	 it++;
// }

 string sTemp = mKey[cAttr];

 printf("get ini value string:%s\n",sTemp.c_str());

 strcpy( pValue,sTemp.c_str() );

 printf("get ini value char:%s\n",pValue);

    return INI_SUCCESS;
}

/******************************************************************************
* 功  能：获取整形的键值
* 参  数：
*       cAttr                     主键
*      cAttr                     子键
* 返回值：正常则返回对应的数值 未读取成功则返回0(键值本身为0不冲突)
* 备  注：
******************************************************************************/
int CIni::GetInt(const char* mAttr, const char* cAttr )
{
 int nRes = 0;

 memset( m_szKey,0,sizeof(m_szKey) );

 if( INI_SUCCESS == GetKey( mAttr,cAttr,m_szKey ) )
 {
  nRes = atoi( m_szKey );
 }
 return nRes;
}

/******************************************************************************
* 功  能：获取键值的字符串
* 参  数：
*       cAttr                     主键
*      cAttr                     子键
* 返回值：正常则返回读取到的子键字符串 未读取成功则返回"NULL"
* 备  注：
******************************************************************************/
char *CIni::GetStr(const char* mAttr, const char* cAttr )
{
 memset( m_szKey,0,sizeof(m_szKey) );

 if( INI_SUCCESS != GetKey( mAttr,cAttr,m_szKey ) )
 {
  strcpy( m_szKey,"NULL" );
 }

// string tmpStr = m_szKey;
// if(tmpStr != "" || tmpStr != " ")
// {
//	 tmpStr = cutFront(tmpStr," ");
//	 tmpStr = cutEnd(tmpStr," ");
//	 for(int i= 0;i<tmpStr.length();i++)
//	 {
//		 m_szKey[i] = tmpStr[i];
//	 }
//	 m_szKey[tmpStr.length()] = '\0';
// }
// m_szKey = tmpStr.c_str();

 return m_szKey;

}

string CIni::cutFront(string strContent,string mark)
{
	string ret = strContent;
	while(ret.substr(0,1) == mark)
	{
		ret = ret.substr(1,ret.size()-1);
	}
	return ret;
}

string CIni::cutEnd(string strContent,string mark)
{
	string ret = strContent;
	while(ret.substr(ret.size()-1,1) == mark)
	{
		ret = ret.substr(0,ret.size()-1);
	}
	return ret;
}
