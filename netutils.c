/*
 * Trabalho de Redes de Computadores II - CC/PUCRS
 *
 * Alunos: João Chagas e Jovani Brasil
 * Professora: Ana Benso
 *
 *
 * netutils.c 
 *
 *
 * Aqui são implementadas as funções necessárias para troca de pacotes UDP em
 * uma rede local.
 *
 * ...
 *
 *
 *
 *
 *
 *
 */



#include "netutils.h"
#include "nettypes.h"

#define DESTMAC0	0x08
#define DESTMAC1	0x00
#define DESTMAC2	0x27
#define DESTMAC3	0xd0
#define DESTMAC4	0x9a
#define DESTMAC5	0x92

/*
 * Envia um ARP_REQUEST em broadcast no enlace e aguarda por um ARP_REPLY.
 *
 *
 * TODO justificar tamanho das mensagens
 *
 */
int send_arp_request(unsigned char *dst_ip, unsigned char *mac_address, char interface[]){
        
	struct ifreq ifreq_i, if_ip, if_mac;
	int sock, index, len;

	unsigned char *buffer = (unsigned char*)malloc(sizeof(char)*60);
	memset(buffer,0x00,60);
        
	// Abre o socket.
	if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
    		perror ("[x] Erro ao abrir socket para envio do arp.");
  	}

	// Busca índice da interface.
	memset(&ifreq_i, 0, sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name, interface, IFNAMSIZ-1);
	if((ioctl(sock, SIOCGIFINDEX, &ifreq_i)) < 0)
		printf("[x] Erro na leitura de INFOS do host local.\n");

	// Busca por MAC e IP do host.
	memset(&if_mac, 0, sizeof(if_mac));
	strncpy(if_mac.ifr_name, interface, IFNAMSIZ-1);
	if((ioctl(sock,SIOCGIFHWADDR,&if_mac))<0)
		printf("[x] Erro na leitura do MAC do host local.\n");

	memset(&if_ip, 0, sizeof(if_ip));
	strncpy(if_ip.ifr_name, interface, IFNAMSIZ-1);
  	if(ioctl(sock,SIOCGIFADDR,&if_ip)<0){
		printf("[x] Erro na leitura do IP do host local.\n");
	} 

	// Monta cabeçalhos Ethernet e ARP.
        struct ethhdr *eth_h = (struct ethhdr *)buffer;	
	struct arp_header *arp_h = (struct arp_header *)(buffer+14);

	memset(&eth_h->h_dest, 0xFF, 6);
        memcpy(&eth_h->h_source, if_mac.ifr_hwaddr.sa_data, sizeof(char)*6);
        memset(&arp_h->ar_tha, 0x00, 6);
        memcpy(&arp_h->ar_sha, if_mac.ifr_hwaddr.sa_data, sizeof(char)*6);
	
        struct sockaddr_ll sadr_ll;

        sadr_ll.sll_family = AF_PACKET;
        sadr_ll.sll_protocol = htons(ETH_P_ARP);
        sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
        sadr_ll.sll_hatype = htons(ARPHRD_ETHER);
        sadr_ll.sll_pkttype = (PACKET_BROADCAST);
        sadr_ll.sll_halen = 0x06;
        memcpy(&sadr_ll.sll_addr, if_mac.ifr_hwaddr.sa_data, sizeof(char)*6);

        eth_h->h_proto = htons(ETH_P_ARP);

        arp_h->ar_hrd = htons(0x01);
        arp_h->ar_pro = htons(0x800);
        arp_h->ar_hln = 0x06;
        arp_h->ar_pln = 0x04;
        arp_h->ar_op = htons(0x01);

	struct in_addr ip_addr = {0};
	if (!inet_aton(dst_ip, &ip_addr)) {
	    printf("Endereco %s invalido.", dst_ip);
	}

	memcpy(&arp_h->ar_tip, &ip_addr.s_addr, sizeof(arp_h->ar_tip));
	memcpy(&arp_h->ar_sip, &if_ip.ifr_addr, sizeof(arp_h->ar_sip));
 
	// Envia ARP REQUEST em broadcast na rede.
        if(sendto(sock, buffer, 42, 0, (struct  sockaddr*)&sadr_ll, sizeof(sadr_ll)) < 0){
                perror("Erro no envio do ARP_REQUEST.");
                exit(1);
        }

        memset(buffer,0x00,60);

	unsigned char aux_ip[4];
	memcpy(&aux_ip, &ip_addr.s_addr, sizeof(arp_h->ar_tip));

	// TODO Tratar o caso onde o reply não é recebido, ou seja, é necessário
	// reenviar o arp_request. 

        while(1){
		BEGIN:
		// Aguarda ARP_REPLY do destino.
                if((len = recvfrom(sock, buffer, 60, 0, NULL, NULL)) < 0){
                        perror("Mensagem recebida com erro!\n");
                }
		else if(htons(eth_h->h_proto) == 0x0806){
			if(htons(arp_h->ar_op) == 0x02){
                        	
				// Verifica se foi o destino correto que enviou.
				int equal = 0;
				for(index=0; index<4; index++){
                                	if(arp_h->ar_sip[index] != aux_ip[index]){
						goto BEGIN;
					}
				}

				// Copia mac para o espaço de memória adequado.	
				sprintf(mac_address, "%02X:%02X:%02X:%02X:%02X", arp_h->ar_sha[0],
					       	arp_h->ar_sha[1], arp_h->ar_sha[2],
					      	arp_h->ar_sha[3], arp_h->ar_sha[4]);
			    	
				return 1;	
			}
                }
        }

        return 0;
}

