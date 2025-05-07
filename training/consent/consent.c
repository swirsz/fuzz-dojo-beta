#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>

void wrap_text(char *line_start, int width) {
  char *last_space = 0;
  char *p;

  for (p = line_start; *p; p++) {
    if (*p == '\n') {
      line_start = p + 1;
    }

    if (*p == ' ') {
      last_space = p;
    }

    if (p - line_start > width && last_space) {
      *last_space = '\n';
      line_start = last_space + 1;
      last_space = 0;
    }
  }
}

int
main(int argc, char **argv, char **envp) 
{
    printf("###\n");
    printf("### Consent Form \n");
    printf("###\n");
    printf("\n");
    
    char const text[] = 
    "As an associate professor of computer science at Arizona State University, I am conducting a research study to improve fuzz drivers for software projects implemented within OSS-Fuzz.  We will provide tools that evaluate the coverage of fuzzing drivers and make suggestions for creating additional fuzz drivers or improving existing ones.  New drivers have the potential to test previously untested code and find additional software bugs that existing fuzz drivers are not seeing.  Applicants will receive any necessary training to create fuzz drivers and will be supplied with tools to analyze the effectiveness and limitations of existing OSS-Fuzz fuzz drivers.  \n\n"

    "I am inviting your participation, which will involve first becoming familiar with an example fuzzing project. The typical process will be selecting an open-source project written in C or C++, becoming familiar with its basic use, looking at the existing fuzz drivers to understand how they work, and seeing what their current code coverage is via static and dynamic analysis tools that will be provided. These tools will also offer suggestions for functions to implement as targets of new fuzz drivers.  These suggestions may or may not be helpful. Your typical activity will be building valid parameters to pass to this function, debugging and testing your code to discover if new code coverage is achieved, and making adjustments and modifications if blockages or unforeseen bugs occur.\n\n"

    "You must be 18 or older to participate in the study. Your participation in this study is completely voluntary. If you choose not to participate or to withdraw from the study at any time, there will be no penalty. There are no foreseeable risks or discomforts to your participation. Participation in this study will require 1 week of work. \n\n"

    "Each subject will utilize a unique pseudonym: this name will be used as a pwn.college user name, survey response name, and a GitLab username. All recorded data will use this pseudonym and not be linked to other identifying information.  De-identified data collected in the current study will be shared with others (e.g., investigators or industry partners) for future research or other uses.\n\n"

    "If you have any questions concerning the research study, please contact the research team at Yan Shoshitaishvili <yans@asu.edu>. If you have any questions about your rights as a subject/participant in this research, or if you feel you have been placed at risk, you can contact the Chair of the Human Subjects Institutional Review Board, through the ASU Office of Research Integrity and Assurance, at (480) 965-6788.\n\n"

    FILE *report, *flag;
    char buf[sizeof(text)];
    char envname2[16];
    char filename[80];
    char response;
    char* envname;
    int ch;
    struct tm *timeinfo;
    struct winsize w;
    time_t rawtime;
    memcpy(buf, text, sizeof(text));
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
    perror("ioctl");
    return 1;
    }

    wrap_text(buf, w.ws_col-2);
    puts(buf);

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    envname = getenv("HOSTNAME");
    envname = strncpy(envname2, envname+8,15);
    strftime(filename, 80, "/home/hacker/.log/%Y-%m%d--%H-%M-%S", timeinfo);
    strcat(filename, envname2);
    strcat(filename, "-plan.txt");

    printf("Do you consent to these terms?  (y/n)");
    scanf(" %c", &response);

    if (response == 'y') {
            report = fopen(filename, "w");
        if (report == NULL) {
            printf("Error opening log file!\n");
            return 1;
        }
        flag = fopen("/flag", "r");
        printf("\n\n\n");
        while ((ch = fgetc(flag)) != EOF) {
          printf("%c", ch);
        }
        fclose(report);
        fclose(flag);
    }
    return 0;
    }
