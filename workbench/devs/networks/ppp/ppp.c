/*
 * $Id$
 */

#include "ppp.h"
#include "device_protos.h"

#define MAXPSIZE 4096
#define CONTROL_SEARCH_BEGIN 1
#define CONTROL_SKIP_HEADER1 2
#define CONTROL_SKIP_HEADER2 3
#define CONTROL_READ_DATA 4

struct packet {
	ULONG packetsize;
	UBYTE header[4];
	UBYTE data[MAXPSIZE];
};

void init_ppp(LIBBASETYPEPTR LIBBASE);

void ConfNetWork();

void Set_phase(UBYTE ph);

void byte_received(UBYTE c);
void ProcessPacket(struct packet * p);
void SendPPP_Packet(struct packet *);
void EscapePacket(struct packet *);

void AddChkSum(struct packet * );
void AddByte(struct packet *p,const UBYTE b);

void LCP_packet(struct packet *);
void SendConfACK(struct packet *);
void SendConfReq();
void SendConfNack( UBYTE *ptr, ULONG len,BYTE num);
void SendConfReject( UBYTE *ptr, ULONG len,BYTE num);
void SendEchoReply(struct packet *);
void SendTerminateReq();

void IPCP_Packet(struct packet * );
void Send_IPCP_ack();
void Send_IPCP_req();
void Send_IPCP_Nack( UBYTE *ptr, ULONG len,BYTE num);
void Send_IPCP_Reject( UBYTE *ptr, ULONG len,BYTE num);


void PAP_Packet(struct packet * );
void Send_PAP_Req();

unsigned char LocalIP[4]={0,0,0,0};
unsigned char RemoteIP[4]={0,0,0,0};
unsigned char PrimaryDNS[4]={0,0,0,0};
unsigned char SecondaryDNS[4]={0,0,0,0};

ULONG async_map,mru;
int Control=0;
int number = 0;
UBYTE *username,*password;

UBYTE phase;
BOOL  my_conf_ok;
BOOL  device_conf_ok;
UBYTE authentication;

#define TIMEOUT 6
#define MAX_TRY 10

int trycounter,timer; 


struct PPPBase *ppp_libbase;

BOOL escape;
struct packet recdpacket;

unsigned long threadid, bytesrecd,byteswritten;


unsigned char writestr[200],ctemp;


