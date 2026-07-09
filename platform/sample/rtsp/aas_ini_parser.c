#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "aas_ini_parser.h"


#define AAS_INI_SECTION_MAX     32  //段个数
#define AAS_INI_SECTION_NAME    64  //段名长度

#define AAS_INI_MAX_KEY         64     //单段最大key个数
#define AAS_INI_MAX_CONTENT     512    //最大内容长数
enum aas_ini_line_attr
{
    eINI_LINE_ATTR_SECTION		= 0,
    eINI_LINE_ATTR_KEY 		    = 1,
    eINI_LINE_ATTR_EXPLAIN 	    = 2,
    eINI_LINE_ATTR_BL		    = 3,    //blank line

    eINI_LINE_ATTR_NUM,
};


typedef struct _aas_ini_section
{
    char section[AAS_INI_SECTION_NAME];

    char key[AAS_INI_MAX_KEY][AAS_INI_SECTION_NAME];
    int  iKeyMaxLen;
    char content[AAS_INI_MAX_KEY][AAS_INI_MAX_CONTENT];
    int type[AAS_INI_MAX_KEY]; // 0:section; 1:key; 2:explain; 3:blank line
    int num;    //有效行数
}aas_ini_section;

typedef struct _aas_ini_info
{
    aas_ini_section *pSection[AAS_INI_SECTION_MAX];
    int  iSecNo;
}aas_ini_info;


/**
 * aas_ini_get_space_line: aas_ini_get_space_line
 * @this[IN]: this
 * return: 0 - success; otherwise error code;
 */
int aas_ini_get_space_line(aas_ini_section *this)
{
    this->key[this->num][0] = ' ';
    this->content[this->num][0] = ' ';
    this->type[this->num] = eINI_LINE_ATTR_BL;
    this->num++;
    return 0;
}

/**
 * aas_ini_get_explain: aas_ini_get_explain
 * @this[IN]: this
 * @buff[IN]: buff
 * @len[IN]: len
 * return: 0 - success; otherwise error code;
 */
int aas_ini_get_explain(aas_ini_section *this,const char *buff,int len)
{
    this->key[this->num][0] = '#';
    memcpy(&this->content[this->num], buff,len);
    this->type[this->num] = eINI_LINE_ATTR_EXPLAIN;
    this->num++;
    //printf("\explain:%s\n",this->content[this->num]);

    return 0;
}

/**
 * aas_ini_get_section: aas_ini_get_section
 * @this[IN]: this
 * @buff[IN]: buff
 * @len[IN]: len
 * return: 0 - success; otherwise error code;
 */
int aas_ini_get_section(aas_ini_section *this,const char *buff,int len)
{
    int k = 0;
    for(int i = 0;i < len; i++)
    {
        if(buff[i] == '[')
        {
            for(int j=i+1;j < len; j++)
            {
                if(buff[j] == ']')
                {
                    break;
                }
                this->section[k++] = buff[j];
            }
        }
    }
    this->type[this->num] = 0;
    this->num++;
    //printf("\nattr:%s\n",this->section);
    return 0;
}

/**
 * aas_ini_letter_is_legal: aas_ini_letter_is_legal
 * @letter[IN]: letter
 * return: 0 - success; otherwise error code;
 */
int aas_ini_letter_is_legal(char letter)
{
    if((letter >= 'a' && letter <= 'z') || (letter >= 'A' && letter <= 'Z')
        || (letter >= '0' && letter <= '9') || letter == '_')
    {
        return 1;
    }
    return 0;
}

/**
 * aas_ini_get_key_and_content: aas_ini_get_key_and_content
 * @this[IN]: this
 * @buff[IN]: buff
 * @len[IN]: len
 * return: 0 - success; otherwise error code;
 */