/*
 * 	Recebe um IP e verifica se o IP está na arp_table. Caso não estiver, manda um arp request
 * 	para o host destino. 
 *
 * 	@param ip_address - IP do host ao qual se procura o mac.
 * 	@param mac_address - Ponteiro para o espaço de memória onde será copiado o endereço
 * 			     MAC do host destino.
 * 	@param interface - Nome da interface utilizada nas operações necessárias.
 *
 * 	@return - Retorna 1 se encontrou o MAC e 0 se não encontrou. 
 *
 */
int get_mac_address(char *ip_address, char *mac_address, char interface[]){ 	

	// Primeiro verifica se mac está na arp cache.
	// Percorre linha a linha a arp cache verificando se o ip destino
	// já possui o registro do seu mac correspondente.

	const char filepath[] = "/proc/net/arp";
	FILE *file = fopen(filepath, "r");
	if (file){
		char line [BUFSIZ];
		fgets(line, sizeof(line), file);
		while (fgets(line, sizeof(line), file)){
			
			char  c[16], d[10], ip[16], mac[18];	
			int a, b, i, equals;

			sscanf(line, "%s 0x%x 0x%x %s %s %s\n", ip, &a, &b, mac, c, d);
			
			for(i=0; i<15; i++){
				// TODO Rever esta lógica.
				if(ip[i] == '\0' || ip_address[i] == '\0'){
					break;
				}
				if(ip[i] != ip_address[i]){
					equals = 0 ;
					break;
				}	
			}	
			if(equals){
				printf("%s\n", mac);
				memcpy(mac_address, &mac, 18);
				return 1;
			}
		}
	}else{
		perror(filepath);
	}

	// Se não estiver na ARP_CACHE, faz um ARP_REQUEST.
	return send_arp_request(ip_address, mac_address, interface);

}

/*
 * 
 *	Checksum routine for Internet Protocol family headers (C Version)
 *
 *
 *	TODO Verificar se esta funcao é a mesma que a professora disponibilizou.
 *
 */
unsigned short check_sum(unsigned short *addr,int len){

        register int sum = 0;
        u_short answer = 0;
        register u_short *w = addr;
        register int nleft = len;

        /*
         * Our algorithm is simple, using a 32 bit accumulator (sum), we add
         * sequential 16 bit words to it, and at the end, fold back all the
         * carry bits from the top 16 bits into the lower 16 bits.
         */
        while (nleft > 1)  {
                sum += *w++;
                nleft -= 2;
        }

        /* mop up an odd byte, if necessary */
        if (nleft == 1) {
                *(u_char *)(&answer) = *(u_char *)w ;
                sum += answer;
        }

        /* add back carry outs from top 16 bits to low 16 bits */
        sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
        sum += (sum >> 16);                     /* add carry */
        answer = ~sum;                          /* truncate to 16 bits */
        
	//printf("Checksum result = %d\n", answer);

	return(answer);
}

/*
 * 	Envia um pacote UDP para o IP destino especificado.
 *
 * 	@param sock - TODO 
 * 	@param dst_ip - IP do host destino do pacote.
 * 	@param data - Ponteiro para área de memória onde está os dados a serem enviados.
 * 	@param data_size - Tamanho total do bloco de dados a ser enviado.
 * 	@param default_port - Considaramos que, o número de porta default utilizada para envio
 * 			      é o mesmo número de porta para recebimento. 
 * 	@param interface - Nome da interface que deve ser utilizada para o envio do pacote.
 *
 * 	@return - Retorna 1 se o pacote foi enviado com sucesso, caso contrário -1.
 *
 */

