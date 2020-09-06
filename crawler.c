#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include "helper.h"

int main( int argc, char* argv[]){
    char *first_page, *response,*response1, *weblist[1000], *visit[100];
    URL_INFO current_url,test_url,info;
    int length, count=0, current=0;
    
    //check that received url input
    if (argc <2){
        fprintf(stderr,"ERROR Please provide url");
        exit(0);
    }
    first_page= argv[1];
    
    visit[count]=check_first(first_page);
    count++;
    //while visited pages are less than 100,continue to crawl
    while((count<100)){
        char *current_page=visit[current];
        current++;
        parse_url(&current_url, current_page);
        //connect to url 
        response1=my_connect(current_url.site,current_url.path);
        int code= find_response(response1);
        char *content=find_content(response1);
        //do nothing if 4XX error or page is not text/html
        if ((code==404)||(code==414)||(code==410)||(code==301)||(!(strstr(content,"text")))||(!(strstr(content,"html")))){
            //find location if page moved
            if ((code==301)){
                char *location=find_location(response1);
                visit[count]=malloc(sizeof(char)*(strlen(location)+1));
                strcpy(visit[count],location);
                free(location);
                count++;
            }else{
                if (response1){
                    free(response1);
                }
            }
        //else process html received looking for links 
        }else{
            if ((code==503)||(code==504)||(code==401)){
                //provide authorization if needed
                if (code==401){
                    free(response1);
                    response1=special_connect(current_url.site,current_url.path);
                    visit[current]=malloc(sizeof(char)*(strlen(visit[current-1])+1));
                    strcpy(visit[count],visit[current-1]);
                    count++;
                    current++;
                //if error 5XX attempt to fetch page again
                }else{
                    free(response1);
                    response1=my_connect(current_url.site,current_url.path);
                    visit[current]=malloc(sizeof(char)*(strlen(visit[current-1])+1));
                    strcpy(visit[count],visit[current-1]);
                    count++;
                    current++;
                }
            }
            //process html response and retrieve individual links
            char *response2=strstr(response1,"\r\n\r\n");
            response2+=4;
            response=malloc(sizeof(char)*(strlen(response2)+1));
            strcpy(response,response2);
            free(response1);
            length=parse_html(response,weblist);
            if (response){
                free(response);
            }
            //if no links found do nothing, else add unique unvisited url
            if (length==0){       
            }else{          
                for (int j=0;j<length;j++){
                    if ((strlen(weblist[j])>1000)){

                    }else{
                        weblist[j]=complete_url(weblist[j],&current_url);
                        if ((strcmp(weblist[j],IU)==0)){

                        }else{
                            parse_url(&test_url,weblist[j]);
                            if(check_path(&test_url)==0){

                            }else{
                                if ((crawl_test(&test_url,&current_url)==1)){
                                    if ((visited_test(visit,count,weblist[j])==1)){
                                        visit[count]=malloc(sizeof(char)*(strlen(weblist[j])+1));
                                        strcpy(visit[count],weblist[j]);
                                        count++;  
                                    }
                                }
                            }
                        }
                    }
                    if(count==100){
                        while (j<length){
                            free(weblist[j]);
                            j++;
                            break;
                        }    
                    }else{
                        free(weblist[j]);
                    } 
                }
            }
        }
        free(content);
        free_url(&current_url); 
        //break if 100 unique url found
        if (count==current){
                break;
        }
    }
    //print log of visit
    for(int i=0;i<count;i++){
        print_out(visit[i]);
       free(visit[i]);
    }
    return 0;
}

