#include <tile/net.h>

/*
 * ==========================================================================
 *
 *	NETWORKING
 *
 * ==========================================================================
 */

static struct {
	int handle[65536];
	TlU16 currPort;
	TlU16 numActivePorts;
	struct sockaddr_in sendAddr;
	struct sockaddr_in fromAddr;
} g_netsock_udp;
static TlBool g_net_init = FALSE;

static int g_net_error = 0;

#if _WIN32
# define tlLastNetError() WSAGetLastError()
#else
# define tlLastNetError() errno
#endif
#define tlSetLastNetError() g_net_error = tlLastNetError()

static TlBool tlNet_InitPort(TlU16 port) {
	struct sockaddr_in badr;
#if _WIN32
	DWORD nb;
#endif

	if (g_netsock_udp.handle[port] != -1) {
		return TRUE;
	}

	g_netsock_udp.handle[port] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (g_netsock_udp.handle[port] == -1) {
		tlSetLastNetError();
		return FALSE;
	}

	memset(&badr, 0, sizeof(badr));
	badr.sin_family = AF_INET;
	badr.sin_addr.s_addr = htonl(INADDR_ANY);
	badr.sin_port = htons(port);

	if (bind(g_netsock_udp.handle[port], (struct sockaddr *)&badr,
	sizeof(badr))==-1) {
		tlSetLastNetError();
#if _WIN32
		closesocket(g_netsock_udp.handle[port]);
#else
		close(g_netsock_udp.handle[port]);
#endif
		g_netsock_udp.handle[port] = -1;
		return FALSE;
	}

#if _WIN32
	nb = 1;

	if (ioctlsocket(g_netsock_udp.handle[port], FIONBIO, &nb) == -1) {
		tlSetLastNetError();
		closesocket(g_netsock_udp.handle[port]);
		g_netsock_udp.handle[port] = -1;
		return FALSE;
	}
#else
	if (fcntl(g_netsock_udp.handle[port], F_SETFL, O_NONBLOCK, 1) == -1) {
		tlSetLastNetError();
		close(g_netsock_udp.handle[port]);
		g_netsock_udp.handle[port] = -1;
		return FALSE;
	}
#endif

	g_netsock_udp.numActivePorts++;
	return TRUE;
}
static void tlNet_FiniPort(TlU16 port) {
	if (g_netsock_udp.handle[port] == -1)
		return;

#if _WIN32
	closesocket(g_netsock_udp.handle[port]);
#else
	close(g_netsock_udp.handle[port]);
#endif
	g_netsock_udp.handle[port] = -1;

	g_netsock_udp.numActivePorts--;
}

TlBool tlNet_Init(TlU16 port) {
	TlUInt i;
#if _WIN32
	WSADATA WsaData;

	if (g_net_init)
		return TRUE;

	g_net_error = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (g_net_error != NO_ERROR)
		return FALSE;
#else
	if (g_net_init)
		return TRUE;
#endif

	for(i=0; i<65536; i++)
		g_netsock_udp.handle[i] = -1;

	g_netsock_udp.currPort = 0;
	g_netsock_udp.numActivePorts = 0;

	memset(&g_netsock_udp.sendAddr, 0, sizeof(g_netsock_udp.sendAddr));
	memset(&g_netsock_udp.fromAddr, 0, sizeof(g_netsock_udp.fromAddr));

	g_net_init = TRUE;

	if (!tlNet_SetCurrentPort(port)) {
		tlNet_Fini();
		return FALSE;
	}

	if (!tlNet_SetSendAddressIP(tlNet_MakeIP(127,0,0,1), port)) {
		tlNet_Fini();
		return FALSE;
	}

	return TRUE;
}

void tlNet_Fini() {
	TlUInt i;

	if (!g_net_init)
		return;

	g_net_init = FALSE;

	for(i=0; i<65536; i++)
		tlNet_FiniPort(i);

#if _WIN32
	WSACleanup();
#endif
}

