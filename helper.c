#define _GNU_SOURCE 
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>

#include "helper.h"

char* my_connect(char url[],char dir[]){
    int  client,length,received ; 
    char response[100000];   
    struct sockaddr_in serv_addr;
    struct hostent * server;
    
    server = gethostbyname(url);
    if (server == NULL){
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    client= socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0){
        fprintf(stderr,"ERROR opening socket");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    bcopy(server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
    int connection_status = connect(client, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
    if (connection_status==-1){
        fprintf(stderr,"ERROR connecting to host");
        exit(0);
    }
    char *request = create_get(url,dir);
    strcat(request,"\r\n");
    send(client, request,strlen(request),0);
    free(request);
    recv(client,&response,100000,0);
    length =find_length(response);
    char *copy=malloc(sizeof(char)*(100000));
    strcpy(copy,response);
    char *data = strstr( response, "\r\n\r\n" );
    data+=4;
    char *html= malloc(sizeof(char)*100000);
    strcpy(html,data);
    received=strlen(data);
    memset(response,0,100000);
    if (length!=0){
        while (received!=length){
            if(recv(client,&response,100000,0)<=0){
                break;
            }
            strcat(html,response);
            strcat(copy,response);    
            received=strlen(html);
            memset(response,0,100000);
        } 
    }else{
        while(1){
            if(recv(client,&response,100000,0)<=0){
                break;
            }else{
                strcat(copy,response);
                strcat(html,response);    
                received=strlen(html);
                memset(response,0,100000);
            }
        }
    }
    close(client); 
    free(html);
    return copy;
}

URL_INFO* parse_url(URL_INFO* info, const char* url){
    char *site, *path;

    if (!info || !url){
        return NULL;
    }
    info->protocol = "http";
    info->port = "80";

    site= malloc(sizeof(char)*(strlen(url)+1));
    path= malloc(sizeof(char)*(strlen(url)+1));
    strcpy(site,url);
    strcpy(path,url);

    char *site1=strstr(site,"://");
    site1+=3;
    site1=strtok(site1,"/");
    info->site= malloc(sizeof(char)*(strlen(site1)+1));
    strcpy(info->site,site1);

    char *path1=strstr(path,"://");
    path1+=3;

    if(strchr(path1,'/')){
        path1=strstr(path1,"/");
        info->path= malloc(sizeof(char)*(strlen(path1)+1));
        strcpy(info->path,path1);
    }else{
        info->path= malloc(sizeof(char)*(strlen(NP)+1));
        strcpy(info->path,NP);
    }
    free(site);
    free(path);

    return info;
}

 char* create_get(char url[],char dir[]){
    char request[1000] ="GET";
    if (strcmp(dir,NP)==0){
        strcat(request, " / HTTP/1.1\r\n");
        strcat(request,"Host:");
        strcat(request,url);
        strcat(request,"\r\nUser-Agent:ncheong1\r\nConnection:close\r\nAccept: html/text\r\n");
    }else{
        strcat(request," ");
        strcat(request,dir);
        strcat(request, " HTTP/1.1\r\n");
        strcat(request,"Host: ");
        strcat(request,url);
        strcat(request,"\r\n");
        strcat(request,"User-Agent: ncheong1\r\nConnection:close\r\nAccept: html/text\r\n");
    }
    char *my_get= malloc(sizeof(char)*(strlen(request)+50));
    strcpy(my_get,request);
    return my_get;
}

char *strlwr(char *str){
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

int find_length(char response[]){
    char length[1000];
    
    if(strcasestr(response,"Content-Length: ")){
        char *temp =strcasestr(response,"Content-Length: ");
    
        sscanf(temp,"%*[^:]:%[^\r\n]",length);
        int answer=atoi(length);
        return answer;
    }else{
        return 0;
    }
}

char *find_location(char response[]){
    char moved[1000];
    
    if(strcasestr(response,"Location: ")){
        char *temp =strcasestr(response,"Location: ");
    
        sscanf(temp,"%*[^:]:%[^\r\n]",moved);
        char *url =malloc(sizeof(char)*(strlen(moved)+1));
        strcpy(url,moved);
        return url;
    }else{
        char *url =malloc(sizeof(char)*(strlen(NP)+1));
        strcpy(url,NP);
        return url;
    }
}

char *find_content(char response[]){
    char content[1000];
    
    if(strcasestr(response,"Content-Type: ")){
        char *temp =strcasestr(response,"Content-Type: ");
    
        sscanf(temp,"%*[^:]:%[^\r\n]",content);
        char *output =malloc(sizeof(char)*(strlen(content)+1));
        strcpy(output,content);
        return output;
    }else{
        char *url =malloc(sizeof(char)*(strlen(NP)+1));
        strcpy(url,NP);
        return url;
    }
}

int find_response(char response[]){
    int code;

    sscanf(response,"%*[^ ] %d%*[^\r\n]",&code);
    
    return code;
}

int parse_html(char *html, char *web[]){
    char *lower, *temp, *comp,website[1000];
    int i=0;

    lower=html;
    while((lower=strcasestr(lower,"<a"))){
       if ((comp=strcasestr(lower,"href")) && (temp=strcasestr(comp,"="))){
            sscanf(temp,"%*[^\"]\"%[^\"]",website);
            web[i]=malloc(sizeof(char)*(strlen(website)+1));
            strcpy(web[i],website);
            i++;
            memset(website,0,1000);
        }
        lower=strcasestr(lower,"</a>");
        
    }
    return i;
}

char *complete_url(char *url, URL_INFO *previous){
    char key[3],key1[5],key2[4],*keylwr,add='/', *slash,*slash1;
    char temp[1000]="http:",temp1[1000]="http";

    strncpy(key1,url,4);
    strncpy(key2,url,3);
    strncpy(key,url,2);
    key[2]='\0';
    key1[4]='\0';
    key2[3]='\0';
    keylwr=strlwr(key1);
    if((strcmp(keylwr,"http")==0) ){ 
        char *output =malloc(sizeof(char)*(strlen(url)+1));
        strcpy(output,url);
        return output;
    }  
    if( (strcmp(key,"//")==0)){ 
        strcat(temp,url);
        char *output =malloc(sizeof(char)*(strlen(temp)+1));
        strcpy(output,temp);
        return output;
    }
    if( (strcmp(key2,"://")==0)){ 
        strcat(temp1,url);
        char *output =malloc(sizeof(char)*(strlen(temp1)+1));
        strcpy(output,temp1);
        return output;
    }

    if (previous){
        if((strcmp(key,"//")==0)){   
            strcat(temp,url);
            char *output =malloc(sizeof(char)*(strlen(temp)+1));
            strcpy(output,temp);
            return output;
        }else if(url[0]=='/'){
            strcat(temp,"//");   
            strcat(temp,previous->site); 
            strcat(temp,url);
            char *output =malloc(sizeof(char)*(strlen(temp)+1));
            strcpy(output,temp);
            return output;
        }else{ //implied host protocol dir
            if (strcmp(previous->path,NP)==0){
                strcat(temp,"//");  
                strcat(temp,previous->site); 
                strncat(temp,&add,1);
                strcat(temp,url);
                char *output =malloc(sizeof(char)*(strlen(temp)+1));
                strcpy(output,temp);    
                return output;
            }else{
                char copy[1000];
                strcpy(copy,previous->path);
                slash=strchr(copy,'/');
                while(slash){
                    if (!(slash1=strchr(++slash,'/'))){
                        *slash='\0';
                        strcat(temp,"//");  
                        strcat(temp,previous->site); 
                        strcat(temp,copy);
                        strcat(temp,url);
                        char *output =malloc(sizeof(char)*(strlen(temp)+1));
                        strcpy(output,temp);    
                        return output;
                    }
                }
            }
        }
    }else{
        char *output =malloc(sizeof(char)*(strlen(IU)+1));
        strcpy(output,IU);    
        return output;
    }
}

int crawl_test(URL_INFO *previous, URL_INFO *current){
    char *temp,*temp1;

    temp1=strchr(previous->site,'.');
    temp=strchr(current->site,'.');
    if (strcmp(temp,temp1)==0){
      return 1;
    }else{
      return 0;
    }
}

int visited_test(char *websites[] , int length, char *url){

    for (int i=0;i<length;i++){
        if (strcmp(websites[i],url)==0){
            return 0;
        }
    }
    return 1;
}

void free_url(URL_INFO *url){
    free(url->path);
    free(url->site);
}

int check_path(URL_INFO *url){
    if ((strstr(url->path,"./"))||(strstr(url->path,".."))||(strstr(url->path,"?"))||(strstr(url->path,"#"))||(strchr(url->path,'%'))){
        return 0;
    }
    return 1;

}

char* special_connect(char url[],char dir[]){
    int  client,length,received ; 
    char response[100000];   
    struct sockaddr_in serv_addr;
    struct hostent * server;
    
    server = gethostbyname(url);
    if (server == NULL){
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    client= socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0){
        fprintf(stderr,"ERROR opening socket");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    bcopy(server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    
    int connection_status = connect(client, (struct sockaddr *) &serv_addr, sizeof(serv_addr)); 
    if (connection_status==-1){
        fprintf(stderr,"ERROR connecting to host");
        exit(0);
    }
    char *request = create_get(url,dir);
    strcat(request,"Authorization: Basic bmNoZW9uZzE6cGFzc3dvcmQ=\r\n\r\n");
    send(client, request,strlen(request),0);
    free(request);
    recv(client,&response,100000,0);
    length =find_length(response);
    char *copy=malloc(sizeof(char)*(100000));
    strcpy(copy,response);
    char *data = strstr( response, "\r\n\r\n" );
    data+=4;
    char *html= malloc(sizeof(char)*100000);
    strcpy(html,data);
    received=strlen(data);
    memset(response,0,100000);
    if (length!=0){
        while (received!=length){
            if(recv(client,&response,100000,0)<=0){
                break;
            }
            strcat(html,response);
            strcat(copy,response);    
            received=strlen(html);
            memset(response,0,100000);
        } 
    }else{
        while(1){
            if(recv(client,&response,100000,0)<=0){
                break;
            }else{
                strcat(copy,response);
                strcat(html,response);    
                received=strlen(html);
                memset(response,0,100000);
            }
        }
    }
    close(client); 
    free(html);
    return copy;
}

char *check_first(char *url){
    char key[5] ,key1[3] ,add[1000]="http:",add1[1000]="http://";

    strncpy(key,url,4);
    strncpy(key1,url,2);
    key[4]='\0';
    key1[2]='\0';
   
    strlwr(key);
    if ((strcmp(key,"http")==0)){
        char *output=malloc(sizeof(char)*(strlen(url)+1));
        strcpy(output,url);
        return output;
    }else if((strcmp(key1,"//")==0)){
        strcat(add,url);
        char *output=malloc(sizeof(char)*(strlen(add)+1));
        strcpy(output,add);
        return output;
    }else{
        strcat(add1,url);
        char *output=malloc(sizeof(char)*(strlen(add1)+1));
        strcpy(output,add1);
        return output;
    }
}

void print_out(char *curr){
    char *test,*token,url[1000]="http://";
    char first[5],two[3];


    test=malloc(sizeof(char)*(strlen(curr)+6));
    strncpy(first,curr,4);
    first[4]='\0';
    strncpy(two,curr,2);
    two[2]='\0';
    strlwr(first);

    if ((strcmp(first,"http")==0)){
        strcpy(test,curr);
        test+=7;
       
        token=strtok(test,"/");
        while(token !=NULL){
            strcat(url,token);
            strcat(url,"/");
            token=strtok(NULL,"//");
        }
        url[strlen(url)-1]='\0';
        printf("%s\n",url);

    }else{
        strcpy(test,curr);
        token=strtok(test,"/");
        while(token !=NULL){
            strcat(url,token);
            strcat(url,"/");
            token=strtok(NULL,"//");
        }
        url[strlen(url)-1]='\0';
        printf("%s\n",url);
    }
}