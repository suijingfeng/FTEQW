/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the included (GNU.txt) GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*
This is the file responsible for handling incoming tcp connections.
This includes mvd recording.
Password checks and stuff are implemented here. This is server side stuff.

*/

#include "qtv.h"



#undef IN
#define IN(x) buffer[(x)&(MAX_PROXY_BUFFER-1)]

void CheckMVDConsistancy(unsigned char *buffer, int pos, int size)
{
/*
	int length;
	int msec, type;
	while(pos < size)
	{
		msec = IN(pos++);
		type = IN(pos++);
		if (type == dem_set)
		{
			pos+=8;
			continue;
		}
		if (type == dem_multiple)
			pos+=4;
		length = (IN(pos+0)<<0) + (IN(pos+1)<<8) + (IN(pos+2)<<16) + (IN(pos+3)<<24);
		pos+=4;
		if (length > 1450)
			printf("too big (%i)\n", length);
		pos += length;
	}
	if (pos != size)
		printf("pos != size\n");
	*/
}





void SV_FindProxies(SOCKET sock, cluster_t *cluster, sv_t *defaultqtv)
{
	oproxy_t *prox;

	sock = accept(sock, NULL, NULL);
	if (sock == INVALID_SOCKET)
		return;

	if (cluster->numproxies >= cluster->maxproxies && cluster->maxproxies)
	{
		const char buffer[] = {dem_all, 1, 'P','r','o','x','y',' ','i','s',' ','f','u','l','l','.'};
		send(sock, buffer, strlen(buffer), 0);
		closesocket(sock);
		return;
	}

	prox = malloc(sizeof(*prox));
	if (!prox)
	{//out of mem?
		closesocket(sock);
		return;
	}
	memset(prox, 0, sizeof(*prox));
	prox->sock = sock;
	prox->file = NULL;

	cluster->numproxies++;

#if 1
	prox->defaultstream = defaultqtv;

	prox->next = cluster->pendingproxies;
	cluster->pendingproxies = prox;
#else
	prox->next = qtv->pendingproxies;
	qtv->pendingproxies = prox;
	Net_SendConnectionMVD(qtv, prox);
#endif
}



void Net_TryFlushProxyBuffer(cluster_t *cluster, oproxy_t *prox)
{
	char *buffer;
	int length;
	int bufpos;

	if (prox->drop)
		return;

	while (prox->bufferpos >= MAX_PROXY_BUFFER)
	{	//so we never get any issues with wrapping..
		prox->bufferpos -= MAX_PROXY_BUFFER;
		prox->buffersize -= MAX_PROXY_BUFFER;
	}

	bufpos = prox->bufferpos&(MAX_PROXY_BUFFER-1);
	length = prox->buffersize - prox->bufferpos;
	if (length > MAX_PROXY_BUFFER-bufpos)	//cap the length correctly.
		length = MAX_PROXY_BUFFER-bufpos;
	if (!length)
		return;	//already flushed.
	buffer = prox->buffer + bufpos;

//	CheckMVDConsistancy(prox->buffer, prox->bufferpos, prox->buffersize);

	if (bufpos+length > MAX_PROXY_BUFFER)
		Sys_Printf(cluster, "oversize flush\n");

	if (prox->file)
		length = fwrite(buffer, 1, length, prox->file);
	else
		length = send(prox->sock, buffer, length, 0);


	switch (length)
	{
	case 0:	//eof / they disconnected
		prox->drop = true;
		break;
	case -1:
		if (qerrno != EWOULDBLOCK)	//not a problem, so long as we can flush it later.
			prox->drop = true;	//drop them if we get any errors
		break;
	default:
		prox->bufferpos += length;
	}
}