TlBool tlNet_SetCurrentPort(TlU16 port) {
	if (!tlNet_InitPort(port))
		return FALSE;

	g_netsock_udp.currPort = port;
	return TRUE;
}
TlU16 tlNet_GetCurrentPort() {
	return g_netsock_udp.currPort;
}
void tlNet_FreePort(TlU16 port) {
	if (g_netsock_udp.currPort==port)
		g_netsock_udp.currPort = 0;

	tlNet_FiniPort(port);
}
TlU32 tlNet_GetActivePortCount() {
	return (TlU32)g_netsock_udp.numActivePorts;
}

static int tlNet_CurrentSocket() {
	return g_netsock_udp.handle[g_netsock_udp.currPort];
}

static TlBool tlNet_TextToAddr(const char *addr, struct sockaddr_in *sadr) {
	unsigned char ip[4];
	unsigned short port;

	if (addr) {
		int part[4];
		int portI;
		int found;

		found = sscanf(addr, "%i.%i.%i.%i:%i",
			&part[0],&part[1],&part[2],&part[3], &portI);

		if (found < 4) {
			TlU32 ip; TlU16 port;

			if (!tlNet_ResolveHostAddressIP(addr, &ip, &port))
				return FALSE;

			part[0] = (int)tlNet_GetIP_A(ip);
			part[1] = (int)tlNet_GetIP_B(ip);
			part[2] = (int)tlNet_GetIP_C(ip);
			part[3] = (int)tlNet_GetIP_D(ip);

			portI = (int)port;
		} else if (found < 5)
			portI = 0;

		ip[0] = (unsigned char)part[3];
		ip[1] = (unsigned char)part[2];
		ip[2] = (unsigned char)part[1];
		ip[3] = (unsigned char)part[0];

		port = (unsigned short)portI;
	} else {
		ip[0] = 127;
		ip[1] = 0;
		ip[2] = 0;
		ip[3] = 1;

		port = 0;
	}

	sadr->sin_family = AF_INET;
	sadr->sin_port = htons(port);
	sadr->sin_addr.s_addr = htonl(*(unsigned int *)&ip[0]);

	return TRUE;
}
#if !__STDC_WANT_SECURE_LIB__
# define sprintf_s snprintf
#endif
static TlBool tlNet_AddrToText(const struct sockaddr_in *sadr, char *addr,
TlUInt addrLen) {
	unsigned int ip;
	int part[4];
	int port;

	if (!addrLen)
		return FALSE;

	ip = ntohl(sadr->sin_addr.s_addr);
	part[0] = (int)tlNet_GetIP_A(ip);
	part[1] = (int)tlNet_GetIP_B(ip);
	part[2] = (int)tlNet_GetIP_C(ip);
	part[3] = (int)tlNet_GetIP_D(ip);

	port = (int)ntohs(sadr->sin_port);

	if (sadr->sin_port != 0) {
		sprintf_s(addr, addrLen, "%i.%i.%i.%i:%i",
			part[0], part[1], part[2], part[3], port);
	} else {
		sprintf_s(addr, addrLen, "%i.%i.%i.%i",
			part[0], part[1], part[2], part[3]);
	}

	addr[addrLen - 1] = '\0';

	return TRUE;
}

