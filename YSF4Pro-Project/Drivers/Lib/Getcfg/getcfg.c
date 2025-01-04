#include "getcfg.h"
#include "string.h"

typedef char PARAMBUF[MAX_LEN_BYTES_OF_LINE+1];
/*
	return first visible char pointer from a string.
*/

#ifndef MIN
#define MIN(a,b) a<b?a:b
#endif





ConfigurationTokens  g_DefaultConfigurationTokens = DEFAULT_CONFIG_TOKENS;

static ConfigurationTokens g_ConfigurationTokens = DEFAULT_CONFIG_TOKENS;

void SetConfigTokens( const ConfigurationTokens * pTokens )
{
	g_ConfigurationTokens = *pTokens;
}
void SetConfigTokensDefault( ConfigurationTokens * pTokens )
{
	*pTokens = g_DefaultConfigurationTokens;
}
char * FirstVisibleBYTE( char* pStr)
{
	while( pStr )
	{
		switch( *pStr )
		{
		case '\t'	:	break;
		case ' '	:	break;
		default		:	return pStr;
		}
		++pStr;
	}
	return NULL;
}

/******************************************************************************
***ret	        : Return value
***buffer	: Source
***line_token	: Posible token of a line. token-标记
***return	: Position next search should be
******************************************************************************/

const char * ReadLnFromBuf(char *ret, int maxlen, const char *buffer, const char *line_token)
{
	char * sz;
	int i=0;

	sz = ret;
	for( i=0; (i<maxlen) && (*buffer); i++, sz++, buffer++ )
	{
          if( strchr( line_token, *buffer )) /* The char is a token 查找line_token出现在buffer中的位置*/
          {
                  for(;strchr( line_token, *buffer );buffer++);/* Skip tocken char */
                  break;
          }
            else
                 *sz = *buffer;
	}
	*sz = '\0';
	return buffer;
}

