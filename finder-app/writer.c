#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
    char * writefile = argv[1];
    char * writestr = argv[2];
    openlog(NULL,0,LOG_USER);

      if (argc != 3) {
        syslog(LOG_ERR,"Invalid number of argument: %d ", argc);
        return 1;
    } 

    syslog(LOG_DEBUG,"writing %s to %s",writestr, writefile);

    FILE *input;
    input = fopen(writefile,"w");
    if (input == NULL) {
        syslog(LOG_ERR,"Cannot open this file %s\n", writefile);
        return 1;
    }
    
    fprintf(input, "%s",writestr);
    fclose(input);

    return 0;
} 