static unsigned short fcstab[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#define PPPINITFCS16 0xffff
#define PPPGOODFCS16 0xf0b8
#define PPP_FCS(fcs, c)	(((fcs) >> 8) ^ fcstab[((fcs) ^ (c)) & 0xff])

void AddChkSum(struct packet *p){

	unsigned char *cp;
	unsigned short fcs;
	int i;

	fcs = PPPINITFCS16;
	fcs = PPP_FCS(fcs, 0xff);   // header simulation...
	fcs = PPP_FCS(fcs, 0x03);

	cp = p->data;
	for( i=0 ;i < p->packetsize ; i++ ){
		fcs = PPP_FCS(fcs, *cp);
		cp++;
	}

	fcs = fcs ^ 0xffff;
	AddByte( p , (fcs & 0x00ff) );
	AddByte( p ,((fcs >> 8 ) & 0x00ff));

}


void ppp_timer(int dt){
	
	if( (phase ==  PPP_PHASE_NETWORK) || (phase ==  PPP_PHASE_DEAD)) return;
	
	timer += dt;
	if( timer < TIMEOUT ) return;
	timer = 0;
	
	if( ++trycounter > MAX_TRY ){
		bug("PPP Giveup :-(\n");
		Set_phase( PPP_PHASE_DEAD );
	}	 
	
	bug("PPP Retrying...\n");
	Set_phase( phase ); // Retry
	
}


BYTE Phase(){ return phase;}
	
void Set_phase(UBYTE ph){
	bug("\nPPP PHASE: ");

    timer = 0;
    if( phase != ph )trycounter = 0; // first try
	
	switch ( ph ){

	case PPP_PHASE_CONFIGURATION :
		bug("CONFIGURATION\n");
		phase = ph;

		async_map = 0xffffffff;
		authentication = 0;
		my_conf_ok = FALSE;
		device_conf_ok = FALSE;
		SendConfReq();

		break;

	case PPP_PHASE_AUTHENTICATION :
		bug("AUTHENTICATION\n");
		phase = ph;

		if( authentication ){
			Send_PAP_Req();
		}else{
			bug("no authentication configured -> next phase\n");
			Set_phase( PPP_PHASE_PROTOCOL_CONF );
		}
		break;

	case PPP_PHASE_PROTOCOL_CONF :
		bug("PROTOCOL_CONF\n");
		phase = ph;
		
		my_conf_ok = FALSE;
		device_conf_ok = FALSE;
		
		LocalIP[0]=0;
		LocalIP[1]=0;
		LocalIP[2]=0;
		LocalIP[3]=0;

		RemoteIP[0]=0;
		RemoteIP[1]=0;
		RemoteIP[2]=0;
		RemoteIP[3]=0;

		PrimaryDNS[0]=0;
		PrimaryDNS[1]=0;
		PrimaryDNS[2]=0;
		PrimaryDNS[3]=0;

		SecondaryDNS[0]=0;
		SecondaryDNS[1]=0;
		SecondaryDNS[2]=0;
		SecondaryDNS[3]=0;
		
		Send_IPCP_req();
		break;

	case PPP_PHASE_NETWORK :
		bug("NETWORK\n");
		phase = ph;
		
		if( (RemoteIP[0] | RemoteIP[1] | RemoteIP[2] | RemoteIP[3]) == 0 ){
			bug("\nCould not determine remote IP address !!  defaulting to 10.64.64.64\n");
			RemoteIP[0]=10;
			RemoteIP[1]=64;
			RemoteIP[2]=64;
			RemoteIP[3]=64;
		}
		
		ConfNetWork();
		break;

	case PPP_PHASE_DEAD :
		bug("DEAD\n");
		phase = ph;
		async_map = 0xffffffff;
		break;

	default:
		bug("ERROR unknown phase:%d\n",ph);
		break;

	}


}

void  init_ppp(LIBBASETYPEPTR LIBBASE){
	mru = 1500;
	number=1;
	Control=CONTROL_SEARCH_BEGIN;
	escape = FALSE;
	recdpacket.packetsize = 0;

	ppp_libbase = LIBBASE;

	username = LIBBASE->username ;
	password = LIBBASE->password ;

	Set_phase( PPP_PHASE_DEAD );
	Set_phase( PPP_PHASE_CONFIGURATION );

}


#define FILEBUFFSIZE 4000
//  This really smells like hack...

void ConfNetWork(){

	BPTR InFile, OutFile;
	UBYTE *buff;
	UBYTE *linebuff;
	BOOL putline;
	BYTE myhostname[] = "SYS:Prefs/Env-Archive/AROSTCP/db/netdb-myhost";

	bug("\n###########################################################\n");
	bug("PPP is ONLINE !\n");
	bug("Local IP address %d.%d.%d.%d\n",LocalIP[0],LocalIP[1],LocalIP[2],LocalIP[3]);
	bug("Remote IP address %d.%d.%d.%d\n",RemoteIP[0],RemoteIP[1],RemoteIP[2],RemoteIP[3]);

	if( buff = AllocMem( FILEBUFFSIZE , MEMF_CLEAR|MEMF_PUBLIC ) ){

		if( ppp_libbase->enable_dns ){
			bug("Primary DNS address %d.%d.%d.%d\n", PrimaryDNS[0],PrimaryDNS[1],
				PrimaryDNS[2],PrimaryDNS[3] );
			bug("Secondary DNS address %d.%d.%d.%d\n", SecondaryDNS[0],SecondaryDNS[1],
				SecondaryDNS[2],SecondaryDNS[3] );

			if(linebuff = AllocMem( FILEBUFFSIZE , MEMF_CLEAR|MEMF_PUBLIC ) ){
				bug( "Open File \"%s\"\n" , myhostname );
				if( InFile  =  Open( myhostname , MODE_OLDFILE ) ){
					buff[0]=0;
					while(FGets( InFile , linebuff, FILEBUFFSIZE-1 )){

						putline = TRUE;

						if( strcasestr( linebuff , "NAMESERVER" ) ){ // remove existing nameservers
							putline = FALSE;
						}

						if( strcasestr( linebuff , "autogenerated by PPP.device" ) ){ // remove existing comment
							putline = FALSE;
						}

						if( putline ) strcat( buff, linebuff );

					}

					strcat( buff, ";Name Servers autogenerated by PPP.device\n" );

					sprintf( linebuff ,"NAMESERVER %d.%d.%d.%d\n", PrimaryDNS[0],PrimaryDNS[1],
							 PrimaryDNS[2],PrimaryDNS[3] );
					strcat( buff, linebuff );
					sprintf( linebuff ,"NAMESERVER %d.%d.%d.%d\n", SecondaryDNS[0],SecondaryDNS[1],
							 SecondaryDNS[2],SecondaryDNS[3] );
					strcat( buff, linebuff );

					Close(InFile);
					if( OutFile  =  Open( myhostname , MODE_NEWFILE ) ){
						FPuts( OutFile , buff );
						Close( OutFile );
					}
					bug( "File \"%s\"  is modified !\n" , myhostname );
				}else{
					bug("FAIL!\n");
				}
				FreeMem( linebuff, FILEBUFFSIZE );
			}
		}

		sprintf(buff,"SYS:System/Network/AROSTCP/c/ifconfig ppp0 %d.%d.%d.%d %d.%d.%d.%d" ,
				LocalIP[0],LocalIP[1],
				LocalIP[2],LocalIP[3],
				RemoteIP[0],RemoteIP[1],
				RemoteIP[2],RemoteIP[3] );
		bug("Executing command:\"%s\"\n",buff);
		InFile  = Open( "NIL:" , MODE_OLDFILE );
		OutFile = Open( "NIL:" , MODE_OLDFILE );
		if( SystemTags( buff, NP_Name, "ppp-ifconfig",
						 SYS_Asynch, TRUE,
						 SYS_Input, InFile,
						 SYS_Output, OutFile ,
						 TAG_END )
				!= 0 ) bug("command FAIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

		bug("\n############################################################*\n");

		FreeMem( buff , FILEBUFFSIZE );
	}
}

void RunRoute(){   // called from ConfNetWork() -> ifconfig -> S2_ONLINE -> handler.c Online()
	BPTR InFile, OutFile;
	UBYTE *buff;
	if( buff = AllocMem( FILEBUFFSIZE , MEMF_CLEAR|MEMF_PUBLIC ) ){
		sprintf(buff,"SYS:System/Network/AROSTCP/c/route add default %d.%d.%d.%d",
				RemoteIP[0],RemoteIP[1],
				RemoteIP[2],RemoteIP[3] );
		bug("Executing command:\"%s\"\n",buff);
		InFile  = Open( "NIL:" , MODE_OLDFILE );
		OutFile = Open( "NIL:" , MODE_NEWFILE );
		if(SystemTags( buff, NP_Name, "ppp-route",
						SYS_Asynch, TRUE,
						SYS_Input, InFile,
						SYS_Output, OutFile ,
						TAG_END )
				!= 0 ) bug("command FAIL !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		FreeMem( buff , FILEBUFFSIZE );
	}
}


void printpacket(struct packet * p){
	unsigned int i;
	for( i=0 ;i < p->packetsize ; i++ ) bug("%x,",p->data[i]);
	bug("\n");
}

void inline AddByte(struct packet *p,const UBYTE b){

	if( p->packetsize >= MAXPSIZE ){
		bug("\nERROR:AddByte maxpsize\n");
		return;
	}
	p->data[p->packetsize] = b;
	p->packetsize ++;
}


void  byte_received( UBYTE c ){
	//  bug("Control=%d c=%d,%c,%x \n",Control,c,c,c);
	
	if( c == 0x7e ){   // start/end mark
		
		if( Control == CONTROL_READ_DATA ){
			// bug("stop\n");
			Control = CONTROL_SEARCH_BEGIN;
			ProcessPacket(&recdpacket);
			recdpacket.packetsize = 0;
			return;
		}else if( Control == CONTROL_SEARCH_BEGIN ){
			//  bug("start\n");
			recdpacket.packetsize=0;
			Control = CONTROL_SKIP_HEADER1;
			return;
		}
		bug("PPP control sync error, unexpected start/end mark!\n");
		
	}else if( c == 0x7d ){ // next byte is escaped
		escape = TRUE;
		return;
	}	
	
	if( escape ){
		 c ^= 0x20;
		 escape = FALSE;
	}	 
	
	if( Control == CONTROL_READ_DATA ){
		AddByte(&recdpacket,c);
		return;
	}

	else if( Control == CONTROL_SKIP_HEADER1 ){
		
		if( c == 0xff ){ 
			Control = CONTROL_SKIP_HEADER2;
			return;
		}
			
		bug("PPP control sync error, byte=0x%x, should be 0xff\n",c);
		Control = CONTROL_SEARCH_BEGIN;
		return;
		
	}
	
	else if( Control == CONTROL_SKIP_HEADER2 ){
		if( c == 0x03 ){ 
			Control = CONTROL_READ_DATA;
		}else{
			bug("PPP control sync error, byte=0x%x, should be 0x03\n",c);
			Control = CONTROL_SEARCH_BEGIN;
		}
		return;
	}
}


void ProcessPacket(struct packet * p){
	
	unsigned int type;

	//UnEscapePacket(&recdpacket,&finalpacket);
	
	p->packetsize-=2;               // -checksum

	// bug("ProcessPacket size=%d\n",p->packetsize);
	// printpacket(p);

	type = ( p->data[0] << 8 ) | p->data[1];
	switch ( type ){

	case 0xc021:
		LCP_packet(p);
		break;

	case 0x8021:
		IPCP_Packet(p);
		break;

	case 0xc023:
		PAP_Packet(p);
		break;

	case 0x0021:
		//bug("IP packet received\n");
		if( ( p->packetsize - 2 ) <=  1500 ){
			Incoming_IP_Packet( ppp_libbase , p->data + 2  , p->packetsize - 2  );
		}else{
			bug("OVERSIZE Incoming IP packet !!!!!!!!!!!!!!!!! %d bytes\n",p->packetsize - 2 );
		}

		break;

	case 0xc025:
		bug("Link Quality Report protocol not supported :-(\n");
		break;

	case 0x80fd:
		bug("Compression Control Protocol not supported :-(\n");
		break;

	default:
		;
		bug("Received unknown packet: type=%x\n",type);
		break;
	}

}


void PAP_Packet(struct packet * f){
	bug("\nPAP packet size=%d\n",f->packetsize);
	printpacket(f);

	switch ( f->data[2] ){

	case 1:
		bug("PAP Req Received\n");
		break;

	case 2:
		bug("PAP Ack Received: PAP authentication succeeded\n");

		if( phase == PPP_PHASE_AUTHENTICATION ){
			Set_phase( PPP_PHASE_PROTOCOL_CONF );
		}

		break;

	case 3:
		bug("PAP NAck received\n");
		break;

	case 4:
		bug("PAP Reject received\n");
		Set_phase( PPP_PHASE_DEAD );
		break;

	default:
		bug("PAP unknown type received\n");
		break;
	}
}


void Send_PAP_Req(){

	struct packet p;
	ULONG i;

	bug("\nSend PAP req \"%s\",\"%s\"\n",username,password);
	p.packetsize=0;
	AddByte( &p , 0xc0 );//PAP
	AddByte( &p , 0x23 );
	AddByte( &p , 0x01 ); // req.
	AddByte( &p , ++number ); // number
	AddByte( &p , 0x00 ); // size
	AddByte( &p , 0x00 ); // size <- p.data[5]

	AddByte( &p , strlen( username ) );
	for( i=0 ; i < strlen( username ) ; i++ ) AddByte( &p , username[i] );

	AddByte( &p , strlen( password ) );
	for( i=0 ; i < strlen( password ) ; i++ ) AddByte( &p , password[i] );

	p.data[5] = p.packetsize - 2; // size

	printpacket(&p);

	AddChkSum(&p);
	SendPPP_Packet(&p);
}


void IPCP_Packet(struct packet * p){
	bug("\nIPCP_Packet size=%d\n",p->packetsize);
	printpacket(p);

	ULONG i;
	UBYTE *ptr = p->data + 6 ;
	UBYTE type,len;

	if( p->data[2] >= 1 && p->data[2] <= 4 ){
		while( (IPTR)ptr < (IPTR)( p->data + p->packetsize ) ){
			type = ptr[0];
			len =  ptr[1];
			bug("  Type %d,len %d: ",type,len);
			for( i=0 ; i < len ; i++ ) bug("%x,",ptr[i]);
			bug("\n");
			ptr += len;
		}
	}

	switch ( p->data[2] ){

	case 1:
	
		bug("ipcp Req Received\n");		
		BOOL ok = TRUE;
		ptr = p->data + 6 ;
		while( (IPTR)ptr < (IPTR)( p->data + p->packetsize ) ){
			type = ptr[0];
			len =  ptr[1];

			if( type == 3 ){ // remote IP address
		    	RemoteIP[0]=ptr[2];
				RemoteIP[1]=ptr[3];
				RemoteIP[2]=ptr[4];
				RemoteIP[3]=ptr[5];
				bug("remote IP address is %d.%d.%d.%d\n",RemoteIP[0],RemoteIP[1],
												    	 RemoteIP[2],RemoteIP[3]);											
			}
			
			else {
				bug("  Unknown type %d ,send reject response...\n",type);
				Send_IPCP_Reject( ptr , len , p->data[3] );
				ok = FALSE;
				break;
			}

			ptr += len;
		}

		if(ok){
			
			Send_IPCP_ack(p);
			
			device_conf_ok = TRUE;
			if( ( phase == PPP_PHASE_PROTOCOL_CONF ) && my_conf_ok ){
				Set_phase( PPP_PHASE_NETWORK );
			}
		}

	break;
		
	case 2:
	
		bug("IPCP Ack Received\n");
	    if( (LocalIP[0] | LocalIP[1] | LocalIP[2] | LocalIP[3]) == 0 ){
			bug(" LocalIP still 0.0.0.0 -> Resend IPCP_req\n");	
			Send_IPCP_req();
		}else{	
			my_conf_ok = TRUE;
			if( phase == PPP_PHASE_PROTOCOL_CONF && device_conf_ok   ){
				Set_phase( PPP_PHASE_NETWORK );
			}
		}
		
	break;

	case 3:
	
		bug("ipcp NAck received\n");
		
		ptr = p->data + 6 ;
		while( (IPTR)ptr < (IPTR)( p->data + p->packetsize ) ){
			type = ptr[0];
			len =  ptr[1];

			if( type == 3 ){ // local IP address
		    	LocalIP[0]=ptr[2];
				LocalIP[1]=ptr[3];
				LocalIP[2]=ptr[4];
				LocalIP[3]=ptr[5];
				bug("Local IP address is %d.%d.%d.%d\n",LocalIP[0],LocalIP[1],
														LocalIP[2],LocalIP[3]);	
														
			}else if( type == 129 ){ // PrimaryDNS  address
		    	PrimaryDNS[0]=ptr[2];
				PrimaryDNS[1]=ptr[3];
				PrimaryDNS[2]=ptr[4];
				PrimaryDNS[3]=ptr[5];
				bug("PrimaryDNS address is %d.%d.%d.%d\n",PrimaryDNS[0],PrimaryDNS[1],
														PrimaryDNS[2],PrimaryDNS[3]);												
			}else if( type == 131 ){ // SecondaryDNS address
		    	SecondaryDNS[0]=ptr[2];
				SecondaryDNS[1]=ptr[3];
				SecondaryDNS[2]=ptr[4];
				SecondaryDNS[3]=ptr[5];
				bug("SecondaryDNS address is %d.%d.%d.%d\n",SecondaryDNS[0],SecondaryDNS[1],
														SecondaryDNS[2],SecondaryDNS[3]);												
			}
				
			else {
				bug("  Unknown type %d\n",type);
			}

			ptr += len;
		}	
			
		Send_IPCP_req();
		
	break;

	case 4:
		bug("IPCP Reject received\n");
	break;

	default:
		bug("unknow IPCP Received:%d\n",p->data[2]);
	break;

	}
}


void Send_IPCP_ack(struct packet *p){
	bug("\nsend IPCP act\n");
	p->data[2]=2;
	printpacket(p);
	AddChkSum(p);
	SendPPP_Packet(p);
}


void Send_IPCP_req(){
	struct packet p;
	bug("\nsend IPCP req\n");
	p.packetsize=0;
	AddByte( &p, 0x80 );
	AddByte( &p, 0x21 );
	AddByte( &p, 0x01 );
	AddByte( &p, ++number );
	AddByte( &p, 0x00 );
	AddByte( &p, 0x00 ); //size

	AddByte( &p, 0x03 );
	AddByte( &p, 0x06 );
	AddByte( &p,LocalIP[0] );
	AddByte( &p,LocalIP[1] );
	AddByte( &p,LocalIP[2] );
	AddByte( &p,LocalIP[3] );

	//if( ppp_libbase->enable_dns ){  // ask DNS addresses too...
		AddByte( &p, 129 );
		AddByte( &p, 0x06 );
		AddByte( &p, PrimaryDNS[0] );
		AddByte( &p, PrimaryDNS[1] );
		AddByte( &p, PrimaryDNS[2] );
		AddByte( &p, PrimaryDNS[3] );

		AddByte( &p, 131 );
		AddByte( &p, 0x06 );
		AddByte( &p, SecondaryDNS[0] );
		AddByte( &p, SecondaryDNS[1] );
		AddByte( &p, SecondaryDNS[2] );
		AddByte( &p, SecondaryDNS[3] );
	//}

	p.data[5] = p.packetsize - 2; // size

	printpacket(&p);
	AddChkSum(&p);
	SendPPP_Packet(&p);
}

void Send_IPCP_Nack( UBYTE *ptr, ULONG len,BYTE num){

	struct packet p;
	ULONG i;
	bug("\nsend IPCP Nack\n");
	p.packetsize=0;
	AddByte( &p , 0x80 );//ipcp
	AddByte( &p , 0x21 );
	AddByte( &p , 0x03 ); // conf Nact
	AddByte( &p , num ); // number
	AddByte( &p , 0x00 ); // size
	AddByte( &p , 0x00 ); // <- data[5]

	for( i=0 ; i<len ;i++ ) AddByte( &p , ptr[i] );

	p.data[5] = p.packetsize - 2; // size
	printpacket(&p);
	AddChkSum(&p);
	SendPPP_Packet(&p);
}


void Send_IPCP_Reject( UBYTE *ptr, ULONG len,BYTE num){

	struct packet p;
	ULONG i;
	bug("\nsend IPCP reject\n");
	p.packetsize=0;
	AddByte( &p , 0x80 );//ipcp
	AddByte( &p , 0x21 );
	AddByte( &p , 0x04 ); // conf reject
	AddByte( &p , num ); // number
	AddByte( &p , 0x00 ); // size
	AddByte( &p , 0x00 ); // <- data[5]

	for( i=0 ; i<len ;i++ ) AddByte( &p , ptr[i] );

	p.data[5] = p.packetsize - 2; // size
	printpacket(&p);
	AddChkSum(&p);
	SendPPP_Packet(&p);
}




void LCP_packet(struct packet *p){

	bug("\nLCP packet size=%d\n",p->packetsize);
	printpacket(p);

	ULONG i;
	UBYTE *ptr ;
	UBYTE type,len;

	if( p->data[2] >= 1 && p->data[2] <= 4 ){
		ptr = p->data + 6 ;
		while( (IPTR)ptr < (IPTR)( p->data + p->packetsize ) ){
			type = ptr[0];
			len =  ptr[1];
			bug("  Type %d,len %d: ",type,len);
			for( i=0 ; i < len ; i++ ) bug("%x,",ptr[i]);
			bug("\n");
			ptr += len;
		}
	}

	switch ( p->data[2] ){

	case 1:
		bug("LCP Req Received\n");

		BOOL ok = TRUE;

		ptr = p->data + 6 ;
		while( (IPTR)ptr < (IPTR)( p->data + p->packetsize ) ){
			type = ptr[0];
			len =  ptr[1];

			if( type == 2 ){ // async control character map
				async_map = ( ptr[2] << 24) | ( ptr[3] << 16) | ( ptr[4] << 8) | (ptr[5] );
				bug("  Async_map = %x\n",async_map);
			}else if( type == 1 ){ // maximum receive unit
				mru = ( ptr[2] << 8) | (ptr[3] );
				bug("  MRU = %d\n",mru);
				//  if( mru !=1500 ){  // unlikely
				//  ptr[2] = 0x05;
				//  ptr[3] = 0xdc;
				//  SendConfNack( ptr , len , p->data[3] );
				//  ok = FALSE;
				//  break;
				//  }
			}

			else if( type == 3  ){ // authentication
				if( ptr[2] == 0xc0 &&  ptr[3] == 0x23 ){
					bug("  Authentication = PAP\n");
					authentication = 0xc0;
				}else{
					ptr[2] = 0xc0; // PAP
					ptr[3] = 0x23;
					SendConfNack( ptr , len , p->data[3] );
					ok = FALSE;
					break;

					/*
					ptr[2] = 0xc2; // CHAP md5
					ptr[3] = 0x23;
					ptr[4] = 0x5;
					SendConfNack( ptr , 5 , p->data[3] );
					ok = FALSE;
					break;
					*/
				}
			}

			else {
				bug("  Unknown type %d ,send reject response...\n",type);
				SendConfReject( ptr , len , p->data[3] );
				ok = FALSE;
				break;
			}

			ptr += len;
		}


		if(ok){
			SendConfACK(p);
			device_conf_ok = TRUE;
			if( phase == PPP_PHASE_CONFIGURATION &&
					my_conf_ok &&
					device_conf_ok   ){
				Set_phase( PPP_PHASE_AUTHENTICATION );
			}
		}

		break;

	case 2:
		bug("LCP Ack Received !\n");
		my_conf_ok = TRUE;
		if( phase == PPP_PHASE_CONFIGURATION &&
				my_conf_ok &&
				device_conf_ok   ){
			Set_phase( PPP_PHASE_AUTHENTICATION );
		}

		break;

	case 3:
		bug("LCP NAck Received HELP !\n");
		break;

	case 4:
		bug("LCP Reject Received HELP !\n");
		break;

	case 9: // LCP echo
		SendEchoReply(p);
		break;

	case 8:
		bug("LCP Protocol-Reject ( %d bytes ) Received WTF!???????\n" , p->packetsize );
		break;

	case 5:
		bug("LCP Terminate request Received :-(\n");
		Set_phase( PPP_PHASE_DEAD );
		break;

	case 6:
		bug("LCP Terminate Act Received\n");
		Set_phase( PPP_PHASE_DEAD );
		break;

	case 11:
		bug("LCP Discard-Request Received ????\n");
		//if( phase == PPP_PHASE_AUTHENTICATION ) Send_PAP_Req(); //huawei workaround
		break;

	case 12:
		bug("LCP  Link-Quality Report Received ????\n");
		break;

	default:
		bug("unknow lcp Received:%d\n",p->data[2]);
		break;

	}

}


void SendConfNack( UBYTE *ptr, ULONG len,BYTE num){

	struct packet p;
	ULONG i;
	bug("\nsend LCP Nack\n");
	p.packetsize=0;
	AddByte( &p , 0xc0 );//LPC
	AddByte( &p , 0x21 );
	AddByte( &p , 0x03 ); // conf Nact
	AddByte( &p , num ); // number
	AddByte( &p , 0x00 ); // size
	AddByte( &p , 0x00 ); // <- data[5]

	for( i=0 ; i<len ;i++ ) AddByte( &p , ptr[i] );

	p.data[5] = p.packetsize - 2; // size
	printpacket(&p);
	AddChkSum(&p);
	SendPPP_Packet(&p);
}


void SendConfReject( UBYTE *ptr, ULONG len,BYTE num){

	struct packet p;
	ULONG i;
	bug("\nsend LCP reject\n");
	p.packetsize=0;
	AddByte( &p , 0xc0 );//LPC
	AddByte( &p , 0x21 );
	AddByte( &p , 0x04 ); // conf reject
	AddByte( &p , num ); // number
	AddByte( &p , 0x00 ); // size
	AddByte( &p , 0x00 ); // <- data[5]

	for( i=0 ; i<len ;i++ ) AddByte( &p , ptr[i] );

	p.data[5] = p.packetsize - 2; // size
	printpacket(&p);
	AddChkSum(&p);
	SendPPP_Packet(&p);
}


void SendEchoReply(struct packet *p){
	bug("\nLCP Send Echo Reply \n");
	p->data[2]=10;
	printpacket(p);
	AddChkSum(p);
	SendPPP_Packet(p);
}



void SendConfReq(){

	struct packet p;
	bug("\nsend LCP Req\n");

	p.packetsize=0;
	AddByte( &p , 0xc0 );//LPC
	AddByte( &p , 0x21 );

	AddByte( &p , 0x01 ); // conf req

	AddByte( &p , ++number ); // number

	AddByte( &p , 0x00 ); // size
	AddByte( &p , 0x00 ); // <- data[5]
	/*
	AddByte( &p , 0x01 ); // max receive unit 1500 (default)
	AddByte( &p , 0x04 );
	AddByte( &p , 0x05 );
	AddByte( &p , 0xdc );
	*/
	AddByte( &p , 0x02 ); // async control char map 0000
	AddByte( &p , 0x06 );
	AddByte( &p , 0x00 );
	AddByte( &p , 0x00 );
	AddByte( &p , 0x00 );
	AddByte( &p , 0x00 );

	p.data[5] = p.packetsize - 2; // size
	printpacket(&p);
	AddChkSum(&p);
	SendPPP_Packet(&p);

}



void SendConfACK(struct packet *p){
	bug("\nsend LCP act\n");
	p->data[2]=2;
	printpacket(p);
	AddChkSum(p);
	SendPPP_Packet(p);
}



void SendTerminateReq(){

	struct packet p;
	bug("\nsend LCP Terminate request \n");
	p.packetsize=0;
	AddByte( &p , 0xc0 );//LPC
	AddByte( &p , 0x21 );

	AddByte( &p , 0x05 ); // terminate req

	AddByte( &p , ++number ); // number

	AddByte( &p , 0x00 ); // size
	AddByte( &p , 0x00 ); // <- data[5]

	p.data[5] = p.packetsize - 2; // size

	printpacket(&p);
	AddChkSum(&p);
	SendPPP_Packet(&p);

	Set_phase( PPP_PHASE_DEAD );

}



void send_IP_packet( BYTE *data ,ULONG len ){
	BYTE *ptr = data;
	struct packet p;
	ULONG lenT = len;
// bug( "Send IP packet:%d bytes\n" , len );

	p.packetsize=0;

	p.header[0] = 0x7e; // escaped header
	p.header[1] = 0xff;
	p.header[2] = 0x7d;
	p.header[3] = 0x23;

	AddByte( &p , 0x00 ); // IP
	AddByte( &p , 0x21 );

	while(lenT--){
		AddByte( &p , *ptr++ );
	}

	AddChkSum(&p);
	EscapePacket(&p);
	AddByte( &p , 0x7e );
	SendBYTES( ppp_libbase , p.header , p.packetsize + 4 );

}


void EscapePacket(struct packet * p){
	struct packet dummy;
	unsigned int i,j;

	for(i=0;i<p->packetsize;i++){
		dummy.data[i]=p->data[i];
	}
	dummy.packetsize=p->packetsize;

	for(j=0,i=0;i<dummy.packetsize;i++,j++){

		if( dummy.data[i]  < 32   ){
			if( async_map & ( 1L << dummy.data[i] ) ){
				p->data[j]=0x7d;
				j++;
				p->data[j]=dummy.data[i]^0x20;
			}else{
				p->data[j]=dummy.data[i];
			}
			continue;
		}

		if( dummy.data[i] == 0x7e || dummy.data[i]== 0x7d ){
			p->data[j]=0x7d;
			j++;
			p->data[j]=dummy.data[i]^0x20;
			continue;
		}

		p->data[j]=dummy.data[i];
	}
	p->packetsize=j;

}

void SendPPP_Packet(struct packet * p){

// bug("SendPPP_Packet size=%d\n",p->packetsize);
	//printpacket(p);

	p->header[0] = 0x7e; // escaped header
	p->header[1] = 0xff;
	p->header[2] = 0x7d;
	p->header[3] = 0x23;

	EscapePacket(p);
	AddByte( p , 0x7e );// end mark

	DoBYTES( ppp_libbase , p->header , p->packetsize + 4 ); // 4 = header size
	// SendBYTES( ppp_libbase , p->header , p->packetsize + 4 ); // 4 = header size
}