void Net_ProxySend(cluster_t *cluster, oproxy_t *prox, char *buffer, int length)
{
	int wrap;

	if (prox->buffersize-prox->bufferpos + length > MAX_PROXY_BUFFER)
	{
		Net_TryFlushProxyBuffer(cluster, prox);	//try flushing
		if (prox->buffersize-prox->bufferpos + length > MAX_PROXY_BUFFER)	//damn, still too big.
		{	//they're too slow. hopefully it was just momentary lag
			prox->flushing = true;
			return;
		}
	}
#if 1
	//just simple
	prox->buffersize+=length;
	for (wrap = prox->buffersize-length; wrap < prox->buffersize; wrap++)
		prox->buffer[wrap&(MAX_PROXY_BUFFER-1)] = *buffer++;
#else
	//we don't do multiple wrappings, the above check cannot succeed if it were required.

	//find the wrap point
	wrap = prox->buffersize-(prox->buffersize&(MAX_PROXY_BUFFER-1)) + MAX_PROXY_BUFFER;
	wrap = wrap - (prox->buffersize&(MAX_PROXY_BUFFER-1));	//the ammount of data we can fit before wrapping.

	if (wrap > length)
	{	//we don't wrap afterall
		memcpy(prox->buffer+(prox->buffersize)&(MAX_PROXY_BUFFER-1), buffer, length);
		prox->buffersize+=length;
		return;
	}
	memcpy(prox->buffer+prox->buffersize&(MAX_PROXY_BUFFER-1), buffer, wrap);
	buffer += wrap;
	length -= wrap;
	memcpy(prox->buffer, buffer, length);

	prox->buffersize+=length;
#endif
}

void Prox_SendMessage(cluster_t *cluster, oproxy_t *prox, char *buf, int length, int dem_type, unsigned int playermask)
{
	netmsg_t msg;
	char tbuf[16];
	InitNetMsg(&msg, tbuf, sizeof(tbuf));
	WriteByte(&msg, 0);
	WriteByte(&msg, dem_type);
	WriteLong(&msg, length);
	if (dem_type == dem_multiple)
		WriteLong(&msg, playermask);

	if (prox->buffersize-prox->bufferpos + length + msg.cursize > MAX_PROXY_BUFFER)
	{
		Net_TryFlushProxyBuffer(cluster, prox);	//try flushing
		if (prox->buffersize-prox->bufferpos + length + msg.cursize > MAX_PROXY_BUFFER)	//damn, still too big.
		{	//they're too slow. hopefully it was just momentary lag
			prox->flushing = true;
			return;
		}
	}


	Net_ProxySend(cluster, prox, msg.data, msg.cursize);

	Net_ProxySend(cluster, prox, buf, length);
}

void Prox_SendPlayerStats(sv_t *qtv, oproxy_t *prox)
{
	char buffer[MAX_MSGLEN];
	netmsg_t msg;
	int player, snum;

	InitNetMsg(&msg, buffer, sizeof(buffer));

	for (player = 0; player < MAX_CLIENTS; player++)
	{
		for (snum = 0; snum < MAX_STATS; snum++)
		{
			if (qtv->players[player].stats[snum])
			{
				if ((unsigned)qtv->players[player].stats[snum] > 255)
				{
					WriteByte(&msg, svc_updatestatlong);
					WriteByte(&msg, snum);
					WriteLong(&msg, qtv->players[player].stats[snum]);
				}
				else
				{
					WriteByte(&msg, svc_updatestat);
					WriteByte(&msg, snum);
					WriteByte(&msg, qtv->players[player].stats[snum]);
				}
			}
		}

		if (msg.cursize)
		{
//			Prox_SendMessage(prox, msg.data, msg.cursize, dem_stats|(player<<3), (1<<player));
			msg.cursize = 0;
		}
	}
}

