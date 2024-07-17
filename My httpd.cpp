/*日期：2024-7-10
* 开发思路:
* 1- 网络通信初始化: startUp()  //返回一个套接字
* 2- 循环监听客户端请求, 创建线程: accept() , CreateThread()
* 3- 在新线程中处理客户端请求: accept_request()
* 

*ps:
* 1-套接字: 用于描述网络上两个节点之间建立的双向通信链路
* 1.1-套接字的组成: IP地址、端口号(port)、协议类型
* 
* 2-
* 
*/

//这个数据包是客户端发送的GET请求
/*数据包格式：
	*	GET / HTTP/1.1\n  // HTTP请求方法和版本
	*	Host: 127.0.0.1:8000\n  // 请求的主机地址和端口
	*	Connection: keep-alive\n  // 保持连接
	*	Cache-Control: max-age=0\n  // 缓存控制，不使用缓存
	*	Upgrade-Insecure-Requests: 1\n  // 请求升级到HTTPS
	*	User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36\n  // 用户代理信息
	*	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,* \
	*	/*; q = 0.8, application / signed - exchange; v = b3; q = 0.9\n  // 可接受的响应内容类型
	*	Sec - Fetch - Site: none\n  // 请求的来源站点
	*	Sec - Fetch - Mode: navigate\n  // 请求的模式
	*	Sec - Fetch - User: ? 1\n  // 用户导航请求
	*	Sec - Fetch - Dest: document\n  // 请求的目标
	*	Accept - Encoding: gzip, deflate, br\n  // 可接受的编码方式
	*	Accept - Language: zh - CN, zh; q = 0.9\n  // 可接受的语言
	*	\n
	*/



#include<stdio.h>
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
#include <sys/types.h>
#include <sys/stat.h>

#define PRINTF(str) printf("[%s - %d]"#str" = %s\n",__FUNCTION__,__LINE__,str);

//错误信息提示
void error_die(const char* str)
{
	const char* cText = " 错误，程序启动失败！";

	char ErrorText[255];

	strcpy_s(ErrorText, 255, str);
	strcat_s(ErrorText, 255, cText);

	perror(ErrorText);
	exit(1);
}

