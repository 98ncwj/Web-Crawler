#ifndef HELPER_H
#define HELPER_H
#define NP "NOT PROVIDED"
#define IU "INVALID URL"
#define ER "ERROR"
#define TY "TRY AGAIN"
#define MV "MOVED"

typedef struct{
    char* protocol ;
    char* site ;
    char* port ;
    char* path ;
} URL_INFO;

//connect to url and returns response
char* my_connect(char url[], char dir[] );
 
//get protocol,host, port, directory from url and stores in URL_INFO struct
URL_INFO* parse_url(URL_INFO* info, const char* url);

//generates GET request
char* create_get(char url[],char dir[]);

//convert string to lowercase
char *strlwr(char *str);

//get individual links from html
int parse_html(char *html,char *web[]);

//complete url by adding protocol,host,directory
char *complete_url(char *url, URL_INFO *previous);

//find length of response
int find_length(char response[]);

//find code of response
int find_response(char response[]);

//decide whether to crawl or not 
int crawl_test(URL_INFO *previous, URL_INFO *current);

//check if visited or not
int visited_test(char *websites[] , int length, char *url);

//free url
void free_url(URL_INFO *url);

//check valid path (no ./ .. ? # %)
int check_path(URL_INFO *url);

//find moved loc for code 301
char *find_location(char response[]);

//get content type from response
char *find_content(char response[]);

//to handle 401 (need special authorization)
char* special_connect(char url[],char dir[]);

//check if first url valid
char *check_first(char *url);

//tidy up and print 
void print_out(char *curr);
#endif