void Net_SendConnectionMVD(sv_t *qtv, oproxy_t *prox)
{
	char buffer[MAX_MSGLEN*8];
	netmsg_t msg;
	int prespawn;

	InitNetMsg(&msg, buffer, sizeof(buffer));

	prox->flushing = false;

	BuildServerData(qtv, &msg, true, 0);
	Prox_SendMessage(qtv->cluster, prox, msg.data, msg.cursize, dem_read, (unsigned)-1);
	msg.cursize = 0;

	for (prespawn = 0;prespawn >= 0;)
	{
		prespawn = SendList(qtv, prespawn, qtv->soundlist, svc_soundlist, &msg);
		Prox_SendMessage(qtv->cluster, prox, msg.data, msg.cursize, dem_read, (unsigned)-1);
		msg.cursize = 0;
	}

	for (prespawn = 0;prespawn >= 0;)
	{
		prespawn = SendList(qtv, prespawn, qtv->modellist, svc_modellist, &msg);
		Prox_SendMessage(qtv->cluster, prox, msg.data, msg.cursize, dem_read, (unsigned)-1);
		msg.cursize = 0;
	}

	Net_TryFlushProxyBuffer(qtv->cluster, prox);	//that should be enough data to fill a packet.

	for(prespawn = 0;prespawn>=0;)
	{
		prespawn = Prespawn(qtv, 0, &msg, prespawn, MAX_CLIENTS-1);

		Prox_SendMessage(qtv->cluster, prox, msg.data, msg.cursize, dem_read, (unsigned)-1);
		msg.cursize = 0;
	}

	//playerstates arn't actually delta-compressed, so the first send (simply forwarded from server) entirly replaces the old.

	//we do need to send entity states.
	Prox_SendInitialEnts(qtv, prox, &msg);
	Prox_SendMessage(qtv->cluster, prox, msg.data, msg.cursize, dem_read, (unsigned)-1);
	msg.cursize = 0;

	WriteByte(&msg, svc_stufftext);
	WriteString(&msg, "skins\n");
	Prox_SendMessage(qtv->cluster, prox, msg.data, msg.cursize, dem_read, (unsigned)-1);
	msg.cursize = 0;

	Net_TryFlushProxyBuffer(qtv->cluster, prox);

	Prox_SendPlayerStats(qtv, prox);
	Net_TryFlushProxyBuffer(qtv->cluster, prox);

	if (!qtv->cluster->lateforward)
		Net_ProxySend(qtv->cluster, prox, qtv->buffer, qtv->buffersize);	//send all the info we've not yet processed.


	if (prox->flushing)
	{
		Sys_Printf(qtv->cluster, "Connection data is too big, dropping proxy client\n");
		prox->drop = true;	//this is unfortunate...
	}
	else
		Net_TryFlushProxyBuffer(qtv->cluster, prox);
}


oproxy_t *Net_FileProxy(sv_t *qtv, char *filename)
{
	oproxy_t *prox;
	FILE *f;

	f = fopen(filename, "wb");
	if (!f)
		return NULL;

	//no full proxy check, this is going to be used by proxy admins, who won't want to have to raise the limit to start recording.

	prox = malloc(sizeof(*prox));
	if (!prox)
		return NULL;
	memset(prox, 0, sizeof(*prox));

	prox->sock = INVALID_SOCKET;
	prox->file = f;

	prox->next = qtv->proxies;
	qtv->proxies = prox;

	qtv->cluster->numproxies++;

	Net_SendConnectionMVD(qtv, prox);

	return prox;
}

qboolean Net_StopFileProxy(sv_t *qtv)
{
	oproxy_t *prox;
	for (prox = qtv->proxies; prox; prox = prox->next)
	{
		if (prox->file)
		{
			prox->drop = true;
			return true;
		}
	}
	return false;
}



void SV_ForwardStream(sv_t *qtv, char *buffer, int length)
{	//forward the stream on to connected clients
	oproxy_t *prox, *next, *fre;

	CheckMVDConsistancy(buffer, 0, length);


	while (qtv->proxies && qtv->proxies->drop)
	{
		next = qtv->proxies->next;
		fre = qtv->proxies;
		if (fre->file)
			fclose(fre->file);
		else
			closesocket(fre->sock);
		free(fre);
		qtv->cluster->numproxies--;
		qtv->proxies = next;
	}

	for (prox = qtv->proxies; prox; prox = prox->next)
	{
		while (prox->next && prox->next->drop)
		{
			next = prox->next->next;
			fre = prox->next;
			if (fre->file)
				fclose(fre->file);
			else
				closesocket(fre->sock);
			free(fre);
			qtv->cluster->numproxies--;
			prox->next = next;
		}

		if (prox->flushing)	//don't send it if we're trying to empty thier buffer.
		{
			if (prox->buffersize == prox->bufferpos)
			{
				if (!qtv->parsingconnectiondata)
					Net_SendConnectionMVD(qtv, prox);	//they're up to date, resend the connection info.
			}
			else
			{
				Net_TryFlushProxyBuffer(qtv->cluster, prox);	//try and flush it.
				continue;
			}
		}

		if (prox->drop)
			continue;

		//add the new data
		Net_ProxySend(qtv->cluster, prox, buffer, length);

		Net_TryFlushProxyBuffer(qtv->cluster, prox);
//		Net_TryFlushProxyBuffer(qtv->cluster, prox);
//		Net_TryFlushProxyBuffer(qtv->cluster, prox);
	}
}

