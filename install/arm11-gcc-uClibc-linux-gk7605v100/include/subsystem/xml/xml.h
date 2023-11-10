/*!
*****************************************************************************
** \file        xml.h
**
** \version     $Id: xml.h 1110 2016-07-26 17:26:58Z heyong
**
** \brief       xml
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef    __H_XML_H__
#define    __H_XML_H__

/***************************************************************************************
 * 
 * XML define
 *
***************************************************************************************/
#define NTYPE_TAG        			0
#define NTYPE_ATTRIB    			1
#define NTYPE_CDATA        			2

#define NTYPE_LAST        			2
#define NTYPE_UNDEF        			(-1)

#define XML_NO_PARENT    			0

#define XML_WS_BEFORE_OPEN        	0    /* Callback for before open tag */
#define XML_WS_AFTER_OPEN        	1    /* Callback for after open tag */
#define XML_WS_BEFORE_CLOSE        	2    /* Callback for before close tag */
#define XML_WS_AFTER_CLOSE        	3    /* Callback for after close tag */

#define XML_TAB         			8

#define LTXML_MAX_STACK_DEPTH    	1024
#define LTXML_MAX_ATTR_NUM        	128

#define CUR_PARSE_START        		1
#define CUR_PARSE_END        		2

/***************************************************************************************/
#define IS_XMLH_START(ptr) 			((*ptr == '<') && (*(ptr+1) == '?'))
#define IS_XMLH_END(ptr)   			((*ptr == '?') && (*(ptr+1) == '>'))
#define IS_XML_COMMENT_START(ptr)	((*ptr == '<') && (*(ptr+1) == '!') && (*(ptr+2) == '-') && (*(ptr+3) == '-'))
#define IS_XML_COMMENT_END(ptr)  	((*ptr == '-') && (*(ptr+1) == '-') && (*(ptr+2) == '>'))
#define IS_WHITE_SPACE(c)  			((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))

/***************************************************************************************/
#define IS_ELEMS_END(ptr)  			((*ptr == '>') || (*ptr == '/' && *(ptr+1) == '>'))
#define IS_ELEME_START(ptr)			((*ptr == '<') && (*(ptr+1) == '/'))

#define RF_NO_END        			0
#define RF_ELES_END        			2
#define RF_ELEE_END        			3

/***************************************************************************************/
#define CHECK_XML_STACK_RET(parse) \
    do{ if(parse->e_stack_index >= LTXML_MAX_STACK_DEPTH || parse->e_stack_index < 0) return -1;}while(0)
    
/***************************************************************************************/

typedef struct ltxd_xmlparser 
{
    char *xmlstart;
    char *xmlend;
    char *ptr;
    
    int xmlsize;

    char *e_stack[LTXML_MAX_STACK_DEPTH];
    int e_stack_index;                    

    const char *attr[LTXML_MAX_ATTR_NUM];

    void *userdata;
    void (*startElement)(void *userdata, const char *name, const char **attr);
    void (*endElement)(void *userdata, const char *name);
    void (*charData)(void *userdata, const char *str, int len);
}LTXMLPRS;

typedef struct XMLN
{
    char *name;
    unsigned int type;
    char *data;
    int dlen;
    int finish;
    struct XMLN *parent;
    struct XMLN *f_child;
    struct XMLN *l_child;
    struct XMLN *prev;
    struct XMLN *next;
    struct XMLN *f_attrib;
    struct XMLN *l_attrib;
}XMLN;

typedef const char *(*xml_save_ws_cb)(XMLN *, int);
typedef int (*xml_putc_cb)(int, void *);

#ifdef __cplusplus
extern "C" {
#endif

/************************************* node *********************************************/
XMLN *xml_node_add_new(XMLN *parent, char *nodeName);
XMLN *xml_node_add(XMLN *parent, XMLN *newNode);
void  xml_node_del(XMLN *rootNode);
XMLN *xml_node_get_child(XMLN *parent, const char *nodeName);
XMLN *xml_node_get_next(XMLN *curNode, const char *nodeName);
XMLN *xml_node_set_data(XMLN *node, const char *data);

/************************************** attr *********************************************/
XMLN *xml_attr_add(XMLN *node, const char *attrName, const char *attrData);
void xml_attr_del(XMLN *node, const char *attrName);
const char *xml_attr_get_data(XMLN *node, const char *attrName);
XMLN *xml_attr_get(XMLN *node, const char *attrName);

/***************************************************************************************/
void  xml_cdata_set(XMLN *node, const char *data, int dataLen);

/***************************************************************************************/
int xml_calc_buf_len(XMLN *node);
int xml_write_buf(XMLN *node, char *xmlBuf);
int xml_nwrite_buf(XMLN *node, char *xmlBuf, int bufLen);

/***************************************************************************************/
/* FIXME(heyong): 由于xml_stream_parse内部会改变p_xml，所以p_xml参数必须为可读写的buffer，不能为const类型的buffer */
XMLN *xml_stream_parse(char *p_xml, int len);
XMLN *xml_file_parse(FILE *fp);
/* FIXME(heyong): xml_save_ws_cb 为格式化处理函数，为NULL表示不进行格式输出 
简单实现如下：
static const char *config_xml_write_ws_cb(XMLN *node, int where)
{
    XMLN *parent = NULL;
    const char *name = NULL;
    int level;
    static const char *tabs = "\t\t\t\t\t\t\t\t";

    name = node->name;

    if (!strncmp(name, "?xml", 4))
    {
        if (where == XML_WS_AFTER_OPEN)
            return ("\n");
        else
            return (NULL);
    }
    else if (where == XML_WS_BEFORE_OPEN)
    {
        for (level = 0, parent = node->parent;parent;level++, parent = parent->parent);
        if ( level > 8)
            level = 8;
        return (tabs + 8 - level);
    }
    else if(where == XML_WS_BEFORE_CLOSE && (node->f_child != NULL && node->f_child->type == NTYPE_TAG))
    {
        for (level = 0, parent = node->parent;parent;level++, parent = parent->parent);
        if ( level > 8)
            level = 8;
        return (tabs + 8 - level);
    }
    else if (where == XML_WS_AFTER_CLOSE)
    {
        return ("\n");
    }
    else if (where == XML_WS_AFTER_OPEN && (node->f_child != NULL && node->f_child->type == NTYPE_TAG))
    {
        return ("\n");
    }

    return (NULL);
}
*/
int xml_file_save(XMLN *node, FILE *fp, xml_save_ws_cb cb);

/***************************************************************************************/
int soap_strcmp(const char *str1, const char *str2);
XMLN  *xml_node_soap_get(XMLN *parent, const char *name);

#ifdef __cplusplus
}
#endif

#endif /* __H_XML_H__ */