int send_udp(int sock, char *dst_ip, unsigned char *data, int data_size, int default_port, char interface[]){

	struct ifreq if_mac, ifreq_i, if_ip; 
	unsigned char *buffer;
       	int len = 0;
	char *dst_mac = (char*)malloc(6*sizeof(char));

	int total_size = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + data_size; 



	// Abrindo socket.
	if((sock = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) < 0)
		printf("[x] Erro na abertura do socket.\n");

	// Alocando e inicializando buffer de dados.	
	buffer = (unsigned char*)malloc(total_size); 
	memset(buffer, 0, total_size);

	// Busca index da interface indicada.
	memset(&ifreq_i, 0, sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name, interface, IFNAMSIZ-1);
	if((ioctl(sock, SIOCGIFINDEX, &ifreq_i)) < 0)
		printf("[x] Erro na leitura de INFOS do host local.\n");

	// Busca por MAC e IP do host.
	memset(&if_mac, 0, sizeof(if_mac));
	strncpy(if_mac.ifr_name, interface, IFNAMSIZ-1);
	if((ioctl(sock, SIOCGIFHWADDR, &if_mac)) < 0)
		printf("[x] Erro na leitura do MAC do host local.\n");

	memset(&if_ip, 0, sizeof(if_ip));
	strncpy(if_ip.ifr_name, interface, IFNAMSIZ-1);
  	if(ioctl(sock, SIOCGIFADDR, &if_ip) < 0){
		printf("[x] Erro na leitura do IP do host local.\n");
	}

	// Busca MAC do IP destino.
	//get_mac_address(dst_ip, dst_mac, interface);

	// Cabeçalho Ethernet.
	// TODO Wireshark aponta que o campo frame check sequence tem problemas.
	struct ethhdr *eth_header = (struct ethhdr *)(buffer);
	memcpy(&eth_header->h_source, if_mac.ifr_hwaddr.sa_data, sizeof(char)*6);
	
	//memcpy(eth_header->h_dest, dst_mac, sizeof(char)*6);
	
	eth_header->h_dest[0] = DESTMAC0;
	eth_header->h_dest[1] = DESTMAC1;
	eth_header->h_dest[2] = DESTMAC2;
	eth_header->h_dest[3] = DESTMAC3;
	eth_header->h_dest[4] = DESTMAC4;
	eth_header->h_dest[5] = DESTMAC5;

    printf("MAC dest: %x:%x:%x:%x:%x:%x\n", DESTMAC0, DESTMAC1, DESTMAC2, DESTMAC3, DESTMAC4, DESTMAC5);

	eth_header->h_proto = htons(ETH_P_IP);
   
	len += sizeof(struct ethhdr);	

	// Cabeçalho IP.
	struct iphdr *ip_header = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	ip_header->ihl = 5;
	ip_header->version = 4;
	ip_header->tos = 16;
	ip_header->id = htons(10201);
	ip_header->ttl = 64;
	ip_header->protocol = 17;
	ip_header->saddr = inet_addr(inet_ntoa((((struct sockaddr_in *)&(if_ip.ifr_addr))->sin_addr)));
	inet_pton(AF_INET, dst_ip, &(ip_header->daddr));	

	len += sizeof(struct iphdr); 

	// Cabeçalho UDP.
	struct udphdr *udp_header = (struct udphdr*)(buffer + sizeof(struct iphdr) + sizeof(struct ethhdr));

	udp_header->source = htons(default_port);
	udp_header->dest = htons(default_port);
	udp_header->check = 0;

	len += sizeof(struct udphdr);

	// Dados do buffer.
	memcpy(&buffer[len], data, data_size);
	len += data_size;

	udp_header->len	= htons((len - sizeof(struct iphdr) - sizeof(struct ethhdr)));

	ip_header->tot_len = htons(len - sizeof(struct ethhdr));
	ip_header->check = htons(check_sum((unsigned short*)(buffer + sizeof(struct ethhdr)), (sizeof(struct iphdr)/2)));

	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
	sadr_ll.sll_halen = ETH_ALEN;
	sadr_ll.sll_family = htons(PF_PACKET);	
	sadr_ll.sll_addr[0] = DESTMAC0;
	sadr_ll.sll_addr[1] = DESTMAC1;
	sadr_ll.sll_addr[2] = DESTMAC2;
	sadr_ll.sll_addr[3] = DESTMAC3;
	sadr_ll.sll_addr[4] = DESTMAC4;
	sadr_ll.sll_addr[5] = DESTMAC5;


	//if(sendto(sock, buffer, total_size, 0, (const struct sockaddr*)&sadr_ll, sizeof(struct sockaddr_ll)) < 0){


	// Faz envio do pacote.
	if(sendto(sock, buffer, total_size, 0, (const struct sockaddr*)&sadr_ll, sizeof(struct sockaddr_ll)) < 0){
		printf("[x] Erro ao enviar pacote UDP.\n");
		return -1;
	}else{
		printf("[v] Pacote UDP enviado com sucesso.\n");
		return 1;
	}

}

/* TODO documentar
 * 
 *
 *
 *
 *
 */
