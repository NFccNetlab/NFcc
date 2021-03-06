#define FUNCTION PC
#include <stdio.h>
#include <pcap.h>
#include <time.h>
#include "basic_classes.h"
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <cstring>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#define RECEIVE_NUM 100000
#define MAX_PACKET_LEN 65535 
#define NUMOFSTREAM 1 

//extern "C" char* gpu_encrypt(char *akey, char *plain); //注意这里的声明
extern  "C" int cpu_encrypt(char* key, char *buf, unsigned long length,char *cipher);
#ifdef u_char
#define u_char unsigned char
#endif

using namespace std;

long _counter,_sum = 0;
clock_t begin_t;
clock_t end_t;
u_char *packet;
pcap_t *phandle;
pcap_t *phandle_eth2;
int _allTput=0;
unordered_map<string,int> F_Type::MAP = unordered_map<string,int>();
int process(Flow &f);
Flow f_glb;
struct itimerval tick;
sigset_t newmask;

struct timeval begin_time;
struct timeval end_time;
u_char* packet_set[1000000];
int total_len[1000000];
long int pktindex = 0;

long sub_num;
char* subs;

char * my_strstr(char *str,char *sub,int str_len,int sub_len)
{
  if(str_len < sub_len) return NULL;
  if(str_len != 0 && sub_len == 0) return NULL;
  if(str_len == 0 && sub_len == 0) return NULL;
  int m, n;
  for(int i=0;i<str_len;++i){
    m = 0;
    n = i;
    if(str[n]==sub[m]){
      while(str[++n] == sub[++m]){
        if(sub[m+1] == '\0') return str+i;
      }
    }
  }
  return NULL;
}

int p_match( char* haystack, int LENGTH, int num_sub, int len, char* subs)
{
  char *position[num_sub];
  char *h_position[num_sub];
//  genRandomString(haystack,LENGTH);
//  genRandomSubString(subs,len*num_sub,len);

  memset(h_position,0,sizeof(char*)*num_sub);
  const size_t smem = sizeof(char)*LENGTH;
  char h_subs[num_sub][len];
  for(long i=0;i<num_sub;++i){
    for(int j=0;j<len;++j){
      h_subs[i][j] = subs[i*len+j];
    }
  }
    

  for(int i=0;i<num_sub;++i)
  { 
    char* ret;
    //printf("string is %s and sub is %s\n",haystack,h_subs[i]);
    ret = my_strstr(haystack,h_subs[i],LENGTH,len);
    if(ret != NULL){
      //printf("find one sub string in %d sub\n",i);
      //printf("%s\n",ret);
    }
    position[i] = ret;
  }
  return(0);
}

void decodePkt(const u_char *pkt_data)
{
    printf("    Mac : %2x:%2x:%2x:%2x:%2x:%2x",pkt_data[6],pkt_data[7],pkt_data[8],pkt_data[9],pkt_data[10],pkt_data[11]);
    printf(" -> %2x:%2x:%2x:%2x:%2x:%2x\n",pkt_data[0],pkt_data[1],pkt_data[2],pkt_data[3],pkt_data[4],pkt_data[5]);
    printf("    IP  : %3d.%3d.%3d.%3d",pkt_data[26],pkt_data[27],pkt_data[28],pkt_data[29]);
    printf("   -> %3d.%3d.%3d.%3d\n",pkt_data[30],pkt_data[31],pkt_data[32],pkt_data[33]);
}


