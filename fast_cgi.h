//fast_cgi.h
#ifndef FAST_CGI_H_
#define FAST_CGI_H_

#define FCGI_MAX_LENGTH 0xffff          // 允许传输的最大数据长度65536
#define FCGI_HOST       "127.0.0.1"     // php-fpm地址
#define FCGI_PORT       9000            // php-fpm监听的端口地址

#define FCGI_VERSION_1  1               // fastcgi协议版本


/* fastcgi通用协议报头,固定为8个字节,不同报头的类型通过type来区分 */
struct FCGI_Header
{
	unsigned char version; //fastcgi协议版本
	unsigned char type;  //操作类型
	unsigned char requestIdB1; //请求id唯一标识一条连接,最大值65535,等于socket的值;
	unsigned char requestIdB0;
	unsigned char contentLengthB1; //内容body长度
	unsigned char contentLengthB0;
	unsigned char paddingLength;  //填充字段
	unsigned char reserved; //保留字段
};

/* 请求头长度固定8字节 */
#define FCGI_HEADER_LEN 8

/* 上述FCGI_Header中type的具体值 */
#define FCGI_BEGIN_REQUEST       1      //开始请求
#define FCGI_ABORT_REQUEST       2      //异常终止请求
#define FCGI_END_REQUEST         3      //正常终止请求
#define FCGI_PARAMS              4      //传递参数
#define FCGI_STDIN               5      //POST内容传递
#define FCGI_STDOUT              6      //正常响应内容
#define FCGI_STDERR              7      //错误输出
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11      //通知webserver所请求type非正常类型
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)



/*=============================================================================*/
/*type=1,开始请求时body的格式 */
struct FCGI_BeginRequestBody
{
	unsigned char RoleB1; //web服务器希望php-fpm扮演的角色
	unsigned char RoleB0;
	unsigned char flags; //希望php-fpm处理完成后连接是否断开
	unsigned char reserved[5]; //保留字段
};

/* tpye=1,开始请求时整个record的格式 */
struct FCGI_BeginRequestRecord
{
	FCGI_Header header;  //请求头部
	FCGI_BeginRequestBody body; //请求体
};

/* web服务器希望php-fpm扮演的角色 */
#define FCGI_RESPONDER  1       //接受http关联的所有信息,并产生http响应，接受来自webserver的PARAMS环境变量
#define FCGI_AUTHORIZER 2       //对于认证的会关联其http请求,未认证的则关闭请求
#define FCGI_FILTER     3       //过滤web服务器中的额外数据流，并产生过滤后的http响应

/* 是否与php-fpm长连接 */
#define FCGI_KEEP_ALIVE 1

/*===============================================================================*/
/*type=3,结束时body的格式 */
struct FCGI_EndRequestBody
{
	unsigned char appStatusB3;
	unsigned char appStatusB2;
	unsigned char appStatusB1;
	unsigned char appStatusB0;
	unsigned char protocolStatus;  //协议状态
	unsigned char reserved[3]; //保留字段
};

/*type=3,结束时record的格式 */
struct FCGI_EndRequestRecord
{	
	FCGI_Header header;  //结束消息头部
	FCGI_EndRequestBody body;  //结束消息body
};

/* 几种结束状态 */
#define FCGI_REQUEST_COMPLETE 0     //正常结束
#define FCGI_CANT_MPX_CONN    1     //拒绝新请求，单线程
#define FCGI_OVERLOADED       2     //拒绝新请求，应用负载了
#define FCGI_UNKNOWN_ROLE     3     //webserver指定了一个应用不能识别的角色

#define FCGI_MAX_CONNS  "FCGI_MAX_CONNS"    //可接受的并发传输线路的最大值,eg "50"
#define FCGI_MAX_REQS   "FCGI_MAX_REQS"     //可接受并发请求的最大值,eg "100"
#define FCGI_MPXS_CONNS "FCGI_MPXS_CONNS"   //是否多路复用，其状态值也不同,"0"or"1"

/*==================================================================================*/

/*type=4时params */
/*struct FCGI_NameValuePair11{
unsigned char nameLengthB0; // nameLengthB0 >> 7 == 0 
unsigned char valueLengthB0; // valueLengthB0 >> 7 == 0
unsigned char nameData[nameLength];
unsigned char valueData[valueLength];
};

struct FCGI_NameValuePair14{
unsigned char nameLengthB0; // nameLengthB0 >> 7 == 0 
unsigned char valueLengthB3; // valueLengthB3 >> 7 == 1
unsigned char valueLengthB2;
unsigned char valueLengthB1;
unsigned char valueLengthB0;
unsigned char nameData[nameLength];
unsigned char valueData[valueLength
((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0];
};

struct FCGI_NameValuePair41{
unsigned char nameLengthB3; // nameLengthB3 >> 7 == 1
unsigned char nameLengthB2;
unsigned char nameLengthB1;
unsigned char nameLengthB0;
unsigned char valueLengthB0; // valueLengthB0 >> 7 == 0
unsigned char nameData[nameLength
((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0];
unsigned char valueData[valueLength];
};

struct FCGI_NameValuePair44{
unsigned char nameLengthB3; // nameLengthB3 >> 7 == 1
unsigned char nameLengthB2;
unsigned char nameLengthB1;
unsigned char nameLengthB0;
unsigned char valueLengthB3; // valueLengthB3 >> 7 == 1
unsigned char valueLengthB2;
unsigned char valueLengthB1;
unsigned char valueLengthB0;
unsigned char nameData[nameLength
((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0];
unsigned char valueData[valueLength
((B3 & 0x7f) << 24) + (B2 << 16) + (B1 << 8) + B0];
};*/

/*FastCGI以名字长度,后跟值的长度,后跟名字,后跟值的形式传送name-value对。
127字节或更少的长度能在一字节中编码,而更长的长度总是在四字节中编码,
长度的第一字节的高位指示长度的编码方式。
高位为0意味着一个字节的编码方式，1意味着四字节的编码方式。*/

/*==================================================================================*/
/*type的类型不能识别的body*/
struct FCGI_UnknownTypeBody
{
    unsigned char type;     
    unsigned char reserved[7];
};

struct FCGI_UnknownTypeRecord
{
    FCGI_Header header;
    FCGI_UnknownTypeBody body;
};

#endif
