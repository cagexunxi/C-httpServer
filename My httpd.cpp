/*���ڣ�2024-7-10
* ����˼·:
* 1- ����ͨ�ų�ʼ��: startUp()  //����һ���׽���
* 2- ѭ�������ͻ�������, �����߳�: accept() , CreateThread()
* 3- �����߳��д���ͻ�������: accept_request()
* 

*ps:
* 1-�׽���: �������������������ڵ�֮�佨����˫��ͨ����·
* 1.1-�׽��ֵ����: IP��ַ���˿ں�(port)��Э������
* 
* 2-
* 
*/

//������ݰ��ǿͻ��˷��͵�GET����
/*���ݰ���ʽ��
	*	GET / HTTP/1.1\n  // HTTP���󷽷��Ͱ汾
	*	Host: 127.0.0.1:8000\n  // �����������ַ�Ͷ˿�
	*	Connection: keep-alive\n  // ��������
	*	Cache-Control: max-age=0\n  // ������ƣ���ʹ�û���
	*	Upgrade-Insecure-Requests: 1\n  // ����������HTTPS
	*	User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36\n  // �û�������Ϣ
	*	Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,* \
	*	/*; q = 0.8, application / signed - exchange; v = b3; q = 0.9\n  // �ɽ��ܵ���Ӧ��������
	*	Sec - Fetch - Site: none\n  // �������Դվ��
	*	Sec - Fetch - Mode: navigate\n  // �����ģʽ
	*	Sec - Fetch - User: ? 1\n  // �û���������
	*	Sec - Fetch - Dest: document\n  // �����Ŀ��
	*	Accept - Encoding: gzip, deflate, br\n  // �ɽ��ܵı��뷽ʽ
	*	Accept - Language: zh - CN, zh; q = 0.9\n  // �ɽ��ܵ�����
	*	\n
	*/



#include<stdio.h>
#include <WinSock2.h>
#pragma comment(lib,"WS2_32.lib")
#include <sys/types.h>
#include <sys/stat.h>

#define PRINTF(str) printf("[%s - %d]"#str" = %s\n",__FUNCTION__,__LINE__,str);

//������Ϣ��ʾ
void error_die(const char* str)
{
	const char* cText = " ���󣬳�������ʧ�ܣ�";

	char ErrorText[255];

	strcpy_s(ErrorText, 255, str);
	strcat_s(ErrorText, 255, cText);

	perror(ErrorText);
	exit(1);
}

