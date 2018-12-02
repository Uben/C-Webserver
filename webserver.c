#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

// #define PORT 9000
#define MAX_OUTPUT 4096

struct {
	char *file_ext;
	char *cont_type;
} extenTypes [] = {
	{"htm",  "text/html"},
	{"html", "text/html"},
	{"css",  "text/css"},
	{"js",   "text/javascript"},
	{"jpg",  "image/jpeg"},
	{"png",  "image/png"},
	{"gif",  "image/gif"},
	{0, 0}
};

void empty_buffer(char* buff) {
	for (int x = 0; x < strlen(buff); x++)
		buff[x] = '\0';
}

void webfiles(int fd) {
	char *basepath, *filepath, *file_content, *file_type;
	char resBuffer[2049], buffer[2049]; /* 2048 + 1 for the end of file char */
	int x, file_fd, track, file_length, buflen, len;
	
	const char *root_folder = "web";

	const char *HTTP_404_CONTENT = "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1>The requested resource could not be found but may be available again in the future.<div style=\"color: #eeeeee; font-size: 8pt;\">Actually, it probably won't ever be available unless this is showing up because of a bug in your program. :(</div></html>";
	const char *HTTP_501_CONTENT = "<html><head><title>501 Not Implemented</title></head><body><h1>501 Not Implemented</h1>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</body></html>";

	const char *HTTP_200_STRING = "OK";
	const char *HTTP_404_STRING = "Not Found";
	const char *HTTP_501_STRING = "Not Implemented";

	// check if we successfully read the request
	if ((track = read(fd, buffer, 2048)) < 1)
		printf("Failed to read the request.\n");

	// check for the first 3 char
	if (strncmp(buffer, "GET", 3) || strncmp(buffer, "get", 3)) {
		// First: Get the url from get request 
		// get the first line
		basepath = strtok(buffer, "\n");
		
		// remove the "get " text
		size_t len = strlen(basepath);
		memmove(basepath, basepath+4, len-4+1);

		// remove the "HTTP/1.1" text
		basepath = strtok(basepath, " ");

		filepath = (char *) malloc(sizeof(basepath)+24); /* 3 char = 24 bits of memory */

		snprintf(filepath, strlen(basepath)+24, "web%s", basepath);

		// check if valid url present, else it will have HTTP/(1.0/1.1) as the value of filepath
		if ((strcmp(filepath,"HTTP/1.0") == 0) || (strcmp(filepath,"HTTP/1.1") == 0)) {
			// ERROR: Send back 501 wrong request type
			sprintf(resBuffer, "HTTP/1.1 501 %s\r\n\r\n%s", HTTP_501_STRING, HTTP_501_CONTENT);
			write(fd, resBuffer, strlen(resBuffer));

			exit(1);
			return;
		}


		// Second: Get the file extension from get request
		buflen = strlen(buffer);
		file_type = (char *)0;
		
		for (x = 0; extenTypes[x].file_ext != 0; x++) {
			len = strlen(extenTypes[x].file_ext);
			
			if (!strncmp(&buffer[buflen - len], extenTypes[x].file_ext, len)) {
				file_type = extenTypes[x].cont_type;
				break;
			}
		}
		
		if (extenTypes[x].file_ext == 0) {
			file_type = "text/plain";
		}


		// Third: Get file and send to client
		file_fd = open(filepath, O_RDONLY);

		// if file doesnt open send 404
		if (file_fd == -1) {
			sprintf(resBuffer, "HTTP/1.1 404 %s\r\n\r\n%s", HTTP_404_STRING, HTTP_404_CONTENT);
			write(fd, resBuffer, strlen(resBuffer));
		
		// file opened, send response
		} else {
			// find size of file
			int file_size = lseek(file_fd, 0, SEEK_END);
			lseek(file_fd, 0, SEEK_SET);

			// write header to resBuffer
			sprintf(resBuffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", file_type, file_size);
			write(fd, resBuffer, strlen(resBuffer));

			/* send file in max 4KB increments */
			while ((track = read(file_fd, resBuffer, MAX_OUTPUT)) > 0)
				(void)write(fd, resBuffer, track);
		}


	} else {
		// ERROR: Send back 501 wrong request type
		sprintf(resBuffer, "HTTP/1.1 501 %s\r\n\r\n%s", HTTP_501_STRING, HTTP_501_CONTENT);
		write(fd, resBuffer, strlen(resBuffer));
	}

	exit(1);
}
		



int main(int argc, char *argv[]) {
	if (argc < 2) {
		exit(1);
	}

	const int PORT = atoi(argv[1]);

	int sockfd, ret, soc_conn, addr_len;
	struct sockaddr_in serverAddr, newAddr;
	char buffer[1024];
	pid_t childpid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("Socket Connection Error.\n");
		return 0;
	}

	printf("Server Socket Connection Created.\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if (ret < 0) {
		printf("Binding Error.\n");
		return 0;
	}

	printf("Bound to port: %d.\n", PORT);

	if (listen(sockfd, 10) == -1) {
		printf("Binding Error.\n");
		return 0;
	}

	while(1) {
		addr_len = sizeof(newAddr);
		soc_conn = accept(sockfd, (struct sockaddr*)&newAddr, &addr_len);
		if (soc_conn < 0){
			printf("connection error with soc_conn\n");
			exit(1);
		}

		if ((childpid = fork()) == 0) { /* child */
			close(sockfd);
			webfiles(soc_conn);

		} else { /* parent */
			close(soc_conn);
		}
	}

	return 0;
}