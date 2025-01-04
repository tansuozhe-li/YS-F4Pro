#ifndef GET_CONFIGURATION_H_
#define GET_CONFIGURATION_H_

#include "comm_types.h"

#define MAX_LEN_BYTES_OF_LINE	200
#define R_USE_DEFAULT	1

typedef struct _st_configuration_tokens
{
	char * SectionBegin;
	char * SectionEnd;
	char * LineEnd;
	char * CommentLineBegin;
	char * EqualMark;
}ConfigurationTokens;

#define DEFAULT_CONFIG_TOKENS {"[","]","\r\n","#","="}

/*	return first visible char pointer from a string.*/
char * FirstVisibleBYTE( char* pStr);


/*	success:return 0
	get failed, use default value: return R_USE_DEFAULT	*/
#ifdef __cplusplus 
extern "C"{   //????
#endif

void SetConfigTokens( const ConfigurationTokens * pTokens );
void SetConfigTokensDefault( ConfigurationTokens * pTokens );

extern ConfigurationTokens g_DefaultConfigurationTokens;
#define UseDefaultConfigTokens() SetConfigTokens( &g_DefaultConfigurationTokens )

unsigned long GetConfigFromBuf( const char     *pSectionName,    // section name
				const char     *pKeyName,        // key name
				const char     *pDefault,        // default string
				char           *pReturnedString, // destination buffer
				unsigned long  nSize,            // size of destination buffer
				const char     *pBuffer          // Source configuration data buffer
				);

#ifdef __cplusplus
	}
#endif

#endif /*_GET_CONFIGURATION_H_*/