int aas_ini_get_key_and_content(aas_ini_section *this,const char *buff,int len)
{
    int cnt = 0;
    int i = 0;
    for(i = 0;i < len; i++)
    {
        if(buff[i] == 32 || buff[i] == 9) //space and tab
        {
            continue;
        }
        break;
    }
    int k =  0;
    for(int j = i; j < len; j++)
    {
        if(aas_ini_letter_is_legal(buff[j]) == 0)
        {
            break;
        }
        this->key[this->num][k++] = buff[j];
        cnt++;
    }
    //printf("key:%s\n",this->key[this->num]);
    this->iKeyMaxLen = (cnt > this->iKeyMaxLen) ? cnt : this->iKeyMaxLen;

    for(i = 0;i < len; i++)
    {
        if(buff[i] != '=') //content
        {
            continue;
        }
        break;
    }
    int start = 0;
    k = 0;
    for(int j = i + 1; j < len; j++)
    {
        if(buff[j] == '\n' && start == 1)
        {
            break;
        }
        else if(buff[j] == ' ' && start == 0)
        {
            continue;
        }
        start = 1;
        this->content[this->num][k++] = buff[j];
        this->type[this->num] = 1;
    }

    for(int n = strlen(this->content[this->num]);n > 0;n--)
    {
        if(this->content[this->num][n-1] == ' ')
        {
            this->content[this->num][n-1] = 0;
        }
        else
        {
            break;
        }
    }

    //printf("content:%s\n",this->content[this->num]);
    this->num++;

    return 0;
}


/**
 * aas_ini_get_attr: 0:section; 1:key; 2:explain; 3:blank line
 * @pStr[OUT]: pStr
 * return: 0 - success; otherwise error code;
 */
int aas_ini_get_attr(const char *pStr)
{
    int i = 0;
    int len = strlen(pStr);
    for(i = 0;i < len; i++)
    {
        if(pStr[i] == ' ')
        {
            continue;
        }
        break;
    }

    if(i == len || len <= 2 )
    {
        return eINI_LINE_ATTR_BL;
    }
    else if(pStr[i] == '\n')
    {
        return eINI_LINE_ATTR_BL;
    }

    if(pStr[i] == '[')
    {
        for(int j = i;j < len; j++)
        {
            if(pStr[j] == ']')
            {
                 return 0;
            }
        }
        return -1;
    }

    if(pStr[i] == ';' || pStr[i] == '#' || pStr[i] == '/')
    {
        return eINI_LINE_ATTR_EXPLAIN;
    }

    if(strstr(pStr,"=") == NULL)
    {
        return -2;
    }
    return eINI_LINE_ATTR_KEY;
}

/**
 * aas_ini_rm_section_end_blank_line: aas_ini_rm_section_end_blank_line
 * @section[IN]: section
 * return: 0 - success; otherwise error code;
 */
int aas_ini_rm_section_end_blank_line(aas_ini_section *section)
{
    for(int j = section->num - 1; j >= 0; j--)
       {
           if(section->type[j] == eINI_LINE_ATTR_BL)
           {
               memset(section->content[j],0,sizeof(section->content[j]) );
               memset(section->key[j],0,sizeof(section->key[j]) );
               section->num--;  //remove the end's blank line
               section->type[j] = -1;
               continue;
           }
           break;
       }
    return 0;
}

/**
 * aas_ini_rm_end_blank_line: aas_ini_rm_end_blank_line
 * @h[IN]: ini info
 * return: 0 - success; otherwise error code;
 */
int aas_ini_rm_end_blank_line(aas_ini_info *h)
{
    for(int i = 0;i < h->iSecNo + 1;i++)
    {
        aas_ini_rm_section_end_blank_line(h->pSection[i]);
    }
    return 0;
}

/**
 * aas_ini_load: aas_ini_load
 * @file[IN]: file
 * return: 0 - success; otherwise error code;
 */