const char *tlNet_ResolveHostAddress(const char *domain) {
	static char text[32];
	TlU32 ip;
	TlU16 port;

	if (!tlNet_ResolveHostAddressIP(domain, &ip, &port))
		return (const char *)0;

	sprintf_s(text, sizeof(text), "%i.%i.%i.%i:%i", (int)tlNet_GetIP_A(ip),
		(int)tlNet_GetIP_B(ip), (int)tlNet_GetIP_C(ip), (int)tlNet_GetIP_D(ip),
		(int)port);

	return text;
}
static int tlNet_ProtoNameToProto(const char *name, int len) {
	if (!strncmp("any", name,len))
		return 0;

	if (!strncmp("udp", name,len))
		return IPPROTO_UDP;

	if (!strncmp("tcp", name,len))
		return IPPROTO_TCP;

#if 0
	if (!strncmp("rm", name,len))
		return IPPROTO_RM;
#endif

	return -1;
}
static const char *tlNet_ProtoTerm(const char *protos) {
	const char *p;

	if (*protos=='\0')
		return (const char *)0;

	p = strchr(protos, '/');
	if (!p)
		return strchr(protos, '\0');

	return p;
}
static TlBool tlNet_CompareProto(int cmp, const char *protos) {
	const char *p;
	int proto;

	if (!cmp)
		return TRUE;

	if (!protos)
		return cmp==IPPROTO_UDP ? TRUE : FALSE;

	while((p = tlNet_ProtoTerm(protos)) != (const char *)0) {
		proto = tlNet_ProtoNameToProto(protos, p - protos);
		protos = *p ? p + 1 : p;

		if (proto == -1)
			continue;

		if (proto == 0)
			return TRUE;

		if (cmp == proto)
			return TRUE;
	}

	return FALSE;
}
TlBool tlNet_ResolveHostAddressIP(const char *domain, TlU32 *ip, TlU16 *port) {
	struct addrinfo *result;
	struct addrinfo *ptr;
	const char *p, *protos;
	char service[256];
	char node[256];
	int r;

	service[0] = '\0';
	protos = (const char *)0;

	p = strchr(domain, ':');
	if (p) {
		if (p[1]=='/' && p[2]=='/') {
			protos = strchr(p + 3, '/');

			sprintf_s(service, sizeof(service), "%.*s", (int)(ptrdiff_t)(p - domain), domain);
			
			if (protos)
				sprintf_s(node, sizeof(node), "%.*s", (int)(ptrdiff_t)(protos - (p + 3)), p + 3);
			else
				sprintf_s(node, sizeof(node), "%s", p + 3);
		} else if(p[1]>='0' && p[1]<='9') {
			int number;

			sscanf(&p[1], "%i", &number);

			sprintf_s(service, sizeof(service), "%i", number);
			sprintf_s(node, sizeof(node), "%.*s", (int)(ptrdiff_t)(p - domain), domain);

			protos = strchr(p, '/');
		} else {
			g_net_error = EINVAL;
			return FALSE;
		}
	} else {
		sprintf_s(node, sizeof(node), "%s", domain);
		protos = strchr(domain, '/');
	}

	if (service[0] == '\0')
		sprintf_s(service, sizeof(service), "%i", (int)tlNet_GetCurrentPort());

	r = getaddrinfo(node, service, NULL, &result);
	if (r != 0) {
		g_net_error = r;
		return FALSE;
	}

	for(ptr=result; ptr; ptr=ptr->ai_next) {
		struct sockaddr_in *sadr;

		if (ptr->ai_family != AF_INET)
			continue;

		if (!tlNet_CompareProto(ptr->ai_protocol, protos))
			continue;

		if (ptr->ai_addrlen < sizeof(*sadr))
			continue;

		sadr = (struct sockaddr_in *)ptr->ai_addr;

		if (ip) *ip = ntohl(sadr->sin_addr.s_addr);
		if (port) *port = ntohs(sadr->sin_port);

		break;
	}


	freeaddrinfo(result);
	if (!ptr) {
#if 1
		g_net_error = TL_NET_ERR_EPROTOTYPE;
#endif
		return FALSE;
	}

	return TRUE;
}