int get_udp(unsigned char *buff, int offset, int type, int default_port, unsigned char *data){


	struct udphdr udp_header;
	memcpy(&udp_header, &buff[offset], sizeof(udp_header));

	if(default_port != htons(udp_header.dest)){
		return -1;
	}	

	
	offset += sizeof(struct udphdr);

	struct data_t data_h;

	memcpy(&data_h, &buff[offset], sizeof(struct data_t));

	memcpy(data, &buff[offset], htons(udp_header.len));

	offset += sizeof(struct data_t);

	//printf("%c\n", buff[offset]);

	if(data_h.cmd < 0 || data_h.cmd > 11){
		return -1;
	}	
	else if(data_h.cmd == type || type == -1){
		return htons(udp_header.len) - sizeof(struct udphdr);
	}
	
	return -1;
	
}

/*
 * TODO Documentar
 *
 *
 *
 */
int get_ipv4(unsigned char *buff, int offset, uint32_t host_ip, int type, int default_port, unsigned char *data, char *dst_ip){

	struct ip ip_header;    	

	memcpy(&ip_header, &buff[offset] , sizeof(ip_header));

	// Se eu não sou o destino, descartar o pacote.
	uint32_t ip_dst = inet_addr(inet_ntoa((ip_header.ip_dst)));		
	
	//printf("%s\n", inet_ntoa(ip_header.ip_src));

	if(!(ip_dst == host_ip)){
		return -1;
	}else{
		memcpy(dst_ip, inet_ntoa(ip_header.ip_src), 16);
		//printf("---> %s\n", dst_ip);	
	}

	offset += sizeof(ip_header);
		
	switch(ip_header.ip_p){
		case IPPROTO_UDP:
			return get_udp(buff, offset, type, default_port, data);
		default:
			break;
	}
	return -1;
}


int get_ethernet(unsigned char *buff, char *host_mac, uint32_t host_ip,  int type, int default_port, unsigned char *data, char *dst_ip, unsigned char *dst_mac){
	
	struct ether_header ethernet_header;
	int offset = 0;

	memcpy(&ethernet_header, buff, sizeof(ethernet_header));

	// TODO MAC destino deve ser o MAC do host.
	memcpy(dst_mac, ethernet_header.ether_shost, 6);
	//printf("----->  %02x:%02x:%02x:%02x:%02x:%02x  \n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);

	offset += 14;		

	switch (htons(ethernet_header.ether_type)){
		case ETHERTYPE_IP:
			return get_ipv4(buff, offset, host_ip, type, default_port, data, dst_ip);
		default:
			break;	
	}
	return -1;
}


/*	Recebe um pacote udp.
 *
 * 	@param sock TODO
 * 	@param ip_dst - TODO
 * 	@param default_port - TODO
 *	@param type - TODO
 *	@param data - Ponteiro para o buffer de dados. TODO Qual deve ser o tamanho máximo?
 *	@param interface - Nome da interface utilizada para recebimento dos pacotes.
 *
 */
int get_packet(int sock, int default_port, char interface[], int type, unsigned char *data, char *dst_ip, unsigned char *dst_mac){

	unsigned char buff[BUFFSIZE];
	struct ifreq if_mac,ifreq_i,if_ip; 
	struct ifreq ifr;

  	// O procedimento abaixo eh utilizado para "setar" a interface em modo promiscuo.
	strcpy(ifr.ifr_name, interface);
	if(ioctl(sock, SIOCGIFINDEX, &ifr) < 0){
		printf("[x] Erro no ioctl!\n");
	}
	ioctl(sock, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags |= IFF_PROMISC;
	ioctl(sock, SIOCSIFFLAGS, &ifr);

	// Busca por MAC e IP do host.
	memset(&if_mac, 0, sizeof(if_mac));
	strncpy(if_mac.ifr_name, interface, IFNAMSIZ-1);
	if((ioctl(sock, SIOCGIFHWADDR, &if_mac)) < 0){
		printf("[x] Erro na leitura do MAC do host local.\n");
	}
	
	memset(&if_ip, 0, sizeof(if_ip));
	strncpy(if_ip.ifr_name, interface, IFNAMSIZ-1);
  	if(ioctl(sock, SIOCGIFADDR, &if_ip) < 0){
		printf("[x] Errp na leitura do IP do host local.\n");
	}

	while (1) {

		recv(sock, (char *) &buff, sizeof(buff), 0x0);
		uint32_t host_ip = inet_addr(inet_ntoa((((struct sockaddr_in*)&(if_ip.ifr_addr))->sin_addr)));		
		// TODO Documentar isso.

		int size = get_ethernet(buff, if_mac.ifr_hwaddr.sa_data, host_ip, type, default_port, data, dst_ip, dst_mac);
	
		//printf("(2)----->  %02x:%02x:%02x:%02x:%02x:%02x  \n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);

		//printf("(2)%s\n", dst_ip);

		if(size > 0){
			return size;
		}


	}

}




