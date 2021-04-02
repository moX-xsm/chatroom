/*************************************************************************
	> File Name: common.c
	> Author: 
	> Mail: 
	> Created Time: 2021年04月01日 星期四 15时55分21秒
 ************************************************************************/

#include"head.h"

char conf_ans[50] = {0};

char *get_value(char *path, char *key){
    FILE *fp = NULL;
    ssize_t nrd;
    char *line = NULL, *sub = NULL;
    size_t linecap;
    //extern char conf_ans[50];
    
    if(path == NULL || key == NULL){
        fprintf(stderr, "Error in arguments!\n");
        return NULL;
    }
    if((fp = fopen(path, "r")) == NULL){
        perror("fopen");
        return NULL;
    }
    while((nrd = getline(&line, &linecap, fp)) != -1){
        if((sub = strstr(line, key)) == NULL) continue;
        else {
            if(line[strlen(key)] == '=') {
                strncpy(conf_ans, sub + strlen(key) + 1, nrd - strlen(key) - 2);
                *(conf_ans + nrd - strlen(key) - 2) = '\0';
                break;
            }

        }
    }

    free(line);
    fclose(fp);
    if(sub == NULL) return NULL;
    return conf_ans;
}