TlBool tlNet_SetSendAddress(const char *addr) {
	if (!tlNet_TextToAddr(addr, &g_netsock_udp.sendAddr))
		return FALSE;

	return TRUE;
}
TlBool tlNet_SetSendAddressIP(TlU32 ip, TlU16 port) {
	g_netsock_udp.sendAddr.sin_family = AF_INET;
	g_netsock_udp.sendAddr.sin_port = htons(port);
	g_netsock_udp.sendAddr.sin_addr.s_addr = htonl(ip);

	return TRUE;
}
const char *tlNet_GetSendAddress() {
	static char text[32];

	if (!tlNet_AddrToText(&g_netsock_udp.sendAddr, text, sizeof(text)))
		return (const char *)0;

	return text;
}
void tlNet_GetSendAddressIP(TlU32 *ip, TlU16 *port) {
	if (ip) *ip = ntohl(g_netsock_udp.sendAddr.sin_addr.s_addr);
	if (port) *port = ntohs(g_netsock_udp.sendAddr.sin_port);
}

const char *tlNet_GetFromAddress() {
	static char text[32];

	if (!tlNet_AddrToText(&g_netsock_udp.fromAddr, text, sizeof(text)))
		return (const char *)0;

	return text;
}
void tlNet_GetFromAddressIP(TlU32 *ip, TlU16 *port) {
	if (ip) *ip = ntohl(g_netsock_udp.fromAddr.sin_addr.s_addr);
	if (port) *port = ntohs(g_netsock_udp.fromAddr.sin_port);
}

TlUInt tlNet_SendPacket(const void *data, TlUInt dataLen) {
	TlUInt tries;
	TlUInt sent;

	tries = dataLen/8 + 3;
	sent = 0;
	do {
		int r;

		r = sendto(tlNet_CurrentSocket(), &((const char *)data)[sent],
			dataLen - sent, 0, (struct sockaddr *)&g_netsock_udp.sendAddr,
			sizeof(g_netsock_udp.sendAddr));
		if (r < 0) {
			tlSetLastNetError();
			return 0;
		}

		sent += (TlUInt)r;
		tries--;
	} while(sent < dataLen && tries > 0);

	return sent;
}
TlUInt tlNet_RecvPacket(void *data, TlUInt dataLen) {
	socklen_t len;
	int r;

	len = sizeof(g_netsock_udp.fromAddr);
	r = recvfrom(tlNet_CurrentSocket(), (char *)data, dataLen, 0,
		(struct sockaddr *)&g_netsock_udp.fromAddr, &len);

	if (r <= 0) {
		tlSetLastNetError();
		return 0;
	}

	return (TlUInt)r;
}

/*
 * XXX: Seems that some installs don't have certain defines.
 */
#if _WIN32
# ifndef WSABASEERR
#  define WSABASEERR 10000
# endif
# ifndef WSA_QOS_EUNKOWNPSOBJ
#  define WSA_QOS_EUNKOWNPSOBJ (WSABASEERR + 1024)
# endif
/*#define WSA_QOS_EPOLICYOBJ	(WSABASEERR + 1025)
#define WSA_QOS_EFLOWDESC	(WSABASEERR + 1026)
#define WSA_QOS_EPSFLOWSPEC	(WSABASEERR + 1027)
#define WSA_QOS_EPSFILTERSPEC	(WSABASEERR + 1028)
#define WSA_QOS_ESDMODEOBJ	(WSABASEERR + 1029)
#define WSA_QOS_ESHAPERATEOBJ	(WSABASEERR + 1030)
#define WSA_QOS_RESERVED_PETYPE	(WSABASEERR + 1031)*/
#endif

