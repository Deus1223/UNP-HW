#include <stdio.h>
#include <pcap.h>
#include <time.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

int main(int argc,char **argv)
{
    /* input file name */
    if (argc < 2) {
        printf("Need input file.\n");
        return -1;
    }
	char *filename = argv[1];
    
    /* open pcap file */
    char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *session;
	if ((session = pcap_open_offline(filename, errbuf)) == NULL) {
		printf("pcap file open failed\n");
		return -1;
	}
	printf("Open %s success\n", filename);

    /* bpf */
    char *filter = "";
	if (argc > 2) {
        filter = argv[2];
        printf("Filter: %s\n", filter);
    }
    
    /* compile filter */
	struct bpf_program filter_mask;
	if (pcap_compile(session, &filter_mask, filter, 1, PCAP_NETMASK_UNKNOWN) == -1) {
		printf("pcap file compile error\n");
		pcap_close(session);
		return -1;
	}

    /* record */
	FILE *fp;
	if ((fp = fopen("record.txt","w")) == NULL) {
		printf("rocord.txt open failed\n");
        return -1;
	}

	while(1) {
		struct pcap_pkthdr *header = NULL;
		const u_char *content = NULL;
		int ret;

		if ((ret = pcap_next_ex(session, &header, &content)) == 1) {
			if (pcap_offline_filter(&filter_mask, header, content) != 0) {
				fprintf(fp, "Length: %d\n", header->len);

				struct tm *now = localtime(&header -> ts.tv_sec);
				char buf[100];
				strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", now);
				fprintf(fp, "Time: %s\n", buf);
	
				struct ip *ip_header = (struct ip*)(content + 14);
				u_char prot = ip_header -> ip_p;
				switch(prot){
					case IPPROTO_UDP:
						fprintf(fp, "Protocol: UDP\n");

						struct udphdr *udp_header = (struct udphdr*)(content + 14 + (ip_header -> ip_hl << 2));
						
                        fprintf(fp, "Source port: %5u\n", ntohs(udp_header -> uh_sport));
						fprintf(fp, "Destination port: %5u\n", ntohs(udp_header -> uh_dport));
						break;
					case IPPROTO_TCP:
						fprintf(fp, "Protocol: TCP\n");

						struct tcphdr *tcp_header = (struct tcphdr*)(content + 14 + (ip_header -> ip_hl << 2));

						fprintf(fp, "Source port : %5u\n", ntohs(tcp_header -> th_sport));
						fprintf(fp, "Destination port : %5u\n", ntohs(tcp_header -> th_dport));
						break;
					default:
						printf("Next: %d\n", prot);
						break;
				}

				fprintf(fp, "Source IP : %s\n", inet_ntoa(ip_header -> ip_src));
				fprintf(fp, "Destination IP : %s\n", inet_ntoa(ip_header -> ip_dst));
			}
		}
		else if(ret == 0) {
			printf("Timeout\n");
		}
		else if(ret == -1) {
			printf("Catch next file failed\n");
		}
		else if(ret == -2) {
			printf("Done\n");
			break;
		}
		fprintf(fp, "\n");
	}

	return 0;
}