//404������
void not_found(int nClient)
{
	char cBuff[1024];

	//״̬�У��汾+�ո�+״̬��+����+�س����У����磺"HTTP/1.0 200 OK\r\n"
	strcpy_s(cBuff, "HTTP/1.0 404 NOT FOUND\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	//Server���������˵��������+�汾��
	strcpy_s(cBuff, "Server: AbcHttpd/1.1\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	strcpy_s(cBuff, "Content-type:text/html; charset=utf-8\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	strcpy_s(cBuff, "\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	//����404��ҳ����
	sprintf(cBuff, "<html>							\
			<title>Not Found</title>		\
			<head>						\
				<meta charset=\"utf - 8\">		\
			</head>						 \
			<body>						 \
					<h1>404 �Ҳ���ҳ�棡</h1>		\
					<p>ҳ������߶���...</p>	   \
				<img src=\"NotFound.jpg\"/>		  \
			</body>								  \
		</html>");
	send(nClient, cBuff, strlen(cBuff), 0);
}

//405������(to do)
void unimplement(int nClient)
{
	//to do
}
//����ͨ�ų�ʼ��
int startUp(unsigned short* port) {//ʹ��*port���㶯̬����˿ں�

	/*���̣�
	* 1������ͨ�ų�ʼ����Linuxϵͳ����Ҫ��ʼ����Windows��Ҫ��
	* 2�������׽���
	* 3�������׽������ԣ��˿ڿɸ����ԣ��ò�����ԣ�
	* 4�����׽��ֺ������ַ
	* 5����̬����˿ںţ��ò�����ԣ�
	* 6�����ü�������
	*/

	//1.����ͨ�ų�ʼ��
	WSAData data;//һ�㲻��Ҫ���ʣ����ǳ�ʼ����Ҫ�����ڲ���ʼ����Ϣ�����ȥ
	int nRet = WSAStartup(
		MAKEWORD(1, 1),//һ��1.1�İ汾Э�飬��һ������Ϊ�汾�ţ��ڶ�������Ϊ�Ӱ汾��
		&data);//nRet==1 ʧ��;nRet==0 �ɹ�;

	if (nRet){
		error_die("WSAStartup");
	}

	//2.�����׽���
	int nServer_sock = socket(PF_INET,//��ʾ�׽�������Ϊ�����׽���
		SOCK_STREAM,//��ʾ����һ����ʽ�׽���
		IPPROTO_TCP);//��ʾʹ�� TCP Э�顣

	if (nServer_sock == -1){
		error_die("�׽���");
	}

	//3.���ö˿ڿɸ�����, ���㿪������
	int nOpt = 1;//�Ƿ���ظ�ʹ�õ�ֵ��1Ϊ���ظ�ʹ��
	nRet = setsockopt(nServer_sock,//ѡ���׽���
		SOL_SOCKET, //�����׽��ֵ�����ʹ��
		SO_REUSEADDR, //ʹ�˿ڿ����ظ�ʹ��
		(const char*)&nOpt, sizeof(nOpt));

	if (nRet == -1)
	{
		error_die("�˿ڿɸ���");
	}

	//4.���׽��ֺ������ַ

	//4.1 ���÷������˵������ַ
	struct sockaddr_in server_addr;//���ò�������������ַ�ṹ��
	memset(&server_addr, 0, sizeof(server_addr));//�����Ĭ������, ��Ϊ0
	server_addr.sin_family = PF_INET;//�����ַ���� PF_INET=>internetwork: UDP, TCP, etc.
	server_addr.sin_port = htons(*port);//����ǿת: htons(u_short)��host to network short
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//IP��ַ INADDR_ANY�������κ��˷��� , htonl(u_long)��host to network long

	//4.2 ���׽���
	nRet = bind(nServer_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if (nRet < 0)
	{
		error_die("sockt bind");
	}

	//5.��̬����˿ں�
	if (*port == 0)
	{
		/*getsockname:���ز���sockfdָ���ı���IP�Ͷ˿ڣ�
		*				���׽��ֵĵ�ַ��INADDR_ANY��ʱ,����ʹ��connect��accept���������������ر���IP���κ���Ϣ��
		*				���Ƕ˿ںſ��Է���
		*/
		int nNameLen = sizeof(server_addr);
		nRet = getsockname(nServer_sock, (struct sockaddr*)&server_addr, &nNameLen);
		if (nRet < 0)
		{
			error_die("getsockname");
		}

		*port = server_addr.sin_port;//getsockname��ȡ����ip�͵�ַ�������server_addr��
	}

	//6.���ü�������
	nRet = listen(nServer_sock, 5);//���ó���Ϊ5�ļ�������,��ʾ��������5���ͻ������ӵ�������
	if (nRet < 0)
	{
		error_die("listen");
	}

	//7.�����׽���
	return nServer_sock;//�����׽���

}

//�ӿͻ����׽��ֶ�ȡһ������
int get_line(int nSocket, char* buff, int nSize)
//��ָ���Ŀͻ����׽��֣���ȡһ�����ݣ����浽buff
//����ʵ�ʶ�ȡ�����ֽ���
{
	char c = 0;//'\0'
	int i = 0;

	while (i < nSize - 1 && c != '\n')//��������С-1��Ϊ���������һ��'\0'
	{
		int n = recv(nSocket, &c, 1, 0);//�ӿͻ����׽��ֶ�ȡһ���ֽ�
		if (n > 0)
		{
			// \r\n  \r�ǻس���\n�ǻ���
			if (c == '\r')
			{
				//ͨ��recv()���������һ������Ϊ0,����ӻ�����ȡ������,
				//����ΪMSG_PEEKʱ����ֻ�ǲ鿴����,����ȡ������
				n = recv(nSocket, &c, 1, MSG_PEEK);
				if (n > 0 && c == '\n')//�����������\r\n��������֤���⣬��Ҫ��һ��
				{
					recv(nSocket, &c, 1, 0);//��'\n'����������
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
	
//������Ӧ����ͷ��Ϣ
void headers(int nClient, const char* cFileType)
{
	char cBuff[1024];

	//״̬�У��汾+�ո�+״̬��+����+�س����У����磺"HTTP/1.0 200 OK\r\n"
	strcpy_s(cBuff, "HTTP/1.0 200 OK\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);

	//Server���������˵��������+�汾��
	strcpy_s(cBuff, "Server: AbcHttpd/1.1\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);


	//Content-type: ��Ӧ���ݵ����ͺ��ַ���
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

	//����ͷ��Ϣ
	strcpy_s(cBuff, "\r\n");
	send(nClient, cBuff, strlen(cBuff), 0);
}

//���ļ����ݷ��͸������
void cat(int nClient, FILE* resource)
{
	char cBuff[4096];
	int nCount = 0;

	while (1)
	{
		int nRet = fread(cBuff, sizeof(char), sizeof(cBuff), resource);
		if (nRet <= 0)
		{
			//���ʹ���: to do....
			break;
		}
		send(nClient, cBuff, nRet, 0);
		nCount += nRet;
	}
	printf("һ������[%d]�ֽڸ��������\n", nCount);
}

//���ļ��ӷ���������ȥ
void server_file(int nClient, const char* fileName)
{
	int nNumberChars = 1;
	char cBuff[1024];

	//�������ʣ�����ݶ�ȡ���
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
	if (resource == NULL)//�����ʧ��, ����404
	{
		not_found(nClient);
	}
	else
	{
		//���� HTTP ��Ӧͷ
		headers(nClient, cFileType);

		//�����������Դ��Ϣ
		cat(nClient, resource);

		printf("��Դ������ϣ�\n");

		//�����ǰʱ��
		SYSTEMTIME currentTime;
		GetSystemTime(&currentTime);

		printf("��ǰʱ��: %u/%u/%u %u:%u:%u:%u %d\n",
			currentTime.wYear, currentTime.wMonth, currentTime.wDay,
			currentTime.wHour, currentTime.wMinute, currentTime.wSecond,
			currentTime.wMilliseconds, currentTime.wDayOfWeek);
	}
	fclose(resource);
}

//��ʾһ���̺߳���
DWORD WINAPI accept_request(LPVOID arg) {//LPVOID == void*  arg=>�׽���

	char cBuff[1024];//1K

	int nClient = (SOCKET)arg;//�ͻ����׽���

	//��һ������
	int nNumberChars = get_line(nClient, cBuff, sizeof(cBuff));

	//����ڷ������򿪵�ʱ��ر������strlen(cBuff)==0,�ᱨ��
	if (strlen(cBuff) <= 0 || strlen(cBuff) >= 1024)
	{
		not_found(nClient);
	}
	PRINTF(cBuff);

	//�������󷽷�
	char cMethod[255];
	int i = 0, j = 0;
	while (!isspace(cBuff[j]) && i < sizeof(cMethod) - 1)
	{
		cMethod[i++] = cBuff[j++];
	}
	cMethod[i] = 0;//'\n'
	PRINTF(cMethod);

	//����ύ�ķ����������Ƿ�֧��
	if (_stricmp(cMethod, "GET") && _stricmp(cMethod, "POST"))//_stricmp �Ƚ�ʱ�����ִ�Сд
	{
		//�����������һ������ҳ��
		//to do 
		unimplement(nClient);
	}

	//�����������Դ·��
	char cURL[255];//����������Դ������·��
	i = 0;
	//�����ո�
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
	//������һ���ַ�Ϊ'/'����Ĭ�Ϸ���index.tml������ֱ�ӷ���ָ������Դ·��
	if (cPath[strlen(cPath) - 1] == '/')
	{
		strcat(cPath, "index.html");
	}
	PRINTF(cPath);

	//�жϷ��ʵ����ļ�����Ŀ¼
	struct stat status;
	int nRet = stat(cPath, &status);
	if (nRet == -1)
	{
		//�������ʣ�����ݶ�ȡ���
		while (nNumberChars > 0 && strcmp(cBuff, "\n"))
		{
			nNumberChars = get_line(nClient, cBuff, sizeof(cBuff));
		}
		not_found(nClient);
	}
	else
	{
		//ʹ��λ����&���������
		if ((status.st_mode & S_IFMT) == S_IFDIR)//S_IFMT:File type mask
		{
			//���ΪĿ¼�������html�ļ�
			strcat(cPath, "/index.html");
		}

		server_file(nClient, cPath);//���ļ�����ȥ
	}

	closesocket(nClient);
	return 0;
} 

int main() {

	//1-����ͨ�ų�ʼ��
	unsigned short port = 80; //�˿ں�, 0��ʾ�Զ�����
	int nServer_sock = startUp(&port); //����ͨ�ų�ʼ��
	printf("���������ڼ����˿�: %d\n", port);

	//2-ѭ�������ͻ�������, �����߳�
	struct sockaddr_in client_addr;//�ͻ��˵�ַ
	int nClientLen = sizeof(client_addr);
	while (1)
	{
		//����ʽ�ȴ��û�ͨ��������������
		//ÿ��һ���ͻ��˷��ʺ�����һ���µ��׽���
		int nClient_sock = accept(nServer_sock, (struct sockaddr*)&client_addr, &nClientLen);//��ȡ�ͻ��˵���Ϣ������
		if (nClient_sock == -1)
		{
			error_die("accept");
		}

		//�����߳�
		DWORD dwThreadID = 0;//�߳�ID
		//3-�����߳��д���ͻ�������
		CreateThread(0, 0, accept_request,//ָ���̺߳�����ָ�룬�����߳�Ҫִ�еĺ���
			(void*)nClient_sock,//���ݸ��̺߳����Ĳ���
			0, &dwThreadID);
	}

	closesocket(nServer_sock);
	return 0;
}