AAS_INI_HANDLE aas_ini_load(const char *file)
{
    FILE *fd = NULL;
    if ((fd=fopen(file, "r"))==NULL)
    {
        printf("%s(%d):cannot open %s\n", __FUNCTION__,__LINE__,file);
        return NULL ;
    }
    char buff[1024] = {0};
    aas_ini_info *h = NULL;
    h = (aas_ini_info *)malloc(sizeof(aas_ini_info));
    if(h == NULL)
    {
        if(fd)
        {
            fclose(fd);
        }
        printf("%s(%d):malloc error %s\n", __FUNCTION__,__LINE__,file);
        return NULL;
    }
    memset(h,0,sizeof(aas_ini_info));
    h->iSecNo = -1;
    int iSpaceLine = 0;
    while(fgets(buff, sizeof(buff), fd) != NULL)
    {
        int len = strlen(buff);
        //纠错
        int type = aas_ini_get_attr(buff);
        if(type == 0) // section
        {
            if(h->iSecNo == AAS_INI_SECTION_MAX - 1)
            {
                printf("%s(%d):max section %d\n", __FUNCTION__,__LINE__,h->iSecNo);
                break;
            }
            void *this = malloc(sizeof(aas_ini_section));
            if(this == NULL)
            {
                printf("%s(%d):malloc error %s\n", __FUNCTION__,__LINE__,file);
                goto error;
            }
            memset(this,0,sizeof(aas_ini_section));
            h->iSecNo++;
            h->pSection[h->iSecNo] = (aas_ini_section*)this;
            aas_ini_get_section(h->pSection[h->iSecNo],buff,len);
            iSpaceLine = 0;
            memset(buff,0,sizeof(buff));
            continue;
        }
        if(h->iSecNo < 0) continue;

        if(type == eINI_LINE_ATTR_KEY)//1:key
        {
            aas_ini_get_key_and_content(h->pSection[h->iSecNo],buff,len);
            iSpaceLine = 0;
        }
        else if(type == eINI_LINE_ATTR_EXPLAIN)//2:explain;
        {
            aas_ini_get_explain(h->pSection[h->iSecNo],buff,len);
            iSpaceLine = 0;
        }
        else if(type == eINI_LINE_ATTR_BL)//3:blank line
        {
            if(iSpaceLine == 0)
            {
                aas_ini_get_space_line(h->pSection[h->iSecNo]);
                iSpaceLine++;
            }
        }
        else
        {
            printf("ini file is illegel.\n");
            goto error;
        }
        memset(buff,0,sizeof(buff));
    }
    aas_ini_rm_end_blank_line(h);

    fclose(fd);
    return h;
error:
    if (h != NULL)
    {
        for(int i = 0;i <= h->iSecNo;i++)
        {
            if(h->pSection[i] == NULL)
            {
                break;
            }
            else
            {
                free(h->pSection[i]);
            }
        }

        free(h);
    }
    
    if(fd)
    {
        fclose(fd);
    }

    printf("%s(%d):error %s\n", __FUNCTION__,__LINE__,file);
    return NULL;
}
/* end of aas_ini_load */

/**
 * aas_ini_free: aas_ini_free
 * @handle[IN]: handle
 * return: 0 - success; otherwise error code;
 */
int aas_ini_free(AAS_INI_HANDLE handle)
{
    if(handle)
    {
        aas_ini_info *h = (aas_ini_info *)handle;
        for(int i = 0;i <= h->iSecNo;i++)
        {
            if(h->pSection[i] == NULL)
            {
                break;
            }
            else
            {
                free(h->pSection[i]);
            }
        }
        free(h);
    }
    return 0;
}

/**
 * aas_ini_save_section: aas_ini_save_section
 * @this[IN]: this
 * @fd[IN]: fd
 * return: 0 - success; otherwise error code;
 */
