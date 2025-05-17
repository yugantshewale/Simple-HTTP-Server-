#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include<string>
#include<pthread.h>
#include<algorithm>
#include <errno.h>
#include <sys/stat.h>
bool startsWith(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}
char * extract_between(const char *str, const char *p1, const char *p2) {
    const char *i1 = strstr(str, p1);
    if(i1 != NULL)
    {
        const size_t pl1 = strlen(p1);
        const char *i2 = strstr(i1 + pl1, p2);
        if(i2 != NULL)
        {
     /* Found both markers, extract text. */
            const size_t mlen = i2 - (i1 + pl1);
            char *ret = (char *)malloc(mlen + 1);
            if(ret != NULL)
            {
                memcpy(ret, i1 + pl1, mlen);
                ret[mlen] = '\0';
                return ret;
            }
        }
    }
    return NULL;
}
char* reversestr(char* str) {
    int s = 0;
    int e = strlen(str) - 1;  // ignore the null character '\0'

    while (s < e) {
        // swap characters at start and end
        char temp = str[s];
        str[s] = str[e];
        str[e] = temp;
        s++;
        e--;
    }
    return str;
}
int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}
bool ends_with(const char* str, const char* suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) return false;

    return strcmp(str + str_len - suffix_len, suffix) == 0;
}
const char* get_mime_type(const char* path) {
    if (ends_with(path,".html")) return "text/html";
    if (ends_with(path,".css")) return "text/css";
    if (ends_with(path,".js")) return "application/javascript";
    if (ends_with(path,".png")) return "image/png";
    if (ends_with(path,".txt")) return "text/plain";
    return "application/octet-stream";
}
std::string direct;
void* handle_req(void * arg){
    int client = *((int *)arg);
    delete static_cast<int*>(arg);
    char buffer[4096];
    int n = read(client,buffer, sizeof buffer -1);
    buffer[n] = '\0';
    const char *pre = "GET / HTTP/1.1";
    //std::cout<<"\n----Buffer Start----\n"<<buffer<<"\n----Buffer End----\n";
    if(startsWith(buffer,pre)){
        const char* msg = "HTTP/1.1 200 OK\r\n\r\n";
        send(client,msg,strlen(msg),0);
    }else if(startsWith(buffer,"GET /echo")){
        const char* msg;
        char* text = extract_between(buffer,"GET /echo/", " HTTP/1.1");
        char* msgop;
        asprintf(&msgop,"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n%s",strlen(text),text);
        msg = msgop;
        send(client,msg,strlen(msg),0);
    }
    else if(startsWith(buffer,"GET /user-agent")){
        const char* msg;
        char* text = extract_between(buffer,"User-Agent: ", "\r\n");
        char* msgop;
        asprintf(&msgop,"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %lu\r\n\r\n%s",strlen(text),text);
        msg = msgop;
        send(client,msg,strlen(msg),0);
        close(client);
        return nullptr;
    }
    else if(startsWith(buffer,"GET /files/")){
        const char* filename = extract_between(buffer,"GET /files/"," HTTP/1.1");
        //std::cout<<"filename -> "<<filename<<std::endl;
        struct stat buff;
        std::string filepath = direct + "/" + filename;
        if(exists(filepath.c_str())){
            const char* mime = get_mime_type(filename);
            char* msgop;
            FILE *fp = fopen(filepath.c_str(), "rb");
            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            rewind(fp);
            char *file_content = (char *)malloc(size + 1);
            fread(file_content, 1, size, fp);
            fclose(fp);
            asprintf(&msgop,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %lu\r\n\r\n",mime,size);
            const char* msg = msgop;
            send(client,msg,strlen(msg),0);  
            send(client, file_content, size, 0);
            free(file_content);
            
            return nullptr; 
        }
        else{
            const char* msg = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client,msg,strlen(msg),0);  
            return nullptr;  
        }
    }
     else if(startsWith(buffer,"POST /files/")){
        std::string ret="";
        for(int i = strlen(buffer)-1; i >= 0; i--){
        if(buffer[i] == '\n')
            break;
        ret += buffer[i];
        
        }
        std::reverse(ret.begin(),ret.end());
        const char* input = ret.c_str();
        const char* filename = extract_between(buffer,"POST /files/"," HTTP/1.1");
        std::string filepath = direct + "/" + filename;
        FILE *fptr = fopen(filepath.c_str(),"w");
        fprintf(fptr, input);
        fclose(fptr);
        //std::cout<<"Input is --> "<<input<<"\n";
        const char* msg = "HTTP/1.1 201 Created\r\n\r\n";
        send(client,msg,strlen(msg),0);
    }
    else{
        const char* msg = "HTTP/1.1 404 Not Found\r\n\r\n";
        send(client,msg,strlen(msg),0);
    }
    return nullptr;
}
int main(int argc, char **argv) {
     direct = ".";
    for (int i = 1; i < argc - 1; ++i) {
        if (std::string(argv[i]) == "--directory") {
            direct = argv[i + 1];
        }
    }
    // std::cout<<"Arguments are \n";
    // for (int i = 0; i < argc; ++i) {
    //     std::cout<<"arg 1 : "<<argv[i]<<"\n";
    // }
    // Flush after every std::cout / std::cerrdf
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    
    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cout << "Logs from your program will appear here!\n";
    //A socket is an endpoint of communication to which a name can be bound.
    //A socket is a combination of IP address and port number. This is required for two programs to find each other and communicate over a network
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    //The socket() call creates a socket in the specified domain and of the specified type.
    //s = socket(domain, type, protocol);
    //Socket types are defined in sys/socket.h. These types--SOCK_STREAM, SOCK_DGRAM, or SOCK_RAW--are supported by AF_INET and AF_UNIX.
    //AF_INET = Internet domain. send - send a message on a socket
    if (server_fd < 0) {
        std::cerr << "Failed to create server socket\n";
        return 1;
    }
    
    // Since the tester restarts your program quite often, setting SO_REUSEADDR
    // ensures that we don't run into 'Address already in use' errors
    int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << "setsockopt failed\n";
        return 1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(4221);
    
    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        std::cerr << "Failed to bind to port 4221\n";
        return 1;
    }
    
    int connection_backlog = 5;
    if (listen(server_fd, connection_backlog) != 0) {
        std::cerr << "listen failed\n";
        return 1;
    }
    
    struct sockaddr_in client_addr;
    while(1){
        int client_addr_len = sizeof(client_addr);
        std::cout << "Waiting for a client to connect...\n";
        int* client = new int;
        *client =accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t *) &client_addr_len);
        if (*client < 0) {
            perror("accept");
            delete client;              
            continue;
        }
        std::cout << "Client connected\n";
        pthread_t thread_id ;
        handle_req(client);
        pthread_create(&thread_id,NULL,handle_req, client);
        pthread_detach(thread_id);
    }
    close(server_fd);
    return 0;
}