#define NET_ERROR_LIST()\
	WSAERR(WSA_INVALID_HANDLE,INVALID_HANDLE,"Specified event object handle is invalid")WSADEL\
	WSAERR(WSA_NOT_ENOUGH_MEMORY,NOT_ENOUGH_MEMORY,"Insufficient memory available")WSADEL\
	WSAERR(WSA_INVALID_PARAMETER,INVALID_PARAMETER,"One or more parameters are invalid")WSADEL\
	WSAERR(WSA_OPERATION_ABORTED,OPERATION_ABORTED,"Overlapped operation aborted")WSADEL\
	WSAERR(WSA_IO_INCOMPLETE,IO_INCOMPLETE,"Overlapped I/O event object not in signaled state")WSADEL\
	WSAERR(WSA_IO_PENDING,IO_PENDING,"Overlapped operations will complete later")WSADEL\
	WSAERR(WSASYSNOTREADY,SYS_NOT_READY,"Network subsystem is unavailable")WSADEL\
	WSAERR(WSAVERNOTSUPPORTED,VER_NOT_SUPPORTED,"Winsock.dll version out of range")WSADEL\
	WSAERR(WSANOTINITIALISED,NOT_INITIALIZED,"Successful WSAStartup not yet performed")WSADEL\
	WSAERR(WSAHOST_NOT_FOUND,HOST_NOT_FOUND,"Host not found")WSADEL\
	WSAERR(WSATRY_AGAIN,TRY_AGAIN,"Nonauthoritative host not found")WSADEL\
	WSAERR(WSANO_RECOVERY,NO_RECOVERY,"This is a nonrecoverable error")WSADEL\
	WSAERR(WSANO_DATA,NO_DATA,"Valid name, no data record of requested type")WSADEL\
	WSAERR(WSASYSCALLFAILURE,SYSCALL_FAILURE,"System call failure")WSADEL\
	WSAERR(WSASERVICE_NOT_FOUND,SERVICE_NOT_FOUND,"Service not found")WSADEL\
	WSAERR(WSATYPE_NOT_FOUND,TYPE_NOT_FOUND,"Class type not found")WSADEL\
	WSAERR(WSA_E_NO_MORE,NO_MORE,"No more results")WSADEL\
	WSAERR(WSA_E_CANCELLED,CANCELLED,"Call was cancelled")WSADEL\
	WSAERR(WSA_QOS_RECEIVERS,QOS_RECEIVERS,"At least one QoS reserve has arrived")WSADEL\
	WSAERR(WSA_QOS_SENDERS,QOS_SENDERS,"At least one QoS send path has arrived")WSADEL\
	WSAERR(WSA_QOS_NO_SENDERS,QOS_NO_SENDERS,"No QoS senders")WSADEL\
	WSAERR(WSA_QOS_NO_RECEIVERS,QOS_NO_RECEIVERS,"No QoS receivers")WSADEL\
	WSAERR(WSA_QOS_REQUEST_CONFIRMED,QOS_REQUEST_CONFIRMED,"QoS request confirmed")WSADEL\
	WSAERR(WSA_QOS_ADMISSION_FAILURE,QOS_ADMISSION_FAILURE,"QoS admission error")WSADEL\
	WSAERR(WSA_QOS_POLICY_FAILURE,QOS_POLICY_FAILURE,"QoS policy failure")WSADEL\
	WSAERR(WSA_QOS_BAD_STYLE,QOS_BAD_STYLE,"QoS bad style")WSADEL\
	WSAERR(WSA_QOS_BAD_OBJECT,QOS_BAD_OBJECT,"QoS bad object")WSADEL\
	WSAERR(WSA_QOS_TRAFFIC_CTRL_ERROR,QOS_TRAFFIC_CTRL_ERROR,"QoS traffic control error")WSADEL\
	WSAERR(WSA_QOS_GENERIC_ERROR,QOS_GENERIC_ERROR,"QoS generic error")WSADEL\
	WSAERR(WSA_QOS_ESERVICETYPE,QOS_SERVICE_TYPE,"QoS service type error")WSADEL\
	WSAERR(WSA_QOS_EFLOWSPEC,QOS_FLOWSPEC,"QoS flowspec error")WSADEL\
	WSAERR(WSA_QOS_EPROVSPECBUF,QOS_PROV_BUF,"Invalid QoS provider buffer")WSADEL\
	WSAERR(WSA_QOS_EFILTERSTYLE,QOS_FILTER_STYLE,"Invalid QoS filter style")WSADEL\
	WSAERR(WSA_QOS_EFILTERTYPE,QOS_FILTER_TYPE,"Invalid QoS filter type")WSADEL\
	WSAERR(WSA_QOS_EFILTERCOUNT,QOS_FILTER_COUNT,"Invalid QoS filter count")WSADEL\
	WSAERR(WSA_QOS_EOBJLENGTH,QOS_OBJ_LENGTH,"Invalid QoS object length")WSADEL\
	WSAERR(WSA_QOS_EFLOWCOUNT,QOS_FLOW_COUNT,"Incorrect QoS flow count")WSADEL\
	WSAERR(WSA_QOS_EUNKOWNPSOBJ,QOS_UNKOWN_PS_OBJ,"Unrecognized QoS object")WSADEL\
	WSAERR(WSA_QOS_EPOLICYOBJ,QOS_POLICY_OBJ,"Invalid QoS policy object")WSADEL\
	WSAERR(WSA_QOS_EFLOWDESC,QOS_FLOWDESC,"Invalid QoS flow descriptor")WSADEL\
	WSAERR(WSA_QOS_EPSFLOWSPEC,QOS_PS_FLOWSPEC,"Invalid QoS provider-specific flowspec")WSADEL\
	WSAERR(WSA_QOS_EPSFILTERSPEC,QOS_PS_FILTERSPEC,"Invalid QoS provider-specific filterspec")WSADEL\
	WSAERR(WSA_QOS_ESDMODEOBJ,QOS_SD_MODE_OBJ,"Invalid QoS shape discard mode object")WSADEL\
	WSAERR(WSA_QOS_ESHAPERATEOBJ,QOS_SHAPERATE_OBJ,"Invalid QoS shaping rate object")WSADEL\
	WSAERR(WSA_QOS_RESERVED_PETYPE,QOS_RESERVED_PETYPE,"Reserved policy QoS element type")WSADEL\
	SERR(EINPROGRESS,"Operation now in progress")SDEL\
	SERR(EALREADY,"Operation already in progress")SDEL\
	SERR(ENOTSOCK,"Socket operation on nonsocket")SDEL\
	SERR(EDESTADDRREQ,"Destination address required")SDEL\
	SERR(EMSGSIZE,"Message too long")SDEL\
	SERR(EPROTOTYPE,"Protocol wrong type for socket")SDEL\
	SERR(ENOPROTOOPT,"Bad protocol option")SDEL\
	SERR(EPROTONOSUPPORT,"Protocol not supported")SDEL\
	SERR(ESOCKTNOSUPPORT,"Socket type not supported")SDEL\
	SERR(EOPNOTSUPP,"Operation not supported")SDEL\
	SERR(EPFNOSUPPORT,"Protocol family not supported")SDEL\
	SERR(EAFNOSUPPORT,"Address family not supported by protocol family")SDEL\
	SERR(EADDRINUSE,"Address already in use")SDEL\
	SERR(EADDRNOTAVAIL,"Cannot assign requested address")SDEL\
	SERR(ENETDOWN,"Network is down")SDEL\
	SERR(ENETUNREACH,"Network is unreachable")SDEL\
	SERR(ENETRESET,"Network dropped connection on reset")SDEL\
	SERR(ECONNABORTED,"Software caused connection abort")SDEL\
	SERR(ECONNRESET,"Connection reset by peer")SDEL\
	SERR(ENOBUFS,"No buffer space available")SDEL\
	SERR(EISCONN,"Socket is already connected")SDEL\
	SERR(ENOTCONN,"Socket is not connected")SDEL\
	SERR(ESHUTDOWN,"Cannot send after socket shutdown")SDEL\
	SERR(ETOOMANYREFS,"Too many references")SDEL\
	SERR(ETIMEDOUT,"Connection timed out")SDEL\
	SERR(ECONNREFUSED,"Connection refused")SDEL\
	SERR(ELOOP,"Cannot translate name")SDEL\
	SERR(ENAMETOOLONG,"Name too long")SDEL\
	SERR(EHOSTDOWN,"Host is down")SDEL\
	SERR(EHOSTUNREACH,"No route to host")SDEL\
	SERR(ENOTEMPTY,"Directory not empty")SDEL\
	SERR(EPROCLIM,"Too many processes")SDEL\
	SERR(EUSERS,"User quota exceeded")SDEL\
	SERR(EDQUOT,"Disk quota exceeded")SDEL\
	SERR(ESTALE,"Stale file handle reference")SDEL\
	SERR(EREMOTE,"Item is remote")SDEL\
	SERR(EDISCON,"Graceful shutdown in progress")SDEL\
	SERR(ENOMORE,"No more results")SDEL\
	SERR(ECANCELLED,"Call has been cancelled")SDEL\
	SERR(EINVALIDPROCTABLE,"Procedure call table is invalid")SDEL\
	SERR(EINVALIDPROVIDER,"Service provider is invalid")SDEL\
	SERR(EPROVIDERFAILEDINIT,"Service provider failed to initialize")SDEL\
	SERR(EREFUSED,"Database query was refused")SDEL\
	SERR(EWOULDBLOCK,"Resource temporarily unavailable")SDEL\
	NERR(EINTR)NDEL\
	NERR(EBADF)NDEL\
	NERR(EACCES)NDEL\
	NERR(EFAULT)NDEL\
	NERR(EINVAL)NDEL\
	NERR(EMFILE)