int aas_ini_save_section(aas_ini_section *this, FILE *fd)
{
    char section[128] = {0};
    char buff[1024] = {0};
    int len = sprintf(section,"[%s]\n",this->section);
    fwrite(section,1,len,fd);
    int i = 0;
    for(i = 1; i < this->num;i++)
    {
        if(this->type[i] == eINI_LINE_ATTR_KEY)//key
        {
            int iKeyLen = strlen(this->key[i]);

            len = sprintf(buff,"%s",this->key[i]);
            for(int k = 0; k < this->iKeyMaxLen + 1 - iKeyLen;k++)
            {
                len += sprintf(buff + len,"%c",' ');
            }
            len += sprintf(buff + len,"%s","= ");
            len += sprintf(buff + len,"%s\n",this->content[i]);
        }
        if(this->type[i] == eINI_LINE_ATTR_EXPLAIN)//explain
        {
            len = sprintf(buff,"%s",this->content[i]);
        }
        if(this->type[i] == eINI_LINE_ATTR_BL)//blank line
        {
            len = sprintf(buff,"\n");
        }
        fwrite(buff,1,len,fd);
        memset(buff,0,sizeof(buff));
    }
    fwrite("\n",1,1,fd);
    return 0;
}

/**
 * aas_ini_save_file: aas_ini_save_file
 * @handle[IN]: handle
 * @file[IN]: file
 * return: 0 - success; otherwise error code;
 */
int aas_ini_save_file(AAS_INI_HANDLE handle, const char *file)
{
    if(handle == NULL)
    {
        printf("handle(%p) is illegel.\n",handle);
        return -1;
    }
    int i = 0;
    aas_ini_info *h = (aas_ini_info *)handle;

    FILE *fd = NULL;
    if ((fd=fopen(file, "w+"))==NULL)
    {
      printf("%s(%d):cannot open %s\n", __FUNCTION__,__LINE__,file);
      return -1 ;
    }
    for(i = 0;i < h->iSecNo + 1;i++)
    {
        aas_ini_save_section(h->pSection[i], fd);
    }
    fclose(fd);
    return 0;
}

/**
 * aas_ini_get_content: aas_ini_get_content
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * return: str
 */
const char *aas_ini_get_content(AAS_INI_HANDLE handle,const char *section,const char *key)
{
    if(handle == NULL)
    {
        printf("handle(%p) is illegel.\n",handle);
        return NULL;
    }
    int i = 0;
    aas_ini_info *h = (aas_ini_info *)handle;

    int iSecNo = h->iSecNo + 1;
    for(i = 0; i < iSecNo; i++)
    {
        if(strcmp(section,h->pSection[i]->section) == 0)
        {
            break;
        }
    }
    if(i == iSecNo)
    {
       printf("session:%s is not exist.\n",section);
       return NULL;
    }

    int j = 0;
    int iFindFlag = 0;
    for(j = 0; j < h->pSection[i]->num; j++)
    {
        if(strcmp(key,h->pSection[i]->key[j]) == 0)
        {
            //strncpy(string,h->pSection[i]->content[j],len);
            iFindFlag = 1;
            break;
        }
    }
    //no find key
    if(iFindFlag == 0)
    {
        printf("(session:%s) key:%s is not exist.\n",section,key);
        return NULL;
    }
    return h->pSection[i]->content[j];
}

/**
 * aas_ini_get_int: aas_ini_get_int
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @def[IN]: def
 * return: value
 */
int aas_ini_get_int(AAS_INI_HANDLE handle,const char *section,const char *key,int def)
{
    const char *p = NULL;
    int result = def;
    p = aas_ini_get_content(handle,section,key);
    if(p != NULL)
    {
        result = atoi(p);
        //printf("section:%s key:%s value:%d\n",section,key,result);
    }
    return result;
}

/**
 * aas_ini_set_content: aas_ini_set_content
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @string[IN]: string
 * return: 0 - success; otherwise error code;
 */
