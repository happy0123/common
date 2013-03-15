/*************************************************************
File name   : CharConvert.h
Author      : 殷勇
Version     : V1.0
Date        : 2012-03-06
Description : 简单的wchar_t 和 char 转换类, 且包含与UTF8的转换
Others      : 
Function List : 

History : 
---------------------------------------------------------------
Date          Author              Version          Modification
2012-03-06    殷  勇              V1.0             创建
***************************************************************/
#ifndef __CHAR_CONVERT_H__
#define __CHAR_CONVERT_H__

#ifdef OS_WINDOWS

#include <SDKDDKVer.h> //"targetver.h"

#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN             //  从 Windows 头文件中排除极少使用的信息
#endif

// Windows 头文件:
#include <windows.h>

#else // Linux
#include "iconv.h"

#ifndef ICONV_BUFFER_SIZE
#define ICONV_BUFFER_SIZE 1024
#endif

#define WCHAR wchar_t
#ifdef _UNICODE
#define TCHAR wchar_t
#else
#define TCHAR char
#endif

#endif
/////////////////////////////
#define E_CHAR      CWCharToChar::EChar
#define E_WCHAR     CWCharToChar::EWChar
#define E_UTF8      CWCharToChar::EUtf8

/////////////////////////////
// 简单的wchar_t 和 char 转换类, 且包含与UTF8的转换
class CWCharToChar
{
public:
    enum
    {
        EChar  = 1,
        EWChar = 2,
        EUtf8  = 4
    };
private:
    char    *   m_cDest;
    wchar_t *   m_wcDest;
    char    *   m_cUtf8;

    unsigned int m_nSrcType;

public:    
    char    *   Char(void) { return m_cDest; }
    wchar_t *   WChar(void){ return m_wcDest; }

#if defined( _UTF8_ )
    char    *   UTF8(void) { return m_cUtf8; }
#endif

    TCHAR   *   TChar(void)
    {
#ifdef _UNICODE
        return WChar();
#else
        return Char();
#endif
    }
    ////////////////////////////////////////////////////////////////////////
    // ANSI/GBK 转 Unicode和UTF8的构造函数

    CWCharToChar(const char* psrc, unsigned int nSrcType = EChar
#if defined( _UTF8_ )
                , unsigned int nDestType = EUtf8)
#else
                , unsigned int nDestType = EWChar)
#endif
        : m_cDest(NULL)
        , m_wcDest(NULL)
        , m_cUtf8(NULL)
        , m_nSrcType(nSrcType)
    {
        // 防止空指针
        if( !psrc || !*psrc ) return;

#ifdef OS_WINDOWS
        if( EChar == m_nSrcType )
        {
            m_cDest = (char*)psrc;

#if defined( _UTF8_ )
            int nLen = MultiByteToWideChar (CP_ACP, 0, m_cDest, -1, NULL, 0);
            m_wcDest = new wchar_t[nLen];
            MultiByteToWideChar (CP_ACP, 0, m_cDest, -1, m_wcDest, nLen);

            nLen = WideCharToMultiByte(CP_UTF8, 0, m_wcDest, -1, NULL, 0, NULL, NULL);
            m_cUtf8 = new char[nLen + 1];
            WideCharToMultiByte (CP_UTF8, 0, m_wcDest, -1, m_cUtf8, nLen, NULL,NULL);
#else
            int nLen = MultiByteToWideChar (CP_ACP, 0, m_cDest, -1, NULL, 0);
            m_wcDest = new wchar_t[nLen];
            MultiByteToWideChar (CP_ACP, 0, m_cDest, -1, m_wcDest, nLen);
#endif
        }
        else if( EUtf8 == m_nSrcType )
        {
            m_cUtf8 = (char*)psrc;

            int nLen = MultiByteToWideChar(CP_UTF8, 0, m_cUtf8, -1, NULL,0);
            m_wcDest = new wchar_t[nLen+1];
            MultiByteToWideChar(CP_UTF8, 0, m_cUtf8, -1, m_wcDest, nLen);

            nLen = WideCharToMultiByte(CP_ACP, 0, m_wcDest, -1, NULL, 0, NULL, NULL);
            m_cDest = new char[nLen + 1];
            WideCharToMultiByte(CP_ACP, 0, m_wcDest, -1, m_cDest, nLen, NULL,NULL);
        }
#else // Linux
        if( EChar == m_nSrcType )
        {
            m_cDest = (char*)psrc;
            if( nDestType & EUtf8 )
                do_iconv_convert(psrc, &m_cUtf8, "GBK", "UTF-8");

            if( nDestType & EWChar)
                do_iconv_convert(psrc, (char**)&m_wcDest, "GBK", "wchar_t");
        }
        else if( EUtf8 == m_nSrcType )
        {
            m_cUtf8 = (char*)psrc;
            if( nDestType & EChar)
                do_iconv_convert(psrc, &m_cDest, "UTF-8", "GBK");

            if( nDestType & EWChar)
                do_iconv_convert(psrc, (char**)&m_wcDest, "UTF-8", "wchar_t");
        }
#endif
    }
    ////////////////////////////////////////////////////////////////////////
    // Unicode 转 ANSI/GBK和UTF8的构造函数

    CWCharToChar(const wchar_t * psrc
#if defined( _UTF8_ )
                , unsigned int nDestType = EUtf8)
