#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "http.h"

//---------------------------------------------------------------------------------
// TODO:  Documentation
//
// Note that this module includes a number of helper functions to support this
// assignment.  YOU DO NOT NEED TO MODIFY ANY OF THIS CODE.  What you need to do
// is to appropriately document the socket_connect(), get_http_header_len(), and
// get_http_content_len() functions. 
//
// NOTE:  I am not looking for a line-by-line set of comments.  I am looking for 
//        a comment block at the top of each function that clearly highlights you
//        understanding about how the function works and that you researched the
//        function calls that I used.  You may (and likely should) add additional
//        comments within the function body itself highlighting key aspects of 
//        what is going on.
//
// There is also an optional extra credit activity at the end of this function. If
// you partake, you need to rewrite the body of this function with a more optimal 
// implementation. See the directions for this if you want to take on the extra
// credit. 
//--------------------------------------------------------------------------------

char *strcasestr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		c = tolower((unsigned char)c);
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == 0)
					return (NULL);
			} while ((char)tolower((unsigned char)sc) != c);
		} while (strncasecmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

char *strnstr(const char *s, const char *find, size_t slen)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != '\0') {
		len = strlen(find);
		do {
			do {
				if ((sc = *s++) == '\0' || slen-- < 1)
					return (NULL);
			} while (sc != c);
			if (len > slen)
				return (NULL);
		} while (strncmp(s, find, len) != 0);
		s--;
	}
	return ((char *)s);
}

/* socket_connect(host, port)

This function takes a host (as a set of characters), and a port number.

First, the function "translates" the host into an IP address using `gethostbyname()`
The function then checks to ensure that it was able to translate and continues
From there, it uses bcopy(), or "byte-copy". This copies the translated bytes of the ip
into the ip address field of the sockaddr struct

Next, it converts the port number from host to network byte order and fills 
the port field of the sockaddr struct.

From there, it creates a socket, and returns an error (-1) if there was an error with the socket

If we continue past this, that means we now have an active socket, and the function attempts to connect
to the host on the socket, and returns if there is an error.

Finally, if no errors were raised this function returns an active socket.

*/

int socket_connect(const char *host, uint16_t port){
    struct hostent *hp;
    struct sockaddr_in addr;
    int sock;

    if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		return -2;
	}
    
    
	bcopy(hp->h_addr_list[0], &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, 0); 
	
	if(sock == -1){
		perror("socket");
		return -1;
	}

    if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		close(sock);
        return -1;
	}

    return sock;
}


/* get_http_header_len(buff, length)

This function takes a pointer to a character buffer, as well as the number of bytes stored in the buffer as
parameters. The buffer will contain the results of a recv() from the host.

This function sets a pointer, end_ptr, to start of the buffer end sequence. To do this, it uses strnstr(), which looks for the first occurence of 
the constant HTTP_HEADER_END, and returns the beginning of the the HTTP_HEADER_END. So this means that the pointer is sitting at the end of the http header/start of the end sequence.

A check is also performed to ensure that we received the full header, and if not, -1 is returned.

From there, we perform the calculation to get the actual header length. To do this, we subtract the end pointer from the http buffer, essentially
subtracting the end of the header (without the end sequence) from the start to get the length, and also add on the length of the header_end, since the end_ptr points to the beginning of that 
sequence, not the end.

At this point we have the header length and it is returned.

*/

int get_http_header_len(char *http_buff, int http_buff_len){
    char *end_ptr;
    int header_len = 0;
    end_ptr = strnstr(http_buff,HTTP_HEADER_END,http_buff_len);

    if (end_ptr == NULL) {
        fprintf(stderr, "Could not find the end of the HTTP header\n");
        return -1;
    }

    header_len = (end_ptr - http_buff) + strlen(HTTP_HEADER_END);

    return header_len;
}

/* get_http_content_len(buff, len)

This function again takes both a pointer to the http_buffer, and the length of the header.

To do this, it goes through a loop that continues through each header line until it is at the end of the header.

It first copies the first line into header_line, by looking for the header EOL.

from there, it checks for an occurence of the CL_Header field, or the Content-Length field.
If that is found, it then looks for the delimiter, a colon (:). Now that we are at the colon,
we move the pointer forward once, meaning that we are now at the length. All we have to do now is convert
the number as a string into an actual number using atoi(). Then we can just return the length.

If the CL_header field is not found on the line, the function moves to the next line by adding the length of the current line (plus the EOL)
to the next_header_line pointer. It then repeats the whole process all over again.

If it goes through the entire header without finding the CL, it returns 0.

*/

int get_http_content_len(char *http_buff, int http_header_len){
    char header_line[MAX_HEADER_LINE];

    char *next_header_line = http_buff; //start of the header
    char *end_header_buff = http_buff + http_header_len; //end of the header

    while (next_header_line < end_header_buff){
        bzero(header_line,sizeof(header_line)); //set header_line to 0
        sscanf(next_header_line,"%[^\r\n]s", header_line); // copys the up to the EOL of the header line into header_line 

        char *isCLHeader = strcasestr(header_line,CL_HEADER); //returns pointer to first occurence of CL_Header
        if(isCLHeader != NULL){
            char *header_value_start = strchr(header_line, HTTP_HEADER_DELIM); // returns pointer to first occurence of the header delimiter (":")
            if (header_value_start != NULL){ //if found the delimeter
                char *header_value = header_value_start + 1; // go one value ahead, which will be the "Content-Length"
                int content_len = atoi(header_value); //convert the length from a string to a number
                return content_len; //return the number
            }
        }
        next_header_line += strlen(header_line) + strlen(HTTP_HEADER_EOL); //increase the current header line that was just scanf'ed and go to the next line
    }
    fprintf(stderr,"Did not find content length\n");
    return 0;
}

//This function just prints the header, it might be helpful for your debugging
//You dont need to document this or do anything with it, its self explanitory. :-)
void print_header(char *http_buff, int http_header_len){
    fprintf(stdout, "%.*s\n",http_header_len,http_buff);
}

//--------------------------------------------------------------------------------------
//EXTRA CREDIT - 10 pts - READ BELOW
//
// Implement a function that processes the header in one pass to figure out BOTH the
// header length and the content length.  I provided an implementation below just to 
// highlight what I DONT WANT, in that we are making 2 passes over the buffer to determine
// the header and content length.
//
// To get extra credit, you must process the buffer ONCE getting both the header and content
// length.  Note that you are also free to change the function signature, or use the one I have
// that is passing both of the values back via pointers.  If you change the interface dont forget
// to change the signature in the http.h header file :-).  You also need to update client-ka.c to 
// use this function to get full extra credit. 
//--------------------------------------------------------------------------------------
int process_http_header(char *http_buff, int http_buff_len, int *header_len, int *content_len){
    int h_len, c_len = 0;
    h_len = get_http_header_len(http_buff, http_buff_len);
    if (h_len < 0) {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }
    c_len = get_http_content_len(http_buff, http_buff_len);
    if (c_len < 0) {
        *header_len = 0;
        *content_len = 0;
        return -1;
    }

    *header_len = h_len;
    *content_len = c_len;
    return 0; //success
}