void SV_GenerateNowPlayingHTTP(cluster_t *cluster, oproxy_t *dest)
{
	int player;
	char *s;
	char buffer[1024];
	char plname[64];
	sv_t *streams;

	s = "HTTP/1.1 200 OK\n"
		"Content-Type: text/html\n"
		"Connection: close\n"
		"\n";
	Net_ProxySend(cluster, dest, s, strlen(s));

	sprintf(buffer, "<HEAD>"
						"<TITLE>QuakeTV: Now Playing</TITLE>"
					"</HEAD>"
					"<BODY>");
	Net_ProxySend(cluster, dest, buffer, strlen(buffer));

	for (streams = cluster->servers; streams; streams = streams->next)
	{
		sprintf(buffer, "<A HREF=\"watch.qtv?sid=%i\">%s (%s: %s)</A><br/>", streams->streamid, streams->server, streams->gamedir, streams->mapname);
		Net_ProxySend(cluster, dest, buffer, strlen(buffer));

		for (player = 0; player < MAX_CLIENTS; player++)
		{
			if (*streams->players[player].userinfo)
			{
				Info_ValueForKey(streams->players[player].userinfo, "name", plname, sizeof(plname));
				sprintf(buffer, "&nbsp;%s<br/>", plname);
				Net_ProxySend(cluster, dest, buffer, strlen(buffer));
			}
		}
	}
	sprintf(buffer, "</BODY>");
	Net_ProxySend(cluster, dest, buffer, strlen(buffer));
}

qboolean SV_GetHTTPHeaderField(char *s, char *field, char *buffer, int buffersize)
{
	char *e;
	char *colon;
	int fieldnamelen = strlen(field);

	buffer[0] = 0;

	e = s;
	while(*e)
	{
		if (*e == '\n')
		{
			*e = '\0';
			colon = strchr(s, ':');
			if (!colon)
			{
				if (!strncmp(field, s, fieldnamelen))
				{
					if (s[fieldnamelen] <= ' ')
					{
						return true;
					}
				}
			}
			else
			{
				if (fieldnamelen == colon - s)
				{
					if (!strncmp(field, s, colon-s))
					{
						colon++;
						while (*colon == ' ')
							colon++;
						while (buffersize > 1)
						{
							if (*colon == '\r' || *colon == '\n')
								break;
							*buffer++ = *colon++;
						}
						*buffer = 0;
						return true;
					}
				}

			}
			s = e+1;
		}

		e++;
	}
	return false;
}

void SV_GenerateQTVStub(cluster_t *cluster, oproxy_t *dest, int streamid)
{
	char *s;
	char hostname[64];
	char buffer[1024];

	if (!SV_GetHTTPHeaderField(dest->inbuffer, "Host", hostname, sizeof(hostname)))
	{
		s = "HTTP/1.1 400 OK\n"
			"Content-Type: text/html\n"
			"Connection: close\n"
			"\n"

			"<HTML>"
			"<HEAD>"
				"<TITLE>QuakeTV: Now Playing</TITLE>"
			"</HEAD>"
			"<BODY>"
			"Your client did not send a Host field\n"
			"</BODY>"
			"</HTML>";

		Net_ProxySend(cluster, dest, s, strlen(s));
		return;
	}

	s = "HTTP/1.1 200 OK\n"
		"Content-Type: text/x-quaketvident\n"
		"Connection: close\n"
		"\n";
	Net_ProxySend(cluster, dest, s, strlen(s));


	sprintf(buffer, "[QTV]\r\n"
					"Stream: %i@%s\r\n"
					"", 
					streamid, hostname);


	Net_ProxySend(cluster, dest, buffer, strlen(buffer));
}

