//FCGI.cpp

#include "FCGI.h"

FCGI::FCGI()
{
	sockfd = 0;
}

FCGI::~FCGI()
{
	//反初始化
	close(sockfd);
}

void FCGI::start_connect(void)
{
    struct sockaddr_in server_address;  //php-fpm server

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    assert(sockfd > 0);

    memset(&server_address, 0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(FCGI_HOST);
    server_address.sin_port = htons(FCGI_PORT);

    int result = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    assert(result >= 0);

}

FCGI_Header FCGI::make_header(int type, int requestId, int contentLength, int paddingLength)
{
	FCGI_Header header;
    
    header.version = FCGI_VERSION_1;
    
    header.type    = (unsigned char)type;   
    
    header.requestIdB1 = (unsigned char)((requestId >> 8) & 0xff);  //用连个字段保存请求ID
    header.requestIdB0 = (unsigned char)(requestId & 0xff);
  
    header.contentLengthB1 = (unsigned char)((contentLength >> 8) & 0xff);//用俩个字段保存内容长度
    header.contentLengthB0 = (unsigned char)(contentLength & 0xff);

    header.paddingLength = (unsigned char)paddingLength;        //填充字节的长度

    header.reserved = 0;    //保留字节赋为0

    return header;
}

FCGI_BeginRequestBody FCGI::make_begin_request_body(int role, int aliveFlags)
{
	FCGI_BeginRequestBody body;

    body.RoleB1 = (unsigned char)((role >> 8) & 0xff);//俩个字节保存我们期望php-fpm扮演的角色
    body.RoleB0 = (unsigned char)(role & 0xff);

    body.flags = (unsigned char)((aliveFlags) ? FCGI_KEEP_ALIVE : 0);//大于0常连接，否则短连接

    memset(&body.reserved, 0, sizeof(body.reserved));

    return body;
}

void FCGI::make_name_value_pair_body(std::string name, int nameLen,
	                               std::string value, int valueLen,
	                               unsigned char *bodybuffer, int *bodyLen)
{
	unsigned char *startbodybuffer = bodybuffer;  //记录body的开始位置
    
    if(nameLen < 128)//如果nameLen长度小于128字节
    {
        *bodybuffer++ = (unsigned char)nameLen;    //nameLen用一个字节保存
    }
    else
    {
        //nameLen用4个字节保存
        *bodybuffer++ = (unsigned char)((nameLen >> 24) | 0x80);
        *bodybuffer++ = (unsigned char)(nameLen >> 16);
        *bodybuffer++ = (unsigned char)(nameLen >> 8);
        *bodybuffer++ = (unsigned char)nameLen;
    }

    if(valueLen < 128)  //valueLen小于128就用一个字节保存
    {
        *bodybuffer++ = (unsigned char)valueLen;
    }
    else
    {
        //valueLen用4个字节保存
        
        *bodybuffer++ = (unsigned char)((valueLen >> 24) | 0x80);
        *bodybuffer++ = (unsigned char)(valueLen >> 16);
        *bodybuffer++ = (unsigned char)(valueLen >> 8);
        *bodybuffer++ = (unsigned char)valueLen;
    }

    //将name中的字节逐一加入body的buffer中
    for(auto ch : name)
    {
        *bodybuffer++ = ch;
    }

    //将value中的值逐一加入body的buffer中
    for(auto ch : value)
    {
        *bodybuffer++ = ch;
    }

    //计算出body的长度
    *bodyLen = bodybuffer - startbodybuffer;
}

void FCGI::send_begin_request_record(void)
{
	FCGI_BeginRequestRecord beginRecord;

    beginRecord.header = make_header(FCGI_BEGIN_REQUEST, sockfd, sizeof(beginRecord.body), 0);
    beginRecord.body = make_begin_request_body(FCGI_RESPONDER, 0);

    int ret = write(sockfd,(char *)&beginRecord, sizeof(beginRecord));
    assert(ret == sizeof(beginRecord));
}

void FCGI::send_params_record(std::string name, std::string value)
{
	unsigned char bodyBuff[PARAMS_BUFF_LEN];
    
    memset(bodyBuff, 0, sizeof(bodyBuff));
    
    int bodyLen;//保存body的长度
    
    //生成PARAMS参数内容的body,并得到长度
    make_name_value_pair_body(name, name.size(), value, value.size(), bodyBuff, &bodyLen);
    
    FCGI_Header nameValueHeader;
    nameValueHeader = make_header(FCGI_PARAMS, sockfd, bodyLen, 0);

    int nameValueRecordLen = bodyLen + FCGI_HEADER_LEN;
    char nameValueRecord[nameValueRecordLen];

    //将头和body拷贝到一块buffer中只需调用一次write
    memcpy(nameValueRecord, (char *)&nameValueHeader, FCGI_HEADER_LEN);
    memcpy(nameValueRecord + FCGI_HEADER_LEN, bodyBuff, bodyLen);

    int ret = write(sockfd, nameValueRecord, nameValueRecordLen);
    assert(ret == nameValueRecordLen);
}

void FCGI::send_empty_params_record(void)
{
    int ret;
    FCGI_Header nvHeader = make_header(FCGI_PARAMS, sockfd, 0, 0);
    ret = write(sockfd, (char *)&nvHeader, FCGI_HEADER_LEN);

    assert(ret == FCGI_HEADER_LEN);
}

void FCGI::send_stdin_record(struct evbuffer *recv_evb)
{
	int content_len = evbuffer_get_length(recv_evb); //总共发送的长度
    int cut_len;  //一次发送的长度
    int padding_len;  //填充长度
    char data[CONTENT_BUFF_LEN + FCGI_HEADER_LEN]; //存储发送数据
    char pad[8] = {0};  //填充字节
    int ret;

    while(content_len > 0)
    {
        cut_len = content_len;
        /* 如果request的content的内容大于最大发送长度,每次发送content_buff_len */
        if(content_len > CONTENT_BUFF_LEN)
        {
            cut_len = CONTENT_BUFF_LEN;
        }

        /* 计算填充长度 */
        padding_len = (cut_len % 8) == 0 ? 0 : (8 - (cut_len % 8));

        FCGI_Header stdinHeader = make_header(FCGI_STDIN, sockfd, cut_len, padding_len);

        memcpy(data, (char *)&stdinHeader, FCGI_HEADER_LEN);
        //ret = write(sockfd, (char *)&stdinHeader, FCGI_HEADER_LEN);  // 发送协议头部
        //assert(ret == FCGI_HEADER_LEN);

        evbuffer_remove(recv_evb, data + FCGI_HEADER_LEN, cut_len); //读取cutlen的数据放入data + 8的位置
        //ret = write(sockfd, data, cut_len); // 发送stdin数据
        //assert(ret == cut_len);

        if (padding_len > 0) {
            //ret = write(sockfd, pad, padding_len); // 发送填充数据
            memcpy(data + FCGI_HEADER_LEN + cut_len, pad, padding_len);
        }

        /* 一起发送减少write次数 */
        ret = write(sockfd, data, FCGI_HEADER_LEN + cut_len + padding_len);
        assert(ret == FCGI_HEADER_LEN + cut_len + padding_len);

        content_len -= cut_len;

    }
}

void FCGI::send_empty_stdin_record(void)
{
	int ret;
    FCGI_Header stdinHeader = make_header(FCGI_STDIN, sockfd, 0, 0);
    ret = write(sockfd, (char *)&stdinHeader, FCGI_HEADER_LEN);

    assert(ret == FCGI_HEADER_LEN);
}

bool FCGI::read_from_php_fpm(struct evbuffer *evb)
{
	FCGI_Header responderHeader;
    char content[CONTENT_BUFF_LEN];
    int contentLen;
    char tmp[8];    //用来暂存padding字节的
    int ret;

    //先将头部8个字节度出来,数据量过大会分块
    while(read(sockfd, &responderHeader, FCGI_HEADER_LEN) > 0)
    {
        if(responderHeader.type == FCGI_STDOUT)
        {
            //获取内容长度
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
            memset(content, '\0', CONTENT_BUFF_LEN);
            //读取获取的内容
            ret = read(sockfd, content, contentLen);
            //assert(ret == contentLen);
            fprintf(stdout,"%s\n",content);
            printf("ret = %d\n",ret);

            //跳过填充部分
            if(responderHeader.paddingLength > 0)
            {
                ret = read(sockfd, tmp, responderHeader.paddingLength);
                //assert(ret == responderHeader.paddingLength);
            }

            //将内容保存在evbuffer里
            evbuffer_add(evb, content, strlen(content));
        }
        else if(responderHeader.type == FCGI_STDERR)
        {
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
            memset(content, '\0', CONTENT_BUFF_LEN);

            ret = read(sockfd, content, contentLen);
            //assert(ret == contentLen);

            fprintf(stdout,"error:%s\n",content);

            //跳过填充部分

            if(responderHeader.paddingLength > 0)
            {
                ret = read(sockfd, tmp, responderHeader.paddingLength);
                //assert(ret == responderHeader.paddingLength);
            }

            return false;

        }
        else if(responderHeader.type == FCGI_END_REQUEST)
        {
            FCGI_EndRequestBody endRequest;

            ret = read(sockfd, &endRequest, sizeof(endRequest));
            //assert(ret == sizeof(endRequest));

            //fprintf(stdout,"endRequest:appStatus:%d\tprotocolStatus:%d\n",(endRequest.appStatusB3 << 24) + (endRequest.appStatusB2 << 16)
              //      + (endRequest.appStatusB1 << 8) + (endRequest.appStatusB0),endRequest.protocolStatus);
        }
    }
    return true;
}