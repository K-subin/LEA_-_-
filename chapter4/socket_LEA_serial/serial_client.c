#include <lea.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <termios.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUF_SIZE	1024
#define DEVPORT	"/dev/ttyAMA0"

int set_uart(char* device_name, int baudrate);
void dust_sensor(unsigned char *command_buf, int serial_fd);
int data_read(int serial_fd, unsigned char* sd_buf);
int set_socket();

int main()
{
	struct sockaddr_in server_addr;
	int serial_fd = 0;
	int i = 0, len = 0;
	int comm_sock = 0;
	int server_addr_len = 0;
	BYTE buf[128] = { 0x00, };
	BYTE recvBuf[MAX_BUF_SIZE] = { 0, };
	BYTE sendBuf[MAX_BUF_SIZE] = { 0, };
	BYTE K[16] = 
	{ 0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78, 0x87, 0x96, 0xa5, 0xb4, 0xc3, 0xd2, 0xe1, 0xf0 };
	WORD RoundKey[144] = { 0 };

	KeySchedule_128(K, RoundKey);
	
	//1. uart setting function
	serial_fd = set_uart(DEVPORT, B9600); 

	//2. socket setting function
	comm_sock = set_socket();

	//3. dust sensor setting
	dust_sensor(buf, serial_fd);

	while (1)
	{
		//4. serial data (dust data) read funciton
		len = data_read(serial_fd, buf);
		printf("read len : %d\n", len); //읽은 바이트 수 출력

		/*먼지데이터 출력*/
		printf("DUST DATA : ");
		for (i = 0; i < 16; i++)
		{
			printf("%02x ", buf[i]); //버퍼값 출력
		}
		printf("\n");
		
		//4.5 encryption
		memset(sendBuf, 0X00, 16);
		Encrypt(24, RoundKey, buf, sendBuf);
		printf("Encrypt data : ");
		for (i = 0; i < 16; i++)
		{
			printf("%02x ", sendBuf[i]);
		}
		printf("\n\n");
		

		//5. send data (server)
		if (write(comm_sock, sendBuf, 16) <= 0)
		{
			printf("write error\n");
			return 1;
		}

	}
	close(comm_sock);

	return 0;
}

int set_uart(char* device_name, int baudrate)
{
	struct termios newtio;
	int serial_fd;

	memset(&newtio, 0, sizeof(newtio));
	serial_fd = open((char *)device_name, O_RDWR | O_NOCTTY);

	printf("serial_fd : %d\n", serial_fd);

	if (serial_fd < 0)
	{
		printf("serial fd open fail !!!\n");
		return -1;
	}

	newtio.c_cflag = baudrate;
	newtio.c_cflag |= CS8;
	newtio.c_cflag |= CLOCAL;
	newtio.c_cflag |= CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 3;
	newtio.c_cc[VMIN] = 1;

	tcflush(serial_fd, TCIFLUSH);
	tcsetattr(serial_fd, TCSANOW, &newtio);

	return serial_fd;
}

void dust_sensor(unsigned char *command_buf, int serial_fd)
{
	command_buf[0] = 0x4E;
	command_buf[1] = 0x49;
	command_buf[2] = 0x55;
	command_buf[3] = 0x52;
	command_buf[4] = (command_buf[0] + command_buf[1] + command_buf[2] + command_buf[3]) & 0xff;
	command_buf[5] = 0x44;
	command_buf[6] = 0x53;

	write(serial_fd, command_buf, 7);
	
	sleep(1);
	command_buf[0] = 0x4E;
	command_buf[1] = 0x49;
	command_buf[2] = 0x55;
	command_buf[3] = 0x43;
	command_buf[4] = (command_buf[0] + command_buf[1] + command_buf[2] + command_buf[3]) & 0xff;
	command_buf[5] = 0x44;
	command_buf[6] = 0x53;

	write(serial_fd, command_buf, 7);

	printf("dust sensor setting done.\n\n");
}

int data_read(int serial_fd, unsigned char* sd_buf)
{
	unsigned char buf[128] = { 0x00, };
	int i = 0, len = 0;

	while (1)
	{
		len = read(serial_fd, &buf[i], 1);
		
		if (buf[0] == 0x4E && buf[1] == 0x49)
		{
			if (buf[9] == 0x44 && buf[10] == 0x53)
			{
				memcpy(sd_buf, buf, 11);
				return i+1;
			}
		}
		i++;
	}
	return 0;
}

int set_socket()
{
	struct sockaddr_in server_addr;
	int comm_sock = 0;
	int server_addr_len = 0;

	comm_sock = socket(PF_INET, SOCK_STREAM, 0);
	if (comm_sock == -1)
	{
		printf("error :\n");
		return 1;
	}

	memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.10");
	server_addr.sin_port = htons(9000);
	server_addr_len = sizeof(server_addr);

	if (connect(comm_sock, (struct sockaddr *)&server_addr, server_addr_len) == -1)
	{
		printf("connect error :\n");
		return 1;
	}

	return comm_sock;
}