//returns true if the pending proxy should be unlinked
//truth does not imply that it should be freed/released, just unlinked.
qboolean SV_ReadPendingProxy(cluster_t *cluster, oproxy_t *pend)
{
	char *s;
	char *e;
	char *colon;
	int usableversion = 0;
	int len;
	qboolean raw;
	sv_t *qtv;

	if (pend->drop)
	{
		closesocket(pend->sock);
		free(pend);
		return true;
	}

	Net_TryFlushProxyBuffer(cluster, pend);

	if (pend->flushing)
	{
		if (pend->bufferpos == pend->buffersize)
			pend->drop = true;
		return false;
	}

	len = sizeof(pend->inbuffer) - pend->inbuffersize - 1;
	len = recv(pend->sock, pend->inbuffer+pend->inbuffersize, len, 0);
	if (len == 0)
	{
		pend->drop = true;
		return false;
	}
	if (len < 0)
		return false;

	pend->inbuffersize += len;
	pend->inbuffer[pend->inbuffersize] = '\0';

	if (pend->inbuffersize >= 4)
	{
		if (strncmp(pend->inbuffer, "QTV\n", 4) && strncmp(pend->inbuffer, "GET ", 4))
		{	//I have no idea what the smeg you are.
			pend->drop = true;
			return false;
		}
	}

	//make sure there's a double \n somewhere

	for (s = pend->inbuffer; *s; s++)
	{
		if (s[0] == '\n' && (s[1] == '\n' || (s[1] == '\r' && s[2] == '\n')))
			break;
	}
	if (!*s)
		return false;	//don't have enough yet

	if (!strncmp(pend->inbuffer, "GET ", 4))
	{
		if (!strncmp(pend->inbuffer+4, "/nowplaying", 11))
		{
			SV_GenerateNowPlayingHTTP(cluster, pend);
		}
		else if (!strncmp(pend->inbuffer+4, "/watch.qtv?sid=", 15))
		{
			SV_GenerateQTVStub(cluster, pend, atoi(pend->inbuffer+19));
		}
		else if (!strncmp(pend->inbuffer+4, "/about", 6))
		{
			s = "HTTP/1.0 302 Found\n"
				"Location: http://www.fteqw.com/\n"
				"\n";
			Net_ProxySend(cluster, pend, s, strlen(s));
		}
		else if (!strncmp(pend->inbuffer+4, "/ ", 2))
		{
			s = "HTTP/1.0 302 Found\n"
				"Location: /nowplaying.html\n"
				"\n";
			Net_ProxySend(cluster, pend, s, strlen(s));
		}
		else if (!strncmp(pend->inbuffer+4, "/demos", 6))
		{
			s = "HTTP/1.0 302 Found\n"
				"Location: http://www.fteqw.com/\n"
				"\n";
			Net_ProxySend(cluster, pend, s, strlen(s));
		}
/*		else
		{
		s = "HTTP/0.9 200 OK\n"
			"Content-Type: text/plain\n"
			"Content-Length: 12\n"
			"\n"
			"Hello World\n";
		Net_ProxySend(cluster, pend, s, strlen(s));
		}*/
		else
		{
			s = "HTTP/1.1 404 OK\n"
				"Content-Type: text/html\n"
				"Connection: close\n"
				"\n"
				"<HEAD><TITLE>QuakeTV</TITLE></HEAD><BODY>The url you have specified was not recognised.</BODY>\n";
			Net_ProxySend(cluster, pend, s, strlen(s));
		}
		pend->flushing = true;
		return false;
	}

	raw = false;

	qtv = pend->defaultstream;

	e = pend->inbuffer;
	s = e;
	while(*e)
	{
		if (*e == '\n')
		{
			*e = '\0';
			colon = strchr(s, ':');
			if (!colon)
			{
				if (!strcmp(s, "SOURCELIST"))
				{	//lists sources that are currently playing
					s = "QTVSV 1\n";
						Net_ProxySend(cluster, pend, s, strlen(s));
					if (!cluster->servers)
					{
						s = "PERROR: No sources currently available\n";
							Net_ProxySend(cluster, pend, s, strlen(s));
					}
					else
					{
						for (qtv = cluster->servers; qtv; qtv = qtv->next)
						{
							s = "ASOURCE: ";
								Net_ProxySend(cluster, pend, s, strlen(s));
							s = qtv->hostname;
							if (!s || !*s)
								s = qtv->server;
								Net_ProxySend(cluster, pend, s, strlen(s));
							s = "\n";
								Net_ProxySend(cluster, pend, s, strlen(s));
						}
						qtv = NULL;
					}
					s = "\n";
						Net_ProxySend(cluster, pend, s, strlen(s));
					pend->flushing = true;
				}
				else if (!strcmp(s, "DEMOLIST"))
				{	//lists the demos available on this proxy

					s = "QTVSV 1\n";
						Net_ProxySend(cluster, pend, s, strlen(s));
					s = "PERROR: DEMOLIST command not yet implemented\n";
						Net_ProxySend(cluster, pend, s, strlen(s));
					s = "\n";
						Net_ProxySend(cluster, pend, s, strlen(s));
					pend->flushing = true;
				}
			}
			else
			{
				*colon++ = '\0';
				if (!strcmp(s, "VERSION"))
				{
					switch(atoi(colon))
					{
					case 1:
						//got a usable version
						usableversion = 1;
						break;
					default:
						//not recognised.
						break;
					}
				}
				else if (!strcmp(s, "RAW"))
					raw = atoi(colon);
				/*else if (!strcmp(s, "ROUTE"))
				{	//pure rewroute...
					//is this safe? probably not.
					s = "QTVSV 1\n"
						"PERROR: ROUTE command not yet implemented\n"
						"\n";
					Net_ProxySend(cluster, pend, s, strlen(s));
					pend->flushing = true;
				}
				*/
				else if (!strcmp(s, "SOURCE"))
				{	//connects, creating a new source
					while (*colon == ' ')
						colon++;
					for (s = colon; *s; s++)
						if (*s < '0' || *s > '9')
							break;
					if (*s)
						qtv = QTV_NewServerConnection(cluster, colon, "", false, true, true);
					else
					{
						//numerical source, use a stream id.
						for (qtv = cluster->servers; qtv; qtv = qtv->next)
							if (qtv->streamid == atoi(colon))
								break;
					}
//					s = "QTVSV 1\n"
//						"PERROR: SOURCE command not yet implemented\n"
//						"\n";
//					Net_ProxySend(cluster, pend, s, strlen(s));
				}
				else if (!strcmp(s, "DEMO"))
				{	//starts a demo off the server... source does the same thing though...
					s = "QTVSV 1\n"
						"PERROR: DEMO command not yet implemented\n"
						"\n";
					Net_ProxySend(cluster, pend, s, strlen(s));
					pend->flushing = true;
				}
			}
			s = e+1;
		}

		e++;
	}

	if (!pend->flushing)
	{
		if (!usableversion)
		{
			s = "QTVSV 1\n"
				"PERROR: Requested protocol version not supported\n"
				"\n";
			Net_ProxySend(cluster, pend, s, strlen(s));
			pend->flushing = true;
		}
		if (!qtv)
		{
			s = "QTVSV 1\n"
				"PERROR: No stream selected\n"
				"\n";
			Net_ProxySend(cluster, pend, s, strlen(s));
			pend->flushing = true;
		}
	}
	if (pend->flushing)
		return false;

	pend->next = qtv->proxies;
	qtv->proxies = pend;

	if (!raw)
	{
		s = "QTVSV 1\n\n";
		Net_ProxySend(cluster, pend, s, strlen(s));
	}
//	else if (passwordprotected)	//raw mode doesn't support passwords, so reject them
//	{
//		pend->flushing = true;
//		return;
//	}


	Net_SendConnectionMVD(qtv, pend);

	return true;
}