//404错误处理
void not_found(int nClient)
{
	char cBuff[1024];

	//状态行：版本+空格+状态码+短语+回车换行，比如："HTTP/1.0 200 OK\r\n"
	strcpy_s(cBuff, "HTTP/1.0 404 NOT FOUND\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	//Server：服务器端的软件名称+版本号
	strcpy_s(cBuff, "Server: AbcHttpd/1.1\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	strcpy_s(cBuff, "Content-type:text/html; charset=utf-8\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	strcpy_s(cBuff, "\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	//发送404网页内容
	sprintf(cBuff, "<html>							\
			<title>Not Found</title>		\
			<head>						\
				<meta charset=\"utf - 8\">		\
			</head>						 \
			<body>						 \
					<h1>404 找不到页面！</h1>		\
					<p>页面可能走丢了...</p>	   \
				<img src=\"NotFound.jpg\"/>		  \
			</body>								  \
		</html>");
	send(nClient, cBuff, strlen(cBuff), 0);
}

//405错误处理(to do)
void unimplement(int nClient)
{
	//to do
}
//网络通信初始化
int startUp(unsigned short* port) {//使用*port方便动态分配端口号

	/*流程：
	* 1、网络通信初始化（Linux系统不需要初始化，Windows需要）
	* 2、创建套接字
	* 3、设置套接字属性，端口可复用性（该步骤可略）
	* 4、绑定套接字和网络地址
	* 5、动态分配端口号（该步骤可略）
	* 6、设置监听队列
	*/

	//1.网络通信初始化
	WSAData data;//一般不需要访问，但是初始化需要，把内部初始化信息保存进去
	int nRet = WSAStartup(
		MAKEWORD(1, 1),//一个1.1的版本协议，第一个参数为版本号，第二个参数为子版本号
		&data);//nRet==1 失败;nRet==0 成功;

	if (nRet){
		error_die("WSAStartup");
	}

	//2.创建套接字
	int nServer_sock = socket(PF_INET,//表示套接字类型为网络套接字
		SOCK_STREAM,//表示创建一个流式套接字
		IPPROTO_TCP);//表示使用 TCP 协议。

	if (nServer_sock == -1){
		error_die("套接字");
	}

	//3.设置端口可复用性, 方便开发测试
	int nOpt = 1;//是否可重复使用的值，1为可重复使用
	nRet = setsockopt(nServer_sock,//选中套接字
		SOL_SOCKET, //设置套接字的属性使用
		SO_REUSEADDR, //使端口可以重复使用
		(const char*)&nOpt, sizeof(nOpt));

	if (nRet == -1)
	{
		error_die("端口可复用");
	}

	//4.绑定套接字和网络地址

	//4.1 配置服务器端的网络地址
	struct sockaddr_in server_addr;//调用并创建服务器地址结构体
	memset(&server_addr, 0, sizeof(server_addr));//清空其默认设置, 设为0
	server_addr.sin_family = PF_INET;//网络地址类型 PF_INET=>internetwork: UDP, TCP, etc.
	server_addr.sin_port = htons(*port);//类型强转: htons(u_short)：host to network short
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//IP地址 INADDR_ANY：允许任何人访问 , htonl(u_long)：host to network long

	//4.2 绑定套接字
	nRet = bind(nServer_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if (nRet < 0)
	{
		error_die("sockt bind");
	}

	//5.动态分配端口号
	if (*port == 0)
	{
		/*getsockname:返回参数sockfd指定的本地IP和端口，
		*				当套接字的地址与INADDR_ANY绑定时,除非使用connect或accept，否则函数将不返回本地IP的任何信息，
		*				但是端口号可以返回
		*/
		int nNameLen = sizeof(server_addr);
		nRet = getsockname(nServer_sock, (struct sockaddr*)&server_addr, &nNameLen);
		if (nRet < 0)
		{
			error_die("getsockname");
		}

		*port = server_addr.sin_port;//getsockname获取到的ip和地址都会存入server_addr中
	}

	//6.设置监听队列
	nRet = listen(nServer_sock, 5);//设置长度为5的监听队列,表示最多可以有5个客户端连接到服务器
	if (nRet < 0)
	{
		error_die("listen");
	}

	//7.返回套接字
	return nServer_sock;//返回套接字

}

//从客户端套接字读取一行数据
int get_line(int nSocket, char* buff, int nSize)
//从指定的客户端套接字，读取一行数据，保存到buff
//返回实际读取到的字节数
{
	char c = 0;//'\0'
	int i = 0;

	while (i < nSize - 1 && c != '\n')//缓冲区大小-1是为了留出最后一个'\0'
	{
		int n = recv(nSocket, &c, 1, 0);//从客户端套接字读取一个字节
		if (n > 0)
		{
			// \r\n  \r是回车，\n是换行
			if (c == '\r')
			{
				//通常recv()函数的最后一个参数为0,代表从缓冲区取走数据,
				//而当为MSG_PEEK时代表只是查看数据,而不取走数据
				n = recv(nSocket, &c, 1, MSG_PEEK);
				if (n > 0 && c == '\n')//正常情况下是\r\n，但不保证意外，需要判一下
				{
					recv(nSocket, &c, 1, 0);//是'\n'正常继续读
				}
				else
				{
					c = '\n';
				}
			}
			buff[i++] = c;
		}
		else
		{
			c = '\n';
		}
	}

	buff[i] = 0;//'\n'

	return i;
}
	
//发送响应包的头信息
void headers(int nClient, const char* cFileType)
{
	char cBuff[1024];

	//状态行：版本+空格+状态码+短语+回车换行，比如："HTTP/1.0 200 OK\r\n"
	strcpy_s(cBuff, "HTTP/1.0 200 OK\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	//Server：服务器端的软件名称+版本号
	strcpy_s(cBuff, "Server: AbcHttpd/1.1\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);


	//Content-type: 响应内容的类型和字符集
	if (!stricmp(cFileType, ".html"))
	{
		strcpy_s(cBuff, "Content-type:text/html; charset=utf-8\n");
	}
	else if (!stricmp(cFileType, ".jpeg") || !stricmp(cFileType, ".jpg"))
	{
		strcpy_s(cBuff, "Content-Type: image/jpeg\n");
	}
	else if (!stricmp(cFileType, ".ZIP"))
	{
		strcpy_s(cBuff, "Content-Type: application/zip\n");
	}
	else if (!stricmp(cFileType, ".pdf"))
	{
		strcpy_s(cBuff, "Content-Type: application/pdf\n");
	}
	else if (!stricmp(cFileType, ".xml"))
	{
		strcpy_s(cBuff, "Content-type: text/xml\n");
	}
	send(nClient, cBuff, strlen(cBuff), 0);

	//结束头信息
	strcpy_s(cBuff, "\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);
}

//把文件内容发送给浏览器
void cat(int nClient, FILE* resource)
{
	char cBuff[4096];
	int nCount = 0;

	while (1)
	{
		int nRet = fread(cBuff, sizeof(char), sizeof(cBuff), resource);
		if (nRet <= 0)
		{
			//发送错误: to do....
			break;
		}
		send(nClient, cBuff, nRet, 0);
		nCount += nRet;
	}
	printf("一共发送[%d]字节给浏览器！\n", nCount);
}

//把文件从服务器发出去
void server_file(int nClient, const char* fileName)
{
	int nNumberChars = 1;
	char cBuff[1024];

	//请求包的剩余数据读取完毕
	while (nNumberChars > 0 && strcmp(cBuff, "\n"))
	{
		nNumberChars = get_line(nClient, cBuff, sizeof(cBuff));
		if (strlen(cBuff) - 1 > 0)
		{
			PRINTF(cBuff);
		}
	}

	FILE* resource = NULL;

	const char* cFileType = strrchr(fileName, '.');
	if (strcmp(cFileType, ".html") == 0)
	{
		resource = fopen(fileName, "r");
	}
	else
	{
		resource = fopen(fileName, "rb");
	}
	if (resource == NULL)//如果打开失败, 发送404
	{
		not_found(nClient);
	}
	else
	{
		//发送 HTTP 响应头
		headers(nClient, cFileType);

		//发送请求的资源信息
		cat(nClient, resource);

		printf("资源发送完毕！\n");

		//输出当前时间
		SYSTEMTIME currentTime;
		GetSystemTime(&currentTime);

		printf("当前时间: %u/%u/%u %u:%u:%u:%u %d\n",
			currentTime.wYear, currentTime.wMonth, currentTime.wDay,
			currentTime.wHour, currentTime.wMinute, currentTime.wSecond,
			currentTime.wMilliseconds, currentTime.wDayOfWeek);
	}
	fclose(resource);
}

//表示一个线程函数
DWORD WINAPI accept_request(LPVOID arg) {//LPVOID == void*  arg=>套接字

	char cBuff[1024];//1K

	int nClient = (SOCKET)arg;//客户端套接字

	//读一行数据
	int nNumberChars = get_line(nClient, cBuff, sizeof(cBuff));

	//如果在服务器打开的时候关闭浏览器strlen(cBuff)==0,会报错
	if (strlen(cBuff) <= 0 || strlen(cBuff) >= 1024)
	{
		not_found(nClient);
	}
	PRINTF(cBuff);

	//解析请求方法
	char cMethod[255];
	int i = 0, j = 0;
	while (!isspace(cBuff[j]) && i < sizeof(cMethod) - 1)
	{
		cMethod[i++] = cBuff[j++];
	}
	cMethod[i] = 0;//'\n'
	PRINTF(cMethod);

	//检查提交的方法服务器是否支持
	if (_stricmp(cMethod, "GET") && _stricmp(cMethod, "POST"))//_stricmp 比较时不区分大小写
	{
		//向浏览器返回一个错误页面
		//to do 
		unimplement(nClient);
	}

	//解析请求的资源路径
	char cURL[255];//存放请求的资源的完整路径
	i = 0;
	//跳过空格
	while (isspace(cBuff[j]) && j < sizeof(cBuff))
	{
		j++;
	}

	while (!isspace(cBuff[j]) && i < sizeof(cURL) - 1 && j < sizeof(cBuff))
	{
		cURL[i++] = cBuff[j++];
	}
	cURL[i] = 0;
	PRINTF(cURL);

	char cPath[512] = "";
	sprintf(cPath, "htdocs%s", cURL);
	//如果最后一个字符为'/'，则默认访问index.tml，否则直接访问指定的资源路径
	if (cPath[strlen(cPath) - 1] == '/')
	{
		strcat(cPath, "index.html");
	}
	PRINTF(cPath);

	//判断访问的是文件还是目录
	struct stat status;
	int nRet = stat(cPath, &status);
	if (nRet == -1)
	{
		//请求包的剩余数据读取完毕
		while (nNumberChars > 0 && strcmp(cBuff, "\n"))
		{
			nNumberChars = get_line(nClient, cBuff, sizeof(cBuff));
		}
		not_found(nClient);
	}
	else
	{
		//使用位操作&，检查类型
		if ((status.st_mode & S_IFMT) == S_IFDIR)//S_IFMT:File type mask
		{
			//如果为目录，则添加html文件
			strcat(cPath, "/index.html");
		}

		server_file(nClient, cPath);//把文件发出去
	}

	closesocket(nClient);
	return 0;
} 

int main() {

	//1-网络通信初始化
	unsigned short port = 80; //端口号, 0表示自动分配
	int nServer_sock = startUp(&port); //网络通信初始化
	printf("服务器正在监听端口: %d\n", port);

	//2-循环监听客户端请求, 创建线程
	struct sockaddr_in client_addr;//客户端地址
	int nClientLen = sizeof(client_addr);
	while (1)
	{
		//阻塞式等待用户通过浏览器发起访问
		//每有一个客户端访问后，生成一个新的套接字
		int nClient_sock = accept(nServer_sock, (struct sockaddr*)&client_addr, &nClientLen);//获取客户端的信息并储存
		if (nClient_sock == -1)
		{
			error_die("accept");
		}

		//创建线程
		DWORD dwThreadID = 0;//线程ID
		//3-在新线程中处理客户端请求
		CreateThread(0, 0, accept_request,//指向线程函数的指针，即新线程要执行的函数
			(void*)nClient_sock,//传递给线程函数的参数
			0, &dwThreadID);
	}

	closesocket(nServer_sock);
	return 0;
}