void stop(int signo)
{
    end_t = clock();
    double total;
    total = (double)(end_t - begin_t)/CLOCKS_PER_SEC;
    printf("\n\n**************************************************\n");
    printf("%ld are forwarded ! Average Tput is %f \n",_counter, _counter / total);
    printf("counter = %ld  sum = %ld  begin = %ld  end = %ld \n", _counter, _sum, begin_t ,end_t);
    printf("**************************************************\n\n");
    _exit(0);
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* INSERTION STARTS FROM HERE*/void _init_(){
    (new F_Type())->init();
}
char *key = "bd5767b5c272dd72bd72123123121231";
int process(Flow &f){
struct timeval head_time, tail_time;
if (1){
//    gettimeofday(&head_time,NULL);    
//    cpu_encrypt((char *)key,(char *)f["payload"], *((unsigned long*) f["payload_length"]) + 34,(char *)f["payload"]);
      p_match( (char *)f["payload"], *((int *)f["payload_length"]) + 34, sub_num, 16, subs);
//    gettimeofday(&tail_time,NULL);
//    double total = tail_time.tv_sec-head_time.tv_sec + (tail_time.tv_usec-head_time.tv_usec)/(1000.0);
//	printf("%f ms\n",total);
}
        f.clean();
        return 0;
}
int PC(u_char * pkt,int totallength) {
    f_glb= Flow(pkt,totallength);
    return process(f_glb);
}


int read_local_pcap(int argc, char* argv[]){

    char filePath[NUMOFSTREAM][100];
    const u_char *packets[NUMOFSTREAM];
    struct pcap_pkthdr hdr[NUMOFSTREAM];
    unsigned packetSize[NUMOFSTREAM];
    pcap_t *descr[NUMOFSTREAM];

    int loop = 0;
    for (loop = 0;loop < NUMOFSTREAM;loop++)
    {
        sprintf(filePath[loop],"/home/ubuntu/Desktop/pcap/%d/1_1_%s.pcap",loop+1,argv[1]);
        //printf("path : %s\n", filePath[loop]);
    }

    char errbuf[PCAP_ERRBUF_SIZE];

    loop = 0;
    for (loop = 0;loop < NUMOFSTREAM;loop++)
    {
        if ((descr[loop] = pcap_open_offline(filePath[loop],errbuf)) == NULL)
        {
            printf("read pcap file error !\n");
            return -1;
        }
        else
        {
            packets[loop] = pcap_next(descr[loop],&hdr[loop]);
            packetSize[loop] = hdr[loop].caplen;
            //printf("trace %d :\n    len = %d Bytes\n",loop+1,packetSize[loop]);
            decodePkt(packets[loop]);
        }
    }

    while(pktindex < 1000)
    {
        for(loop=0; loop < NUMOFSTREAM; loop++){

            packet_set[pktindex] = new u_char[MAX_PACKET_LEN];
            memcpy(packet_set[pktindex],packets[loop],packetSize[loop]);

            total_len[pktindex] = packetSize[loop];
            pktindex++;
        }
    }
    return 0;

}

void genRandomSubString(char *str,int length,int sub_len)
{
  for(int i=0;i<length-1;++i)
  {
    if(i>0 && ((i+1)%sub_len==0)) str[i] = '\0';
    else str[i] = 'a' + rand()%26;
  }
  str[length-1] = '\0';
}

int main(int argc, char* argv[])
{
    _init_();
    scanf("%ld",&sub_num);
    subs = (char*)calloc(sub_num*16, sizeof(char));
    genRandomSubString(subs,16*sub_num,16);
    long int _s =0;
    long int _drop=0;

    if(read_local_pcap(argc,argv)==-1)
    {
        printf("read pcap wrong!\n");
        return 0;
    }

    printf("Packet NUM: %ld\n",pktindex );

    gettimeofday(&begin_time,NULL);
    int _times=1;
    
    printf("Send times: %d\n", _times);
    for (int j =0;j<_times;j++){
        int i=0;
    for(;i<pktindex;i++){
        if (FUNCTION(packet_set[i],total_len[i])==-1){
            _drop++;
        }
    }}
    _s= _times * pktindex;
    gettimeofday(&end_time,NULL);

    double total = (end_time.tv_sec-begin_time.tv_sec)*1000000 + (end_time.tv_usec-begin_time.tv_usec);
    printf("    size = %s	time = %f        ave_time = %f	 p_num = %d	pps = %f     sum = %ld    drop = %ld    drop_rate = %f \n", argv[1],total, total / 1000,sub_num, _s / total, _s, _drop, (double)_drop/(double)_s);
    FILE *fp = fopen("data.txt","a+");
    fprintf(fp,"size = %s    time = %f        ave_time = %f	p_num = %d	pps = %f     sum = %ld    drop = %ld    drop_rate = %f \n", argv[1],total, total / 1000,sub_num, _s / total, _s, _drop, (double)_drop/(double)_s);
    fclose(fp);
    free(subs);
    return 0;
}
