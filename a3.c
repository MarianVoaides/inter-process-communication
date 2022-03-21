#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>

void write_to_pipe(char resp[], int size, int resp_pipe)
{
	write(resp_pipe, &size, 1);
	write(resp_pipe, resp, size);
}

int main()
{

	int request_pipe, response_pipe;
	int part_Fd;
	char *part_mem;
	if (mkfifo("RESP_PIPE_95678", 0644))
	{
		printf("ERROR\ncannot create the response time | cannot open the request pipe");
		return -1;
	}

	request_pipe = open("REQ_PIPE_95678", O_RDONLY);
	if (request_pipe == -1)
	{
		printf("ERROR\ncannot create the response time | cannot open the request pipe");
		return -2;
	}

	response_pipe = open("RESP_PIPE_95678", O_WRONLY);
	if (response_pipe == -1)
	{
		printf("ERROR\ncannot create the response time | cannot open the request pipe");
		return -3;
	}

	write_to_pipe("CONNECT", strlen("CONNECT"), response_pipe);

	char comanda[50];
	int size_comanda = 0;
	unsigned int ping_number = 95678, part_size;
	while (1)
	{
		read(request_pipe, &size_comanda, 1);
		read(request_pipe, comanda, size_comanda);
		comanda[size_comanda] = '\0';

		if (strncmp("PING", comanda, 4) == 0)
		{
			write_to_pipe("PING", strlen("PING"), response_pipe);
			write_to_pipe("PONG", strlen("PONG"), response_pipe);
			write(response_pipe, &ping_number, 4);
			continue;
		}

		if (strncmp("CREATE_SHM", comanda, 10) == 0)
		{
			read(request_pipe, &part_size, 4);
			part_Fd = shm_open("/K4Edj1r", O_RDWR | O_CREAT, 0644);
			if (part_Fd < 0)
			{
				write_to_pipe("CREATE_SHM", strlen("CREATE_SHM"), response_pipe);
				write_to_pipe("ERROR", strlen("ERROR"), response_pipe);
				continue;
			}
			ftruncate(part_Fd, part_size);
			part_mem = (char *)mmap(0, part_size, PROT_READ | PROT_WRITE, MAP_SHARED, part_Fd, 0);
			if (part_mem == (void *)-1)
			{
				write_to_pipe("CREATE_SHM", strlen("CREATE_SHM"), response_pipe);
				write_to_pipe("ERROR", strlen("ERROR"), response_pipe);
				continue;
			}
			write_to_pipe("CREATE_SHM", strlen("CREATE_SHM"), response_pipe);
			write_to_pipe("SUCCESS", strlen("SUCCESS"), response_pipe);
			continue;
		}
		if (strncmp("WRITE_TO_SHM", comanda, 13) == 0)
		{
			int offset, value;
			read(request_pipe, &offset, 4);
			read(request_pipe, &value, 4);
			if (offset > 1945815)
			{
				write_to_pipe("WRITE_TO_SHM", strlen("WRITE_TO_SHM"), response_pipe);
				write_to_pipe("ERROR", strlen("ERROR"), response_pipe);
			}
			memcpy(part_mem + offset, &value, 4);
			write_to_pipe("WRITE_TO_SHM", strlen("WRITE_TO_SHM"), response_pipe);
			write_to_pipe("SUCCESS", strlen("SUCCESS"), response_pipe);
			continue;
		}
		if (strncmp("MAP_FILE", comanda, 8) == 0)
		{
			char file_name[100];
			int fn_size = 0;
			read(request_pipe, &fn_size, 1);
			read(request_pipe, file_name, fn_size);
			file_name[fn_size] = '\0';
			int fd = open(file_name, O_RDONLY);
			if (fd == -1)
			{
				write_to_pipe("MAP_FILE", strlen("MAP_FILE"), response_pipe);
				write_to_pipe("ERROR", strlen("ERROR"), response_pipe);
			}
			int fsize = lseek(fd, 0, SEEK_END);
			mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
			write_to_pipe("MAP_FILE", strlen("MAP_FILE"), response_pipe);
			write_to_pipe("SUCCESS", strlen("SUCCESS"), response_pipe);
			continue;
		}
		if (strncmp("EXIT", comanda, 4) == 0)
		{
			break;
		}

		break;
	}

	return 0;
}