/*****************************************************************************
***GetConfiguration:                  Get configuration value from buffer
***success:                           return 0
***get failed, use default value:     return R_USE_DEFAULT
*****************************************************************************/
unsigned long GetConfigFromBuf(    const char     *pSectionName,    // section name
				   const char     *pKeyName,        // key name
				   const char     *pDefault,        // default string
				   char           *pReturnedString, // destination buffer
				   unsigned long   nSize,            // size of destination buffer
				   const char     *pBuffer          // Source configuration data buffer
				)
{
#if 1  
  static char pLine[MAX_LEN_BYTES_OF_LINE+1], pOneSectionName[MAX_LEN_BYTES_OF_LINE+1];//？？为什么要加1
  int	i,j,nIsMatch;
   char	*pLineHead = NULL, *pValueHead = NULL, 
        *pSectionNameHead = NULL, *pSectionTail = NULL, *pEquelMark = NULL;//等号=
   char	QuotationMark;//引号[]	
  unsigned long	nRetIndex, dwRet = R_USE_DEFAULT;
  
#endif
  memset( pLine, 0, MAX_LEN_BYTES_OF_LINE+1 );//将数组清0
  memset( pOneSectionName, 0, MAX_LEN_BYTES_OF_LINE+1 );
  memset( pReturnedString, 0, nSize );

  if( ( pSectionName ) && ( *pSectionName ) ) /*Want find section*/
  {
    while( pBuffer && (*pBuffer) )
    {//===1
        pBuffer = ReadLnFromBuf( pLine, MAX_LEN_BYTES_OF_LINE, pBuffer, g_ConfigurationTokens.LineEnd );
        pLineHead = FirstVisibleBYTE( pLine );
        if( !pLineHead ) continue;//如果第一个字节为0则跳出单次循环 执行while判断
        if( strchr( g_ConfigurationTokens.CommentLineBegin, *pLineHead ) ) continue; /*Comment line*/
        if( !strchr( g_ConfigurationTokens.SectionBegin, *pLineHead ) )/* Not a section head*[]*/
                continue;
        pSectionNameHead = FirstVisibleBYTE( pLineHead + 1 );/*Parse one section name*parse: 解析*/
        if( !pSectionNameHead )
                continue;
        i = 0;
        nIsMatch = 1;
        while( pSectionNameHead[i] && ( !strchr( g_ConfigurationTokens.SectionEnd, *pSectionTail) )
                && pSectionName[i])
        {
                if( pSectionNameHead[i] != pSectionName[i] )//大小写敏感应
                {
                        nIsMatch = 0;
                        break;
                }             
                i++;
        }
        if( !nIsMatch )
                continue;
        if( pSectionName[i] != '\0' )	/*Not full match*/
                continue;
        pSectionTail = FirstVisibleBYTE( &pSectionNameHead[i] );//tail:尾巴
        if( ! pSectionTail )	
                continue;
        if( !strchr( g_ConfigurationTokens.SectionEnd, *pSectionTail) )		/*This char is not section end char*/
                continue;
        else
                break;
    }//===1 
  }

	while( pBuffer && (*pBuffer) )	/*Want find key*/
	{
		pBuffer = ReadLnFromBuf( pLine, MAX_LEN_BYTES_OF_LINE, pBuffer, g_ConfigurationTokens.LineEnd );
		pLineHead = FirstVisibleBYTE( pLine );
		if( !pLineHead ) continue;
		if( strchr( g_ConfigurationTokens.CommentLineBegin, *pLineHead ) ) continue; /*Comment line*/
		if( ( pSectionName ) && ( *pSectionName ) )
			if( strchr( g_ConfigurationTokens.SectionBegin, *pLineHead ) ) break; /* Next section */

		i=0;
		nIsMatch = 1;
		while( pLineHead[i] && pKeyName[i] )
		{
	           if( pLineHead[i] != pKeyName[i] )
			{
				nIsMatch = 0;
				break;
			}
			i++;
		}
		if( !nIsMatch )
			continue;
		pEquelMark = FirstVisibleBYTE( &pLineHead[i] );
		if( !pEquelMark )
			continue;
		if( (*pEquelMark) != '=' )			/*Not full match key name*/		
			continue;
		
		/*Key name found and founded char '='*/
		pValueHead = FirstVisibleBYTE( pEquelMark+1 );	/*Get value addr*/
		nRetIndex = 0;
		if( ! pValueHead )		/*Empty value*/
		{
			*pReturnedString = '\0';
			dwRet = 0;
			goto end;
		}
		
		if( pValueHead[0] == '\'' || pValueHead[0] == '\"' )	/* Format "****" or '****' */
		{
			QuotationMark = pValueHead[0];
			j = 1;
			while( pValueHead[j] && ( nRetIndex < nSize) 
				&& ( pValueHead[j] != QuotationMark ) )
			{
				if( pValueHead[j] == '\\' )
				{
					switch( pValueHead[j+1] )
					{
					case '\''	:pReturnedString[nRetIndex] = '\'';break;
					case 't'	:pReturnedString[nRetIndex] = '\t';break;
					case 'n'	:pReturnedString[nRetIndex] = '\n';break;
					case 'r'	:pReturnedString[nRetIndex] = '\r';break;
					case '\"'	:pReturnedString[nRetIndex] = '\"';break;
					case '\\'	:pReturnedString[nRetIndex] = '\\';break;
					case '\0'	:pReturnedString[nRetIndex] = '\0';break;
					default		:pReturnedString[nRetIndex] = pValueHead[j+1];break;
					}
					j+=2;nRetIndex++;
				}
				else
				{
					pReturnedString[nRetIndex] = pValueHead[j];
					j++; nRetIndex++;
				}
			}
			dwRet = 0;
			goto end;
		}
		else
		{
			strncpy( pReturnedString, pValueHead, nSize );//copy
			dwRet = 0;
			goto end;
		}
	}
end:
	if( dwRet != 0 )
		if( pDefault )
			strncpy( pReturnedString, pDefault, nSize );//strncpy char * strncpy(char *s1,char *s2,size_t n);
                                                                   //将字符串s2中最多n个字符复制到字符数组s1中，返回指向s1的指针
	return dwRet;
}