#else
                , unsigned int nDestType = EChar)
#endif
        : m_cDest(NULL)
        , m_wcDest(NULL)
        , m_cUtf8(NULL)
        , m_nSrcType(EWChar)
    {
        char * p = (char*)psrc;
        if( !p || (0 == *p && 0 == *(p+1)) ) return ;

        m_wcDest = (wchar_t*)psrc;

#ifdef OS_WINDOWS
        // ANSI/GBK
        {
            int nLen = WideCharToMultiByte(CP_OEMCP,NULL, m_wcDest,-1,NULL,0,NULL,FALSE);
            m_cDest = new char[nLen];
            WideCharToMultiByte (CP_OEMCP,NULL,m_wcDest,-1, m_cDest, nLen,NULL,FALSE);
        }

#if defined( _UTF8_ )
        {
            int nLen = WideCharToMultiByte(CP_UTF8, 0, m_wcDest, -1, NULL, 0, NULL, NULL);
            m_cUtf8 = new char[nLen + 1];
            WideCharToMultiByte (CP_UTF8, 0, m_wcDest, -1, m_cUtf8, nLen, NULL,NULL);
        }
#endif
#else  // Linux

        if( nDestType & EChar)
            do_iconv_convert((char*)psrc, &m_cDest, "wchar_t", "GBK");

        if( nDestType & EUtf8)
            do_iconv_convert((char*)psrc, (char**)&m_cUtf8, "wchar_t", "UTF-8");
#endif
    }

    ////////////////////////////////////////////////////////////////////////
    ~CWCharToChar()
    {
        if( EChar == m_nSrcType )
        {
            if( m_wcDest ) delete [] m_wcDest;
            if( m_cUtf8 )  delete [] m_cUtf8; 
        }
        else if( EWChar == m_nSrcType )
        {
            if( m_cDest )  delete [] m_cDest; 
            if( m_cUtf8 )  delete [] m_cUtf8; 
        }
        else if( EUtf8 == m_nSrcType )
        {
            if( m_cDest )  delete [] m_cDest; 
            if( m_wcDest ) delete [] m_wcDest;
        }
    }

    ////////////////////////////////////////////////////////////////////////
    // Using iconv  for Linux
#ifndef OS_WINDOWS

    int  do_iconv_convert(const char* pSrc, char**ppOut, const char* pszFromCode, const char* pszToCode)
    {
        iconv_t cd = iconv_open(pszToCode, pszFromCode); 
        if(cd == (iconv_t)(-1))
        {
            printf("iconv_open failed, errno: %d - %s\n", errno, strerror(errno));
            return -1;
        }
        iconv(cd, NULL, NULL, NULL, NULL);

        size_t nSLen = 0, nOLen = 0;
        if( m_nSrcType == EWChar )
#ifdef OS_WINDOWS
            nSLen = wcslen((wchar_t*)pSrc) * 2;  // 乘以2，转换成char的长度
#else  // Linux
            nSLen = wcslen((wchar_t*)pSrc) * 4;  // for Unicode -> GBK/UTF8, Linux 乘以4
#endif
        else
             nSLen = strlen(pSrc);

        if( 0 == strcmp( pszToCode, "wchar_t"))
            nOLen = nSLen * 4;        // for GBK -> Unicode
        else
            nOLen = nSLen * 2;
        
        size_t nORawLen = nOLen;
        char* pOut = new char[nOLen+4];
        char* pOutStart = pOut;
        
        char ** ppin = (char**)&pSrc;
        char ** ppout = &pOut;
#ifdef _DEBUG 
        printf("Convert: %s[%lu] -> %s[%lu] bytes\n", pszFromCode, nSLen, pszToCode, nOLen);
#endif
        size_t ret = iconv(cd, (char**)ppin, &nSLen, (char**)ppout, &nOLen);
        if( 0 == ret )  // 转换完成成功
        {
            int nOutLen = nORawLen - nOLen;
            *(pOutStart + nOutLen) = 0;
            *(pOutStart + nOutLen + 1) = 0; // 给转换Unicode之用
            *ppOut = pOutStart;

            iconv_close(cd);
            return nOutLen;
        }
        else  // 全部或部分错误
        {
            printf("iconv failed, errno: %d - %s\n", errno, strerror(errno));
            delete [] pOutStart;
            iconv_close(cd);
            return -1;
        }
    }

#endif
};

#endif //__CHAR_CONVERT_H__
