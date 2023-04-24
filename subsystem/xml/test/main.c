
/*
 * Simple test for libxml.
 *
 * Author : Heyong
 * Date   : 2016.06.16
 * Copyright (c) 2016 Harric He(Heyong)
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml.h"

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


int xml_save_test(const char *FilePATH)
{
    XMLN *xmlTitle = NULL;
    XMLN *imageNode = NULL;
    XMLN *FPSNode = NULL;

    FILE *fp = fopen(FilePATH, "wb");
    if (!fp) {
        printf("open %s config file failed!\n", FilePATH);
        return -1;
    }

    xmlTitle = xml_node_add_new(XML_NO_PARENT, "?xml version=\"1.0\" encoding=\"UTF-8\"?");
    imageNode = xml_node_add_new(XML_NO_PARENT, "image");
	xml_attr_add(imageNode, "width", "1920");
	xml_attr_add(imageNode, "height", "1080");
	xml_node_set_data(imageNode, "imageNode");
				
    FPSNode   = xml_node_add_new(imageNode, "FPS");
	xml_node_set_data(FPSNode, "25");

#if 1/* xml_save_ws_cb 为格式化处理函数，为NULL表示不进行格式输出 */
    xml_file_save(xmlTitle, fp, config_xml_write_ws_cb);
    xml_file_save(imageNode, fp, config_xml_write_ws_cb);
#else
    xml_file_save(xmlTitle, fp, NULL)
    xml_file_save(imageNode, fp, NULL);
#endif
    fclose(fp);

    xml_node_del(FPSNode);
    xml_node_del(imageNode);
    xml_node_del(xmlTitle);
    return 0;
}

int xml_parse_test(char *FilePATH)
{
    XMLN *rootNode = NULL;
    XMLN *FPSNode = NULL;

    FILE *fp = fopen(FilePATH, "rb");
    if (!fp) {
        printf("open %s config file failed!\n", FilePATH);
        return -1;
    }
    rootNode = xml_file_parse(fp);//rootNode == imageNode
    if (!rootNode) {
        printf("Unable to read imageNode\n");
        return -1;
    }
    fclose(fp);
	
	printf("width = %s\n", xml_attr_get_data(rootNode, "width"));
	printf("height = %s\n", xml_attr_get_data(rootNode, "height"));
	
	FPSNode = xml_node_get_child(rootNode, "FPS");
    if (!FPSNode) {
        printf("Unable to read FPSNode\n");
        return -1;
    }
	printf("FPS = %s\n", FPSNode->data);
	
    xml_node_del(FPSNode);
    xml_node_del(rootNode);
	return 0;
}

char *xml_buffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
<image width=\"1024\" height=\"720\">\
<FPS>30</FPS>\
</image>";

char *xml_buf2 = "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\
<XML_TOPSEE>\
<MESSAGE_HEADER \
Msg_type=\"SYSTEM_SEARCHIPC_MESSAGE\"\
Msg_code=\"1\"\
Msg_flag=\"0\"\
/>\
<MESSAGE_BODY>\
</MESSAGE_BODY>\
</XML_TOPSEE>";

int xml_buffer_parse_test(char *p_xml, int len)
{
    XMLN *rootNode = NULL;
    XMLN *FPSNode = NULL;

    char XMLdata[1024] = {0};
	
	/* FIXME(heyong): p_xml为const类型，不能直接用于xml_stream_parse */
    strcpy(XMLdata, p_xml);

    printf("XMLdata:%s\n", XMLdata);
    
    rootNode = xml_stream_parse(XMLdata, len);//rootNode == imageNode
    if (!rootNode) {
        printf("Unable to read imageNode\n");
        return -1;
    }
    printf("111:%s\n", rootNode->data);
    
	printf("width = %s\n", xml_attr_get_data(rootNode, "width"));
	printf("height = %s\n", xml_attr_get_data(rootNode, "height"));
	
	FPSNode = xml_node_get_child(rootNode, "FPS");
    if (!FPSNode) {
        printf("Unable to read FPSNode\n");
        return -1;
    }
	printf("FPS = %s\n", FPSNode->data);
	
    xml_node_del(FPSNode);
    xml_node_del(rootNode);
	return 0;
}

int xml_buffer_parse_test2(char *p_xml, int len)
{
    XMLN *rootNode = NULL;
    XMLN *FPSNode = NULL;

    char XMLdata[1024] = {0};
	
	/* FIXME(heyong): p_xml为const类型，不能直接用于xml_stream_parse */
    strcpy(XMLdata, p_xml);

    printf("XMLdata:%s\n", XMLdata);

    rootNode = xml_stream_parse(XMLdata, len);//rootNode == imageNode
    if (!rootNode) {
        printf("Unable to read imageNode\n");
        return -1;
    }
    printf("111:%s\n", rootNode->data);

	XMLN *header_node = xml_node_get_child(rootNode, "MESSAGE_HEADER");
    if (!header_node) {
        printf("Unable to read XML_TOPSEE\n");
        return -1;
    }
    printf("header_node = %s\n", header_node->data);
    
	printf("Msg_type = %s\n", xml_attr_get_data(header_node, "Msg_type"));
	printf("Msg_code = %s\n", xml_attr_get_data(header_node, "Msg_code"));
    printf("Msg_flag = %s\n", xml_attr_get_data(header_node, "Msg_flag"));

    xml_node_del(header_node);
    xml_node_del(rootNode);
	return 0;
}

int main (int argc, const char * argv[])
{
	printf("xml test BEGIN\n");
	printf(">>>>xml_save_test..\n");
	xml_save_test("./xmltest.xml");
	printf(">>>>xml_parse_test..\n");
	xml_parse_test("./xmltest.xml");
	printf(">>>>xml_buffer_parse_test..\n");
	xml_buffer_parse_test(xml_buffer, strlen(xml_buffer));
	printf("xml test END\n");

    printf("len:%d\n", strlen(xml_buf2));
    xml_buffer_parse_test2(xml_buf2, strlen(xml_buf2));
	printf("xml test END\n");
	return 0;
}
