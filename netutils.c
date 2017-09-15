/*
 * Autor: Jovani Brasil
 *
 *
 * Aqui são implementadas as funções necessárias para troca de pacotes UDP entre
 * hosts de uma rede local.
 *
 *
 */


#include "netutils.h"
#include "nettypes.h"

/*
 * 
 *	Checksum routine for Internet Protocol family headers (C Version)
 *	(Disponibilizado pela professora).
 *
 */
unsigned short check_sum(unsigned short *addr, int len){

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
 * 	@param sock - socket inicializado. 
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

int send_udp(int sock, char *dst_ip, u_char mac_dst[6], unsigned char *data, int data_size, int default_port, char interface[]){

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

	// Cabeçalho Ethernet.
	struct ethhdr *eth_header = (struct ethhdr *)(buffer);
	memcpy(&eth_header->h_source, if_mac.ifr_hwaddr.sa_data, sizeof(char)*6);
	
	memcpy(eth_header->h_dest, mac_dst, 6);
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
	ip_header->check = htons(check_sum((unsigned short*)(buffer + 14), 20));

	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
	sadr_ll.sll_halen = ETH_ALEN;
	sadr_ll.sll_family = htons(PF_PACKET);

	memcpy(sadr_ll.sll_addr, mac_dst, 6);

	// Faz envio do pacote.
	if(sendto(sock, buffer, total_size, 0, (const struct sockaddr*)&sadr_ll, sizeof(struct sockaddr_ll)) < 0){
		printf("[x] Erro ao enviar pacote UDP.\n");
		return -1;
	}else{
		printf("[v] Pacote UDP enviado com sucesso.\n");
		return 1;
	}

}

/* Processa cabeçalho UDP.
 *
 */
int get_udp(unsigned char *buff, int offset, int type, int default_port, unsigned char *data){

	struct udphdr udp_header;
	memcpy(&udp_header, &buff[offset], sizeof(udp_header));

	if(default_port != htons(udp_header.dest))
		return -1;	
	
	offset += sizeof(struct udphdr);

	struct data_t data_h;

	memcpy(&data_h, &buff[offset], sizeof(struct data_t));
	memcpy(data, &buff[offset], htons(udp_header.len));

	offset += sizeof(struct data_t);

	if(data_h.cmd < 0 || data_h.cmd > 11)
		return -1;
	else if(data_h.cmd == type || type == -1)
		return htons(udp_header.len) - sizeof(struct udphdr);

	return -1;
	
}

/*
 * Processa o cabeçalho IP. 
 *
 * O pacote é descartado nas seguintes condições:
 *		1) Se o cabeçalho estiver corrompido. 
 *		2) Se ele não emcapsular um cabeçalho UDP.
 *		3) O destino não é a máquina que está recebendo. 
 *
 * Quando um pacote é descartado, é retornado o valor -1.
 *
 */
int get_ipv4(unsigned char *buff, int offset, uint32_t host_ip, int type, int default_port, unsigned char *data, char *dst_ip){

	struct ip ip_header;    	

	memcpy(&ip_header, &buff[14] , sizeof(ip_header));

	// Faz verificação do checksum
	u_char hdr[20];
	memcpy(hdr, &buff[14], 20);
	hdr[10] = 0x0;
	hdr[11] = 0x0;

	uint16_t check = htons(check_sum((unsigned short*)(hdr), 20));

	// Se checksum é inválido, descarta o pacote.
	if(check != htons(ip_header.ip_sum)){
		//printf("checksum not ok\n");
		//return -1;
	}else{
		//printf("checksum ok\n");
	}

	// Se eu não sou o destino, descartar o pacote.
	uint32_t ip_dst = inet_addr(inet_ntoa((ip_header.ip_dst)));		
	
	if(!(ip_dst == host_ip)){
		return -1;
	}else{
		memcpy(dst_ip, inet_ntoa(ip_header.ip_src), 16);	
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

	// TODO MAC destino do pacote deve ser o MAC do host.
	//memcpy(dst_mac, ethernet_header.ether_shost, 6);


	dst_mac[0] = ethernet_header.ether_shost[0];
	dst_mac[1] = ethernet_header.ether_shost[1];
	dst_mac[2] = ethernet_header.ether_shost[2];
	dst_mac[3] = ethernet_header.ether_shost[3];
	dst_mac[4] = ethernet_header.ether_shost[4];
	dst_mac[5] = ethernet_header.ether_shost[5];

	offset += 14;		

	switch (htons(ethernet_header.ether_type)){
		case ETHERTYPE_IP:
			return get_ipv4(buff, offset, host_ip, type, default_port, data, dst_ip);
		default:
			break;	
	}
	return -1;
}


/*	Captura um pacote na rede. 
 *
 *	@param type - define qual tipo de ação deve conter o pacote a ser capturado (exemplo: MOVE, etc).
 *				- type = -1 significa qualquer tipo de ação.  
 *
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
		printf("[x] Erro na leitura do IP do host local.\n");
	}

	while (1) {

		recv(sock, (char *) &buff, sizeof(buff), 0x0);
		uint32_t host_ip = inet_addr(inet_ntoa((((struct sockaddr_in*)&(if_ip.ifr_addr))->sin_addr)));		
		
		int size = get_ethernet(buff, if_mac.ifr_hwaddr.sa_data, host_ip, type, default_port, data, dst_ip, dst_mac);
	
		// Se size < 0, então significa que o pacote foi descartado. Maior que isso então é o 
		// tamanho dos dados encapsulados no UDP. 
		if(size > 0)
			return size;

	}

}