int aas_ini_set_content(AAS_INI_HANDLE handle,const char *section,const char *key,char* string)
{
    if(handle == NULL)
    {
        printf("handle(%p) is illegel.\n",handle);
        return -1;
    }
    int i = 0;
    aas_ini_info *h = (aas_ini_info *)handle;
    int iSecNo = h->iSecNo + 1;
    for(i = 0; i < iSecNo; i++)
    {
        if(strcmp(section,h->pSection[i]->section) == 0)
        {
            break;
        }
    }
    if(i == iSecNo)//new session
    {
        if(h->iSecNo == AAS_INI_SECTION_MAX - 1)
        {
            printf("%s(%d):insert iSecNo:%d > AAS_INI_SECTION_MAX\n", __FUNCTION__,__LINE__,h->iSecNo);
            return -1;
        }
        printf("insert new session(%s) and key(%s)\n",section,key);
        h->iSecNo++;
        void *this = malloc(sizeof(aas_ini_section));
        if(this == NULL)
        {
            printf("aas_ini_set_content line =%d malloc error.\n",__LINE__);
            return -1;
        }
        memset(this,0,sizeof(aas_ini_section));
        h->pSection[h->iSecNo] = (aas_ini_section*)this;
        strcpy(h->pSection[h->iSecNo]->section,section);
        h->pSection[h->iSecNo]->num++;
    }

    int j = 0;
    for(j = 0; j < h->pSection[i]->num; j++)
    {
        if(strcmp(key,h->pSection[i]->key[j]) == 0)
        {
            memset(h->pSection[i]->content[j],0,sizeof(h->pSection[j]->content[j]));
            strcpy(h->pSection[i]->content[j],string);
            break;
        }
    }

    if(j == h->pSection[i]->num)
    {
        int iKeyLen = strlen(key);
        strcpy(h->pSection[i]->key[j],key);
        h->pSection[i]->iKeyMaxLen = (iKeyLen > h->pSection[i]->iKeyMaxLen) ? iKeyLen : h->pSection[i]->iKeyMaxLen;
        sprintf(h->pSection[i]->content[j],"%s",string);
        h->pSection[i]->num++;
        h->pSection[i]->type[j] = 1;
    }

    return 0;
}

/**
 * aas_ini_set_int: aas_ini_set_int
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @value[IN]: value
 * return: 0 - success; otherwise error code;
 */
int aas_ini_set_int(AAS_INI_HANDLE handle,const char *section,const char *key,int value)
{
    char buff[64] = {0};
    sprintf(buff,"%d",value);
    return aas_ini_set_content(handle,section,key,buff);
}

/**
 * aas_ini_get_double: aas_ini_get_double
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @def[IN]: def
 * return: value
 */
double aas_ini_get_double(AAS_INI_HANDLE handle,const char *section,const char *key,double def)
{
    const char *p = NULL;
    double result = def;
    p = aas_ini_get_content(handle,section,key);
    if(p != NULL)
    {
        result = atof(p);
    }
    return result;
}

/**
 * aas_ini_set_double: aas_ini_set_double
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @value[IN]: value
 * return: 0 - success; otherwise error code;
 */
int aas_ini_set_double(AAS_INI_HANDLE handle,const char *section,const char *key,double value)
{
    char buff[64] = {0};
    sprintf(buff,"%lf",value);
    for(int i = strlen(buff) - 1; i > 1;i--)
    {
        if(buff[i] == '0' && buff[i-1] != '.')
        {
            buff[i] = 0;
            continue;
        }
        break;
    }
    return aas_ini_set_content(handle,section,key,buff);
}

/**
 * aas_ini_get_string: aas_ini_get_string
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @def[IN]: def
 * return: str
 */
const char *aas_ini_get_string(AAS_INI_HANDLE handle,const char *section,const char *key,char* def)
{
    const char *p = NULL;
    p = aas_ini_get_content(handle,section,key);
    if(p)
    {
        return p;
    }
    else
    {
        return def;
    }
}

/**
 * aas_ini_set_string: aas_ini_set_string
 * @handle[IN]: handle
 * @section[IN]: section
 * @key[IN]: key
 * @string[IN]: string
 * return: 0 - success; otherwise error code;
 */
int aas_ini_set_string(AAS_INI_HANDLE handle,const char *section,const char *key,char* string)
{
    return aas_ini_set_content(handle,section,key,string);
}