int tlNet_LastError() {
#if _WIN32
# define WSADEL ;
# define SDEL ;
# define NDEL ;
# define WSAERR(oc,nc,st) case oc: return TL_NET_ERR_##nc
# define SERR(x,st) case WSA##x: return TL_NET_ERR_##x
# define NERR(x) case WSA##x: return x
	switch(g_net_error) {
	NET_ERROR_LIST();
	default:
		break;
	}
# undef NERR
# undef SERR
# undef WSAERR
# undef NDEL
# undef SDEL
# undef WSADEL
#endif

	return g_net_error;
}
const char *tlNet_ErrorText(int code) {
#if __STDC_WANT_SECURE_LIB__
	static char errbuf[256];
#endif

#define WSADEL ;
#define SDEL ;
#define NDEL
#define WSAERR(oc,nc,st) case TL_NET_ERR_##nc: return st
#define SERR(x,st) case TL_NET_ERR_##x: return st
#define NERR(x)
	switch(code) {
	NET_ERROR_LIST();
	default:
		break;
	}
#undef NERR
#undef WSAERR
#undef NDEL
#undef WSADEL

#if __STDC_WANT_SECURE_LIB__
	errbuf[0] = '\0';
	strerror_s(errbuf, sizeof(errbuf), code);
	errbuf[sizeof(errbuf) - 1] = '\0';

	return errbuf;
#else
	return strerror(code);
#endif
}

void tlNet_ClearError() {
	g_net_error = 0;
}

const char *tlNet_LastErrorText() {
	return tlNet_ErrorText(tlNet_LastError());
}

/*

	USEFUL DOCUMENTATION

	http://gafferongames.com/networking-for-game-programmers/sending-and-receiving-packets/

	socket: http://www.kernel.org/doc/man-pages/online/pages/man2/socket.2.html
	sendto: http://msdn.microsoft.com/en-us/library/windows/desktop/ms740148(v=vs.85).aspx
	recvfrom: http://msdn.microsoft.com/en-us/library/windows/desktop/ms740120(v=vs.85).aspx
	sockaddr: http://msdn.microsoft.com/en-us/library/windows/desktop/ms740496(v=vs.85).aspx
	in_addr: http://msdn.microsoft.com/en-us/library/windows/desktop/ms738571(v=vs.85).aspx
	addrinfo: http://msdn.microsoft.com/en-us/library/windows/desktop/ms737530(v=vs.85).aspx

*/
