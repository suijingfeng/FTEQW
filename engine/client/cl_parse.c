/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cl_parse.c  -- parse a message received from the server

#include "quakedef.h"
#include "cl_ignore.h"

void CL_GetNumberedEntityInfo (int num, float *org, float *ang);
void CLDP_ParseDarkPlaces5Entities(void);
void CL_SetStatInt (int pnum, int stat, int value);
static qboolean CL_CheckModelResources (char *name);

char cl_dp_csqc_progsname[128];
int cl_dp_csqc_progssize;
int cl_dp_csqc_progscrc;
int cl_dp_serverextension_download;


char *svc_qwstrings[] =
{
	"svc_bad",
	"svc_nop",
	"svc_disconnect",
	"svcqw_updatestatbyte",
	"svc_version",		// [long] server version
	"svc_setview",		// [short] entity number
	"svc_sound",			// <see code>
	"svc_time",			// [float] server time
	"svc_print",			// [string] null terminated string
	"svc_stufftext",		// [string] stuffed into client's console buffer
						// the string should be \n terminated
	"svc_setangle",		// [vec3] set the view angle to this absolute value

	"svc_serverdata",		// [long] version ...
	"svc_lightstyle",		// [qbyte] [string]
	"svc_updatename",		// [qbyte] [string]
	"svc_updatefrags",	// [qbyte] [short]
	"svc_clientdata",		// <shortbits + data>
	"svc_stopsound",		// <see code>
	"svc_updatecolors",	// [qbyte] [qbyte]
	"svc_particle",		// [vec3] <variable>
	"svc_damage",			// [qbyte] impact [qbyte] blood [vec3] from

	"svc_spawnstatic",
	"svc_spawnstatic2",
	"svc_spawnbaseline",

	"svc_temp_entity",		// <variable>
	"svc_setpause",
	"svc_signonnum",
	"svc_centerprint",
	"svc_killedmonster",
	"svc_foundsecret",
	"svc_spawnstaticsound",
	"svc_intermission",
	"svc_finale",

	"svc_cdtrack",
	"svc_sellscreen",

	"svc_smallkick",
	"svc_bigkick",

	"svc_updateping",
	"svc_updateentertime",

	"svc_updatestatlong",
	"svc_muzzleflash",
	"svc_updateuserinfo",
	"svc_download",
	"svc_playerinfo",
	"svc_nails",
	"svc_choke",
	"svc_modellist",
	"svc_soundlist",
	"svc_packetentities",
 	"svc_deltapacketentities",
	"svc_maxspeed",
	"svc_entgravity",

	"svc_setinfo",
	"svc_serverinfo",
	"svc_updatepl",
	"MVD svc_nails2",
	"svcfte_soundextended",
	"svcfte_soundlistshort",
	"FTE svc_lightstylecol",
	"FTE svc_bulletentext", // obsolete
	"FTE svc_lightnings",
	"FTE svc_modellistshort",
	"FTE svc_ftesetclientpersist",
	"FTE svc_setportalstate",
	"FTE svc_particle2",
	"FTE svc_particle3",
	"FTE svc_particle4",
	"FTE svc_spawnbaseline2",
	"FTE svc_customtempent",
	"FTE svc_choosesplitclient",

	"svcfte_showpic",
	"svcfte_hidepic",
	"svcfte_movepic",
	"svcfte_updatepic",

	"???",

	"svcfte_effect",
	"svcfte_effect2",

	"svcfte_csqcentities",

	"svcfte_precache",

	"svcfte_updatestatstring",
	"svcfte_updatestatfloat",

	"svcfte_trailparticles",
	"svcfte_pointparticles",
	"svcfte_pointparticles1",

	"svcfte_cgamepacket",
	"svcfte_voicechat",
	"svcfte_setangledelta",
	"svcfte_updateentities",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
	"???",
};

char *svc_nqstrings[] =
{
	"nqsvc_bad",
	"nqsvc_nop",
	"nqsvc_disconnect",
	"nqsvc_updatestatlong",
	"nqsvc_version",		// [long] server version
	"nqsvc_setview",		// [short] entity number
	"nqsvc_sound",			// <see code>
	"nqsvc_time",			// [float] server time
	"nqsvc_print",			// [string] null terminated string
	"nqsvc_stufftext",		// [string] stuffed into client's console buffer
						// the string should be \n terminated
	"nqsvc_setangle",		// [vec3] set the view angle to this absolute value

	"nqsvc_serverinfo",		// [long] version
						// [string] signon string
						// [string]..[0]model cache [string]...[0]sounds cache
						// [string]..[0]item cache
	"nqsvc_lightstyle",		// [qbyte] [string]
	"nqsvc_updatename",		// [qbyte] [string]
	"nqsvc_updatefrags",	// [qbyte] [short]
	"nqsvc_clientdata",		// <shortbits + data>
	"nqsvc_stopsound",		// <see code>
	"nqsvc_updatecolors",	// [qbyte] [qbyte]
	"nqsvc_particle",		// [vec3] <variable>
	"nqsvc_damage",			// [qbyte] impact [qbyte] blood [vec3] from

	"nqsvc_spawnstatic",
	"nqsvcfte_spawnstatic2(21)",
	"nqsvc_spawnbaseline",

	"nqsvc_temp_entity",		// <variable>
	"nqsvc_setpause",
	"nqsvc_signonnum",
	"nqsvc_centerprint",
	"nqsvc_killedmonster",
	"nqsvc_foundsecret",
	"nqsvc_spawnstaticsound",
	"nqsvc_intermission",
	"nqsvc_finale",			// [string] music [string] text
	"nqsvc_cdtrack",			// [qbyte] track [qbyte] looptrack
	"nqsvc_sellscreen",
	"nqsvc_cutscene",	//34

	"NEW PROTOCOL",	//35
	"NEW PROTOCOL",	//36
	"fitzsvc_skybox",	//37
	"NEW PROTOCOL",	//38
	"NEW PROTOCOL",	//39
	"fitzsvc_bf",		//40
	"fitzsvc_fog",	//41
	"fitzsvc_spawnbaseline2",	//42
	"fitzsvc_spawnstatic2",	//43
	"fitzsvc_spawnstaticsound2",	//44
	"NEW PROTOCOL",	//45
	"NEW PROTOCOL",	//46
	"NEW PROTOCOL",	//47
	"NEW PROTOCOL",	//48
	"NEW PROTOCOL",	//49
	"dpsvc_downloaddata",		//50
	"dpsvc_updatestatubyte",	//51
	"dpsvc_effect",				//52
	"dpsvc_effect2",			//53
	"dp6svc_precache/dp5svc_sound2",	//54
	"dpsvc_spawnbaseline2",		//55
	"dpsvc_spawnstatic2",	//56 obsolete
	"dpsvc_entities",		//57
	"dpsvc_csqcentities",			//58
	"dpsvc_spawnstaticsound2",	//59
	"dpsvc_trailparticles",	//60
	"dpsvc_pointparticles",	//61
	"dpsvc_pointparticles1",	//62
	"NEW PROTOCOL(63)",	//63
	"NEW PROTOCOL(64)",	//64
	"NEW PROTOCOL(65)",	//65
	"ftenqsvc_spawnbaseline2",	//66
	"NEW PROTOCOL(67)",	//67
	"NEW PROTOCOL(68)",	//68
	"NEW PROTOCOL(69)",	//69
	"NEW PROTOCOL(70)",	//70
	"NEW PROTOCOL(71)",	//71
	"NEW PROTOCOL(72)",	//72
	"NEW PROTOCOL(73)",	//73
	"NEW PROTOCOL(74)",	//74
	"NEW PROTOCOL(75)",	//75
	"NEW PROTOCOL(76)",	//76
	"NEW PROTOCOL(77)",	//77
	"NEW PROTOCOL(78)",	//78
	"NEW PROTOCOL(79)",	//79
	"NEW PROTOCOL(80)",	//80
	"NEW PROTOCOL(81)",	//81
	"NEW PROTOCOL(82)",	//82
	"nqsvcfte_cgamepacket(83)",	//83
	"NEW PROTOCOL(84)",	//84
	"NEW PROTOCOL(85)",	//85
	"nqsvcfte_updateentities",	//86
	"NEW PROTOCOL(87)",	//87
	"NEW PROTOCOL(88)"	//88
};

extern cvar_t requiredownloads, cl_standardchat, msg_filter, msg_filter_frags, cl_countpendingpl, cl_download_mapsrc;
int	oldparsecountmod;
int	parsecountmod;
double	parsecounttime;

int		cl_spikeindex, cl_playerindex, cl_h_playerindex, cl_flagindex, cl_rocketindex, cl_grenadeindex, cl_gib1index, cl_gib2index, cl_gib3index;

//called after disconnect, purges all memory that was allocated etc
void CL_Parse_Disconnected(void)
{
	if (cls.download)
	{
		//note: not all downloads abort when the server disconnects, as they're fully out of bounds (ie: http)
		if (cls.download->method <= DL_QWPENDING)
			DL_Abort(cls.download, QDL_DISCONNECT);
	}

	{
		downloadlist_t *next;
		while(cl.downloadlist)
		{
			next = cl.downloadlist->next;
			Z_Free(cl.downloadlist);
			cl.downloadlist = next;
		}
		while(cl.faileddownloads)
		{
			next = cl.faileddownloads->next;
			Z_Free(cl.faileddownloads);
			cl.faileddownloads = next;
		}
	}

	CL_ClearParseState();
}

//=============================================================================

int packet_latency[NET_TIMINGS];

int CL_CalcNet (float scale)
{
	int		i;
	outframe_t	*frame;
	int lost = 0;
	int percent;
	int sent;
//	char st[80];

	sent = NET_TIMINGS;

	for (i=cl.movesequence-UPDATE_BACKUP+1
		; i <= cl.movesequence
		; i++)
	{
		frame = &cl.outframes[i&UPDATE_MASK];
		if (i > cl.ackedmovesequence)
		{
			// no response yet
			if (cl_countpendingpl.ival)
			{
				packet_latency[i&NET_TIMINGSMASK] = 9999;
				lost++;
			}
			else
				packet_latency[i&NET_TIMINGSMASK] = 10000;
		}
		else if (frame->latency == -1)
		{
			packet_latency[i&NET_TIMINGSMASK] = 9999;	// dropped
			lost++;
		}
		else if (frame->latency == -2)
			packet_latency[i&NET_TIMINGSMASK] = 10000;	// choked
		else if (frame->latency == -3)
		{
			packet_latency[i&NET_TIMINGSMASK] = 9997;	// c2spps
			sent--;
		}
//		else if (frame->invalid)
//			packet_latency[i&NET_TIMINGSMASK] = 9998;	// invalid delta
		else
			packet_latency[i&NET_TIMINGSMASK] = frame->latency * 60 * scale;
	}

	if (sent < 1)
		percent = 100;	//shouldn't ever happen.
	else
		percent = lost * 100 / sent;

	return percent;
}

void CL_CalcNet2 (float *pings, float *pings_min, float *pings_max, float *pingms_stddev, float *pingfr, int *pingfr_min, int *pingfr_max, float *dropped, float *choked, float *invalid)
{
	int		i;
	outframe_t	*frame;
	int lost = 0;
	int pending = 0;
	int sent;
	int valid = 0;
	int fr;
	int nchoked = 0;
	int ninvalid = 0;
//	char st[80];

	*pings = 0;
	*pings_max = 0;
	*pings_min = 1000000000000;
	*pingfr = 0;
	*pingfr_max = 0;
	*pingfr_min = 0x7fffffff;
	*pingms_stddev = 0;


	sent = NET_TIMINGS;

	for (i=cl.movesequence-UPDATE_BACKUP+1
		; i <= cl.movesequence
		; i++)
	{
		frame = &cl.outframes[i&UPDATE_MASK];
		if (i > cl.lastackedmovesequence)
		{	// no response yet
			if (cl_countpendingpl.ival)
				lost++;
		}
		else if (frame->latency == -1)
			lost++;										// lost
		else if (frame->latency == -2)
			nchoked++;									// choked
		else if (frame->latency == -3)
			sent--;										// c2spps
		else if (frame->latency == -4)
			ninvalid++;									//corrupt/wrong/dodgy/egads
		else
		{
			*pings += frame->latency;
			if (*pings_max < frame->latency)
				*pings_max = frame->latency;
			if (*pings_min > frame->latency)
				*pings_min = frame->latency;

			fr = frame->cmd_sequence-frame->server_message_num;
			*pingfr += fr;
			if (*pingfr_max < fr)
				*pingfr_max = fr;
			if (*pingfr_min > fr)
				*pingfr_min = fr;
			valid++;
		}
	}

	if (valid)
	{
		*pings /= valid;
		*pingfr /= valid;

		//determine stddev, in milliseconds instead of seconds.
		for (i=cl.movesequence-UPDATE_BACKUP+1; i <= cl.movesequence; i++)
		{
			frame = &cl.outframes[i&UPDATE_MASK];
			if (i <= cl.lastackedmovesequence && frame->latency >= 0)
			{
				float dev = (frame->latency - *pings) * 1000;
				*pingms_stddev += dev*dev;
			}
		}
		*pingms_stddev = sqrt(*pingms_stddev/valid);
	}

	if (pending == sent || sent < 1)
		*dropped = 1;	//shouldn't ever happen.
	else
		*dropped = (float)lost / sent;
	*choked = (float)nchoked / sent;
	*invalid = (float)ninvalid / sent;
}

void CL_AckedInputFrame(int inseq, int outseq, qboolean worldstateokay)
{
	unsigned int i;
	unsigned int newmod;
	outframe_t *frame;

	newmod = outseq & UPDATE_MASK;

	//calc the latency for this frame, but only if its not a dupe ack. we want the youngest, not the oldest, so we can calculate network latency rather than simply packet frequency
	if (outseq != cl.lastackedmovesequence)
	{
		frame = &cl.outframes[newmod];
	// calculate latency
		frame->latency = realtime - frame->senttime;
		if (frame->latency < 0 || frame->latency > 1.0)
		{
	//		Con_Printf ("Odd latency: %5.2f\n", latency);
		}
		else
		{
		// drift the average latency towards the observed latency
			if (frame->latency < cls.latency)
				cls.latency = frame->latency;
			else
				cls.latency += 0.001;	// drift up, so correction are needed
		}

		if (cl.inframes[inseq&UPDATE_MASK].invalid)
			frame->latency = -4;

		//and mark any missing ones as dropped
		for (i = (cl.lastackedmovesequence+1) & UPDATE_MASK; i != newmod; i=(i+1)&UPDATE_MASK)
		{
//nq has no concept of choking. outbound packets that are accepted during a single frame will be erroneoulsy considered dropped. nq never had a netgraph based upon outgoing timings.
//			Con_Printf("Dropped moveframe %i\n", i);
			cl.outframes[i].latency = -1;
		}
	}
	cl.inframes[inseq&UPDATE_MASK].ackframe = outseq;
	if (worldstateokay)
		cl.ackedmovesequence = outseq;
	cl.lastackedmovesequence = outseq;
}

//=============================================================================

int CL_IsDownloading(const char *localname)
{
	downloadlist_t *dl;
	/*check for dupes*/
	for (dl = cl.downloadlist; dl; dl = dl->next)	//It's already on our list. Ignore it.
	{
		if (!strcmp(dl->localname, localname))
			return 2;	//queued
	}

	if (cls.download)
		if (!strcmp(cls.download->localname, localname))
			return 1;	//downloading
	return 0;
}

//note: this will overwrite existing files.
//returns true if the download is going to be downloaded after the call.
qboolean CL_EnqueDownload(const char *filename, const char *localname, unsigned int flags)
{
	extern cvar_t cl_downloads;
	downloadlist_t *dl;
	qboolean webdl = false;
	char ext[8];
	if (!strncmp(filename, "http://", 7) || !strncmp(filename, "https://", 8))
	{
		if (!localname)
			return false;

		webdl = true;
	}
	else
	{
		if (!localname)
			localname = filename;

		if (cls.demoplayback && cls.demoplayback != DPB_EZTV)
			return false;
	}
	COM_FileExtension(localname, ext, sizeof(ext));
	if (!stricmp(ext, "dll") || !stricmp(ext, "so") || strchr(localname, '\\') || strchr(localname, ':') || strstr(localname, ".."))
	{
		Con_Printf("Denying download of \"%s\"\n", filename);
		return false;
	}

	if (!(flags & DLLF_USEREXPLICIT) && !cl_downloads.ival)
	{
		if (flags & DLLF_VERBOSE)
			Con_Printf("cl_downloads setting prevents download of \"%s\"\n", filename);
		return false;
	}

	/*reject if it already failed*/
	if (!(flags & DLLF_IGNOREFAILED))
	{
#ifdef NQPROT
		if (!webdl && cls.protocol == CP_NETQUAKE)
			if (!cl_dp_serverextension_download)
				return false;
#endif

		for (dl = cl.faileddownloads; dl; dl = dl->next)	//yeah, so it failed... Ignore it.
		{
			if (!strcmp(dl->rname, filename))
			{
				if (flags & DLLF_VERBOSE)
					Con_Printf("We've failed to download \"%s\" already\n", filename);
				return false;
			}
		}
	}

	/*check for dupes*/
	switch(CL_IsDownloading(localname))
	{
	case 2:
		if (flags & DLLF_VERBOSE)
			Con_Printf("Already waiting for \"%s\"\n", filename);
		return true;
	default:
	case 1:
		if (flags & DLLF_VERBOSE)
			Con_Printf("Already downloading \"%s\"\n", filename);
		return true;
	case 0:
		break;
	}

	if (!*filename)
	{
		Con_Printf("Download \"\"? Huh?\n");
		return true;
	}

	dl = Z_Malloc(sizeof(downloadlist_t));
	Q_strncpyz(dl->rname, filename, sizeof(dl->rname));
	Q_strncpyz(dl->localname, localname, sizeof(dl->localname));
	dl->next = cl.downloadlist;
	cl.downloadlist = dl;
	dl->size = 0;
	dl->flags = flags | DLLF_SIZEUNKNOWN;

	if (!webdl && (cls.fteprotocolextensions & (PEXT_CHUNKEDDOWNLOADS
#ifdef PEXT_PK3DOWNLOADS
		| PEXT_PK3DOWNLOADS
#endif
		)) && !(dl->flags & DLLF_TEMPORARY))
	{
		CL_SendClientCommand(true, "dlsize \"%s\"", dl->rname);
	}

	if (flags & DLLF_VERBOSE)
		Con_Printf("Enqued download of \"%s\"\n", filename);

	return true;
}

#ifdef warningmsg
#pragma warningmsg("fix this")
#endif
int downloadsize;
void CL_GetDownloadSizes(unsigned int *filecount, qofs_t *totalsize, qboolean *somesizesunknown)
{
	downloadlist_t *dl;
	qdownload_t *d;
	*filecount = 0;
	*totalsize = 0;
	*somesizesunknown = false;
	for(dl = cl.downloadlist; dl; dl = dl->next)
	{
		*filecount += 1;
		if (dl->flags & DLLF_SIZEUNKNOWN)
			*somesizesunknown = true;
		else
			*totalsize += dl->size;
	}

	d = cls.download;
	if (d)
	{
		if (d->sizeunknown)
			*somesizesunknown = true;
		*totalsize += d->size;
	}
}

void CL_DisenqueDownload(char *filename)
{
	downloadlist_t *dl, *nxt;
	if(cl.downloadlist)	//remove from enqued download list
	{
		if (!strcmp(cl.downloadlist->rname, filename))
		{
			dl = cl.downloadlist;
			cl.downloadlist = cl.downloadlist->next;
			Z_Free(dl);
		}
		else
		{
			for (dl = cl.downloadlist; dl->next; dl = dl->next)
			{
				if (!strcmp(dl->next->rname, filename))
				{
					nxt = dl->next->next;
					Z_Free(dl->next);
					dl->next = nxt;
					break;
				}
			}
		}
	}
}

#ifdef WEBCLIENT
void CL_WebDownloadFinished(struct dl_download *dl)
{
	if (dl->status == DL_FAILED)
		CL_DownloadFailed(dl->url, &dl->qdownload);
	else if (dl->status == DL_FINISHED)
	{
		if (dl->file)
			VFS_CLOSE(dl->file);
		dl->file = NULL;
		CL_DownloadFinished(&dl->qdownload);
	}
}
#endif

void CL_SendDownloadStartRequest(char *filename, char *localname, unsigned int flags)
{
	static int dlsequence;
	qdownload_t *dl;

#ifdef WEBCLIENT
	if (!strncmp(filename, "http://", 7))
	{
		if (!HTTP_CL_Get(filename, localname, CL_WebDownloadFinished))
			CL_DownloadFailed(filename, NULL);
		return;
	}
#endif
	if (cls.download)
		return;	//no!
	
	dl = Z_Malloc(sizeof(*dl));
	dl->filesequence = ++dlsequence;

	Q_strncpyz(dl->remotename, filename, sizeof(dl->remotename));
	Q_strncpyz(dl->localname, localname, sizeof(dl->localname));
	if (!(flags & DLLF_TEMPORARY))
		Con_TPrintf ("Downloading %s...\n", dl->localname);

	// download to a temp name, and only rename
	// to the real name when done, so if interrupted
	// a runt file wont be left
	COM_StripExtension (localname, dl->tempname, sizeof(dl->tempname)-5);
	Q_strncatz (dl->tempname, ".tmp", sizeof(dl->tempname));

	CL_SendClientCommand(true, "download %s", filename);

	dl->method = DL_QWPENDING;
	dl->percent = 0;
	dl->sizeunknown = true;
	dl->flags = flags&DLLF_OVERWRITE;

	CL_DisenqueDownload(filename);

	cls.download = dl;
}

//Do any reloading for the file that just reloaded.
void CL_DownloadFinished(qdownload_t *dl)
{
	int i;
	char ext[8];

	char filename[MAX_QPATH];
	char tempname[MAX_QPATH];

	Q_strncpyz(filename, dl->localname, sizeof(filename));
	Q_strncpyz(tempname, dl->tempname, sizeof(tempname));

	DL_Abort(dl, QDL_COMPLETED);

	COM_RefreshFSCache_f();

	COM_FileExtension(filename, ext, sizeof(ext));


	//should probably ask the filesytem code if its a package format instead.
	if (!strncmp(filename, "package/", 8) || !strncmp(ext, "pk4", 3) || !strncmp(ext, "pk3", 3) || !strncmp(ext, "pak", 3))
	{
		FS_ReloadPackFiles();
		CL_CheckServerInfo();
	}
	else if (!strcmp(filename, "gfx/palette.lmp"))
	{
		Cbuf_AddText("vid_restart\n", RESTRICT_LOCAL);
	}
	else
	{
		CL_CheckModelResources(filename);
		if (!cl.sendprespawn)
		{
/*
			extern int mod_numknown;
			extern model_t	*mod_known;
			for (i = 0; i < mod_numknown; i++)	//go and load this model now.
			{
				if (!strcmp(mod_known[i].name, filename))
				{
					Mod_ForName(mod_known[i].name, false);	//throw away result.
					break;
				}
			}
*/
			for (i = 0; i < MAX_PRECACHE_MODELS; i++)	//go and load this model now.
			{
				if (!strcmp(cl.model_name[i], filename))
				{
					cl.model_precache[i] = Mod_ForName(cl.model_name[i], MLV_WARN);	//throw away result.
					if (i == 1)
						cl.worldmodel = cl.model_precache[i];
					break;
				}
			}
			for (i = 0; i < MAX_CSMODELS; i++)	//go and load this model now.
			{
				if (!strcmp(cl.model_csqcname[i], filename))
				{
					cl.model_csqcprecache[i] = Mod_ForName(cl.model_csqcname[i], MLV_WARN);	//throw away result.
					break;
				}
			}
			for (i = 0; i < MAX_VWEP_MODELS; i++)
			{
				if (!strcmp(cl.model_name_vwep[i], filename))
				{
					cl.model_precache_vwep[i] = Mod_ForName(cl.model_name_vwep[i], MLV_WARN);
					break;
				}
			}
		}
		S_ResetFailedLoad();	//okay, so this can still get a little spammy in bad places...

		//this'll do the magic for us
		Skin_FlushSkin(filename);
	}
}

qboolean CL_CheckFile(const char *filename)
{
	if (strstr (filename, ".."))
	{
		Con_TPrintf ("Refusing to download a path with ..\n");
		return true;
	}

	if (COM_FCheckExists (filename))
	{	// it exists, no need to download
		return true;
	}
	return false;
}

qboolean CL_CheckDLFile(const char *filename)
{
	if (!strncmp(filename, "package/", 8))
	{
		vfsfile_t *f;
		f = FS_OpenVFS(filename+8, "rb", FS_ROOT);
		if (f)
		{
			VFS_CLOSE(f);
			return true;
		}
		return false;
	}
	else
		return COM_FCheckExists(filename);
}
/*
===============
CL_CheckOrEnqueDownloadFile

Returns true if the file exists, returns false if it triggered a download.
===============
*/

qboolean	CL_CheckOrEnqueDownloadFile (const char *filename, const char *localname, unsigned int flags)
{	//returns false if we don't have the file yet.
	COM_AssertMainThread("CL_CheckOrEnqueDownloadFile");
	if (flags & DLLF_NONGAME)
	{
		/*pak/pk3 downloads have an explicit leading package/ as an internal/network marker*/
		if (!strchr(filename, ':'))
			filename = va("package/%s", filename);
		localname = va("package/%s", localname);
	}
	/*files with a leading * should not be downloaded (inline models, sexed sounds, etc). also block anyone trying to explicitly download a package/ because our code (wrongly) uses that name internally*/
	else if (*filename == '*' || !strncmp(filename, "package/", 8))
		return true;

	if (!localname)
		localname = filename;

#ifndef CLIENTONLY
	/*no downloading if we're the one we'd be downloading from*/
	if (sv.state)
		return true;
#endif

	if (!(flags & DLLF_OVERWRITE))
	{
		if (CL_CheckDLFile(localname))
			return true;
	}

	//ZOID - can't download when recording
	if (cls.demorecording)
	{
		Con_TPrintf ("Unable to download %s in record mode.\n", filename);
		return true;
	}
	//ZOID - can't download when playback
//	if (cls.demoplayback && cls.demoplayback != DPB_EZTV)
//		return true;

	SCR_EndLoadingPlaque();	//release console.

	if (*cl_download_mapsrc.string)
	if (!strncmp(filename, "maps/", 5))
	if (!strcmp(filename + strlen(filename)-4, ".bsp"))
	{
		char base[MAX_QPATH];
		COM_FileBase(filename, base, sizeof(base));
		filename = va("%s%s.bsp", cl_download_mapsrc.string, base);
	}

	if (!CL_EnqueDownload(filename, localname, flags))
		return true;	/*don't stall waiting for it if it failed*/


	if (!(flags & DLLF_IGNOREFAILED))
	{
		downloadlist_t *dl;
		for (dl = cl.faileddownloads; dl; dl = dl->next)
		{
			if (!strcmp(dl->rname, filename))
			{
				//if its on the failed list, don't block waiting for it to download
				return true;
			}
		}
	}
	return false;
}



static qboolean CL_CheckMD2Skins (qbyte *precache_model)
{
	qboolean ret = false;
	md2_t *pheader;
	int skin = 1;
	char *str;

	pheader = (md2_t *)precache_model;
	if (LittleLong (pheader->version) != MD2ALIAS_VERSION)
	{
		//bad version.
		return false;
	}

	pheader = (md2_t *)precache_model;
	for (skin = 0; skin < LittleLong(pheader->num_skins); skin++)
	{
		str = (char *)precache_model +
			LittleLong(pheader->ofs_skins) +
			skin*MD2MAX_SKINNAME;
		COM_CleanUpPath(str);
		if (!CL_CheckOrEnqueDownloadFile(str, str, 0))
			ret = true;
	}
	return ret;
}

qboolean CL_CheckHLBspWads(char *file)
{
	lump_t lump;
	dheader_t *dh;
	char *s;
	char *w;
	char key[256];
	char wads[4096];
	dh = (dheader_t *)file;

	lump.fileofs = LittleLong(dh->lumps[LUMP_ENTITIES].fileofs);
	lump.filelen = LittleLong(dh->lumps[LUMP_ENTITIES].filelen);

	s = file + lump.fileofs;

	s = COM_Parse(s);
	if (strcmp(com_token, "{"))
		return false;

	while (*s)
	{
		s = COM_ParseOut(s, key, sizeof(key));
		if (!strcmp(key, "}"))
			break;

		s = COM_ParseOut(s, wads, sizeof(wads));

		if (!strcmp(key, "wad"))
		{
			s = wads;
			while ((s = COM_ParseToken(s, ";")))
			{
				if (!strcmp(com_token, ";"))
					continue;
				while ((w = strchr(com_token, '\\')))
					*w = '/';
				w = COM_SkipPath(com_token);
				Con_Printf("wads: %s\n", w);
				if (!CL_CheckFile(w))
					CL_CheckOrEnqueDownloadFile(va("textures/%s", w), NULL, DLLF_REQUIRED);
			}
			return false;
		}
	}
	return false;
}

qboolean CL_CheckQ2BspWals(char *file)
{
	qboolean gotone = false;

	q2dheader_t *dh;
	lump_t lump;
	q2texinfo_t *tinf;
	unsigned int i, j, count;

	dh = (q2dheader_t*)file;
	if (LittleLong(dh->version) != BSPVERSION_Q2)
	{
		//quake3? unknown?
		return false;
	}
	lump.fileofs = LittleLong(dh->lumps[Q2LUMP_TEXINFO].fileofs);
	lump.filelen = LittleLong(dh->lumps[Q2LUMP_TEXINFO].filelen);

	count = lump.filelen / sizeof(*tinf);
	if (lump.filelen != count*sizeof(*tinf))
		return false;

	tinf = (q2texinfo_t*)(file + lump.fileofs);
	for (i = 0; i < count; i++)
	{
		//ignore duplicate files (to save filesystem hits)
		for (j = 0; j < i; j++)
			if (!strcmp(tinf[i].texture, tinf[j].texture))
				break;

		if (i == j)
		{
			if (!CL_CheckDLFile(va("textures/%s.wal", tinf[i].texture)))
				if (!CL_CheckDLFile(va("textures/%s.tga", tinf[i].texture)))
					if (!CL_CheckOrEnqueDownloadFile(tinf[i].texture, NULL, 0))
						gotone = true;
		}
	}
	return gotone;
}

static qboolean CL_CheckModelResources (char *name)
{
	//returns true if we triggered a download
	qboolean ret;
	qbyte *file;

	if (!(strstr(name, ".md2") || strstr(name, ".bsp")))
		return false;

	// checking for skins in the model

	FS_LoadFile(name, (void **)&file);
	if (!file)
	{
		return false; // couldn't load it
	}
	if (LittleLong(*(unsigned *)file) == MD2IDALIASHEADER)
		ret = CL_CheckMD2Skins(file);
	else if (LittleLong(*(unsigned *)file) == BSPVERSIONHL)
		ret = CL_CheckHLBspWads(file);
	else if (LittleLong(*(unsigned *)file) == IDBSPHEADER)
		ret = CL_CheckQ2BspWals(file);
	else
		ret = false;
	FS_FreeFile(file);

	return ret;
}

/*
=================
Model_NextDownload
=================
*/
void Model_CheckDownloads (void)
{
	char	*s;
	int		i;
	char ext[8];

//	Con_TPrintf (TLC_CHECKINGMODELS);

	R_SetSky(cl.skyname);
#ifdef Q2CLIENT
	if (cls.protocol == CP_QUAKE2)
	{
		for (i = 0; i < Q2MAX_IMAGES; i++)
		{
			char picname[256];
			if (!cl.image_name[i] || !*cl.image_name[i])
				continue;
			Q_snprintfz(picname, sizeof(picname), "pics/%s.pcx", cl.image_name[i]);
			CL_CheckOrEnqueDownloadFile(picname, picname, 0);
		}
		if (!CLQ2_RegisterTEntModels())
			return;
	}
#endif

	for (i = 1; cl.model_name[i][0]; i++)
	{
		s = cl.model_name[i];
		if (s[0] == '*')
			continue;	// inline brush model

		if (!stricmp(COM_FileExtension(s, ext, sizeof(ext)), "dsp"))	//doom sprites are weird, and not really downloadable via this system
			continue;

#ifdef Q2CLIENT
		if (cls.protocol == CP_QUAKE2 && s[0] == '#')	//this is a vweap
			continue;
#endif

		CL_CheckOrEnqueDownloadFile(s, s, (i==1)?DLLF_REQUIRED:0);	//world is required to be loaded.
		CL_CheckModelResources(s);
	}

	for (i = 0; i < MAX_VWEP_MODELS; i++)
	{
		s = cl.model_name_vwep[i];

		if (!stricmp(COM_FileExtension(s, ext, sizeof(ext)), "dsp"))	//doom sprites are weird, and not really downloadable via this system
			continue;

		if (!*s)
			continue;

		CL_CheckOrEnqueDownloadFile(s, s, 0);
		CL_CheckModelResources(s);
	}
}

int CL_LoadModels(int stage, qboolean dontactuallyload)
{
	int i;

	float giveuptime = Sys_DoubleTime()+1;	//small things get padded into a single frame

#define atstage() ((cl.contentstage == stage++ && !dontactuallyload)?++cl.contentstage:false)
#define endstage() if (!cls.timedemo && giveuptime<Sys_DoubleTime()) return -1;

	pmove.numphysent = 0;
	pmove.physents[0].model = NULL;

/*#ifdef PEXT_CSQC
	if (atstage())
	{
		extern cvar_t  cl_nocsqc;
		if (cls.protocol == CP_NETQUAKE && !cl_nocsqc.ival && !cls.demoplayback)
		{
			char *s;
			SCR_SetLoadingFile("csprogs");
			s = Info_ValueForKey(cl.serverinfo, "*csprogs");
			if (*s)	//only allow csqc if the server says so, and the 'checksum' matches.
			{
				extern cvar_t cl_download_csprogs;
				unsigned int chksum = strtoul(s, NULL, 0);
				if (cl_download_csprogs.ival)
				{
					char *str = va("csprogsvers/%x.dat", chksum);
					if (CL_CheckOrEnqueDownloadFile("csprogs.dat", str, DLLF_REQUIRED))
						return stage;	//its kinda required
				}
				else
				{
					Con_Printf("Not downloading csprogs.dat due to allow_download_csprogs\n");
				}
			}
		}
		endstage();
	}
#endif*/

#ifdef HLCLIENT
	if (atstage())
	{
		SCR_SetLoadingFile("hlclient");
		CLHL_LoadClientGame();
		endstage();
	}
#endif

#ifdef PEXT_CSQC
	if (atstage())
	{
		char *s;
		qboolean anycsqc;
		char *endptr;
		unsigned int chksum;
#ifdef _DEBUG
		anycsqc = true;
#else
		anycsqc = atoi(Info_ValueForKey(cl.serverinfo, "anycsqc"));
#endif
		if (cls.demoplayback)
			anycsqc = true;
		s = Info_ValueForKey(cl.serverinfo, "*csprogs");
		chksum = strtoul(s, &endptr, 0);
		if (*endptr)
		{
			Con_Printf("corrupt *csprogs key in serverinfo\n");
			anycsqc = true;
			chksum = 0;
		}
		SCR_SetLoadingFile("csprogs");
		if (!CSQC_Init(anycsqc, *s?true:false, chksum))
		{
			Sbar_Start();	//try and start this before we're actually on the server,
							//this'll stop the mod from sending so much stuffed data at us, whilst we're frozen while trying to load.
							//hopefully this'll make it more robust.
							//csqc is expected to use it's own huds, or to run on decent servers. :p
		}
		endstage();
	}
#endif

	if (atstage())
	{
		SCR_SetLoadingFile("prenewmap");
		Surf_PreNewMap();

		endstage();
	}

	if (cl.playerview[0].playernum == -1)
	{	//q2 cinematic - don't load the models.
		cl.worldmodel = cl.model_precache[1] = Mod_ForName ("", MLV_WARN);
	}
	else
	{
		for (i=1 ; i<MAX_PRECACHE_MODELS ; i++)
		{
			if (!cl.model_name[i][0])
				continue;

			if (atstage())
			{
#if 0
				SCR_SetLoadingFile(cl.model_name[i]);
#ifdef CSQC_DAT
				if (i == 1)
					CSQC_LoadResource(cl.model_name[i], "map");
				else
					CSQC_LoadResource(cl.model_name[i], "model");
#endif
#endif
#ifdef Q2CLIENT
				if (cls.protocol == CP_QUAKE2 && *cl.model_name[i] == '#')
					cl.model_precache[i] = NULL;
				else
#endif
					cl.model_precache[i] = Mod_ForName (Mod_FixName(cl.model_name[i], cl.model_name[1]), MLV_WARN);

				S_ExtraUpdate();

				endstage();
			}
		}
		for (i = 0; i < MAX_VWEP_MODELS; i++)
		{
			if (!cl.model_name_vwep[i][0])
				continue;

			if (atstage())
			{
#if 0
				SCR_SetLoadingFile(cl.model_name_vwep[i]);
#ifdef CSQC_DAT
				CSQC_LoadResource(cl.model_name_vwep[i], "vwep");
#endif
#endif
				cl.model_precache_vwep[i] = Mod_ForName (cl.model_name_vwep[i], MLV_WARN);
				endstage();
			}
		}
	}



	if (atstage())
	{
		cl.worldmodel = cl.model_precache[1];
		if (!cl.worldmodel || cl.worldmodel->type == mod_dummy)
		{
			if (!cl.model_name[1][0])
				Host_EndGame("Worldmodel name wasn't sent\n");
//			else
//				return stage;
//				Host_EndGame("Worldmodel wasn't loaded\n");
		}

		SCR_SetLoadingFile("csprogs world");

#ifdef CSQC_DAT
		CSQC_WorldLoaded();
#endif

		endstage();
	}

	for (i=1 ; i<MAX_CSMODELS ; i++)
	{
		if (!cl.model_csqcname[i][0])
			continue;
		if (atstage())
		{
#if 0
			SCR_SetLoadingFile(cl.model_csqcname[i]);
#ifdef CSQC_DAT
			if (i == 1)
				CSQC_LoadResource(cl.model_csqcname[i], "map");
			else
				CSQC_LoadResource(cl.model_csqcname[i], "model");
#endif
#endif
			cl.model_csqcprecache[i] = Mod_ForName (cl.model_csqcname[i], MLV_WARN);

			S_ExtraUpdate();

			endstage();
		}
	}

	if (atstage())
	{
		SCR_SetLoadingFile("wads");
		if (cl.worldmodel && cl.worldmodel->loadstate == MLS_LOADING)
			return -1;
		if (cl.worldmodel && cl.worldmodel->loadstate == MLS_LOADING)
			COM_WorkerPartialSync(cl.worldmodel, &cl.worldmodel->loadstate, MLS_LOADING);
		Mod_ParseInfoFromEntityLump(cl.worldmodel);

		Wad_NextDownload();

		endstage();
	}

	if (atstage())
	{
		SCR_SetLoadingFile("external textures");
		if (cl.worldmodel && cl.worldmodel->loadstate == MLS_LOADING)
			COM_WorkerPartialSync(cl.worldmodel, &cl.worldmodel->loadstate, MLS_LOADING);
		if (cl.worldmodel && cl.worldmodel->loadstate == MLS_LOADED)
			Mod_NowLoadExternal(cl.worldmodel);

		endstage();
	}


	// all done
	if (atstage())
	{
		SCR_SetLoadingFile("newmap");
//		if (!cl.worldmodel || cl.worldmodel->type == mod_dummy)
//			Host_EndGame("No worldmodel was loaded\n");
		cl.model_precaches_added = false;
		Surf_NewMap ();

		pmove.physents[0].model = cl.worldmodel;

		endstage();
	}

#ifdef CSQC_DAT
	if (atstage())
	{
		SCR_SetLoadingFile("csqc init");
		if (CSQC_Inited())
		{
			if (cls.fteprotocolextensions & PEXT_CSQC)
				CL_SendClientCommand(true, "enablecsqc");
		}
		else
		{
			if (cls.fteprotocolextensions & PEXT_CSQC)
				CL_SendClientCommand(true, "disablecsqc");
		}
		endstage();
	}
#endif

	return stage;
}

int CL_LoadSounds(int stage, qboolean dontactuallyload)
{
	int i;
	float giveuptime = Sys_DoubleTime()+0.1;	//small things get padded into a single frame

//#define atstage() ((cl.contentstage == stage++)?++cl.contentstage:false)
//#define endstage() if (giveuptime<Sys_DoubleTime()) return -1;

	for (i=1 ; i<MAX_PRECACHE_SOUNDS ; i++)
	{
		if (!cl.sound_name[i][0])
			break;

		if (atstage())
		{
#if 0
			SCR_SetLoadingFile(cl.sound_name[i]);
#ifdef CSQC_DAT
			CSQC_LoadResource(cl.sound_name[i], "sound");
#endif
#endif
			cl.sound_precache[i] = S_PrecacheSound (cl.sound_name[i]);

			S_ExtraUpdate();
			endstage();
		}
	}
	return stage;
}

void Sound_CheckDownload(const char *s)
{
	char mangled[512];

	if (*s == '*')	//q2 sexed sound
		return;

	if (!S_HaveOutput())
		return;

	//check without the sound/ prefix
	if (CL_CheckFile(s))
		return;	//we have it already

	//the things I do for nexuiz... *sigh*
	COM_StripExtension(s, mangled, sizeof(mangled));
	COM_DefaultExtension(mangled, ".ogg", sizeof(mangled));
	if (CL_CheckFile(mangled))
		return;

	//check with the sound/ prefix
	s = va("sound/%s",s);

	if (CL_CheckFile(s))
		return;	//we have it already

	//the things I do for nexuiz... *sigh*
	COM_StripExtension(s, mangled, sizeof(mangled));
	COM_DefaultExtension(mangled, ".ogg", sizeof(mangled));
	if (CL_CheckFile(mangled))
		return;

	//download the one the server said.
	CL_CheckOrEnqueDownloadFile(s, NULL, 0);
}

/*
=================
Sound_NextDownload
=================
*/
void Sound_CheckDownloads (void)
{
	int		i;


//	Con_TPrintf (TLC_CHECKINGSOUNDS);

#ifdef CSQC_DAT
//	if (cls.fteprotocolextensions & PEXT_CSQC)
	{
		char	*s;
		s = Info_ValueForKey(cl.serverinfo, "*csprogs");
		if (*s)	//only allow csqc if the server says so, and the 'checksum' matches.
		{
			extern cvar_t cl_download_csprogs, cl_nocsqc;
			char *endptr;
			unsigned int chksum = strtoul(s, &endptr, 0);
			if (cl_nocsqc.ival || cls.demoplayback || *endptr)
			{
			}
			else if (cl_download_csprogs.ival)
			{
				char *str = va("csprogsvers/%x.dat", chksum);
				CL_CheckOrEnqueDownloadFile("csprogs.dat", str, DLLF_REQUIRED);
			}
			else
			{
				Con_Printf("Not downloading csprogs.dat\n");
			}
		}
	}
#endif

	for (i = 1; cl.sound_name[i][0]
		; i++)
	{
		Sound_CheckDownload(cl.sound_name[i]);
	}
}

/*
======================
CL_RequestNextDownload
======================
*/
void CL_RequestNextDownload (void)
{

	int stage;
	/*already downloading*/
	if (cls.download && !cls.demoplayback)
		return;

	/*request downloads only if we're at the point where we've received a complete list of them*/
	if (cl.sendprespawn || cls.state == ca_active)
		if (cl.downloadlist)
		{
			downloadlist_t *dl;
			unsigned int fl;

			//download required downloads first
			for (dl = cl.downloadlist; dl; dl = dl->next)
			{
				if (dl->flags & DLLF_NONGAME)
					break;
			}
			if (!dl)
			{
				for (dl = cl.downloadlist; dl; dl = dl->next)
				{
					if (dl->flags & DLLF_REQUIRED)
						break;
				}
				if (!dl)
					dl = cl.downloadlist;
			}
			fl = dl->flags;

			/*if we don't require downloads don't queue requests until we're actually on the server, slightly more deterministic*/
			if (cls.state == ca_active || (requiredownloads.value && !cls.demoplayback) || (fl & DLLF_REQUIRED))
			{
				if ((fl & DLLF_OVERWRITE) || !CL_CheckFile (dl->localname))
				{
					CL_SendDownloadStartRequest(dl->rname, dl->localname, fl);
					return;
				}
				else
				{
					Con_Printf("Already have %s\n", dl->localname);
					CL_DisenqueDownload(dl->rname);

					//recurse a bit.
					CL_RequestNextDownload();
					return;
				}
			}
		}

	if (cl.sendprespawn)
	{	// get next signon phase
		extern int total_loading_size, current_loading_size;

		if (!cl.contentstage)
		{
			stage = 0;
			stage = CL_LoadModels(stage, true);
			stage = CL_LoadSounds(stage, true);
			total_loading_size = stage;
			cl.contentstage = 0;
		}

		stage = 0;
		stage = CL_LoadModels(stage, false);
		current_loading_size = cl.contentstage;
		if (stage < 0)
			return;	//not yet
		stage = CL_LoadSounds(stage, false);
		current_loading_size = cl.contentstage;
		if (stage < 0)
			return;
		if (requiredownloads.ival && COM_HasWork())
		{
			SCR_SetLoadingFile("loading content");
			return;
		}
		SCR_SetLoadingFile("receiving game state");

		cl.sendprespawn = false;

		if (cl.worldmodel && cl.worldmodel->loadstate == MLS_LOADING)
			COM_WorkerPartialSync(cl.worldmodel, &cl.worldmodel->loadstate, MLS_LOADING);

#ifdef warningmsg
#pragma warningmsg("timedemo timer should start here")
#endif

		if (!cl.worldmodel || cl.worldmodel->loadstate != MLS_LOADED)
		{
			Con_Printf("\n\n-------------\nCouldn't download %s - cannot fully connect\n", cl.worldmodel->name);
			SCR_SetLoadingStage(LS_NONE);
			return;
		}

#ifdef Q2CLIENT
		if (cls.protocol == CP_QUAKE2)
		{
			Skin_NextDownload();
			SCR_SetLoadingStage(LS_NONE);
			CL_SendClientCommand(true, "begin %i\n", cl.servercount);
		}
		else
#endif
		{
			if (cls.demoplayback == DPB_EZTV)
			{
				if (CL_RemoveClientCommands("qtvspawn"))
					Con_DPrintf("Multiple prespawns\n");
				CL_SendClientCommand(true, "qtvspawn %i 0 %i", cl.servercount, COM_RemapMapChecksum(LittleLong(cl.worldmodel->checksum2)));
				SCR_SetLoadingStage(LS_NONE);
			}
			else
			{
		// done with modellist, request first of static signon messages
				if (CL_RemoveClientCommands("prespawn"))
					Con_DPrintf("Multiple prespawns\n");
				if (cls.protocol == CP_NETQUAKE)
					CL_SendClientCommand(true, "prespawn");
				else
				{
		//			CL_SendClientCommand("prespawn %i 0 %i", cl.servercount, cl.worldmodel->checksum2);
					CL_SendClientCommand(true, prespawn_name, cl.servercount, COM_RemapMapChecksum(LittleLong(cl.worldmodel->checksum2)));
				}
			}
		}

	}
}

int CL_RequestADownloadChunk(void);
void CL_SendDownloadReq(sizebuf_t *msg)
{
	if (cls.demoplayback == DPB_EZTV)
		return;	//tcp connection, so no need to constantly ask

	if (!cls.download)
	{
		if (cl.downloadlist)
			CL_RequestNextDownload();
		return;
	}

#ifdef PEXT_CHUNKEDDOWNLOADS
	if (cls.download->method == DL_QWCHUNKS)
		DLC_Poll(cls.download);
#endif
}

#ifdef PEXT_ZLIBDL
#ifdef _WIN32
#define ZEXPORT VARGS
#include "../../zip/zlib.h"

//# pragma comment (lib, "zip/zlib.lib")
#else
#include <zlib.h>
#endif

char *ZLibDownloadDecode(int *messagesize, char *input, int finalsize)
{
	char *outbuf = Hunk_TempAlloc(finalsize);
	z_stream zs;

	*messagesize = (*(short*)input);
	input+=2;

	if (!*messagesize)
	{
		*messagesize = finalsize+2;
		return input;
	}

	memset(&zs, 0, sizeof(zs));


	zs.next_in = input;
    zs.avail_in = *messagesize;	//tell it that it has a lot. Possibly a bad idea.
    zs.total_in = 0;

    zs.next_out = outbuf;
    zs.avail_out = finalsize;	//this is the limiter.
    zs.total_out = 0;

    zs.data_type = Z_BINARY;

	inflateInit(&zs);
	inflate(&zs, Z_FINISH);	//decompress it in one go.
	inflateEnd(&zs);

	*messagesize = zs.total_in+2;
	return outbuf;
}
#endif

downloadlist_t *CL_DownloadFailed(const char *name, qdownload_t *qdl)
{
	//add this to our failed list. (so we don't try downloading it again...)
	downloadlist_t *failed, **link, *dl;
	failed = Z_Malloc(sizeof(downloadlist_t));
	failed->next = cl.faileddownloads;
	cl.faileddownloads = failed;
	Q_strncpyz(failed->rname, name, sizeof(failed->rname));

	//if this is what we're currently downloading, close it up now.
	//don't do this if we're just marking the file as unavailable for download.
	if (qdl && (!stricmp(qdl->remotename, name) || !*name))
	{
		DL_Abort(qdl, QDL_FAILED);
	}

	link = &cl.downloadlist;
	while(*link)
	{
		dl = *link;
		if (!strcmp(dl->rname, name))
		{
			*link = dl->next;
			failed->flags |= dl->flags;
			Z_Free(dl);
		}
		else
			link = &(*link)->next;
	}

	return failed;
}

#ifdef PEXT_CHUNKEDDOWNLOADS
#define DLBLOCKSIZE 1024

int CL_DownloadRate(void)
{
	qdownload_t *dl = cls.download;
	if (dl)
	{
		double curtime = Sys_DoubleTime();
		if (!dl->ratetime)
		{
			dl->ratetime = curtime;
			return dl->completedbytes/(Sys_DoubleTime() - dl->starttime);
		}
		if (curtime - dl->ratetime > 1)
		{
			dl->rate = dl->ratebytes / (curtime - dl->ratetime);
			dl->ratetime = curtime;
			dl->ratebytes = 0;
		}
		return dl->rate;
	}
	return 0;
}

#ifdef _MSC_VER
#define strtoull _strtoui64
#endif

//called when the server acks the download. opens the local file and stuff. returns false on failure
qboolean DL_Begun(qdownload_t *dl)
{
	//figure out where the file is meant to be going.
	dl->prefixbytes = 0;
	if (!strncmp(dl->tempname, "package/", 8))
	{
		dl->prefixbytes = 8;	//ignore the package/ part
		dl->fsroot = FS_ROOT;	//and put it in the root dir (-basedir), and hope the name includes a gamedir part
	}
	else if (!strncmp(dl->tempname,"skins/",6))
		dl->fsroot = FS_PUBBASEGAMEONLY;	//shared between gamedirs, so only use the basegame.
	else
		dl->fsroot = FS_PUBGAMEONLY;//FS_GAMEONLY;	//other files are relative to the active gamedir.

	Q_snprintfz(dl->dclname, sizeof(dl->dclname), "%s.dcl", dl->tempname);

	if (dl->method == DL_QWCHUNKS)
	{
		qboolean error = false;
		char partline[256];
		char partterm[128];
		char *p, t;
		qofs_t lastend = 0;
		qofs_t start, end;
		struct dlblock_s **link = &dl->dlblocks;
		vfsfile_t *parts = FS_OpenVFS(dl->dclname+dl->prefixbytes, "rb", dl->fsroot);
		if (!parts)
			error = true;
		while(!error && VFS_GETS(parts, partline, sizeof(partline)))
		{
			p = COM_ParseOut(partline, partterm, sizeof(partterm));
			t = *partterm;
			p = COM_ParseOut(p, partterm, sizeof(partterm));
			start = strtoull(partterm, NULL, 0);
			p = COM_ParseOut(p, partterm, sizeof(partterm));
			end = strtoull(partterm, NULL, 0);

			(*link) = Z_Malloc(sizeof(**link));
			(*link)->start = start;
			(*link)->end = end;
			(*link)->state = (t == 'c')?DLB_RECEIVED:DLB_MISSING;
			link = &(*link)->next;

			if (t == 'c')
				dl->completedbytes += end - start;

			if (start != lastend)
				error = true;
			lastend = end;
		}
		if (lastend != dl->size)
			error = true;
		if (parts)
			VFS_CLOSE(parts);
		if (!error)
			dl->file = FS_OpenVFS(dl->tempname+dl->prefixbytes, "w+b", dl->fsroot);
	}
	if (!dl->file)
	{
		struct dlblock_s *b;
		//make sure we don't get confused if someone end-tasks us before the download is complete.
		FS_Remove(dl->dclname+dl->prefixbytes, dl->fsroot);
		dl->completedbytes = 0;
		while (dl->dlblocks)
		{
			b = dl->dlblocks;
			dl->dlblocks = b->next;
			Z_Free(b);
		}
		FS_CreatePath(dl->tempname+dl->prefixbytes, dl->fsroot);
		dl->file = FS_OpenVFS(dl->tempname+dl->prefixbytes, "wb", dl->fsroot);
	}
	if (!dl->file)
	{
		char native[MAX_OSPATH];
		FS_NativePath(dl->tempname+dl->prefixbytes, dl->fsroot, native, sizeof(native));
		Con_TPrintf("Unable to open \"%s\"\n", native);
		return false;
	}

	if (dl->method == DL_QWPENDING)
		Con_TPrintf("method is still 'pending'\n");

	if (dl->method == DL_QWCHUNKS && !dl->dlblocks)
	{
		dl->dlblocks = Z_Malloc(sizeof(*dl->dlblocks));
		dl->dlblocks->start = 0;
		dl->dlblocks->end = dl->size;
		dl->dlblocks->state = DLB_MISSING;
	}
	dl->flags |= DLLF_BEGUN;

	dl->starttime = Sys_DoubleTime();
	return true;
}

void DL_Completed(qdownload_t *dl, qofs_t start, qofs_t end)
{
	struct dlblock_s *prev = NULL, *b, *n, *e;
	if (end <= start)
		return;	//ignore invalid ranges.
	for (b = dl->dlblocks; b; )
	{
		if (b->state == DLB_RECEIVED)
		{
			//nothing to be done. dupe. somehow. or simply a different range.
		}
		else if (b->start >= start && b->end <= end)
		{
			//whole block
//			Con_Printf("Whole block\n");
			b->state = DLB_RECEIVED;
			dl->completedbytes += b->end - b->start;
			dl->ratebytes += b->end - b->start;
		}
		else if (start > b->start && end < b->end)
		{
//			Con_Printf("chop out middle\n");
			//in the middle, no need to merge
			n = Z_Malloc(sizeof(*n));
			e = Z_Malloc(sizeof(*e));
			e->next = b->next;
			n->next = e;
			b->next = n;

			e->state = b->state;
			e->sequence = b->sequence;
			n->state = DLB_RECEIVED;

			e->end = b->end;
			b->end = start;
			n->start = start;
			n->end = end;
			e->start = end;

			dl->completedbytes += n->end - n->start;
			dl->ratebytes += n->end - n->start;
		}
		//data overlaps the start (data end must be smaller than block end)
		else if (start <= b->start && end > b->start)
		{
//			Con_Printf("complete start\n");

			//split it. new(non-complete) block is second.
			n = Z_Malloc(sizeof(*n));
			n->next = b->next;
			b->next = n;
			//second block keeps original block's state. first block gets completed
			n->state = b->state;
			n->sequence = b->sequence;
			b->state = DLB_RECEIVED;

			n->start = end;
			n->end = b->end;
			b->end = end;

			dl->completedbytes += b->end - b->start;
			dl->ratebytes += b->end - b->start;
		}
		//new data overlaps the end
		else if (start > b->start && start < b->end)
		{
//			Con_Printf("complete end\n");
			//split it. new(completed) block is second.
			n = Z_Malloc(sizeof(*n));
			n->next = b->next;
			b->next = n;
			//second block keeps original block's state. first block gets completed
			n->state = DLB_RECEIVED;
			n->sequence = 0;

			n->start = end;
			n->end = b->end;
			b->end = end;

			dl->completedbytes += n->end - n->start;
			dl->ratebytes += n->end - n->start;

			prev = b;
			b = n;
		}
		else
		{//don't bother merging, as nothing changed
			prev = b;
			b = b->next;
			continue;
		}

		//merge with next block
		if (b->next && b->next->state == DLB_RECEIVED)
		{
			n = b->next;
			b->next = n->next;
			b->end = n->end;
			Z_Free(n);
		}
		//merge with previous block if possible
		if (prev && prev->state == DLB_RECEIVED)
		{
			n = b;
			prev->end = b->end;
			prev->next = b->next;
			Z_Free(b);
			b = prev->next;
			continue;//careful here
		}
		prev = b;
		b = b->next;
	}
}

qboolean CL_AllowArbitaryDownload(char *localfile);

static float chunkrate;

void CL_ParseChunkedDownload(qdownload_t *dl)
{
	qbyte	*svname;
	int flag;
	qofs_t filesize;
	qofs_t chunknum;
	char data[DLBLOCKSIZE];

	chunknum = MSG_ReadLong();
	if (chunknum == -1)
	{
		flag = MSG_ReadLong();
		if (flag == 0x80000000)
		{	//really big files need special handling here.
			flag = MSG_ReadLong();
			filesize = qofs_Make(flag, MSG_ReadLong());
			flag = 0;
		}
		else
			filesize = flag;

		svname = MSG_ReadString();
		if (cls.demoplayback)
			return;

		if (!*svname)
		{
			//stupid mvdsv.
			/*if (totalsize < 0)
				svname = cls.downloadname;
			else*/
			{
				Con_Printf("ignoring nameless download\n");
				return;
			}
		}

		if (flag < 0)
		{
			if (flag == -4)
			{
				if (CL_AllowArbitaryDownload(svname))
				{
					Con_Printf("Download of \"%s\" redirected to \"%s\"\n", dl->remotename, svname);
					if (CL_CheckOrEnqueDownloadFile(svname, NULL, 0))
						Con_Printf("However, \"%s\" already exists. You may need to delete it.\n", svname);
				}
				svname = dl->remotename;
			}
			else if (flag == -3)
				Con_Printf("Server reported an error when downloading file \"%s\"\n", svname);
			else if (flag == -2)
				Con_Printf("Server permissions deny downloading file \"%s\"\n", svname);
			else
				Con_Printf("Couldn't find file \"%s\" on the server\n", svname);

			if (dl)
			{
				CL_DownloadFailed(svname, dl);

				CL_RequestNextDownload();
			}
			return;
		}

		if (!dl)
		{
			Con_Printf("ignoring download start. we're not meant to be downloading\n");
			return;
		}

		if (dl->method == DL_QWCHUNKS)
			Host_EndGame("Received second download - \"%s\"\n", svname);

		if (stricmp(dl->remotename, svname))
		{
			//fixme: we should allow extension changes, in the case of ogg/mp3/wav, or tga/png/jpg/pcx, or the addition of .gz or whatever
			Host_EndGame("Server sent the wrong download - \"%s\" instead of \"%s\"\n", svname, dl->remotename);
		}


		//start the new download
		dl->method = DL_QWCHUNKS;
		dl->percent = 0;
		dl->size = filesize;

		dl->starttime = Sys_DoubleTime();

		/*
		strcpy(cls.downloadname, svname);
		COM_StripExtension(svname, cls.downloadtempname);
		COM_DefaultExtension(cls.downloadtempname, ".tmp");
		*/

		if (!DL_Begun(dl))
		{
			CL_DownloadFailed(svname, dl);
			return;
		}

		return;
	}

//	Con_Printf("Received dl block %i: ", chunknum);

	MSG_ReadData(data, DLBLOCKSIZE);

	if (!dl)
	{
		Con_Printf("ignoring download data packet\n");
		return;
	}

	if (chunknum*DLBLOCKSIZE > dl->size+DLBLOCKSIZE)
		return;

	if (!dl->file)
		return;

	if (cls.demoplayback)
	{	//err, yeah, when playing demos we don't actually pay any attention to this.
		return;
	}

	VFS_SEEK(dl->file, chunknum*DLBLOCKSIZE);
	if (dl->size - chunknum*DLBLOCKSIZE < DLBLOCKSIZE)	//final block is actually meant to be smaller than we recieve.
		VFS_WRITE(dl->file, data, dl->size - chunknum*DLBLOCKSIZE);
	else
		VFS_WRITE(dl->file, data, DLBLOCKSIZE);

	DL_Completed(dl, chunknum*DLBLOCKSIZE, (chunknum+1)*DLBLOCKSIZE);

	dl->percent = dl->completedbytes/(float)dl->size*100;

	chunkrate += 1;
}

int CL_CountQueuedDownloads(void)
{
	int count = 0;
	downloadlist_t *dl;
	for (dl = cl.downloadlist; dl; dl = dl->next)
		count++;

	return count;
}

static void DLC_RequestDownloadChunks(qdownload_t *dl, float frametime)
{
	char *cmd;
	qofs_t chunk;
	struct dlblock_s *b, *n;
	qboolean stillpending = false;
	qboolean haveloss = false;
	int chunks, chunksremaining;
	static float slop;	//try to keep things as integers
//	int cmds = 20;
	if (frametime < 0)
		frametime = 0;
	if (frametime > 0.1)
		frametime = 0.1;	//erg?

	if (chunkrate < 0)
		chunkrate = 0;
	slop += chunkrate*frametime;
	chunksremaining = slop;
	slop -= chunksremaining;
	if (chunksremaining < 1)
	{
		if (chunkrate < 30)
			chunksremaining = 1;
		else
			return;
/*		if (!chunkrate)
			chunkrate = 72;
		else
			chunkrate+=frametime;
		return;
*/	}
	if (chunksremaining > 100)
	{	//we're going to need some sanity limit, for cpu resources.
		chunkrate -= (chunksremaining-100);
		chunksremaining = 100;
	}
//Con_DPrintf("%i\n", chunksremaining);
	for (b = dl->dlblocks; b; b = b->next)
	{
		//packetloss reverts blocks to missing.
		if (b->state == DLB_PENDING)
		{
			if (b->sequence < cls.netchan.incoming_sequence-10)
			{
				haveloss = true;
				b->state = DLB_MISSING;
				for (;;)	//merge it with the next if they're all invalid
				{
					n = b->next;
					if (!n)
						break;
					if (n->state == DLB_MISSING || (n->state == DLB_PENDING && n->sequence < cls.netchan.incoming_sequence-10))
					{
						b->next = n->next;
						b->end = n->end;
						Z_Free(n);
						continue;
					}
					break;
				}
			}
			else
				stillpending = true;
		}
		if (b->state == DLB_MISSING && chunksremaining)
		{
			chunk = b->start / DLBLOCKSIZE;
			chunks = 1;//((b->end+DLBLOCKSIZE-1)/DLBLOCKSIZE) - (b->start / DLBLOCKSIZE);
			if (chunks > chunksremaining)
				chunks = chunksremaining;

			//if this block is bigger than a chunk, split the two blocks.
			if (b->end - b->start > DLBLOCKSIZE*chunks)
			{
				n = Z_Malloc(sizeof(*n));
				n->next = b->next;
				n->start = (chunk+chunks)*DLBLOCKSIZE;
				n->end = b->end;
				b->end = n->start;
				n->state = DLB_MISSING;
				b->next = n;
			}
			b->state = DLB_PENDING;
			b->sequence = cls.netchan.outgoing_sequence;
			stillpending = true;

			if (chunks > 1)
				cmd = va("nextdl %u %3g %i %i\n", (unsigned int)chunk, dl->percent, dl->filesequence, chunks);
			else
				cmd = va("nextdl %u %3g %i\n", (unsigned int)chunk, dl->percent, dl->filesequence);
			CL_RemoveClientCommands(cmd);
			CL_SendClientCommand(false, "%s", cmd);
			chunksremaining -= chunks;
			if (chunksremaining <= 0)
				break;
			/*if (--cmds <= 0)
			{
				chunkrate -= chunksremaining;
//				haveloss = true;
				break;
			}*/
		}
	}
	if (haveloss)
	{
		chunkrate *= 0.98;
	}
	if (!stillpending)
	{	//when there's nothing still pending, the download is complete.
		Con_DPrintf("Download took %i seconds (%i more)\n", (int)(Sys_DoubleTime() - dl->starttime), CL_CountQueuedDownloads());
		CL_DownloadFinished(dl);
	}
}

void DLC_Poll(qdownload_t *dl)
{
	static float lasttime;
	DLC_RequestDownloadChunks(dl, realtime - lasttime);
	lasttime = realtime;
}

#endif

void DL_Abort(qdownload_t *dl, enum qdlabort aborttype)
{
	struct dlblock_s *b, *n;

	if (dl->file)
	{
		VFS_CLOSE(dl->file);
		dl->file = NULL;
	}

	if (dl->flags & DLLF_BEGUN)
	{
		if (aborttype == QDL_COMPLETED)
		{
			//this file isn't needed now the download has finished.
			FS_Remove(dl->dclname+dl->prefixbytes, dl->fsroot);

			if (dl->flags & DLLF_TEMPORARY)
			{
#ifdef TERRAIN
				if (!Terr_DownloadedSection(dl->tempname+dl->prefixbytes))
#endif
					Con_Printf("Downloaded unusable temporary file\n");
				FS_Remove(dl->tempname+dl->prefixbytes, dl->fsroot);
			}
			else if (Q_strcasecmp(dl->tempname, dl->localname))
			{
				if (dl->flags & DLLF_OVERWRITE)
					FS_Remove(dl->localname+dl->prefixbytes, dl->fsroot);
				if (!FS_Rename(dl->tempname+dl->prefixbytes, dl->localname+dl->prefixbytes, dl->fsroot))
				{
					char nativetmp[MAX_OSPATH], nativefinal[MAX_OSPATH];
					FS_NativePath(dl->tempname+dl->prefixbytes, dl->fsroot, nativetmp, sizeof(nativetmp));
					FS_NativePath(dl->localname+dl->prefixbytes, dl->fsroot, nativefinal, sizeof(nativefinal));
					Con_Printf("Couldn't rename %s to %s\n", nativetmp, nativefinal);
				}
			}
		}
		else
		{
			//file was aborted half way through...
			if (dl->dlblocks)
			{
				//save the list of valid chunks so we don't have to redownload those.
				vfsfile_t *parts;
				parts = FS_OpenVFS(dl->dclname+dl->prefixbytes, "wb", dl->fsroot);
				if (parts)
				{
					for (b = dl->dlblocks; b; b = n)
					{
						if (b->state == DLB_RECEIVED)
							VFS_PRINTF(parts, "c "fPRIllx" "fPRIllx"\n", (long long)b->start, (long long)b->end);
						else
						{
							for(;;)
							{
								n = b->next;
								if (n && n->state != DLB_RECEIVED)
								{
									b->end = n->end;
									b->next = n->next;
									Z_Free(n);
									continue;
								}
								break;
							}
							VFS_PRINTF(parts, "m "fPRIllx" "fPRIllx"\n", (long long)b->start, (long long)b->end);
						}

						n = b->next;
						Z_Free(b);
					}
					dl->dlblocks = NULL;
					VFS_CLOSE(parts);
				}
				else
					FS_Remove(dl->tempname + dl->prefixbytes, dl->fsroot);
			}
			else
			{
				//download looks like it was non-resumable. just delete it.
				FS_Remove(dl->tempname + dl->prefixbytes, dl->fsroot);
			}
		}

		if (aborttype != QDL_DISCONNECT)
		{
			switch(dl->method)
			{
			default:
				break;
#ifdef Q3CLIENT
			case DL_Q3:
				CLQ3_SendClientCommand("stopdl");
				break;
#endif
			case DL_DARKPLACES:
			case DL_QW:
			case DL_QWCHUNKS:
				{
					char *serverversion = Info_ValueForKey(cl.serverinfo, "*version");
					if (strncmp(serverversion , "MVDSV ", 6))	//don't tell mvdsv to stop, because it has retarded annoying clientprints that are spammy as fuck, and we don't want that.
						CL_SendClientCommand(true, "stopdownload");
				}
				break;
			}
		}
	}

	for (b = dl->dlblocks; b; b = n)
	{
		n = b->next;
		Z_Free(b);
	}
	dl->dlblocks = NULL;

	Z_Free(dl);
	if (cls.download == dl)
		cls.download = NULL;
}

/*
=====================
CL_ParseDownload

A download message has been received from the server
=====================
*/
void CL_ParseDownload (void)
{
	extern cvar_t cl_dlemptyterminate;
	int		size, percent;
	qbyte	name[1024];
	qdownload_t *dl = cls.download;

#ifdef PEXT_CHUNKEDDOWNLOADS
	if (cls.fteprotocolextensions & PEXT_CHUNKEDDOWNLOADS)
	{
		if (cls.demoplayback == DPB_EZTV)
			Host_EndGame("CL_ParseDownload: chunked download on qtv proxy.");
		CL_ParseChunkedDownload(dl);
		return;
	}
#endif

	// read the data
	size = MSG_ReadShort ();
	percent = MSG_ReadByte ();

	if (size == -2)
	{
		/*quakeforge*/
		MSG_ReadString();
		return;
	}
	if (size == -3)
	{
		char *requestedname;
		Q_strncpyz(name, MSG_ReadString(), sizeof(name));
		requestedname = MSG_ReadString();
		Con_DPrintf("Download for %s redirected to %s\n", requestedname, name);
		/*quakeforge http download redirection*/
		if (dl)
			CL_DownloadFailed(dl->remotename, dl);
		//FIXME: find some safe way to do this and actually test it. we should already know the local name, but we might have gained a .gz or something (this is quakeforge after all).
//		CL_CheckOrEnqueDownloadFile(name, localname, DLLF_IGNOREFAILED);
		return;
	}

	if (cls.demoplayback && cls.demoplayback != DPB_EZTV)
	{
		if (size > 0)
			msg_readcount += size;
		return; // not in demo playback, we don't know the name of the file.
	}
	if (!dl)
	{
		//download packet without file requested.
		if (size > 0)
			msg_readcount += size;
		return; // not in demo playback
	}

	if (size < 0)
	{
		Con_TPrintf ("File not found.\n");

		if (dl)
			CL_DownloadFailed(dl->remotename, dl);
		return;
	}

	// open the file if not opened yet
	if (dl->method == DL_QWPENDING)
	{
		dl->method = DL_QW;
		if (!DL_Begun(dl))
		{
			msg_readcount += size;
			Con_TPrintf ("Failed to open %s\n", dl->tempname);
			CL_DownloadFailed(dl->remotename, dl);
			CL_RequestNextDownload ();
			return;
		}
		SCR_EndLoadingPlaque();
	}

#ifdef PEXT_ZLIBDL
	if (percent >= 101 && percent <= 201)// && cls.fteprotocolextensions & PEXT_ZLIBDL)
	{
		int compsize;

		percent = percent - 101;

		VFS_WRITE (cls.download, ZLibDownloadDecode(&compsize, net_message.data + msg_readcount, size), size);

		msg_readcount += compsize;
	}
	else
#endif
	{
		VFS_WRITE (dl->file, net_message.data + msg_readcount, size);
		msg_readcount += size;
	}

	dl->completedbytes += size;
	dl->ratebytes += size;
	if (dl->percent != percent)	//try and guess the size (its most acurate when the percent value changes)
		dl->size = ((float)dl->completedbytes*100)/percent;

	if (percent != 100 && size == 0 && cl_dlemptyterminate.ival)
	{
		Con_Printf(CON_WARNING "WARNING: Client received empty svc_download, assuming EOF.\n");
		percent = 100;
	}

	if (percent != 100)
	{
		// request next block
		dl->percent = percent;

		CL_SendClientCommand(true, "nextdl");
	}
	else
	{
		Con_DPrintf("Download took %i seconds\n", (int)(Sys_DoubleTime() - dl->starttime));

		CL_DownloadFinished(dl);

		// get another file if needed

		CL_RequestNextDownload ();
	}
}

qboolean CL_ParseOOBDownload(void)
{
	qdownload_t *dl = cls.download;
	if (!dl)
		return false;

	if (MSG_ReadLong() != dl->filesequence)
		return false;

	if (MSG_ReadChar() != svc_download)
		return false;

	CL_ParseDownload();
	return true;
}

void CLDP_ParseDownloadData(void)
{
	qdownload_t *dl = cls.download;
	unsigned char buffer[1<<16];
	qofs_t start;
	int size;
	start = MSG_ReadLong();
	size = (unsigned short)MSG_ReadShort();

	MSG_ReadData(buffer, size);

	if (!dl)
		return;

	if (dl->file)
	{
		VFS_SEEK(dl->file, start);
		VFS_WRITE(dl->file, buffer, size);

		dl->percent = (start+size) / (float)VFS_GETLEN(dl->file) * 100;
	}

	//this is only reliable because I'm lazy
	MSG_WriteByte(&cls.netchan.message, clcdp_ackdownloaddata);
	MSG_WriteLong(&cls.netchan.message, start);
	MSG_WriteShort(&cls.netchan.message, size);
}

void CLDP_ParseDownloadBegin(char *s)
{
	qdownload_t *dl = cls.download;
	char buffer[8192];
	qofs_t size, pos, chunk;
	char *fname;
	Cmd_TokenizeString(s+1, false, false);
	size = (qofs_t)strtoull(Cmd_Argv(1), NULL, 0);
	fname = Cmd_Argv(2);

	if (!dl || strcmp(fname, dl->remotename))
	{
		Con_Printf("Warning: server started sending a file we did not request. Ignoring.\n");
		return;
	}

	if (dl->method != DL_DARKPLACES)
	{
		Con_Printf("Warning: download method isn't right.\n");
		return;
	}

	dl->sizeunknown = false;
	dl->size = size;
	if (!DL_Begun(dl))
	{
		CL_DownloadFailed(dl->remotename, dl);
		return;
	}

	CL_SendClientCommand(true, "sv_startdownload");

	//fill the file with 0 bytes, for some reason
	memset(buffer, 0, sizeof(buffer));
	for (pos = 0, chunk = 1; chunk; pos += chunk)
	{
		chunk = size - pos;
		if (chunk > sizeof(buffer))
			chunk = sizeof(buffer);
		VFS_WRITE(dl->file, buffer, chunk);
	}
}

void CLDP_ParseDownloadFinished(char *s)
{
	qdownload_t *dl = cls.download;
	unsigned short runningcrc = 0;
	char buffer[8192];
	int size, pos, chunk;
	if (!dl || !dl->file)
		return;

	Cmd_TokenizeString(s+1, false, false);

	VFS_CLOSE (dl->file);

	dl->file = FS_OpenVFS (dl->tempname+dl->prefixbytes, "rb", dl->fsroot);
	if (dl->file)
	{
		size = dl->size;
		QCRC_Init(&runningcrc);
		for (pos = 0, chunk = 1; chunk; pos += chunk)
		{
			chunk = size - pos;
			if (chunk > sizeof(buffer))
				chunk = sizeof(buffer);
			VFS_READ(dl->file, buffer, chunk);
			QCRC_AddBlock(&runningcrc, buffer, chunk);
		}
		VFS_CLOSE (dl->file);
		dl->file = NULL;
	}
	else
	{
		Con_Printf("Download failed: unable to check CRC of download\n");
		CL_DownloadFailed(dl->remotename, dl);
		return;
	}

	Cmd_TokenizeString(s+1, false, false);
	if (size != atoi(Cmd_Argv(1)))
	{
		Con_Printf("Download failed: wrong file size\n");
		CL_DownloadFailed(dl->remotename, dl);
		return;
	}
	if (runningcrc != atoi(Cmd_Argv(2)))
	{
		Con_Printf("Download failed: wrong crc\n");
		CL_DownloadFailed(dl->remotename, dl);
		return;
	}

	Con_DPrintf("Download took %i seconds\n", (int)(Sys_DoubleTime() - dl->starttime));

	CL_DownloadFinished(dl);

	// get another file if needed

	CL_RequestNextDownload ();
}

static vfsfile_t *upload_file;
static qbyte *upload_data;
static int upload_pos;
static int upload_size;

void CL_NextUpload(void)
{
	qbyte	buffer[1024];
	int		r;
	int		percent;
	int		size;

	r = upload_size - upload_pos;
	if (r > 768)
		r = 768;

	if (upload_data)
	{
		memcpy(buffer, upload_data + upload_pos, r);
	}
	else if (upload_file)
	{
		r = VFS_READ(upload_file, buffer, r);
		if (r == 0)
		{
			CL_StopUpload();
			return;
		}
	}
	else
		return;
	MSG_WriteByte (&cls.netchan.message, clc_upload);
	MSG_WriteShort (&cls.netchan.message, r);

	upload_pos += r;
	size = upload_size;
	if (!size)
		size = 1;
	percent = upload_pos*100/size;
	MSG_WriteByte (&cls.netchan.message, percent);
	SZ_Write (&cls.netchan.message, buffer, r);

Con_DPrintf ("UPLOAD: %6d: %d written\n", upload_pos - r, r);

	if (upload_pos != upload_size)
		return;

	Con_TPrintf ("Upload completed\n");

	CL_StopUpload();
}

void CL_StartUpload (qbyte *data, int size)
{
	if (cls.state < ca_onserver)
		return; // gotta be connected

	// override
	CL_StopUpload();

Con_DPrintf("Upload starting of %d...\n", size);

	upload_data = BZ_Malloc(size);
	memcpy(upload_data, data, size);
	upload_size = size;
	upload_pos = 0;

	CL_NextUpload();
}

qboolean CL_IsUploading(void)
{
	if (upload_data || upload_file)
		return true;
	return false;
}

void CL_StopUpload(void)
{
	if (upload_data)
		BZ_Free(upload_data);
	if (upload_file)
		VFS_CLOSE(upload_file);
	upload_file = NULL;
	upload_data = NULL;
	upload_pos = upload_size = 0;
}

qboolean CL_StartUploadFile(char *filename)
{
	if (!COM_CheckParm("-fileul"))
	{
		Con_Printf("You must currently use the -fileul commandline parameter in order to use this functionality\n");
		return false;
	}

	if (cls.state < ca_onserver)
		return false; // gotta be connected

	CL_StopUpload();

	upload_file = FS_OpenVFS(filename, "rb", FS_ROOT);
	upload_size = VFS_GETLEN(upload_file);
	upload_pos = 0;

	if (upload_file)
	{
		CL_NextUpload();
		return true;
	}
	return false;
}

/*
=====================================================================

  SERVER CONNECTING MESSAGES

=====================================================================
*/
#ifdef CLIENTONLY
float nextdemotime;
#endif

void CL_ClearParseState(void)
{
	// done with sounds, request models now
	memset (cl.model_precache, 0, sizeof(cl.model_precache));
	cl_playerindex = -1;
	cl_h_playerindex = -1;
	cl_spikeindex = -1;
	cl_flagindex = -1;
	cl_rocketindex = -1;
	cl_grenadeindex = -1;
	cl_gib1index = -1;
	cl_gib2index = -1;
	cl_gib3index = -1;

	if (cl_baselines)
	{
		BZ_Free(cl_baselines);
		cl_baselines = NULL;
	}
	cl_baselines_count = 0;

	cl_max_static_entities = 0;
	if (cl_static_entities)
	{
		BZ_Free(cl_static_entities);
		cl_static_entities = NULL;
	}
}

/*
==================
CL_ParseServerData
==================
*/
void CLQW_ParseServerData (void)
{
	int pnum;
	int clnum;
	char	*str;
	int protover, svcnt;

	float maxspeed, entgrav;

	if (cls.download && cls.download->method == DL_QWPENDING)
	{
		//if we didn't actually start downloading it yet, cancel the current download.
		//this is to avoid qizmo not responding to the download command, resulting in hanging downloads that cause the client to then be unable to connect anywhere simply because someone's skin was set.
		CL_DownloadFailed(cls.download->remotename, cls.download);
	}

	Con_DPrintf ("Serverdata packet received.\n");
//
// wipe the client_state_t struct
//

	SCR_SetLoadingStage(LS_CLIENT);
	SCR_BeginLoadingPlaque();

// parse protocol version number
// allow 2.2 and 2.29 demos to play
#ifdef PROTOCOL_VERSION_FTE
	cls.fteprotocolextensions=0;
	cls.fteprotocolextensions2=0;
	for(;;)
	{
		protover = MSG_ReadLong ();
		if (protover == PROTOCOL_VERSION_FTE)
		{
			cls.fteprotocolextensions = MSG_ReadLong();
			continue;
		}
		if (protover == PROTOCOL_VERSION_FTE2)
		{
			cls.fteprotocolextensions2 = MSG_ReadLong();
			continue;
		}
		if (protover == PROTOCOL_VERSION_VARLENGTH)
		{
			int ident;
			int len;
			char data[1024];
			ident = MSG_ReadLong();
			len = MSG_ReadLong();
			if (len <= sizeof(data))
			{
				MSG_ReadData(data, len);
				switch(ident)
				{
				default:
					break;
				}
				continue;
			}
		}
		if (protover == PROTOCOL_VERSION_QW)	//this ends the version info
			break;
		if (cls.demoplayback && (protover == 26 || protover == 27 || protover == 28))	//older versions, maintain demo compatability.
			break;
		Host_EndGame ("Server returned version %i, not %i\n", protover, PROTOCOL_VERSION_QW);
	}
#else
	protover = MSG_ReadLong ();
	if (protover != PROTOCOL_VERSION_QW &&
		!(cls.demoplayback && (protover == 26 || protover == 27 || protover == 28)))
		Host_EndGame ("Server returned version %i, not %i\n", protover, PROTOCOL_VERSION_QW);
#endif

	if (cls.fteprotocolextensions2||cls.fteprotocolextensions)
		if (developer.ival || cl_shownet.ival)
			Con_TPrintf ("Using FTE extensions 0x%x%08x\n", cls.fteprotocolextensions2, cls.fteprotocolextensions);

	if (cls.fteprotocolextensions & PEXT_FLOATCOORDS)
	{
		cls.netchan.netprim.coordsize = 4;
		cls.netchan.netprim.anglesize = 2;
	}
	else
	{
		cls.netchan.netprim.coordsize = 2;
		cls.netchan.netprim.anglesize = 1;
	}
	cls.netchan.message.prim = cls.netchan.netprim;
	MSG_ChangePrimitives(cls.netchan.netprim);

	svcnt = MSG_ReadLong ();

	// game directory
	str = MSG_ReadString ();
	Con_DPrintf("Server is using gamedir \"%s\"\n", str);
	if (!*str)
		str = "qw";

#ifndef CLIENTONLY
	if (!sv.state)
#endif
	{
		COM_FlushTempoaryPacks();
		COM_Gamedir(str, NULL);
#ifndef CLIENTONLY
		Info_SetValueForStarKey (svs.info, "*gamedir", str, MAX_SERVERINFO_STRING);
#endif
		Cvar_ForceCallback(Cvar_FindVar("r_particlesdesc"));
	}

	CL_ClearState ();
	Stats_NewMap();
	cl.servercount = svcnt;

	cl.teamfortress = !Q_strcasecmp(str, "fortress");

	if (cl.gamedirchanged)
	{
		cl.gamedirchanged = false;
#ifndef CLIENTONLY
		if (!sv.state)
#endif
			Wads_Flush();
	}

	/*mvds have different parsing*/
	if (cls.demoplayback == DPB_MVD || cls.demoplayback == DPB_EZTV)
	{
		extern float olddemotime;
		int i,j;

		if (cls.fteprotocolextensions2 & PEXT2_MAXPLAYERS)
		{
			cl.allocated_client_slots = MSG_ReadByte();
			if (cl.allocated_client_slots > MAX_CLIENTS)
				cl.allocated_client_slots = MAX_CLIENTS;
		}

		cl.gametime = MSG_ReadFloat();
		cl.gametimemark = realtime;
		cl.oldgametime = cl.gametime;
		cl.oldgametimemark = realtime;

		cl.demogametimebias = cl.gametime - olddemotime;

		for (j = 0; j < MAX_SPLITS; j++)
		{
			cl.playerview[j].playernum = cl.allocated_client_slots + j;
			cl.playerview[j].viewentity = 0;	//free floating.
			for (i = 0; i < UPDATE_BACKUP; i++)
			{
				cl.inframes[i].playerstate[cl.playerview[j].playernum].pm_type = PM_SPECTATOR;
				cl.inframes[i].playerstate[cl.playerview[j].playernum].messagenum = 1;
			}
		}
		cl.spectator = true;

		cl.splitclients = 1;
	}
	else if (cls.fteprotocolextensions2 & PEXT2_MAXPLAYERS)
	{
		cl.allocated_client_slots = MSG_ReadByte();
		if (cl.allocated_client_slots > MAX_CLIENTS)
		{
			cl.allocated_client_slots = MAX_CLIENTS;
			Host_EndGame("Server sent us too many alternate clients\n");
		}

		/*parsing here is slightly different to allow us 255 max players instead of 127*/
		cl.splitclients = MSG_ReadByte();
		if (cl.splitclients & 128)
		{
			cl.spectator = true;
			cl.splitclients &= ~128;
		}
		if (cl.splitclients > MAX_SPLITS)
			Host_EndGame("Server sent us too many alternate clients\n");
		for (pnum = 0; pnum < cl.splitclients; pnum++)
		{
			if (cls.z_ext & Z_EXT_VIEWHEIGHT)
				cl.playerview[pnum].viewheight = 0;
			cl.playerview[pnum].playernum = MSG_ReadByte();
			if (cl.playerview[pnum].playernum >= cl.allocated_client_slots)
				Host_EndGame("unsupported local player slot\n");
			cl.playerview[pnum].viewentity = cl.playerview[pnum].playernum+1;
		}
	}
	else 
	{
		// parse player slot, high bit means spectator
		pnum = MSG_ReadByte ();
		for (clnum = 0; ; clnum++)
		{
			if (clnum == MAX_SPLITS)
				Host_EndGame("Server sent us too many alternate clients\n");
			if (cls.z_ext & Z_EXT_VIEWHEIGHT)
				cl.playerview[pnum].viewheight = 0;
			cl.playerview[clnum].playernum = pnum;
			if (cl.playerview[clnum].playernum & 128)
			{
				cl.spectator = true;
				cl.playerview[clnum].playernum &= ~128;
			}

			if (cl.playerview[clnum].playernum >= cl.allocated_client_slots)
				Host_EndGame("unsupported local player slot\n");

			cl.playerview[clnum].viewentity = cl.playerview[clnum].playernum+1;
			if (!(cls.fteprotocolextensions & PEXT_SPLITSCREEN))
				break;

			pnum = MSG_ReadByte ();
			if (pnum == 128)
				break;
		}
		cl.splitclients = clnum+1;
	}

	// get the full level name
	str = MSG_ReadString ();
	Q_strncpyz (cl.levelname, str, sizeof(cl.levelname));

	// get the movevars
	movevars.gravity			= MSG_ReadFloat();
	movevars.stopspeed			= MSG_ReadFloat();
	maxspeed					= MSG_ReadFloat();
	movevars.spectatormaxspeed	= MSG_ReadFloat();
	movevars.accelerate			= MSG_ReadFloat();
	movevars.airaccelerate		= MSG_ReadFloat();
	movevars.wateraccelerate	= MSG_ReadFloat();
	movevars.friction			= MSG_ReadFloat();
	movevars.waterfriction		= MSG_ReadFloat();
	entgrav						= MSG_ReadFloat();

	for (clnum = 0; clnum < cl.splitclients; clnum++)
	{
		cl.playerview[clnum].maxspeed = maxspeed;
		cl.playerview[clnum].entgravity = entgrav;
	}

	// seperate the printfs so the server message can have a color
#if 1
	Con_Printf ("\n\n");
	Con_Printf ("^Ue01d^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01f");
	Con_Printf ("\n\n");
	Con_Printf ("\1%s\n", str);
#else
	Con_TPrintf ("\n\n^Ue01d^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01f\n\n");
	Con_Printf ("%c%s\n", 2, str);
#endif

	if (CL_RemoveClientCommands("new"))	//mvdsv is really appaling some times.
	{
	//	Con_Printf("Multiple 'new' commands?!?!? This server needs reinstalling!\n");
	}

	memset(cl.sound_name, 0, sizeof(cl.sound_name));
#ifdef PEXT_PK3DOWNLOADS
	if (cls.fteprotocolextensions & PEXT_PK3DOWNLOADS)	//instead of going for a soundlist, go for the pk3 list instead. The server will make us go for the soundlist after.
	{
		if (CL_RemoveClientCommands("pk3list"))
			Con_DPrintf("Multiple pk3lists\n");
		CL_SendClientCommand ("pk3list %i 0", cl.servercount, 0);
	}
	else
#endif
	{
		if (cls.demoplayback == DPB_EZTV)
		{
			if (CL_RemoveClientCommands("qtvsoundlist"))
				Con_DPrintf("Multiple soundlists\n");
			CL_SendClientCommand (true, "qtvsoundlist %i 0", cl.servercount);
		}
		else
		{
			if (CL_RemoveClientCommands("soundlist"))
				Con_DPrintf("Multiple soundlists\n");
			// ask for the sound list next
//			CL_SendClientCommand ("soundlist %i 0", cl.servercount);
			CL_SendClientCommand (true, soundlist_name, cl.servercount, 0);
		}
	}

	// now waiting for downloads, etc
	cls.state = ca_onserver;
	Cam_AutoTrack_Update(NULL);

	cl.sendprespawn = false;

#ifdef VOICECHAT
	S_Voip_MapChange();
#endif

#ifdef VM_CG
	CG_Stop();
#endif
#ifdef CSQC_DAT
	CSQC_Shutdown();	//revive it when we get the serverinfo saying the checksum.
#endif
}

#ifdef Q2CLIENT
void CLQ2_ParseServerData (void)
{
	char	*str;
	int		i;
	int svcnt;
//	int cflag;

	cls.netchan.netprim.coordsize = 2;
	cls.netchan.netprim.anglesize = 1;
	MSG_ChangePrimitives(cls.netchan.netprim);

	Con_DPrintf ("Serverdata packet received.\n");
//
// wipe the client_state_t struct
//
	SCR_SetLoadingStage(LS_CLIENT);
	SCR_BeginLoadingPlaque();
//	CL_ClearState ();
	cls.state = ca_onserver;

// parse protocol version number
	i = MSG_ReadLong ();
//	cls.serverProtocol = i;

	if (i > PROTOCOL_VERSION_Q2 || i < PROTOCOL_VERSION_Q2_MIN)
		Host_EndGame ("Server returned version %i, not %i", i, PROTOCOL_VERSION_Q2);

	svcnt = MSG_ReadLong ();
	/*cl.attractloop =*/ MSG_ReadByte ();

	// game directory
	str = MSG_ReadString ();
//	strncpy (cl.gamedir, str, sizeof(cl.gamedir)-1);

	// set gamedir
	if (!*str)
		COM_Gamedir("baseq2", NULL);
	else
		COM_Gamedir(str, NULL);
//	if ((*str && (!fs_gamedirvar->string || !*fs_gamedirvar->string || strcmp(fs_gamedirvar->string, str))) || (!*str && (fs_gamedirvar->string || *fs_gamedirvar->string)))
//		Cvar_Set("game", str);

	Cvar_Get("timescale", "1", 0, "Q2Admin hacks");	//Q2Admin will kick players who have a timescale set to something other than 1
													//FTE doesn't actually have a timescale cvar, so create one to fool q2admin.
													//I can't really blame q2admin for rejecting engines that don't have this cvar, as it could have been renamed via a hex-edit.

	CL_ClearState ();
	CLQ2_ClearState ();
	cl.minpitch = -89;
	cl.maxpitch = 89;
	cl.servercount = svcnt;
	Cam_AutoTrack_Update(NULL);

	Stats_NewMap();


	// parse player entity number
	cl.playerview[0].playernum = MSG_ReadShort ();
	cl.playerview[0].viewentity = cl.playerview[0].playernum+1;
	cl.splitclients = 1;
	cl.spectator = false;

	cl.numq2visibleweapons = 1;	//give it a default.
	cl.q2visibleweapons[0] = "weapon.md2";

	// get the full level name
	str = MSG_ReadString ();
	Q_strncpyz (cl.levelname, str, sizeof(cl.levelname));

	if (cl.playerview[0].playernum == -1)
	{	// playing a cinematic or showing a pic, not a level
		SCR_EndLoadingPlaque();
		if (!Media_PlayFilm(str, false))
		{
			CL_SendClientCommand(true, "nextserver %i", cl.servercount);
		}

		CL_MakeActive("Quake2");
	}
	else
	{
		// seperate the printfs so the server message can have a color
		Con_TPrintf ("\n\n^Ue01d^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01f\n\n");
		Con_Printf ("%c%s\n", 2, str);

		Media_StopFilm(true);

		// need to prep refresh at next oportunity
		//cl.refresh_prepped = false;
	}

	Cvar_ForceCallback(Cvar_FindVar("r_particlesdesc"));

	Surf_PreNewMap();
}
#endif


void CL_ParseEstablished(void)
{
#ifdef NQPROT
	cl_dp_serverextension_download = false;
	cl_dp_csqc_progscrc = 0;
	cl_dp_csqc_progssize = 0;
#endif
}

#ifdef NQPROT
void CLNQ_ParseProtoVersion(void)
{
	int protover;
	struct netprim_s netprim;

	cls.fteprotocolextensions = 0;
	cls.fteprotocolextensions2 = 0;
	for(;;)
	{
		protover = MSG_ReadLong ();
		switch(protover)
		{
		case PROTOCOL_VERSION_FTE:
			cls.fteprotocolextensions = MSG_ReadLong();
			continue;
		case PROTOCOL_VERSION_FTE2:
			cls.fteprotocolextensions2 = MSG_ReadLong();
			continue;
		default:
			break;
		}
		break;
	}

	netprim.coordsize = 2;
	netprim.anglesize = 1;

	cls.protocol_nq = (cls.protocol_nq==CPNQ_PROQUAKE3_4)?CPNQ_PROQUAKE3_4:CPNQ_ID;
	cls.z_ext = 0;

	if (protover == NEHD_PROTOCOL_VERSION)
		Host_EndGame ("Nehahra demo net protocol is not supported\n");
	else if (protover == FITZ_PROTOCOL_VERSION)
	{
		//fitzquake 0.85
		cls.protocol_nq = CPNQ_FITZ666;
		Con_DPrintf("FitzQuake 666 protocol\n");
	}
	else if (protover == RMQ_PROTOCOL_VERSION)
	{
		int fl;
		cls.protocol_nq = CPNQ_FITZ666;
		Con_DPrintf("RMQ extensions to FitzQuake's protocol\n");
		fl = MSG_ReadLong();

		if (fl & RMQFL_SHORTANGLE)
			netprim.anglesize = 2;
		if (fl & RMQFL_FLOATANGLE)
			netprim.anglesize = 4;
		if (fl & RMQFL_24BITCOORD)
			netprim.coordsize = 3;
		if (fl & RMQFL_FLOATCOORD)
			netprim.coordsize = 4;
		if (fl & ~(RMQFL_SHORTANGLE|RMQFL_FLOATANGLE|RMQFL_24BITCOORD|RMQFL_FLOATCOORD|RMQFL_EDICTSCALE))
			Con_Printf("WARNING: Server is using unsupported RMQ extensions\n");
	}
	else if (protover == DP5_PROTOCOL_VERSION)
	{
		//darkplaces5
		cls.protocol_nq = CPNQ_DP5;
		netprim.coordsize = 4;
		netprim.anglesize = 2;

		Con_DPrintf("DP5 protocols\n");
	}
	else if (protover == DP6_PROTOCOL_VERSION)
	{
		//darkplaces6 (it's a small difference from dp5)
		cls.protocol_nq = CPNQ_DP6;
		netprim.coordsize = 4;
		netprim.anglesize = 2;

		cls.z_ext = Z_EXT_VIEWHEIGHT;

		Con_DPrintf("DP6 protocols\n");
	}
	else if (protover == DP7_PROTOCOL_VERSION)
	{
		//darkplaces7 (it's a small difference from dp5)
		cls.protocol_nq = CPNQ_DP7;
		netprim.coordsize = 4;
		netprim.anglesize = 2;

		cls.z_ext = Z_EXT_VIEWHEIGHT;

		Con_DPrintf("DP7 protocols\n");
	}
	else if (protover == H2_PROTOCOL_VERSION)
	{
		Host_EndGame ("\nUnable to connect to standard Hexen2 servers. Host the game with "DISTRIBUTION"\n");
	}
	else if (protover != NQ_PROTOCOL_VERSION)
	{
		Host_EndGame ("Server is using protocol version %i, which is not supported by this version of " FULLENGINENAME ".", protover);
	}
	else
	{
		Con_DPrintf("Standard NQ protocols\n");
	}
	if (cls.fteprotocolextensions & PEXT_FLOATCOORDS)
	{
		if (netprim.anglesize < 2)
			netprim.anglesize = 2;
		if (netprim.coordsize < 4)
			netprim.coordsize = 4;
	}
	cls.netchan.message.prim = cls.netchan.netprim = netprim;
	MSG_ChangePrimitives(netprim);
}

//FIXME: move to header
void CL_KeepaliveMessage(void){}
void CLNQ_ParseServerData(void)		//Doesn't change gamedir - use with caution.
{
	int	nummodels, numsounds;
	char	*str;
	int gametype;
	if (developer.ival)
		Con_TPrintf ("Serverdata packet received.\n");
	SCR_SetLoadingStage(LS_CLIENT);
	CL_ClearState ();
	Stats_NewMap();
	Cvar_ForceCallback(Cvar_FindVar("r_particlesdesc"));

	CLNQ_ParseProtoVersion();

	if (MSG_ReadByte() > MAX_CLIENTS)
	{
		Con_Printf ("\nWarning, this server supports more than %i clients, additional clients will do bad things\n", MAX_CLIENTS);
	}

	cl.splitclients = 1;

	gametype = MSG_ReadByte ();

	str = MSG_ReadString ();
	Q_strncpyz (cl.levelname, str, sizeof(cl.levelname));

	// seperate the printfs so the server message can have a color
#if 1
	Con_Printf ("\n\n");
	Con_Printf ("^Ue01d^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01f");
	Con_Printf ("\n\n");
	Con_Printf ("\1%s\n", str);
#else
	Con_TPrintf ("\n\n^Ue01d^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01e^Ue01f\n\n");
	Con_Printf ("%c%s\n", 2, str);
#endif

	SCR_BeginLoadingPlaque();

	Surf_PreNewMap();

	memset (cl.model_name, 0, sizeof(cl.model_name));
	for (nummodels=1 ; ; nummodels++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (nummodels==MAX_PRECACHE_MODELS)
		{
			Con_TPrintf ("Server sent too many model precaches\n");
			return;
		}
		strcpy (cl.model_name[nummodels], str);
		if (*str != '*' && strcmp(str, "null"))	//not inline models!
			CL_CheckOrEnqueDownloadFile(str, NULL, ((nummodels==1)?DLLF_REQUIRED:0));
		Mod_TouchModel (str);
	}

	memset (cl.sound_name, 0, sizeof(cl.sound_name));
	for (numsounds=1 ; ; numsounds++)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		if (numsounds==MAX_PRECACHE_SOUNDS)
		{
			Con_TPrintf ("Server sent too many sound precaches\n");
			return;
		}
		strcpy (cl.sound_name[numsounds], str);

		Sound_CheckDownload(str);
		S_TouchSound (str);
	}

	cls.signon = 0;
	cls.state = ca_onserver;
	Cam_AutoTrack_Update(NULL);


	//fill in the csqc stuff
	if (!cl_dp_csqc_progscrc)
	{
		Info_RemoveKey(cl.serverinfo, "*csprogs");
		Info_RemoveKey(cl.serverinfo, "*csprogssize");
		Info_RemoveKey(cl.serverinfo, "*csprogsname");
	}
	else
	{
		Info_SetValueForStarKey(cl.serverinfo, "*csprogs", va("%i", cl_dp_csqc_progscrc), sizeof(cl.serverinfo));
		Info_SetValueForStarKey(cl.serverinfo, "*csprogssize", va("%i", cl_dp_csqc_progssize), sizeof(cl.serverinfo));
		Info_SetValueForStarKey(cl.serverinfo, "*csprogsname", va("%s", cl_dp_csqc_progsname), sizeof(cl.serverinfo));
	}

	//update gamemode
	if (gametype != GAME_COOP)
		Info_SetValueForStarKey(cl.serverinfo, "deathmatch", "1", sizeof(cl.serverinfo));
	else
		Info_SetValueForStarKey(cl.serverinfo, "deathmatch", "0", sizeof(cl.serverinfo));
	Info_SetValueForStarKey(cl.serverinfo, "teamplay", "0", sizeof(cl.serverinfo));

	//allow some things by default that quakeworld bans by default
	Info_SetValueForStarKey(cl.serverinfo, "watervis", "1", sizeof(cl.serverinfo));
	Info_SetValueForStarKey(cl.serverinfo, "mirrors", "1", sizeof(cl.serverinfo));

	//prohibit some things that QW/FTE has enabled by default
	Info_SetValueForStarKey(cl.serverinfo, "fbskins", "0", sizeof(cl.serverinfo));

	//pretend it came from the server, and update cheat/permissions/etc
	CL_CheckServerInfo();

	#if _MSC_VER > 1200
	Sys_RecentServer("+nqconnect", cls.servername, cls.servername, "Join NQ Server");
	#endif

#ifdef PEXT_CSQC
	CSQC_Shutdown();
	CSQC_Init(cls.demoplayback, false, 0);
#endif
}
void CLNQ_SignonReply (void)
{
	extern cvar_t	topcolor;
	extern cvar_t	bottomcolor;
	extern cvar_t	rate;
	extern cvar_t	model;
	extern cvar_t	skin;

Con_DPrintf ("CL_SignonReply: %i\n", cls.signon);

	switch (cls.signon)
	{
	case 1:
		cl.sendprespawn = true;
		SCR_SetLoadingFile("loading data");
		CL_RequestNextDownload();
		break;

	case 2:
		CL_SendClientCommand(true, "name \"%s\"\n", name.string);

		CL_SendClientCommand(true, "color %i %i\n", topcolor.ival, bottomcolor.ival);

		CL_SendClientCommand(true, "spawn %s", "");

		if (CPNQ_IS_DP)	//dp needs a couple of extras to work properly.
		{
			CL_SendClientCommand(true, "rate %s", rate.string);

			CL_SendClientCommand(true, "playermodel %s", model.string);
			CL_SendClientCommand(true, "playerskin %s", skin.string);
			/*
#ifdef PEXT_CSQC
			{
				char *s;
				s = Info_ValueForKey(cl.serverinfo, "*csprogs");
				CSQC_Init(false, *s?true:false, atoi(s));
			}
#endif
			*/
		}
		break;

	case 3:
		CL_SendClientCommand(true, "begin");
#ifdef VM_CG
		CG_Start();
#endif
		break;

	case 4:
		SCR_EndLoadingPlaque ();		// allow normal screen updates
		SCR_SetLoadingStage(LS_NONE);
		break;
	}
}

#define	DEFAULT_VIEWHEIGHT	22
void CLNQ_ParseClientdata (void)
{
	int		i;
	player_state_t *pl = &cl.inframes[cl.validsequence&UPDATE_MASK].playerstate[cl.playerview[0].playernum];

	unsigned int bits;

	bits = (unsigned short)MSG_ReadShort();

	if (bits & SU_EXTEND1)
		bits |= (MSG_ReadByte() << 16);
	if (bits & SU_EXTEND2)
		bits |= (MSG_ReadByte() << 24);

	if (bits & SU_VIEWHEIGHT)
		CL_SetStatInt(0, STAT_VIEWHEIGHT, MSG_ReadChar ());
	else if (!CPNQ_IS_DP || cls.protocol_nq <= CPNQ_DP5)
		CL_SetStatInt(0, STAT_VIEWHEIGHT, DEFAULT_VIEWHEIGHT);

	if (bits & SU_IDEALPITCH)
		/*cl.idealpitch =*/ MSG_ReadChar ();
	/*else
		cl.idealpitch = 0;*/

	for (i=0 ; i<3 ; i++)
	{
		if (bits & (SU_PUNCH1<<i) )
			/*cl.punchangle[i] =*/ CPNQ_IS_DP?MSG_ReadAngle16():MSG_ReadChar();
//		else
//			cl.punchangle[i] = 0;

		if (CPNQ_IS_DP && bits & (DPSU_PUNCHVEC1<<i))
		{
			/*cl.punchvector[i] =*/ MSG_ReadCoord();
		}
//		else
//			cl.punchvector[i] = 0;

		if (bits & (SU_VELOCITY1<<i))
		{
			if (CPNQ_IS_DP)
				pl->velocity[i] = MSG_ReadFloat();
			else
				pl->velocity[i] = MSG_ReadChar()*16;
		}
		else
			pl->velocity[i] = 0;
	}

	if ((bits & SU_ITEMS) || cls.protocol_nq == CPNQ_ID)	//hipnotic bug - hipnotic demos don't always have SU_ITEMS set, yet they update STAT_ITEMS anyway.
		CL_SetStatInt(0, STAT_ITEMS, MSG_ReadLong());

	pl->onground = (bits & SU_ONGROUND) != 0;
//	cl.inwater = (bits & SU_INWATER) != 0;

	if (cls.protocol_nq == CPNQ_DP5)
	{
		CL_SetStatInt(0, STAT_WEAPONFRAME, (bits & SU_WEAPONFRAME)?(unsigned short)MSG_ReadShort():0);
		CL_SetStatInt(0, STAT_ARMOR, (bits & SU_ARMOR)?MSG_ReadShort():0);
		CL_SetStatInt(0, STAT_WEAPONMODELI, (bits & SU_WEAPONMODEL)?MSG_ReadShort():0);

		CL_SetStatInt(0, STAT_HEALTH, MSG_ReadShort());

		CL_SetStatInt(0, STAT_AMMO, MSG_ReadShort());

		CL_SetStatInt(0, STAT_SHELLS, MSG_ReadShort());
		CL_SetStatInt(0, STAT_NAILS, MSG_ReadShort());
		CL_SetStatInt(0, STAT_ROCKETS, MSG_ReadShort());
		CL_SetStatInt(0, STAT_CELLS, MSG_ReadShort());

		CL_SetStatInt(0, STAT_ACTIVEWEAPON, (unsigned short)MSG_ReadShort());
	}
	else if (CPNQ_IS_DP && cls.protocol_nq > CPNQ_DP5)
	{
		/*nothing in dp6+*/
	}
	else
	{
		int weaponmodel = 0, armor = 0, weaponframe = 0, health = 0, currentammo = 0, shells = 0, nails = 0, rockets = 0, cells = 0, activeweapon = 0;

		if (bits & SU_WEAPONFRAME)	weaponframe |= (unsigned char)MSG_ReadByte();
		if (bits & SU_ARMOR)		armor |= (unsigned char)MSG_ReadByte();
		if (bits & SU_WEAPONMODEL)	weaponmodel |= (unsigned char)MSG_ReadByte();
		health |= MSG_ReadShort();
		currentammo |= MSG_ReadByte();
		shells |= MSG_ReadByte();
		nails |= MSG_ReadByte();
		rockets |= MSG_ReadByte();
		cells |= MSG_ReadByte();
		activeweapon |= MSG_ReadByte();

		if (cls.protocol_nq == CPNQ_FITZ666)
		{
			if (bits & FITZSU_WEAPONMODEL2)
				weaponmodel |= MSG_ReadByte() << 8;
			if (bits & FITZSU_ARMOR2)
				armor |= MSG_ReadByte() << 8;
			if (bits & FITZSU_AMMO2)
				currentammo |= MSG_ReadByte() << 8;
			if (bits & FITZSU_SHELLS2)
				shells |= MSG_ReadByte() << 8;
			if (bits & FITZSU_NAILS2)
				nails |= MSG_ReadByte() << 8;
			if (bits & FITZSU_ROCKETS2)
				rockets |= MSG_ReadByte() << 8;
			if (bits & FITZSU_CELLS2)
				cells |= MSG_ReadByte() << 8;
			if (bits & FITZSU_WEAPONFRAME2)
				weaponframe |= MSG_ReadByte() << 8;
			if (bits & FITZSU_WEAPONALPHA)
				MSG_ReadByte();
		}

		CL_SetStatInt(0, STAT_WEAPONFRAME, weaponframe);
		CL_SetStatInt(0, STAT_ARMOR, armor);
		CL_SetStatInt(0, STAT_WEAPONMODELI, weaponmodel);

		CL_SetStatInt(0, STAT_HEALTH, health);

		CL_SetStatInt(0, STAT_AMMO, currentammo);

		CL_SetStatInt(0, STAT_SHELLS, shells);
		CL_SetStatInt(0, STAT_NAILS, nails);
		CL_SetStatInt(0, STAT_ROCKETS, rockets);
		CL_SetStatInt(0, STAT_CELLS, cells);

		CL_SetStatInt(0, STAT_ACTIVEWEAPON, activeweapon);
	}

	if (CPNQ_IS_DP)
	{
		if (bits & DPSU_VIEWZOOM)
		{
			if (cls.protocol_nq >= CPNQ_DP5)
				i = (unsigned short) MSG_ReadShort();
			else
				i = MSG_ReadByte();
			if (i < 2)
				i = 2;
			CL_SetStatInt(0, STAT_VIEWZOOM, i);
		}
		else
			CL_SetStatInt(0, STAT_VIEWZOOM, 255);
	}
}
#endif
/*
==================
CL_ParseSoundlist
==================
*/
void CL_ParseSoundlist (qboolean lots)
{
	int	numsounds;
	char	*str;
	int n;

// precache sounds
//	memset (cl.sound_precache, 0, sizeof(cl.sound_precache));

	if (lots)
		numsounds = MSG_ReadShort();
	else
		numsounds = MSG_ReadByte();

	for (;;)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		numsounds++;
		if (numsounds >= MAX_PRECACHE_SOUNDS)
			Host_EndGame ("Server sent too many sound_precache");

//		if (strlen(str)>4)
//		if (!strcmp(str+strlen(str)-4, ".mp3"))	//don't let the server send us a specific mp3. convert it to wav and this way we know not to look outside the quake path for it.
//			strcpy(str+strlen(str)-4, ".wav");

		strcpy (cl.sound_name[numsounds], str);
	}

	n = MSG_ReadByte();

	if (n)
	{
		if (cls.demoplayback != DPB_EZTV)
		{
			if (CL_RemoveClientCommands("soundlist"))
				Con_DPrintf("Multiple soundlists\n");
//			CL_SendClientCommand("soundlist %i %i", cl.servercount, n);
			CL_SendClientCommand(true, soundlist_name, cl.servercount, (numsounds&0xff00) + n);
		}
		return;
	}

#ifdef Q2CLIENT
	if (cls.protocol == CP_QUAKE2)
	{
		CL_AllowIndependantSendCmd(false);	//stop it now, the indep stuff *could* require model tracing.

		cl.sendprespawn = true;
		SCR_SetLoadingFile("loading data");
	}
	else
#endif
	{
		if (cls.demoplayback == DPB_EZTV)
		{
			if (CL_RemoveClientCommands("qtvmodellist"))
				Con_DPrintf("Multiple modellists\n");
			CL_SendClientCommand (true, "qtvmodellist %i 0", cl.servercount);
		}
		else
		{
			if (CL_RemoveClientCommands("modellist"))
				Con_DPrintf("Multiple modellists\n");
//			CL_SendClientCommand ("modellist %i 0", cl.servercount);
			CL_SendClientCommand (true, modellist_name, cl.servercount, 0);
		}
	}
}

/*
==================
CL_ParseModellist
==================
*/
void CL_ParseModellist (qboolean lots)
{
	int	nummodels;
	char	*str;
	int n;

// precache models and note certain default indexes
	if (lots)
		nummodels = MSG_ReadShort();
	else
		nummodels = MSG_ReadByte();

	for (;;)
	{
		str = MSG_ReadString ();
		if (!str[0])
			break;
		nummodels++;
		if (nummodels>=MAX_PRECACHE_MODELS)
			Host_EndGame ("Server sent too many model_precache");
		strcpy (cl.model_name[nummodels], str);

		//qw has a special network protocol for spikes.
		if (!strcmp(cl.model_name[nummodels],"progs/spike.mdl"))
			cl_spikeindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/player.mdl"))
			cl_playerindex = nummodels;
		if (*cl.model_name_vwep[0] && !strcmp(cl.model_name[nummodels],cl.model_name_vwep[0]) && cl_playerindex == -1)
			cl_playerindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/h_player.mdl"))
			cl_h_playerindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/flag.mdl"))
			cl_flagindex = nummodels;

		//rocket to grenade
		if (!strcmp(cl.model_name[nummodels],"progs/missile.mdl"))
			cl_rocketindex = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/grenade.mdl"))
			cl_grenadeindex = nummodels;

		//cl_gibfilter
		if (!strcmp(cl.model_name[nummodels],"progs/gib1.mdl"))
			cl_gib1index = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/gib2.mdl"))
			cl_gib2index = nummodels;
		if (!strcmp(cl.model_name[nummodels],"progs/gib3.mdl"))
			cl_gib3index = nummodels;
	}

	if (nummodels)
		SCR_ImageName(cl.model_name[1]);

	n = MSG_ReadByte();

	if (n)
	{
		if (cls.demoplayback != DPB_EZTV)
		{
			if (CL_RemoveClientCommands("modellist"))
				Con_DPrintf("Multiple modellists\n");
//			CL_SendClientCommand("modellist %i %i", cl.servercount, n);
			CL_SendClientCommand(true, modellist_name, cl.servercount, (nummodels&0xff00) + n);
		}
		return;
	}

	Sound_CheckDownloads();
	Model_CheckDownloads();

	CL_AllowIndependantSendCmd(false);	//stop it now, the indep stuff *could* require model tracing.

	//set the flag to load models and send prespawn
	cl.sendprespawn = true;
	SCR_SetLoadingFile("loading data");
}

void CL_ProcessUserInfo (int slot, player_info_t *player);

#ifdef Q2CLIENT
void CLQ2_ParseClientinfo(int i, char *s)
{
	char *model, *name;
	player_info_t *player;
	//s contains "name\model/skin"

	player = &cl.players[i];

	*player->userinfo = '\0';

	model = strchr(s, '\\');
	if (model)
	{
		*model = '\0';
		model++;
		name = s;
	}
	else
	{
		name = "Unnammed";
		model = "male";
	}
#if 0
	skin = strchr(model, '/');
	if (skin)
	{
		*skin = '\0';
		skin++;
	}
	else
		skin = "";
	Info_SetValueForKey(player->userinfo, "model", model, MAX_INFO_STRING);
	Info_SetValueForKey(player->userinfo, "skin", skin, MAX_INFO_STRING);
#else
	Info_SetValueForKey(player->userinfo, "skin", model, sizeof(player->userinfo));
#endif
	Info_SetValueForKey(player->userinfo, "name", name, sizeof(player->userinfo));

	cl.players[i].userid = i;
	cl.players[i].rbottomcolor = 1;
	cl.players[i].rtopcolor = 1;
	CL_ProcessUserInfo (i, player);
}

void CLQ2_ParseConfigString (void)
{
	int		i;
	char	*s;
//	char	olds[MAX_QPATH];

	i = MSG_ReadShort ();
	if (i < 0 || i >= Q2MAX_CONFIGSTRINGS)
		Host_EndGame ("configstring > Q2MAX_CONFIGSTRINGS");
	s = MSG_ReadString();

//	strncpy (olds, cl.configstrings[i], sizeof(olds));
//	olds[sizeof(olds) - 1] = 0;

//	strcpy (cl.configstrings[i], s);

	// do something apropriate

	if (i == Q2CS_SKY)
	{
		Q_strncpyz (cl.skyname, s, sizeof(cl.skyname));
	}
	else if (i == Q2CS_SKYAXIS)
	{
		s = COM_Parse(s);
		if (s)
		{
			cl.skyaxis[0] = atof(com_token);
			s = COM_Parse(s);
			if (s)
			{
				cl.skyaxis[1] = atof(com_token);
				s = COM_Parse(s);
				if (s)
					cl.skyaxis[2] = atof(com_token);
			}
		}
	}
	else if (i == Q2CS_SKYROTATE)
		cl.skyrotate = atof(s);
	else if (i == Q2CS_STATUSBAR)
	{
		Q_strncpyz(cl.q2statusbar, s, sizeof(cl.q2statusbar));
	}
	else if (i >= Q2CS_LIGHTS && i < Q2CS_LIGHTS+Q2MAX_LIGHTSTYLES)
	{
		R_UpdateLightStyle(i, s, 1, 1, 1);
	}
	else if (i == Q2CS_CDTRACK)
	{
		Media_NamedTrack (s, NULL);
	}
	else if (i >= Q2CS_MODELS && i < Q2CS_MODELS+Q2MAX_MODELS)
	{
		Q_strncpyz(cl.model_name[i-Q2CS_MODELS], s, MAX_QPATH);
		if (cl.model_name[i-Q2CS_MODELS][0] == '#')
		{
			if (cl.numq2visibleweapons < Q2MAX_VISIBLE_WEAPONS)
			{
				cl.q2visibleweapons[cl.numq2visibleweapons] = cl.model_name[i-Q2CS_MODELS]+1;
				cl.numq2visibleweapons++;
			}
			cl.model_precache[i-Q2CS_MODELS] = NULL;
		}
		else
			cl.model_precache[i-Q2CS_MODELS] = Mod_ForName (cl.model_name[i-Q2CS_MODELS], MLV_WARN);
	}
	else if (i >= Q2CS_SOUNDS && i < Q2CS_SOUNDS+Q2MAX_SOUNDS)
	{
		Q_strncpyz(cl.sound_name[i-Q2CS_SOUNDS], s, MAX_QPATH);
		cl.sound_precache[i-Q2CS_SOUNDS] = S_PrecacheSound (s);
	}
	else if (i >= Q2CS_IMAGES && i < Q2CS_IMAGES+Q2MAX_IMAGES)
	{
		Z_Free(cl.image_name[i-Q2CS_IMAGES]);
		cl.image_name[i-Q2CS_IMAGES] = Z_StrDup(s);
	}
	else if (i >= Q2CS_ITEMS && i < Q2CS_ITEMS+Q2MAX_ITEMS)
	{
		Z_Free(cl.item_name[i-Q2CS_ITEMS]);
		cl.item_name[i-Q2CS_ITEMS] = Z_StrDup(s);
	}
	else if (i >= Q2CS_PLAYERSKINS && i < Q2CS_PLAYERSKINS+Q2MAX_CLIENTS)
	{
		CLQ2_ParseClientinfo (i-Q2CS_PLAYERSKINS, s);
	}
	else if (i == Q2CS_MAPCHECKSUM)
	{
		extern int map_checksum;
		int serverchecksum = atoi(s);

		if (cl.worldmodel && (cl.worldmodel->fromgame == fg_quake2 || cl.worldmodel->fromgame == fg_quake3))
		{
			// the Q2 client normally exits here, however for our purposes we might as well ignore it
			if (map_checksum != serverchecksum)
				Con_Printf(CON_WARNING "WARNING: Client checksum does not match server checksum (%i != %i)", map_checksum, serverchecksum);
		}
	}

#ifdef VM_UI
	UI_StringChanged(i);
#endif
}
#endif


qboolean CL_CheckBaselines (int size)
{
	int i;

	if (size < 0)
		return false;
	if (size > MAX_EDICTS)
		return false;

	size = (size + 64) & ~63; // round up to next 64
	if (size <= cl_baselines_count)
		return true;

	cl_baselines = BZ_Realloc(cl_baselines, sizeof(*cl_baselines)*size);
	for (i = cl_baselines_count; i < size; i++)
	{
		memcpy(cl_baselines + i, &nullentitystate, sizeof(*cl_baselines));
	}

	cl_baselines_count = size;

	return true;
}

/*
==================
CL_ParseBaseline
==================
*/
void CL_ParseBaseline (entity_state_t *es)
{
	int			i;

	memcpy(es, &nullentitystate, sizeof(entity_state_t));

 	es->modelindex = MSG_ReadByte ();
	es->frame = MSG_ReadByte ();
	es->colormap = MSG_ReadByte();
	es->skinnum = MSG_ReadByte();

	for (i=0 ; i<3 ; i++)
	{
		es->origin[i] = MSG_ReadCoord ();
		es->angles[i] = MSG_ReadAngle ();
	}
}
void CL_ParseBaseline2 (void)
{
	entity_state_t es;

	if (cls.fteprotocolextensions2 & PEXT2_REPLACEMENTDELTAS)
		CLFTE_ParseBaseline(&es, true);
	else
		CLQW_ParseDelta(&nullentitystate, &es, (unsigned short)MSG_ReadShort(), true);
	if (!CL_CheckBaselines(es.number))
		Host_EndGame("CL_ParseBaseline2: check baselines failed with size %i", es.number);
	memcpy(cl_baselines + es.number, &es, sizeof(es));
}

void CLFitz_ParseBaseline2 (entity_state_t *es)
{
	int			i;
	int			bits;

	memcpy(es, &nullentitystate, sizeof(entity_state_t));

	bits = MSG_ReadByte();
	es->modelindex = (bits & FITZ_B_LARGEMODEL) ? MSG_ReadShort() : MSG_ReadByte();
	es->frame = (bits & FITZ_B_LARGEFRAME) ? MSG_ReadShort() : MSG_ReadByte();
	es->colormap = MSG_ReadByte();
	es->skinnum = MSG_ReadByte();

	for (i=0 ; i<3 ; i++)
	{
		es->origin[i] = MSG_ReadCoord ();
		es->angles[i] = MSG_ReadAngle ();
	}

	es->trans = (bits & FITZ_B_ALPHA) ? MSG_ReadByte() : 255;
	es->scale = (bits & RMQFITZ_B_SCALE) ? MSG_ReadByte() : 16;
}

void CLQ2_Precache_f (void)
{
	Model_CheckDownloads();
	Sound_CheckDownloads();

	cl.contentstage = 0;
	cl.sendprespawn = true;
	SCR_SetLoadingFile("loading data");

#ifdef VM_CG
	CG_Start();
#endif
}



/*
=====================
CL_ParseStatic

Static entities are non-interactive world objects
like torches
=====================
*/
void R_StaticEntityToRTLight(int i);
void CL_ParseStatic (int version)
{
	entity_t *ent;
	int		i;
	entity_state_t	es;
	vec3_t mins,maxs;

	if (version == 3)
	{
		CLFitz_ParseBaseline2(&es);
		i = cl.num_statics;
		cl.num_statics++;
	}
	else if (version == 1)
	{
		//old nq/qw style
		CL_ParseBaseline (&es);
		i = cl.num_statics;
		cl.num_statics++;
	}
	else if (version == 2)
	{
		//new deltaed style ('full' extension support)
		if (cls.fteprotocolextensions2 & PEXT2_REPLACEMENTDELTAS)
			CLFTE_ParseBaseline(&es, false);
		else
			CLQW_ParseDelta(&nullentitystate, &es, (unsigned short)MSG_ReadShort(), true);

		if (!es.number)
			i = cl.num_statics++;
		else
		{
			es.number+=MAX_EDICTS;

			for (i = 0; i < cl.num_statics; i++)
				if (cl_static_entities[i].ent.keynum == es.number)
				{
					pe->DelinkTrailstate (&cl_static_entities[i].emit);
					break;
				}

			if (i == cl.num_statics)
				cl.num_statics++;
		}
	}
	else
		return;

	if (i == cl_max_static_entities)
	{
		cl_max_static_entities += 16;
		cl_static_entities = BZ_Realloc(cl_static_entities, sizeof(*cl_static_entities)*cl_max_static_entities);
	}

	cl_static_entities[i].mdlidx = es.modelindex;
	cl_static_entities[i].emit = NULL;

	cl_static_entities[i].state = es;
	ent = &cl_static_entities[i].ent;
	V_ClearEntity(ent);
	memset(&cl_static_entities[i].pvscache, 0, sizeof(cl_static_entities[i].pvscache));

	ent->keynum = es.number;

// copy it to the current state
	ent->model = cl.model_precache[es.modelindex];
	memset(&ent->framestate, 0, sizeof(ent->framestate));
	ent->framestate.g[FS_REG].frame[0] = ent->framestate.g[FS_REG].frame[1] = es.frame;
	ent->framestate.g[FS_REG].lerpweight[0] = 1;
	ent->skinnum = es.skinnum;
	ent->drawflags = es.hexen2flags;

#ifdef PEXT_SCALE
	ent->scale = es.scale/16.0;
#endif
	ent->shaderRGBAf[0] = (8.0f/256.0f)*es.colormod[0];
	ent->shaderRGBAf[1] = (8.0f/256.0f)*es.colormod[1];
	ent->shaderRGBAf[2] = (8.0f/256.0f)*es.colormod[2];
	ent->shaderRGBAf[3] = es.trans/255.0f;

	ent->fatness = es.fatness/16.0;
	ent->abslight = es.abslight;

	ent->flags = 0;
	if (es.dpflags & RENDER_VIEWMODEL)
		ent->flags |= RF_WEAPONMODEL|Q2RF_MINLIGHT|RF_DEPTHHACK;
	if (es.dpflags & RENDER_EXTERIORMODEL)
		ent->flags |= RF_EXTERNALMODEL;
	if (es.effects & NQEF_ADDITIVE)
		ent->flags |= RF_ADDITIVE;
	if (es.effects & EF_NODEPTHTEST)
		ent->flags |= RF_NODEPTHTEST;
	if (es.effects & DPEF_NOSHADOW)
		ent->flags |= RF_NOSHADOW;
	if (es.trans != 0xff)
		ent->flags |= RF_TRANSLUCENT;

	VectorCopy (es.origin, ent->origin);
	VectorCopy (es.angles, ent->angles);
	es.angles[0]*=-1;
	AngleVectors(es.angles, ent->axis[0], ent->axis[1], ent->axis[2]);
	VectorInverse(ent->axis[1]);

	if (!cl.worldmodel || cl.worldmodel->loadstate != MLS_LOADED)
		return;
	if (ent->model)
	{
		//FIXME: wait for model to load so we know the correct size?
		/*FIXME: compensate for angle*/
		VectorAdd(es.origin, ent->model->mins, mins);
		VectorAdd(es.origin, ent->model->maxs, maxs);
	}
	else
	{
		VectorCopy(es.origin, mins);
		VectorCopy(es.origin, maxs);
	}
	cl.worldmodel->funcs.FindTouchedLeafs(cl.worldmodel, &cl_static_entities[i].pvscache, mins, maxs);

#ifdef RTLIGHTS
	//and now handle any rtlight fields on it
	R_StaticEntityToRTLight(i);
#endif
}

/*
===================
CL_ParseStaticSound
===================
*/
void CL_ParseStaticSound (qboolean large)
{
	extern cvar_t cl_staticsounds;
	vec3_t		org;
	int			sound_num;
	float		vol, atten;
	int			i;

	for (i=0 ; i<3 ; i++)
		org[i] = MSG_ReadCoord ();
	if (large)
		sound_num = (unsigned short)MSG_ReadShort();
	else
		sound_num = MSG_ReadByte ();
	vol = MSG_ReadByte ()/255.0;
	atten = MSG_ReadByte ()/64.0;

	vol *= cl_staticsounds.value;
	if (vol < 0)
		return;

	S_StaticSound (cl.sound_precache[sound_num], org, vol, atten);
}



/*
=====================================================================

ACTION MESSAGES

=====================================================================
*/

/*
==================
CL_ParseStartSoundPacket
==================
*/
void CLQW_ParseStartSoundPacket(void)
{
    vec3_t  pos;
    int 	channel, ent;
    int 	sound_num;
    int 	volume;
    float 	attenuation;
 	int		i;

    channel = MSG_ReadShort();

    if (channel & SND_VOLUME)
		volume = MSG_ReadByte ();
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;

    if (channel & SND_ATTENUATION)
		attenuation = MSG_ReadByte () / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

	sound_num = MSG_ReadByte ();

	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();

	ent = (channel>>3)&1023;
	channel &= 7;

	if (ent > MAX_EDICTS)
		Host_EndGame ("CL_ParseStartSoundPacket: ent = %i", ent);

#ifdef PEXT_CSQC
	if (!CSQC_StartSound(ent, channel, cl.sound_name[sound_num], pos, volume/255.0, attenuation, 100, 0, 0))
#endif
	{
		if (!sound_num)
			S_StopSound(ent, channel);
		else
			S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation, 0, 0, 0);
	}

	for (i = 0; i < cl.splitclients; i++)
	{
		if (ent == cl.playerview[i].playernum+1)
		{
			TP_CheckPickupSound(cl.sound_name[sound_num], pos, i);
			return;
		}
	}
	TP_CheckPickupSound(cl.sound_name[sound_num], pos, -1);
}

#ifdef Q2CLIENT
void CLQ2_ParseStartSoundPacket(void)
{
    vec3_t  pos_v;
	float	*pos;
    int 	channel, ent;
    int 	sound_num;
    float 	volume;
    float 	attenuation;
	int		flags;
	float	ofs;
	sfx_t	*sfx;

	flags = MSG_ReadByte ();
	sound_num = MSG_ReadByte ();

    if (flags & Q2SND_VOLUME)
		volume = MSG_ReadByte () / 255.0;
	else
		volume = Q2DEFAULT_SOUND_PACKET_VOLUME;

    if (flags & Q2SND_ATTENUATION)
		attenuation = MSG_ReadByte () / 64.0;
	else
		attenuation = Q2DEFAULT_SOUND_PACKET_ATTENUATION;

    if (flags & Q2SND_OFFSET)
		ofs = MSG_ReadByte () / 1000.0;
	else
		ofs = 0;

	if (flags & Q2SND_ENT)
	{	// entity reletive
		channel = MSG_ReadShort();
		ent = channel>>3;
		if (ent > MAX_EDICTS)
			Host_EndGame ("CL_ParseStartSoundPacket: ent = %i", ent);

		channel &= 7;
	}
	else
	{
		ent = 0;
		channel = 0;
	}

	if (flags & Q2SND_POS)
	{	// positioned in space
		MSG_ReadPos (pos_v);

		pos = pos_v;
	}
	else	// use entity number
	{
		CL_GetNumberedEntityInfo(ent, pos_v, NULL);
		pos = pos_v;
//		pos = NULL;
	}

	if (!cl.sound_precache[sound_num])
		return;

	sfx = cl.sound_precache[sound_num];
	if (sfx->name[0] == '*')
	{	//a 'sexed' sound
		if (ent > 0 && ent <= MAX_CLIENTS)
		{
			char *model = Info_ValueForKey(cl.players[ent-1].userinfo, "skin");
			char *skin;
			skin = strchr(model, '/');
			if (skin)
				*skin = '\0';
			if (*model)
				sfx = S_PrecacheSound(va("players/%s/%s", model, cl.sound_precache[sound_num]->name+1));
		}
		//fall back to male if it failed to load.
		//note: threaded loading can still make it silent the first time we hear it.
		if (sfx->loadstate == SLS_FAILED)
			sfx = S_PrecacheSound(va("players/male/%s", cl.sound_precache[sound_num]->name+1));
	}
	S_StartSound (ent, channel, sfx, pos, volume, attenuation, ofs, 0, 0);
}
#endif

#if defined(NQPROT) || defined(PEXT_SOUNDDBL)
void CLNQ_ParseStartSoundPacket(void)
{
	vec3_t  pos;
	int 	channel, ent;
	int 	sound_num;
	int 	volume;
	int 	field_mask;
	float 	attenuation;
 	int		i;
	int		pitchadj;
	float	timeofs;
	unsigned int flags;

	field_mask = MSG_ReadByte();

	if (field_mask & NQSND_VOLUME)
		volume = MSG_ReadByte ();
	else
		volume = DEFAULT_SOUND_PACKET_VOLUME;

	if (field_mask & NQSND_ATTENUATION)
		attenuation = MSG_ReadByte () / 64.0;
	else
		attenuation = DEFAULT_SOUND_PACKET_ATTENUATION;

	if (field_mask & FTESND_PITCHADJ)
		pitchadj = MSG_ReadByte();
	else
		pitchadj = 100;

	if (field_mask & FTESND_TIMEOFS)
		timeofs = MSG_ReadShort() / 1000.0;
	else
		timeofs = 0;

//	if (field_mask & FTESND_FLAGS)
//		flags = MSG_ReadByte();
//	else
		flags = 0;

	if (field_mask & DPSND_LARGEENTITY)
	{
		ent = MSGCL_ReadEntity();
		channel = MSG_ReadByte();
	}
	else
	{	//regular
		channel = MSG_ReadShort ();
		ent = channel >> 3;
		channel &= 7;
	}

	/*unpack mangling*/
	channel = (channel & 7) | ((channel & 0x0f1) << 1);

	if (field_mask & DPSND_LARGESOUND)
		sound_num = (unsigned short)MSG_ReadShort();
	else
		sound_num = MSG_ReadByte ();

	if (ent > MAX_EDICTS)
		Host_EndGame ("CL_ParseStartSoundPacket: ent = %i", ent);

	for (i=0 ; i<3 ; i++)
		pos[i] = MSG_ReadCoord ();

#ifdef PEXT_CSQC
	if (!CSQC_StartSound(ent, channel, cl.sound_name[sound_num], pos, volume/255.0, attenuation, pitchadj, timeofs, flags))
#endif
	{
		if (!sound_num)
			S_StopSound(ent, channel);
		else
			S_StartSound (ent, channel, cl.sound_precache[sound_num], pos, volume/255.0, attenuation, timeofs, pitchadj, flags);
	}

	for (i = 0; i < cl.splitclients; i++)
	{
		if (ent == cl.playerview[i].playernum+1)
		{
			TP_CheckPickupSound(cl.sound_name[sound_num], pos, i);
			return;
		}
	}
	TP_CheckPickupSound(cl.sound_name[sound_num], pos, -1);
}
#endif


/*
==================
CL_ParseClientdata

Server information pertaining to this client only, sent every frame
==================
*/
void CL_ParseClientdata (void)
{
	int				i;

// calculate simulated time of message
	oldparsecountmod = parsecountmod;

	i = cls.netchan.incoming_acknowledged;
#ifdef NQPROT
	if (cls.demoplayback == DPB_NETQUAKE)
	{
		i = cls.netchan.incoming_sequence-1;
		cl.oldparsecount = i - 1;
		oldparsecountmod = cl.oldparsecount & UPDATE_MASK;
	}
	else
#endif
	if (cls.demoplayback == DPB_MVD || cls.demoplayback == DPB_EZTV)
	{
		cl.oldparsecount = i - 1;
		oldparsecountmod = cl.oldparsecount & UPDATE_MASK;
	}
	cl.parsecount = i;
	i &= UPDATE_MASK;
	parsecountmod = i;
	parsecounttime = realtime;//cl.outframes[i].senttime;

	if (cls.protocol == CP_QUAKEWORLD)
		CL_AckedInputFrame(cls.netchan.incoming_sequence, cl.parsecount, false);
}

/*
=====================
CL_NewTranslation
=====================
*/
void CL_NewTranslation (int slot)
{
	int		top, bottom;
	int local;

	char *s;
	player_info_t	*player;

	if (slot >= MAX_CLIENTS)
		Host_Error ("CL_NewTranslation: slot > MAX_CLIENTS");

	player = &cl.players[slot];

	if (cls.protocol == CP_QUAKE2)
	{
		char *mod, *skin;
		player->qwskin = NULL;
		player->skinid = 0;
		player->model = NULL;
		player->ttopcolor = TOP_DEFAULT;
		player->tbottomcolor = BOTTOM_DEFAULT;

		mod = Info_ValueForKey(player->userinfo, "skin");
		skin = strchr(mod, '/');
		if (skin)
			*skin++ = 0;
		if (!skin || !*skin)
			skin = "grunt";
		if (!mod || !*mod)
			mod = "male";

		player->model = Mod_ForName(va("players/%s/tris.md2", mod), 0);
		if (player->model->loadstate == MLS_FAILED && strcmp(mod, "male"))
		{	//fall back on male if the model doesn't exist. yes, sexist.
			mod = "male";
			player->model = Mod_ForName(va("players/%s/tris.md2", mod), 0);
		}
		player->skinid = Mod_RegisterSkinFile(va("players/%s/%s.skin", mod,skin));
		if (!player->skinid)
			player->skinid = Mod_ReadSkinFile(va("players/%s/%s.skin", mod,skin), va("replace \"\" \"players/%s/%s.pcx\"", mod,skin));
		return;
	}

	s = Skin_FindName (player);
	COM_StripExtension(s, s, MAX_QPATH);
	if (player->qwskin && !stricmp(s, player->qwskin->name))
		player->qwskin = NULL;
	player->skinid = 0;
	player->model = NULL;



	top = player->rtopcolor;
	bottom = player->rbottomcolor;
	if (cl.splitclients < 2 && !(cl.fpd & FPD_NO_FORCE_COLOR))	//no colour/skin forcing in splitscreen.
	{
		if (cl.teamplay && cl.spectator)
		{
			local = Cam_TrackNum(&cl.playerview[0]);
			if (local < 0)
				local = cl.playerview[0].playernum;
		}
		else
			local = cl.playerview[0].playernum;
		if ((cl.teamplay || cls.protocol == CP_NETQUAKE) && !strcmp(player->team, cl.players[local].team))
		{
			if (cl_teamtopcolor != ~0)
				top = cl_teamtopcolor;
			if (cl_teambottomcolor != ~0)
				bottom = cl_teambottomcolor;

			if (player->colourised)
			{
				if (player->colourised->topcolour != ~0)
					top = player->colourised->topcolour;
				if (player->colourised->bottomcolour != ~0)
					bottom = player->colourised->bottomcolour;
			}
		}
		else
		{
			if (cl_enemytopcolor != ~0)
				top = cl_enemytopcolor;
			if (cl_enemybottomcolor != ~0)
				bottom = cl_enemybottomcolor;
		}
	}
/*
	if (top > 13 || top < 0)
		top = 13;
	if (bottom > 13 || bottom < 0)
		bottom = 13;
*/
	//other renderers still need the team stuff set, but that's all
	player->ttopcolor = top;
	player->tbottomcolor = bottom;
}

/*
==============
CL_UpdateUserinfo
==============
*/
void CL_ProcessUserInfo (int slot, player_info_t *player)
{
	char *col;
	Q_strncpyz (player->name, Info_ValueForKey (player->userinfo, "name"), sizeof(player->name));
	Q_strncpyz (player->team, Info_ValueForKey (player->userinfo, "team"), sizeof(player->team));

	col = Info_ValueForKey (player->userinfo, "topcolor");
	if (!strncmp(col, "0x", 2))
		player->rtopcolor = 0xff000000|strtoul(col+2, NULL, 16);
	else
		player->rtopcolor = atoi(col);

	col = Info_ValueForKey (player->userinfo, "bottomcolor");
	if (!strncmp(col, "0x", 2))
		player->rbottomcolor = 0xff000000|strtoul(col+2, NULL, 16);
	else
		player->rbottomcolor = atoi(col);

	if (atoi(Info_ValueForKey (player->userinfo, "*spectator")))
		player->spectator = true;
	else
		player->spectator = false;
/*
	if (player->rtopcolor > 13)
		player->rtopcolor = 13;
	if (player->rbottomcolor > 13)
		player->rbottomcolor = 13;
*/
	player->model = NULL;

#ifdef HEXEN2
	/*if we're running hexen2, they have to be some class...*/
	player->h2playerclass = atoi(Info_ValueForKey (player->userinfo, "cl_playerclass"));
	if (player->h2playerclass > 5)
		player->h2playerclass = 5;
	if (player->h2playerclass < 1)
		player->h2playerclass = 1;
#endif

	player->colourised = TP_FindColours(player->name);

	// If it's us
	if (slot == cl.playerview[0].playernum && player->name[0])
	{
		cl.spectator = player->spectator;

		// Update the rules since spectators can bypass everything but players can't
		CL_CheckServerInfo();

		Skin_FlushPlayers();
	}
	else if (cls.state == ca_active)
		Skin_Find (player);

	Sbar_Changed ();
	CL_NewTranslation (slot);
}

/*
==============
CL_UpdateUserinfo
==============
*/
void CL_UpdateUserinfo (void)
{
	int		slot;
	player_info_t	*player;

	slot = MSG_ReadByte ();
	if (slot >= MAX_CLIENTS)
		Host_EndGame ("CL_ParseServerMessage: svc_updateuserinfo > MAX_SCOREBOARD");

	player = &cl.players[slot];
	player->userid = MSG_ReadLong ();
	Q_strncpyz (player->userinfo, MSG_ReadString(), sizeof(player->userinfo));

	CL_ProcessUserInfo (slot, player);



	if (slot == cl.playerview[0].playernum && player->name[0])
	{
		char *qz;
		qz = Info_ValueForKey(player->userinfo, "Qizmo");
		if (*qz)
			TP_ExecTrigger("f_qizmoconnect", false);
	}
}

/*
==============
CL_SetInfo
==============
*/
void CL_ParseSetInfo (void)
{
	int		slot;
	player_info_t	*player;
	char key[MAX_QWMSGLEN];
	char value[MAX_QWMSGLEN];

	slot = MSG_ReadByte ();

	Q_strncpyz (key, MSG_ReadString(), sizeof(key));
	Q_strncpyz (value, MSG_ReadString(), sizeof(value));

	if (slot >= MAX_CLIENTS)
		Con_Printf("INVALID SETINFO %i: %s=%s\n", slot, key, value);
	else
	{
		player = &cl.players[slot];

		Con_DPrintf("SETINFO %s: %s=%s\n", player->name, key, value);

		Info_SetValueForStarKey (player->userinfo, key, value, sizeof(player->userinfo));

		CL_ProcessUserInfo (slot, player);
	}
}

/*
==============
CL_ServerInfo
==============
*/
void CL_ServerInfo (void)
{
//	int		slot;
//	player_info_t	*player;
	char key[MAX_QWMSGLEN];
	char value[MAX_QWMSGLEN];

	Q_strncpyz (key, MSG_ReadString(), sizeof(key));
	Q_strncpyz (value, MSG_ReadString(), sizeof(value));

	Con_DPrintf("SERVERINFO: %s=%s\n", key, value);

	Info_SetValueForStarKey (cl.serverinfo, key, value, MAX_SERVERINFO_STRING);

	CL_CheckServerInfo();
}

/*
=====================
CL_SetStat
=====================
*/
static void CL_SetStat_Internal (int pnum, int stat, int ivalue, float fvalue)
{
	int	j;
	if (cl.playerview[pnum].stats[stat] != ivalue)
		Sbar_Changed ();

	if (stat == STAT_ITEMS)
	{	// set flash times
		for (j=0 ; j<32 ; j++)
			if ( (ivalue & (1<<j)) && !(cl.playerview[pnum].stats[stat] & (1<<j)))
				cl.playerview[pnum].item_gettime[j] = cl.time;
	}

	if (stat == STAT_WEAPONMODELI)
	{
		if (cl.playerview[pnum].stats[stat] != ivalue)
		{
			if (ivalue == 0)
				TP_ExecTrigger ("f_reloadstart", false);
			else if (cl.playerview[pnum].stats[stat] == 0)
				TP_ExecTrigger ("f_reloadend", false);
		}
	}

	if (stat == STAT_VIEWHEIGHT && ((cls.z_ext & Z_EXT_VIEWHEIGHT) || cls.protocol == CP_NETQUAKE))
		cl.playerview[pnum].viewheight = fvalue;

	cl.playerview[pnum].stats[stat] = ivalue;
	cl.playerview[pnum].statsf[stat] = fvalue;

	if (pnum == 0)
		TP_StatChanged(stat, ivalue);
}

void CL_SetStatMovevar(int pnum, int stat, float value)
{
	switch(stat)
	{
	case STAT_MOVEVARS_GRAVITY:
		movevars.gravity = value;
		break;
	case STAT_MOVEVARS_STOPSPEED:
		movevars.stopspeed = value;
		break;
	case STAT_MOVEVARS_MAXSPEED:
		cl.playerview[pnum].maxspeed = value;
		break;
	case STAT_MOVEVARS_SPECTATORMAXSPEED:
		movevars.spectatormaxspeed = value;
		break;
	case STAT_MOVEVARS_ACCELERATE:
		movevars.accelerate = value;
		break;
	case STAT_MOVEVARS_AIRACCELERATE:
		movevars.airaccelerate = value;
		break;
	case STAT_MOVEVARS_WATERACCELERATE:
		movevars.wateraccelerate = value;
		break;
	case STAT_MOVEVARS_FRICTION:
		movevars.friction = value;
		break;
	case STAT_MOVEVARS_WATERFRICTION:
		movevars.waterfriction = value;
		break;
	case STAT_MOVEVARS_ENTGRAVITY:
		cl.playerview[pnum].entgravity = value;
		break;
	}
}

void CL_SetStatInt (int pnum, int stat, int value)
{
	if (stat < 0 || stat >= MAX_CL_STATS)
		return;
//		Host_EndGame ("CL_SetStat: %i is invalid", stat);

	if (stat == STAT_TIME && (cls.fteprotocolextensions & PEXT_ACCURATETIMINGS))
	{
		cl.oldgametime = cl.gametime;
		cl.oldgametimemark = cl.gametimemark;

		cl.gametime = value * 0.001;
		cl.gametimemark = realtime;
	}

	if (cls.demoplayback == DPB_MVD || cls.demoplayback == DPB_EZTV)
	{
		extern int cls_lastto;
		cl.players[cls_lastto].stats[stat]=value;
		cl.players[cls_lastto].statsf[stat]=value;

		for (pnum = 0; pnum < cl.splitclients; pnum++)
			if (cl.playerview[pnum].cam_spec_track == cls_lastto && cl.playerview[pnum].cam_state != CAM_FREECAM)
				CL_SetStat_Internal(pnum, stat, value, value);
	}
	else
		CL_SetStat_Internal(pnum, stat, value, value);

	if (cls.protocol == CP_NETQUAKE && CPNQ_IS_DP && !(cls.fteprotocolextensions2 & PEXT2_PREDINFO))
		CL_SetStatMovevar(pnum, stat, *(float*)&value);	//DP sucks.
}
void CL_SetStatFloat (int pnum, int stat, float value)
{
	if (stat < 0 || stat >= MAX_CL_STATS)
		return;
//		Host_EndGame ("CL_SetStat: %i is invalid", stat);

	if (cls.demoplayback == DPB_MVD || cls.demoplayback == DPB_EZTV)
	{
		extern int cls_lastto;
		cl.players[cls_lastto].statsf[stat]=value;
		cl.players[cls_lastto].stats[stat]=value;

		for (pnum = 0; pnum < cl.splitclients; pnum++)
			if (cl.playerview[pnum].cam_spec_track == cls_lastto && cl.playerview[pnum].cam_state != CAM_FREECAM)
			{
				cl.playerview[pnum].statsf[stat] = value;
				cl.playerview[pnum].stats[stat] = value;
			}
	}
	else
	{
		cl.playerview[pnum].statsf[stat] = value;
		cl.playerview[pnum].stats[stat] = value;
	}

	if (stat == STAT_VIEWHEIGHT && ((cls.z_ext & Z_EXT_VIEWHEIGHT) || cls.protocol == CP_NETQUAKE))
		cl.playerview[pnum].viewheight = value;

	if (cls.fteprotocolextensions2 & PEXT2_PREDINFO)
		CL_SetStatMovevar(pnum, stat, value);
}
void CL_SetStatString (int pnum, int stat, char *value)
{
	if (stat < 0 || stat >= MAX_CL_STATS)
		return;
//		Host_EndGame ("CL_SetStat: %i is invalid", stat);

	if (cls.demoplayback == DPB_MVD || cls.demoplayback == DPB_EZTV)
	{
/*		extern int cls_lastto;
		cl.players[cls_lastto].statsstr[stat]=value;

		for (pnum = 0; pnum < cl.splitclients; pnum++)
			if (spec_track[pnum] == cls_lastto)
				cl.statsstr[pnum][stat] = value;*/
	}
	else
	{
		if (cl.playerview[pnum].statsstr[stat])
			Z_Free(cl.playerview[pnum].statsstr[stat]);
		cl.playerview[pnum].statsstr[stat] = Z_Malloc(strlen(value)+1);
		strcpy(cl.playerview[pnum].statsstr[stat], value);
	}
}
/*
==============
CL_MuzzleFlash
==============
*/
void CL_MuzzleFlash (int entnum)
{
	dlight_t	*dl;
	player_state_t	*pl;

	packet_entities_t *pack;
	entity_state_t *s1;
	int pnum;
	vec3_t org = {0,0,0};
	vec3_t axis[3] = {{0,0,0}};
	int dlightkey = 0;
	extern int pt_muzzleflash;
	extern cvar_t cl_muzzleflash;

	//was it us?
	if (!cl_muzzleflash.ival) // remove all muzzleflashes
		return;

	if (cl_muzzleflash.value == 2)
	{
		//muzzleflash 2 removes muzzleflashes on us
		for (pnum = 0; pnum < cl.splitclients; pnum++)
			if (entnum-1 == cl.playerview[pnum].playernum)
				return;
	}

	if (!dlightkey)
	{
		pack = &cl.inframes[cl.validsequence&UPDATE_MASK].packet_entities;

		for (pnum=0 ; pnum<pack->num_entities ; pnum++)	//try looking for an entity with that id first
		{
			s1 = &pack->entities[pnum];

			if (s1->number == entnum)
			{
				dlightkey = entnum;
				VectorCopy(s1->origin, org);
				AngleVectors(s1->angles, axis[0], axis[1], axis[2]);
				break;
			}
		}
	}
	if (!dlightkey)
	{	//that ent number doesn't exist, go for a player with that number
		if ((unsigned)(entnum) <= cl.allocated_client_slots && entnum > 0)
		{
			pl = &cl.inframes[cl.validsequence&UPDATE_MASK].playerstate[entnum-1];

			if (pl->messagenum == cl.validsequence)
			{
				dlightkey = -entnum;
				VectorCopy(pl->origin, org);
				AngleVectors(pl->viewangles, axis[0], axis[1], axis[2]);
				if (pl->szmins[2] == 0)	/*hull is 0-based, so origin is bottom of model, move the light up slightly*/
					org[2] += pl->szmaxs[2]/2;
			}
		}
	}

	if (!dlightkey)
		return;

	if (P_RunParticleEffectType(org, axis[0], 1, pt_muzzleflash))
	{
		dl = CL_AllocDlight (dlightkey);
		VectorMA (org, 15, axis[0], dl->origin);
		memcpy(dl->axis, axis, sizeof(dl->axis));

		dl->radius = 200 + (rand()&31);
		dl->minlight = 32;
		dl->die = cl.time + 0.1;
		dl->color[0] = 1.5;
		dl->color[1] = 1.3;
		dl->color[2] = 1.0;

		dl->channelfade[0] = 1.5;
		dl->channelfade[1] = 0.75;
		dl->channelfade[2] = 0.375;
		dl->decay = 1000;
#ifdef RTLIGHTS
		dl->lightcolourscales[2] = 4;
#endif
	}
}

#ifdef Q2CLIENT
void Q2S_StartSound(vec3_t origin, int entnum, int entchannel, sfx_t *sfx, float fvol, float attenuation, float timeofs);
void CLQ2_ParseMuzzleFlash (void)
{
	vec3_t		fv, rv, dummy;
	dlight_t	*dl;
	int			i, weapon;
	vec3_t		org, ang;
	int			silenced;
	float		volume;
	char		soundname[64];

	i = (unsigned short)(short)MSG_ReadShort ();
	if (i < 1 || i >= Q2MAX_EDICTS)
		Host_Error ("CL_ParseMuzzleFlash: bad entity");

	weapon = MSG_ReadByte ();
	silenced = weapon & Q2MZ_SILENCED;
	weapon &= ~Q2MZ_SILENCED;

	CL_GetNumberedEntityInfo(i, org, ang);

	dl = CL_AllocDlight (i);
	VectorCopy (org,  dl->origin);
	AngleVectors (ang, fv, rv, dummy);
	VectorMA (dl->origin, 18, fv, dl->origin);
	VectorMA (dl->origin, 16, rv, dl->origin);
	if (silenced)
		dl->radius = 100 + (rand()&31);
	else
		dl->radius = 200 + (rand()&31);
	dl->minlight = 32;
	dl->die = cl.time+0.05; //+ 0.1;
	dl->decay = 1;

	dl->channelfade[0] = 2;
	dl->channelfade[1] = 2;
	dl->channelfade[2] = 2;

	if (silenced)
		volume = 0.2;
	else
		volume = 1;


	switch (weapon)
	{
	case Q2MZ_BLASTER:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/blastf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_BLUEHYPERBLASTER:
		dl->color[0] = 0;dl->color[1] = 0;dl->color[2] = 1;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/hyprbf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_HYPERBLASTER:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/hyprbf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_MACHINEGUN:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		Q_snprintfz(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound(soundname), volume, ATTN_NORM, 0);
		break;

	case Q2MZ_SHOTGUN:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/shotgf1b.wav"), volume, ATTN_NORM, 0);
		Q2S_StartSound (NULL, i, CHAN_AUTO,   S_PrecacheSound("weapons/shotgr1b.wav"), volume, ATTN_NORM, 0.1);
		break;
	case Q2MZ_SSHOTGUN:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/sshotf1b.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_CHAINGUN1:
		dl->radius = 200 + (rand()&31);
		dl->color[0] = 1;dl->color[1] = 0.25;dl->color[2] = 0;
		Q_snprintfz(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound(soundname), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_CHAINGUN2:
		dl->radius = 225 + (rand()&31);
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0;
		dl->die = cl.time  + 0.1;	// long delay
		Q_snprintfz(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound(soundname), volume, ATTN_NORM, 0);
		Q_snprintfz(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		Q2S_StartSound (NULL, i, CHAN_AUTO, S_PrecacheSound(soundname), volume, ATTN_NORM, 0.05);
		break;
	case Q2MZ_CHAINGUN3:
		dl->radius = 250 + (rand()&31);
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		dl->die = cl.time  + 0.1;	// long delay
		Q_snprintfz(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound(soundname), volume, ATTN_NORM, 0);
		Q_snprintfz(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		Q2S_StartSound (NULL, i, CHAN_AUTO, S_PrecacheSound(soundname), volume, ATTN_NORM, 0.033);
		Q_snprintfz(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		Q2S_StartSound (NULL, i, CHAN_AUTO, S_PrecacheSound(soundname), volume, ATTN_NORM, 0.066);
		break;

	case Q2MZ_RAILGUN:
		dl->color[0] = 0.5;dl->color[1] = 0.5;dl->color[2] = 1;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/railgf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_ROCKET:
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0.2;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/rocklf1a.wav"), volume, ATTN_NORM, 0);
		Q2S_StartSound (NULL, i, CHAN_AUTO,   S_PrecacheSound("weapons/rocklr1b.wav"), volume, ATTN_NORM, 0.1);
		break;
	case Q2MZ_GRENADE:
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/grenlf1a.wav"), volume, ATTN_NORM, 0);
		Q2S_StartSound (NULL, i, CHAN_AUTO,   S_PrecacheSound("weapons/grenlr1b.wav"), volume, ATTN_NORM, 0.1);
		break;
	case Q2MZ_BFG:
		dl->color[0] = 0;dl->color[1] = 1;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/bfg__f1y.wav"), volume, ATTN_NORM, 0);
		break;

	case Q2MZ_LOGIN:
		dl->color[0] = 0;dl->color[1] = 1; dl->color[2] = 0;
		dl->die = cl.time + 1.0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
//		CL_LogoutEffect (pl->current.origin, weapon);
		break;
	case Q2MZ_LOGOUT:
		dl->color[0] = 1;dl->color[1] = 0; dl->color[2] = 0;
		dl->die = cl.time + 1.0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
//		CL_LogoutEffect (pl->current.origin, weapon);
		break;
	case Q2MZ_RESPAWN:
		dl->color[0] = 1;dl->color[1] = 1; dl->color[2] = 0;
		dl->die = cl.time + 1.0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
//		CL_LogoutEffect (pl->current.origin, weapon);
		break;
	// RAFAEL
	case Q2MZ_PHALANX:
		dl->color[0] = 1;dl->color[1] = 0.5; dl->color[2] = 0.5;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/plasshot.wav"), volume, ATTN_NORM, 0);
		break;
	// RAFAEL
	case Q2MZ_IONRIPPER:
		dl->color[0] = 1;dl->color[1] = 0.5; dl->color[2] = 0.5;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/rippfire.wav"), volume, ATTN_NORM, 0);
		break;

// ======================
// PGM
	case Q2MZ_ETF_RIFLE:
		dl->color[0] = 0.9;dl->color[1] = 0.7;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/nail1.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_SHOTGUN2:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/shotg2.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_HEATBEAM:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		dl->die = cl.time + 100;
	//	Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/bfg__l1a.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_BLASTER2:
		dl->color[0] = 0;dl->color[1] = 1;dl->color[2] = 0;
		// FIXME - different sound for blaster2 ??
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/blastf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_TRACKER:
		// negative flashes handled the same in gl/soft until CL_AddDLights
		dl->color[0] = -1;dl->color[1] = -1;dl->color[2] = -1;
		Q2S_StartSound (NULL, i, CHAN_WEAPON, S_PrecacheSound("weapons/disint2.wav"), volume, ATTN_NORM, 0);
		break;
	case Q2MZ_NUKE1:
		dl->color[0] = 1;dl->color[1] = 0;dl->color[2] = 0;
		dl->die = cl.time + 100;
		break;
	case Q2MZ_NUKE2:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		dl->die = cl.time + 100;
		break;
	case Q2MZ_NUKE4:
		dl->color[0] = 0;dl->color[1] = 0;dl->color[2] = 1;
		dl->die = cl.time + 100;
		break;
	case Q2MZ_NUKE8:
		dl->color[0] = 0;dl->color[1] = 1;dl->color[2] = 1;
		dl->die = cl.time + 100;
		break;
// PGM
// ======================
	}
}

void CLQ2_ParseMuzzleFlash2 (void)
{
	int			ent;
	int			flash_number;

	ent = (unsigned short)(short)MSG_ReadShort ();
	if (ent < 1 || ent >= Q2MAX_EDICTS)
		Host_EndGame ("CL_ParseMuzzleFlash2: bad entity");

	flash_number = MSG_ReadByte ();

	CLQ2_RunMuzzleFlash2(ent, flash_number);
}

void CLQ2_ParseInventory (void)
{
	unsigned int		i;
	for (i=0 ; i<Q2MAX_ITEMS ; i++)
		cl.inventory[i] = MSG_ReadShort ();
}
#endif

//return if we want to print the message.
char *CL_ParseChat(char *text, player_info_t **player, int *msgflags)
{
	extern cvar_t cl_chatsound, cl_nofake, cl_teamchatsound, cl_enemychatsound;
	int flags;
	int offset=0;
	qboolean	suppress_talksound;
	char *p;
	char *s;
	int check_flood;

	flags = TP_CategorizeMessage (text, &offset, player);
	*msgflags = flags;

	s = text + offset;

	if (flags)
	{
		if (!cls.demoplayback)
			Sys_ServerActivity();	//chat always flashes the screen..

		//check f_ stuff
		if (*player && !strncmp(s, "f_", 2))
		{
			Validation_Auto_Response(*player - cl.players, s);
			return s;
		}

		Validation_CheckIfResponse(text);

#ifdef PLUGINS
		if (!Plug_ChatMessage(text + offset, *player ? (int)(*player - cl.players) : -1, flags))
			return NULL;
#endif

		if (flags & (TPM_TEAM|TPM_OBSERVEDTEAM) && !TP_FilterMessage(text + offset))
			return NULL;

		if (flags & (TPM_TEAM|TPM_OBSERVEDTEAM) && Sbar_UpdateTeamStatus(*player, text+offset))
			return NULL;


		if ((int)msg_filter.value & flags)
			return NULL;	//filter chat

		check_flood = Ignore_Check_Flood(*player, s, flags);
		if (check_flood == IGNORE_NO_ADD)
			return NULL;
		else if (check_flood == NO_IGNORE_ADD)
			Ignore_Flood_Add(*player, s);
	}
#ifdef PLUGINS
	else
	{
		if (!Plug_ServerMessage(text + offset, PRINT_CHAT))
			return NULL;
	}
#endif

	suppress_talksound = false;

	if (flags == 2 || (!cl.teamplay && flags))
		suppress_talksound = TP_CheckSoundTrigger (text + offset);

	if (cls.demoseeking ||
		!cl_chatsound.value ||		// no sound at all
		(cl_chatsound.value == 2 && flags != 2))	// only play sound in mm2
		suppress_talksound = true;


	if (!suppress_talksound)
	{
		if (flags & (TPM_OBSERVEDTEAM|TPM_TEAM) && cl.teamplay)
			S_LocalSound (cl_teamchatsound.string);
		else
			S_LocalSound (cl_enemychatsound.string);
	}

	if (flags)
	{
		if (cl_nofake.value == 1 || (cl_nofake.value == 2 && !(flags & (TPM_OBSERVEDTEAM | TPM_TEAM))))
		{
			for (p = s; *p; p++)
				if (*p == 13 || (*p == 10 && p[1]))
					*p = ' ';
		}
	}

	return s;
}

// CL_PlayerColor: returns color and mask for player_info_t
int CL_PlayerColor(player_info_t *plr, qboolean *name_coloured)
{
	char *t;
	unsigned int c;

	*name_coloured = false;

	if (cl.teamfortress) //override based on team
	{
		//damn spies
		if (!Q_strcasecmp(plr->team, "red"))
			c = 1;
		else if (!Q_strcasecmp(plr->team, "blue"))
			c = 5;
		else
		// TODO: needs some work
		switch (plr->rbottomcolor)
		{	//translate q1 skin colours to console colours
		case 10:
		case 1:
			*name_coloured = true;
		case 4:	//red
			c = 1;
			break;
		case 11:
			*name_coloured = true;
		case 3: // green
			c = 2;
			break;
		case 5:
			*name_coloured = true;
		case 12:
			c = 3;
			break;
		case 6:
		case 7:
			*name_coloured = true;
		case 8:
		case 9:
			c = 6;
			break;
		case 2: // light blue
			*name_coloured = true;
		case 13: //blue
		case 14: //blue
			c = 5;
			break;
		default:
			*name_coloured = true;
		case 0: // white
			c = 7;
			break;
		}
	}
	else if (cl.teamplay)
	{
		// team name hacks
		if (!strcmp(plr->team, "red"))
			c = 1;
		else if (!strcmp(plr->team, "blue"))
			c = 5;
		else
		{
			char *t;

			t = plr->team;
			c = 0;

			for (t = plr->team; *t; t++)
			{
				c >>= 1;
				c ^= *t; // TODO: very weak hash, replace
			}

			if ((c / 7) & 1)
				*name_coloured = true;

			c = 1 + (c % 7);
		}
	}
	else
	{
		// override chat color with tc infokey
		// 0-6 is standard colors (red to white)
		// 7-13 is using secondard charactermask
		// 14 and afterwards repeats
		t = Info_ValueForKey(plr->userinfo, "tc");
		if (*t)
			c = atoi(t);
		else
			c = plr->userid; // Quake2 can start from 0

		if ((c / 7) & 1)
			*name_coloured = true;

		c = 1 + (c % 7);
	}

	return c;
}

void TTS_SayChatString(char **stringtosay);

// CL_PrintChat: takes chat strings and performs name coloring and cl_parsewhitetext parsing
// NOTE: text in rawmsg/msg is assumed destroyable and should not be used afterwards
void CL_PrintChat(player_info_t *plr, char *msg, int plrflags)
{
	extern cvar_t con_separatechat;
	char *name = NULL;
	int c;
	qboolean name_coloured = false;
	extern cvar_t cl_parsewhitetext;
	qboolean memessage = false;
	char fullchatmessage[2048];

	fullchatmessage[0] = 0;
	/*if (plrflags & TPM_FAKED)
	{
		name = rawmsg; // use rawmsg pointer and msg modification to generate null-terminated string
		if (msg)
			*(msg - 2) = 0; // it's assumed that msg has 2 chars before it due to strstr
	}*/

	if (0)//*msg == '\r')
	{
		name = msg;
		msg = strstr(msg, ": ");
		if (msg)
		{
			name++;
			*msg = 0;
			msg+=2;
			plrflags &= ~TPM_TEAM|TPM_OBSERVEDTEAM;
		}
		else
		{
			msg = name;
			name = NULL;
		}
	}

	if (msg[0] == '/' && msg[1] == 'm' && msg[2] == 'e' && msg[3] == ' ')
	{
		msg += 4;
		memessage = true; // special /me formatting
	}

	if (plr && !name) // use special formatting with a real chat message
		name = plr->name; // use player's name

	if (cl_standardchat.ival)
	{
		name_coloured = true;
		c = 7;
	}
	else
	{
		if (plrflags & TPM_SPECTATOR) // is an observer
		{
			// TODO: we don't even check for this yet...
			if (plrflags & (TPM_TEAM | TPM_OBSERVEDTEAM)) // is on team
				c = 0; // blacken () on observers
			else
			{
				name_coloured = true;
				c = 7;
			}
		}
		else if (plr)
			c = CL_PlayerColor(plr, &name_coloured);
		else
		{
			// defaults for fake clients
			name_coloured = true;
			c = 7;
		}
	}

	c = '0' + c;

	if (plrflags & TPM_QTV)
		Q_strncatz(fullchatmessage, "QTV ^m", sizeof(fullchatmessage));
	else if (name)
	{
		if (memessage)
		{
			if (!cl_standardchat.value && (plrflags & TPM_SPECTATOR))
				Q_strncatz(fullchatmessage, "^0*^7 ", sizeof(fullchatmessage));
			else
				Q_strncatz(fullchatmessage, "* ", sizeof(fullchatmessage));
		}
		else
			Q_strncatz(fullchatmessage, "\1", sizeof(fullchatmessage));

#if defined(_WIN32) && !defined(NOMEDIA) && !defined(WINRT)
		TTS_SayChatString(&msg);
#endif

		if (plrflags & (TPM_TEAM|TPM_OBSERVEDTEAM)) // for team chat don't highlight the name, just the brackets
		{
			Q_strncatz(fullchatmessage, va("(^[^7%s%s^d\\player\\%i^])", name_coloured?"^m":"", name, (int)(plr-cl.players)), sizeof(fullchatmessage));
		}
		else if (cl_standardchat.ival)
		{
			Q_strncatz(fullchatmessage, va("^[^7%s%s^d\\player\\%i^]", name_coloured?"^m":"", name, (int)(plr-cl.players)), sizeof(fullchatmessage));
		}
		else
		{
			Q_strncatz(fullchatmessage, va("^[^7%s^%c%s^d\\player\\%i^]", name_coloured?"^m":"", c, name, (int)(plr-cl.players)), sizeof(fullchatmessage));
		}

		if (!memessage)
		{
			// only print seperator with an actual player name
			if (!cl_standardchat.value && (plrflags & TPM_SPECTATOR))
				Q_strncatz(fullchatmessage, "^0: ^d", sizeof(fullchatmessage));
			else
				Q_strncatz(fullchatmessage, ": ", sizeof(fullchatmessage));
		}
		else
			Q_strncatz(fullchatmessage, " ", sizeof(fullchatmessage));
	}
	else
		Q_strncatz(fullchatmessage, "\1", sizeof(fullchatmessage));

	// print message
	if (cl_parsewhitetext.value && (cl_parsewhitetext.value == 1 || (plrflags & (TPM_TEAM|TPM_OBSERVEDTEAM))))
	{
		char *t, *u;

		while ((t = strchr(msg, '{')))
		{
			int c;
			if (t > msg && t[-1] == '^')
			{
				for (c = 1; t-c > msg; c++)
				{
					if (t[-c] == '^')
						break;
				}
				if (c & 1)
				{
					*t = '\0';
					Q_strncatz(fullchatmessage, va("%s{", msg), sizeof(fullchatmessage));
					msg = t+1;
					continue;
				}
			}
			u = strchr(t, '}');
			if (u)
			{
				*t = 0;
				*u = 0;
				Q_strncatz(fullchatmessage, va("%s", msg), sizeof(fullchatmessage));
				Q_strncatz(fullchatmessage, va("^m%s^m", t+1), sizeof(fullchatmessage));
				msg = u+1;
			}
			else
				break;
		}
		Q_strncatz(fullchatmessage, va("%s", msg), sizeof(fullchatmessage));
	}
	else
	{
		Q_strncatz(fullchatmessage, va("%s", msg), sizeof(fullchatmessage));
	}

#ifdef CSQC_DAT
	if (CSQC_ParsePrint(fullchatmessage, PRINT_CHAT))
		return;
#endif


	if (con_separatechat.ival)
	{
		if (!con_chat)
			con_chat = Con_Create("chat", CONF_HIDDEN|CONF_NOTIFY|CONF_NOTIFY_BOTTOM);
		if (con_chat)
		{
			Con_PrintCon(con_chat, fullchatmessage, con_chat->parseflags);

			if (con_separatechat.ival == 1)
			{
				con_main.flags |= CONF_NOTIMES;
				Con_PrintCon(&con_main, fullchatmessage, con_main.parseflags);
				con_main.flags &= CONF_NOTIMES;
				return;
			}
		}
	}

	Con_Printf("%s", fullchatmessage);
}

// CL_PrintStandardMessage: takes non-chat net messages and performs name coloring
// NOTE: msg is considered destroyable
char acceptedchars[] = {'.', '?', '!', '\'', ',', ':', ' ', '\0'};
void CL_PrintStandardMessage(char *msgtext, int printlevel)
{
	int i;
	player_info_t *p;
	extern cvar_t cl_standardmsg, msg;
	char *begin = msgtext;
	char fullmessage[2048];

	if (printlevel < msg.ival)
		return;

	fullmessage[0] = 0;

	// search for player names in message
	for (i = 0, p = cl.players; i < cl.allocated_client_slots; p++, i++)
	{
		char *v;
		char *name;
		int len;
		qboolean coloured;
		char c;

		name = p->name;
		if (!(*name))
			continue;
		len = strlen(name);
		v = strstr(msgtext, name);
		while (v)
		{
			// name parsing rules
			if (v != begin && *(v-1) != ' ') // must be space before name
			{
					v = strstr(v+len, name);
					continue;
			}

			{
				int i;
				char aftername = *(v + len);

				// search for accepted chars in char after name in msg
				for (i = 0; i < sizeof(acceptedchars); i++)
				{
					if (acceptedchars[i] == aftername)
						break;
				}

				if (sizeof(acceptedchars) == i)
				{
					v = strstr(v+len, name);
					continue; // no accepted char found
				}
			}

			*v = 0; // cut off message

			// print msg chunk
			Q_strncatz(fullmessage, msgtext, sizeof(fullmessage));
			msgtext = v + len; // update search point

			// get name color
			if (p->spectator || cl_standardmsg.ival)
			{
				coloured = false;
				c = '7';
			}
			else
				c = '0' + CL_PlayerColor(p, &coloured);

			// print name
			Q_strncatz(fullmessage, va("^[%s^%c%s^d\\player\\%i^]", coloured?"^m":"", c, name, (int)(p - cl.players)), sizeof(fullmessage));
			break;
		}
	}

	// print final chunk
	Q_strncatz(fullmessage, msgtext, sizeof(fullmessage));
	Con_Printf("%s", fullmessage);
}

char printtext[4096];
void CL_ParsePrint(char *msg, int level)
{
	char n;
	if (strlen(printtext) + strlen(msg) >= sizeof(printtext))
	{
		Con_Printf("%s", printtext);
		Q_strncpyz(printtext, msg, sizeof(printtext));
	}
	else
		strcat(printtext, msg);	//safe due to size on if.
	while((msg = strchr(printtext, '\n')))
	{
		n = msg[1];
		msg[1] = 0;

		if (!cls.demoseeking)
		{
			if (level == PRINT_CHAT)
			{
				char *body;
				int msgflags;
				player_info_t *plr = NULL;

				if (!TP_SuppressMessage(printtext))
				{
					body = CL_ParseChat(printtext, &plr, &msgflags);
					if (body)
						CL_PrintChat(plr, body, msgflags);
				}
			}
			else
			{
#ifdef PLUGINS
				if (Plug_ServerMessage(printtext, level))
#endif
#ifdef CSQC_DAT
				if (!CSQC_ParsePrint(printtext, level))
#endif
					if (!Stats_ParsePrintLine(printtext) || !msg_filter_frags.ival)
						CL_PrintStandardMessage(printtext, level);
			}
		}

		TP_SearchForMsgTriggers(printtext, level);
		msg[1] = n;
		msg++;

		memmove(printtext, msg, strlen(msg)+1);
	}
}


void CL_ParseTeamInfo(void)
{
	unsigned int pidx = atoi(Cmd_Argv(1));
	vec3_t org =
	{
		atof(Cmd_Argv(2)),
		atof(Cmd_Argv(3)),
		atof(Cmd_Argv(4))
	};
	float health = atof(Cmd_Argv(5));
	float armour = atof(Cmd_Argv(6));
	unsigned int items = strtoul(Cmd_Argv(7), NULL, 0);
	char *nick = Cmd_Argv(8);

	if (pidx < cl.allocated_client_slots)
	{
		player_info_t *pl = &cl.players[pidx];
		pl->tinfo.time = cl.time+5;
		pl->tinfo.health = health;
		pl->tinfo.armour = armour;
		pl->tinfo.items = items;
		VectorCopy(org, pl->tinfo.org);
		Q_strncpyz(pl->tinfo.nick, nick, sizeof(pl->tinfo.nick));
	}
}


char stufftext[4096];
void CL_ParseStuffCmd(char *msg, int destsplit)	//this protects stuffcmds from network segregation.
{
	strncat(stufftext, msg, sizeof(stufftext)-1);
	while((msg = strchr(stufftext, '\n')))
	{
		*msg = '\0';
		Con_DPrintf("stufftext: %s\n", stufftext);
		if (!strncmp(stufftext, "fullserverinfo ", 15))
		{
			Cmd_ExecuteString(stufftext, RESTRICT_SERVER+destsplit);	//do this NOW so that it's done before any models or anything are loaded
			#if _MSC_VER > 1200
			if (cls.netchan.remote_address.type != NA_LOOPBACK)
				Sys_RecentServer("+connect", cls.servername, va("%s (%s)", Info_ValueForKey(cl.serverinfo, "hostname"), cls.servername), "Join QW Server");
			#endif
		}
		else
		{
			if (!strncmp(stufftext, "//querycmd ", 11))
			{
				COM_Parse(stufftext + 11);
				if (Cmd_Exists(com_token))
				{
					Cbuf_AddText ("cmd cmdsupported ", RESTRICT_SERVER+destsplit);
					Cbuf_AddText (com_token, RESTRICT_SERVER+destsplit);
					Cbuf_AddText ("\n", RESTRICT_SERVER+destsplit);
				}
			}
			else if (!strncmp(stufftext, "//paknames ", 11))
			{
				Q_strncatz(cl.serverpaknames, stufftext+11, sizeof(cl.serverpaknames));
				cl.serverpakschanged = true;
			}
			else if (!strncmp(stufftext, "//paks ", 7))
			{
				Q_strncatz(cl.serverpakcrcs, stufftext+7, sizeof(cl.serverpakcrcs));
				cl.serverpakschanged = true;
				CL_CheckServerPacks();
			}
			else if (!strncmp(stufftext, "//vwep ", 7))
			{
				int i;
				char *mname;
				Cmd_TokenizeString(stufftext+7, false, false);
				for (i = 0; i < Cmd_Argc(); i++)
				{
					mname = Cmd_Argv(i);
					if (strcmp(mname, "-"))
					{
						mname = va("progs/%s.mdl", Cmd_Argv(i));
						Q_strncpyz(cl.model_name_vwep[i], mname, sizeof(cl.model_name_vwep[i]));
						if (cls.state == ca_active)
						{
							CL_CheckOrEnqueDownloadFile(cl.model_name_vwep[i], NULL, 0);
							cl.model_precache_vwep[i] = Mod_ForName(cl.model_name_vwep[i], MLV_WARN);
						}
					}
				}
			}
			else if (!strncmp(stufftext, "//exectrigger ", 14))
			{
				COM_Parse(stufftext + 14);
				if (Cmd_AliasExist(com_token, RESTRICT_SERVER))
					Cmd_ExecuteString(com_token, RESTRICT_SERVER);	//do this NOW so that it's done before any models or anything are loaded
			}
			else if (!strncmp(stufftext, "//set ", 6))
			{
				Cmd_ExecuteString(stufftext+2, RESTRICT_SERVER+destsplit);	//do this NOW so that it's done before any models or anything are loaded
			}
			else if (!strncmp(stufftext, "//at ", 5))
			{
				Cam_SetModAutoTrack(atoi(stufftext+5));
			}
			else if (!strncmp(stufftext, "//wps ", 5))
			{
				//weapon stats, eg:
				//wps CLIENT WNAME attacks hits
			}
			else if (!strncmp(stufftext, "//kickfile ", 11))
			{
				flocation_t loc;
				Cmd_TokenizeString(stufftext+2, false, false);
				if (FS_FLocateFile(Cmd_Argv(1), FSLFRT_IFFOUND, &loc))
					Con_Printf("You have been kicked due to the file \"%s\" being modified.\n", Cmd_Argv(1));
			}
#ifdef PLUGINS
			else if (!strncmp(stufftext, "//tinfo ", 8))
			{
				Cmd_TokenizeString(stufftext+2, false, false);
				CL_ParseTeamInfo();
				Plug_Command_f();	//FIXME: deprecate this call
			}
			else if (!strncmp(stufftext, "//sn ", 5))
			{
				Cmd_TokenizeString(stufftext+2, false, false);
				Plug_Command_f();
			}
#endif
#ifdef CSQC_DAT
			else if (CSQC_StuffCmd(destsplit, stufftext, msg))
			{
			}
#endif
			else
			{
				if (!strncmp(stufftext, "cmd ", 4))
					Cbuf_AddText (va("p%i ", destsplit+1), RESTRICT_SERVER+destsplit);	//without this, in_forceseat can break directed cmds.
				Cbuf_AddText (stufftext, RESTRICT_SERVER+destsplit);
				Cbuf_AddText ("\n", RESTRICT_SERVER+destsplit);
			}
		}
		msg++;

		memmove(stufftext, msg, strlen(msg)+1);
	}
}

void CL_ParsePrecache(void)
{
	int i, code = (unsigned short)MSG_ReadShort();
	char *s = MSG_ReadString();
	i = code & ~PC_TYPE;
	switch(code & PC_TYPE)
	{
	case PC_MODEL:
		if (i >= 1 && i < MAX_PRECACHE_MODELS)
		{
			model_t *model;
			CL_CheckOrEnqueDownloadFile(s, s, 0);
			model = Mod_ForName(Mod_FixName(s, cl.model_name[1]), (i == 1)?MLV_ERROR:MLV_WARN);
			if (!model)
				Con_Printf("svc_precache: Mod_ForName(\"%s\") failed\n", s);
			cl.model_precache[i] = model;
			Q_strncpyz (cl.model_name[i], s, sizeof(cl.model_name[i]));

			cl.model_precaches_added = true;
		}
		else
			Con_Printf("svc_precache: model index %i outside range %i...%i\n", i, 1, MAX_PRECACHE_MODELS);
		break;
	case PC_UNUSED:
		break;
	case PC_SOUND:
		if (i >= 1 && i < MAX_PRECACHE_SOUNDS)
		{
			sfx_t *sfx;
			if (S_HaveOutput())
				CL_CheckOrEnqueDownloadFile(va("sound/%s", s), NULL, 0);
			sfx = S_PrecacheSound (s);
			if (!sfx)
				Con_Printf("svc_precache: S_PrecacheSound(\"%s\") failed\n", s);
			cl.sound_precache[i] = sfx;
			Q_strncpyz (cl.sound_name[i], s, sizeof(cl.sound_name[i]));
		}
		else
			Con_Printf("svc_precache: sound index %i outside range %i...%i\n", i, 1, MAX_PRECACHE_SOUNDS);
		break;
	case PC_PARTICLE:
		if (i >= 1 && i < MAX_SSPARTICLESPRE)
		{
			if (cl.particle_ssname[i])
				free(cl.particle_ssname[i]);
			cl.particle_ssname[i] = strdup(s);
			cl.particle_ssprecache[i] = P_FindParticleType(s);
			cl.particle_ssprecaches = true;
		}
		else
			Con_Printf("svc_precache: particle index %i outside range %i...%i\n", i, 1, MAX_SSPARTICLESPRE);
		break;
	}
}

void Con_HexDump(qbyte *packet, size_t len)
{
	int i;
	int pos;

	pos = 0;
	while(pos < len)
	{
		Con_Printf("%5i ", pos);
		for (i = 0; i < 16; i++)
		{
			if (pos >= len)
				Con_Printf(" - ");
			else
				Con_Printf("%2x ", packet[pos]);
			pos++;
		}
		pos-=16;
		for (i = 0; i < 16; i++)
		{
			if (pos >= len)
				Con_Printf("X");
			else if (packet[pos] == 0 || packet[pos] == '\t' || packet[pos] == '\r' || packet[pos] == '\n')
				Con_Printf(".");
			else
				Con_Printf("%c", packet[pos]);
			pos++;
		}
		Con_Printf("\n");
	}

}
void CL_DumpPacket(void)
{
	Con_HexDump(net_message.data, net_message.cursize);
}

void CL_ParsePortalState(void)
{
	int mode = MSG_ReadByte();
	int a1, a2;

	switch(mode&0xc0)
	{
	case 0x80:
		if (mode&2)
			a1 = MSG_ReadShort();
		else
			a1 = MSG_ReadByte();
#ifdef Q2BSPS
		CMQ2_SetAreaPortalState(cl.worldmodel, a1, !!(mode&1));
#endif
		break;
	case 0xc0:
		if (mode&2)
		{
			a1 = MSG_ReadShort();
			a2 = MSG_ReadShort();
		}
		else
		{
			a1 = MSG_ReadByte();
			a2 = MSG_ReadByte();
		}
#ifdef Q3BSPS
		CMQ3_SetAreaPortalState(cl.worldmodel, a1, a2, !!(mode&1));
#endif
		break;

	default:
		//to be phased out.
		mode |= MSG_ReadByte()<<8;
#ifdef Q2BSPS
		CMQ2_SetAreaPortalState(cl.worldmodel, mode & 0x7fff, !!(mode&0x8000));
#endif
		break;
	}
}

#define SHOWNET(x) if(cl_shownet.value>=2)Con_Printf ("%3i:%s\n", msg_readcount-1, x);
#define SHOWNET2(x, y) if(cl_shownet.value>=2)Con_Printf ("%3i:%3i:%s\n", msg_readcount-1, y, x);
/*
=====================
CL_ParseServerMessage
=====================
*/
int	received_framecount;
void CLQW_ParseServerMessage (void)
{
	int			cmd;
	char		*s;
	int			i, j;
	int			destsplit;
	float f;
	qboolean	csqcpacket = false;
	inframe_t	*inf;
	extern vec3_t demoangles;

	received_framecount = host_framecount;
	cl.last_servermessage = realtime;
	CL_ClearProjectiles ();

	//clear out fixangles stuff
	inf = &cl.inframes[cls.netchan.incoming_sequence&UPDATE_MASK];
	for (j = 0; j < MAX_SPLITS; j++)
		inf->packet_entities.fixangles[j] = false;
	if (cls.demoplayback == DPB_QUAKEWORLD)
	{
		inf->packet_entities.fixangles[0] = 2;
		VectorCopy(demoangles, inf->packet_entities.fixedangles[0]);
	}

//
// if recording demos, copy the message out


	//
	if (cl_shownet.value == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.value >= 2)
		Con_Printf ("------------------\n");


	CL_ParseClientdata ();

	//vanilla QW has no timing info in the client and depends upon the client for all timing.
	//using the demo's timing for interpolation prevents unneccesary drift, and solves issues with demo seeking and other such things.
	if (cls.demoplayback == DPB_QUAKEWORLD && !(cls.fteprotocolextensions & PEXT_ACCURATETIMINGS))
	{
		extern float demtime;
		if (cl.gametime != demtime)
		{
			cl.oldgametime = cl.gametime;
			cl.oldgametimemark = cl.gametimemark;
			cl.gametime = demtime;
			cl.gametimemark = realtime;
		}
	}

//
// parse the message
//
	while (1)
	{
		if (msg_badread)
		{
			CL_DumpPacket();
			Host_EndGame ("CL_ParseServerMessage: Bad server message");
			break;
		}

		cmd = MSG_ReadByte ();

		if (cmd == svcfte_choosesplitclient)
		{
			SHOWNET2(svc_qwstrings[cmd], cmd);

			destsplit = MSG_ReadByte() % MAX_SPLITS;
			cmd = MSG_ReadByte();
		}
		else
			destsplit = cl.defaultnetsplit;

		if (cmd == -1)
		{
			msg_readcount++;	// so the EOM showner has the right value
			SHOWNET("END OF MESSAGE");
			break;
		}

		SHOWNET2(svc_qwstrings[cmd], cmd);

	// other commands
		switch (cmd)
		{
		default:
			CL_DumpPacket();
			Host_EndGame ("CLQW_ParseServerMessage: Illegible server message (%i@%i)%s", cmd, msg_readcount-1, (!cl.csqcdebug && csqcpacket)?"\n'sv_csqcdebug 1' might aid in debugging this.":"" );
			return;

		case svc_time:
			cl.oldgametime = cl.gametime;
			cl.gametime = MSG_ReadFloat();
			cl.gametimemark = realtime;
			break;

		case svc_nop:
//			Con_Printf ("svc_nop\n");
			break;

		case svc_disconnect:
			if (cls.demoplayback == DPB_EZTV)	//eztv fails to detect the end of demos.
				MSG_ReadString();
			else if (cls.demoplayback)
			{
				CL_Disconnect_f();
				return;
			}
			else if (cls.state == ca_connected)
			{
				Host_EndGame ("Server disconnected\n");
			}
			else
				Host_EndGame ("Server disconnected");
			break;

		case svc_print:
			i = MSG_ReadByte ();
			s = MSG_ReadString ();
			CL_ParsePrint(s, i);
			break;

		case svc_centerprint:
			s = MSG_ReadString ();

#ifdef PLUGINS
			if (Plug_CenterPrintMessage(s, destsplit))
#endif
				SCR_CenterPrint (destsplit, s, false);
			break;

		case svc_stufftext:
			s = MSG_ReadString ();

			CL_ParseStuffCmd(s, destsplit);
			break;

		case svc_damage:
			V_ParseDamage (&cl.playerview[destsplit]);
			break;

		case svc_serverdata:
			Cbuf_Execute ();		// make sure any stuffed commands are done
 			CLQW_ParseServerData ();
			break;
		case svc_signonnum:
			cl.splitclients = MSG_ReadByte();
			for (i = 0; i < cl.splitclients && i < 4; i++)
			{
				cl.playerview[i].playernum = MSG_ReadByte();
				cl.playerview[i].viewentity = cl.playerview[i].playernum+1;
			}
			if (i < cl.splitclients)
			{
				Con_Printf("Server sent us too many seats!\n");
				for (; i < cl.splitclients; i++)
				{	//svcfte_choosesplitclient has a modulo that is also broken, but at least there's no parse errors this way
					MSG_ReadByte();
//					CL_SendClientCommand(true, va("%i drop", i+1));
				}
				cl.splitclients = MAX_SPLITS;
			}
			break;
#ifdef PEXT_SETVIEW
		case svc_setview:
			if (!(cls.fteprotocolextensions & PEXT_SETVIEW))
				Con_Printf("^1PEXT_SETVIEW is meant to be disabled\n");
			cl.playerview[destsplit].viewentity=MSGCL_ReadEntity();
			break;
#endif
		case svcfte_setangledelta:
			for (i=0 ; i<3 ; i++)
				cl.playerview[destsplit].viewangles[i] += MSG_ReadAngle16 ();
//			VectorCopy (cl.playerview[destsplit].viewangles, cl.playerview[destsplit].simangles);
			break;
		case svc_setangle:
			if (cls.demoplayback == DPB_MVD || cls.demoplayback == DPB_EZTV)
			{
				//I really don't get the point of fixangles in an mvd. to disable interpolation for that frame?
				vec3_t ang;
				i = MSG_ReadByte();
				for (i=0 ; i<3 ; i++)
					ang[i] = MSG_ReadAngle();
				for (j = 0; j < cl.splitclients; j++)
				{
					playerview_t *pv = &cl.playerview[j];
					if (Cam_TrackNum(pv) == i)
					{
						inf->packet_entities.fixangles[j] = true;
						VectorCopy(ang, inf->packet_entities.fixedangles[j]);
					}
				}
				break;
			}
			inf->packet_entities.fixangles[destsplit] = true;
			for (i=0 ; i<3 ; i++)
			{
				cl.playerview[destsplit].viewangles[i] = cl.playerview[destsplit].intermissionangles[i] = inf->packet_entities.fixedangles[destsplit][i] = MSG_ReadAngle ();
			}
			break;

		case svc_lightstyle:
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
				Host_EndGame ("svc_lightstyle > MAX_LIGHTSTYLES");
			R_UpdateLightStyle(i, MSG_ReadString(), 1, 1, 1);
			break;
#ifdef PEXT_LIGHTSTYLECOL
		case svcfte_lightstylecol:
			if (!(cls.fteprotocolextensions & PEXT_LIGHTSTYLECOL))
				Host_EndGame("PEXT_LIGHTSTYLECOL is meant to be disabled\n");
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
				Host_EndGame ("svc_lightstyle > MAX_LIGHTSTYLES");
			{
				int bits;
				vec3_t rgb;
				bits = MSG_ReadByte();
				if (bits & 0x80)
				{
					rgb[0] = MSG_ReadShort()/1024.0;
					rgb[1] = MSG_ReadShort()/1024.0;
					rgb[2] = MSG_ReadShort()/1024.0;
				}
				else
				{
					rgb[0] = (bits&1)?1:0;
					rgb[1] = (bits&2)?1:0;
					rgb[2] = (bits&4)?1:0;
				}
				R_UpdateLightStyle(i, MSG_ReadString(), rgb[0], rgb[1], rgb[2]);
			}
			break;
#endif

		case svc_sound:
			CLQW_ParseStartSoundPacket();
			break;
#ifdef PEXT_SOUNDDBL
		case svcfte_soundextended:
			CLNQ_ParseStartSoundPacket();
			break;
#endif

		case svc_stopsound:
			i = MSG_ReadShort();
			S_StopSound(i>>3, i&7);
			break;

#ifdef PEXT2_VOICECHAT
		case svcfte_voicechat:
			S_Voip_Parse();
			break;
#endif

#ifdef TERRAIN
		case svcfte_brushedit:
			CL_Parse_BrushEdit();
			break;
#endif

		case svc_updatefrags:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updatefrags > MAX_SCOREBOARD");
			cl.players[i].frags = MSG_ReadShort ();
			break;

		case svc_updateping:
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updateping > MAX_SCOREBOARD");
			cl.players[i].ping = MSG_ReadShort ();
			break;

		case svc_updatepl:
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updatepl > MAX_SCOREBOARD");
			cl.players[i].pl = MSG_ReadByte ();
			break;

		case svc_updateentertime:
		// time is sent over as seconds ago
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				Host_EndGame ("CL_ParseServerMessage: svc_updateentertime > MAX_SCOREBOARD");
			cl.players[i].realentertime = realtime - MSG_ReadFloat ();
			break;

		case svc_spawnbaseline:
			i = MSGCL_ReadEntity ();
			if (!CL_CheckBaselines(i))
				Host_EndGame("CL_ParseServerMessage: svc_spawnbaseline failed with size %i", i);
			CL_ParseBaseline (cl_baselines + i);
			break;
		case svcfte_spawnbaseline2:
			CL_ParseBaseline2 ();
			break;
		case svc_spawnstatic:
			CL_ParseStatic (1);
			break;
		case svcfte_spawnstatic2:
			CL_ParseStatic (2);
			break;
		case svc_temp_entity:
#ifdef NQPROT
			CL_ParseTEnt (false);
#else
			CL_ParseTEnt ();
#endif
			break;
		case svcfte_customtempent:
			CL_ParseCustomTEnt();
			break;

		case svc_particle:
			CLNQ_ParseParticleEffect ();
			break;
		case svcfte_particle2:
			CL_ParseParticleEffect2 ();
			break;
		case svcfte_particle3:
			CL_ParseParticleEffect3 ();
			break;
		case svcfte_particle4:
			CL_ParseParticleEffect4 ();
			break;

		case svc_killedmonster:
			//fixme: update all player stats
			cl.playerview[destsplit].stats[STAT_MONSTERS]++;
			cl.playerview[destsplit].statsf[STAT_MONSTERS]++;
			break;
		case svc_foundsecret:
			//fixme: update all player stats
			cl.playerview[destsplit].stats[STAT_SECRETS]++;
			cl.playerview[destsplit].statsf[STAT_SECRETS]++;
			break;

		case svcqw_updatestatbyte:
			i = MSG_ReadByte ();
			j = MSG_ReadByte ();
			CL_SetStatFloat (destsplit, i, j);
			CL_SetStatInt (destsplit, i, j);
			break;
		case svcqw_updatestatlong:
			i = MSG_ReadByte ();
			j = MSG_ReadLong ();	//make qbyte if nq compatability?
			CL_SetStatFloat (destsplit, i, j);
			CL_SetStatInt (destsplit, i, j);
			break;

		case svcfte_updatestatstring:
			i = MSG_ReadByte();
			s = MSG_ReadString();
			CL_SetStatString (destsplit, i, s);
			break;
		case svcfte_updatestatfloat:
			i = MSG_ReadByte();
			f = MSG_ReadFloat();
			CL_SetStatInt (destsplit, i, f);
			CL_SetStatFloat (destsplit, i, f);
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound (false);
			break;

		case svc_cdtrack:
			{
				//quakeworld got a crippled svc_cdtrack.
				unsigned int firsttrack;
				firsttrack = MSG_ReadByte ();
				Media_NumberedTrack (firsttrack, firsttrack);
			}
			break;

		case svc_intermission:
			if (!cl.intermission)
			{
				TP_ExecTrigger ("f_mapend", false);
				if (cl.spectator)
					TP_ExecTrigger ("f_specmapend", true);
			}
			cl.intermission = 1;
			cl.completed_time = cl.gametime;
			for (i=0 ; i<3 ; i++)
				cl.playerview[destsplit].simorg[i] = MSG_ReadCoord ();
			for (i=0 ; i<3 ; i++)
				cl.playerview[destsplit].intermissionangles[i] = MSG_ReadAngle ();
			break;

		case svc_finale:
			if (!cl.intermission)
			{
				for (i = 0; i < MAX_SPLITS; i++)
					cl.playerview[i].simorg[2] += cl.playerview[i].viewheight;
				VectorCopy (cl.playerview[destsplit].simangles, cl.playerview[destsplit].intermissionangles);
			}

			cl.intermission = 2;
			cl.completed_time = cl.gametime;
			SCR_CenterPrint (destsplit, MSG_ReadString (), false);
			break;

		case svc_sellscreen:
			Cmd_ExecuteString ("help", RESTRICT_SERVER);
			break;

		case svc_smallkick:
			cl.playerview[destsplit].punchangle = -2;
			break;
		case svc_bigkick:
			cl.playerview[destsplit].punchangle = -4;
			break;

		case svc_muzzleflash:
			CL_MuzzleFlash (MSGCL_ReadEntity());
			break;

		case svc_updateuserinfo:
			CL_UpdateUserinfo ();
			break;

		case svc_setinfo:
			CL_ParseSetInfo ();
			break;

		case svc_serverinfo:
			CL_ServerInfo ();
			break;

		case svc_download:
			CL_ParseDownload ();
			break;

		case svc_playerinfo:
			CL_ParsePlayerinfo ();
			break;

		case svc_nails:
			CL_ParseProjectiles (cl_spikeindex, false);
			break;
		case svc_nails2:
			CL_ParseProjectiles (cl_spikeindex, true);
			break;

		case svc_chokecount:		// some preceding packets were choked
			i = MSG_ReadByte ();
			for (j=0 ; j<i ; j++)
				cl.outframes[(cls.netchan.incoming_acknowledged-1-j)&UPDATE_MASK].latency = -2;
			break;

		case svc_modellist:
			CL_ParseModellist (false);
			break;
		case svcfte_modellistshort:
			CL_ParseModellist (true);
			break;

		case svc_soundlist:
			CL_ParseSoundlist (false);
			break;
#ifdef PEXT_SOUNDDBL
		case svcfte_soundlistshort:
			CL_ParseSoundlist (true);
			break;
#endif

		case svc_packetentities:
			CLQW_ParsePacketEntities (false);
			break;

		case svc_deltapacketentities:
			CLQW_ParsePacketEntities (true);
			break;
		case svcfte_updateentities:
			CLFTE_ParseEntities();
			break;

		case svc_maxspeed:
			cl.playerview[destsplit].maxspeed = MSG_ReadFloat();
			break;

		case svc_entgravity:
			cl.playerview[destsplit].entgravity = MSG_ReadFloat();
			break;

		case svc_setpause:
			cl.paused = MSG_ReadByte ();
//			Media_SetPauseTrack(!!cl.paused);
			break;

//		case svc_ftesetclientpersist:
//			CL_ParseClientPersist();
//			break;
		case svc_setportalstate:
			CL_ParsePortalState();
			break;

		case svcfte_showpic:
			SCR_ShowPic_Create();
			break;
		case svcfte_hidepic:
			SCR_ShowPic_Hide();
			break;
		case svcfte_movepic:
			SCR_ShowPic_Move();
			break;
		case svcfte_updatepic:
			SCR_ShowPic_Update();
			break;

		case svcfte_effect:
			CL_ParseEffect(false);
			break;
		case svcfte_effect2:
			CL_ParseEffect(true);
			break;

#ifdef PEXT_CSQC
		case svcfte_csqcentities:
			csqcpacket = true;
			CSQC_ParseEntities();
			break;
#endif
		case svcfte_precache:
			CL_ParsePrecache();
			break;

		case svcfte_trailparticles:
			CL_ParseTrailParticles();
			break;
		case svcfte_pointparticles:
			CL_ParsePointParticles(false);
			break;
		case svcfte_pointparticles1:
			CL_ParsePointParticles(true);
			break;

		case svcfte_cgamepacket:
			csqcpacket = true;
#ifdef CSQC_DAT
			if (CSQC_ParseGamePacket())
				break;
#endif
#ifdef HLCLIENT
			if (CLHL_ParseGamePacket())
				break;
#endif
			Con_Printf("Unable to parse gamecode packet\n");
			break;
		}
	}
}

#ifdef Q2CLIENT
void CLQ2_ParseServerMessage (void)
{
	int			cmd;
	char		*s;
	int			i;
//	int			j;

	received_framecount = host_framecount;
	cl.last_servermessage = realtime;
	CL_ClearProjectiles ();

//
// if recording demos, copy the message out
//
	if (cl_shownet.value == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.value == 2)
		Con_Printf ("------------------\n");


	CL_ParseClientdata ();

//
// parse the message
//
	while (1)
	{
		if (msg_badread)
		{
			Host_EndGame ("CLQ2_ParseServerMessage: Bad server message");
			break;
		}

		cmd = MSG_ReadByte ();

		if (cmd == -1)
		{
			msg_readcount++;	// so the EOM showner has the right value
			SHOWNET("END OF MESSAGE");
			break;
		}

		SHOWNET(va("%i", cmd));

	// other commands
		switch (cmd)
		{
		default:
			Host_EndGame ("CLQ2_ParseServerMessage: Illegible server message (%i)", cmd);
			return;

	//known to game
		case svcq2_muzzleflash:
			CLQ2_ParseMuzzleFlash();
			break;
		case svcq2_muzzleflash2:
			CLQ2_ParseMuzzleFlash2();
			return;
		case svcq2_temp_entity:
			CLQ2_ParseTEnt();
			break;
		case svcq2_layout:
			s = MSG_ReadString ();
			Q_strncpyz (cl.q2layout, s, sizeof(cl.q2layout));
#ifdef VM_UI
			UI_Q2LayoutChanged();
#endif
			break;
		case svcq2_inventory:
			CLQ2_ParseInventory();
			break;

	// the rest are private to the client and server
		case svcq2_nop:			//6
			Host_EndGame ("CL_ParseServerMessage: svcq2_nop not implemented");
			return;
		case svcq2_disconnect:
			if (cls.state == ca_connected)
				Host_EndGame ("Server disconnected\n"
					"Server version may not be compatible");
			else
				Host_EndGame ("Server disconnected");
			return;
		case svcq2_reconnect:	//8
			Con_TPrintf ("reconnecting...\n");
			CL_SendClientCommand(true, "new");
			break;
		case svcq2_sound:		//9			// <see code>
			CLQ2_ParseStartSoundPacket();
			break;
		case svcq2_print:		//10			// [qbyte] id [string] null terminated string
			i = MSG_ReadByte ();
			s = MSG_ReadString ();

			CL_ParsePrint(s, i);
			break;
		case svcq2_stufftext:	//11			// [string] stuffed into client's console buffer, should be \n terminated
			s = MSG_ReadString ();
			Con_DPrintf ("stufftext: %s\n", s);
			if (!strncmp(s, "precache", 8))	//big major hack. Q2 uses a command that q1 has as a cvar.
			{	//call the q2 precache function.
				CLQ2_Precache_f();
			}
			else
				Cbuf_AddText (s, RESTRICT_SERVER);	//don't let the local user cheat
			break;
		case svcq2_serverdata:	//12			// [long] protocol ...
			Cbuf_Execute ();		// make sure any stuffed commands are done
			CLQ2_ParseServerData ();
			break;
		case svcq2_configstring:	//13		// [short] [string]
			CLQ2_ParseConfigString();
			break;
		case svcq2_spawnbaseline://14
			CLQ2_ParseBaseline();
			break;
		case svcq2_centerprint:	//15		// [string] to put in center of the screen
			s = MSG_ReadString();

#ifdef PLUGINS
			if (Plug_CenterPrintMessage(s, 0))
#endif
				SCR_CenterPrint (0, s, false);
			break;
		case svcq2_download:		//16		// [short] size [size bytes]
			CL_ParseDownload();
			break;
		case svcq2_playerinfo:	//17			// variable
			Host_EndGame ("CL_ParseServerMessage: svcq2_playerinfo not implemented");
			return;
		case svcq2_packetentities://18			// [...]
			Host_EndGame ("CL_ParseServerMessage: svcq2_packetentities not implemented");
			return;
		case svcq2_deltapacketentities://19	// [...]
			Host_EndGame ("CL_ParseServerMessage: svcq2_deltapacketentities not implemented");
			return;
		case svcq2_frame:			//20 (the bastard to implement.)
			CLQ2_ParseFrame();
			break;
		}
	}
	CL_SetSolidEntities ();
}
#endif

#ifdef NQPROT
//Proquake specific stuff
#define pqc_nop			1
#define pqc_new_team	2
#define pqc_erase_team	3
#define pqc_team_frags	4
#define	pqc_match_time	5
#define pqc_match_reset	6
#define pqc_ping_times	7
int MSG_ReadBytePQ (char **s)
{
	int ret = (*s)[0] * 16 + (*s)[1] - 272;
	*s+=2;
	return ret;
}
int MSG_ReadShortPQ (char **s)
{
	return MSG_ReadBytePQ(s) * 256 + MSG_ReadBytePQ(s);
}
void CLNQ_ParseProQuakeMessage (char *s)
{
	int cmd;
	int ping;
	int team, shirt, frags;

	s++;
	cmd = *s++;

	switch (cmd)
	{
	default:
		Con_DPrintf("Unrecognised ProQuake Message %i\n", cmd);
		break;
	case pqc_new_team:
		cl.teamplay = true;
		team = MSG_ReadBytePQ(&s) - 16;
		shirt = MSG_ReadBytePQ(&s) - 16;
		Sbar_PQ_Team_New(team, shirt);
		break;

	case pqc_erase_team:
		team = MSG_ReadBytePQ(&s) - 16;
		Sbar_PQ_Team_New(team, 0);
		Sbar_PQ_Team_Frags(team, 0);
		break;

	case pqc_team_frags:
		team = MSG_ReadBytePQ(&s) - 16;
		frags = MSG_ReadShortPQ(&s);
		if (frags & 32768)
			frags = frags - 65536;
		Sbar_PQ_Team_Frags(team, frags);
		break;

	case pqc_match_time:
		cl.matchgametimestart = MSG_ReadBytePQ(&s)*60;
		cl.matchgametimestart += MSG_ReadBytePQ(&s);
		cl.matchgametimestart = cl.gametime - cl.matchgametimestart;
		break;

	case pqc_match_reset:
		Sbar_PQ_Team_Reset();
		break;

	case pqc_ping_times:
		cl.last_ping_request = realtime;
		while ((ping = MSG_ReadShortPQ(&s)))
		{
			if ((ping / 4096) >= MAX_CLIENTS)
				Host_Error ("CL_ParseProQuakeMessage: pqc_ping_times > MAX_CLIENTS");
			cl.players[ping / 4096].ping = ping & 4095;
		}
		break;
	}
}

static enum {
	CLNQPP_NONE,
	CLNQPP_PINGS
} cl_nqparseprint;
qboolean CLNQ_ParseNQPrints(char *s)
{
	int i;
	char *start = s;
	if (cl_nqparseprint == CLNQPP_PINGS)
	{
		char *pingstart;
		cl_nqparseprint = CLNQPP_NONE;
		while(*s == ' ')
			s++;
		pingstart = s;
		if (*s == '-')
			s++;
		if (*s >= '0' && *s <= '9')
		{
			while(*s >= '0' && *s <= '9')
				s++;
			if (*s == ' ' && s-start >= 3)
			{
				s++;
				start = s;
				s = strchr(s, '\n');
				if (!s)
					return false;
				*s = 0;

				for (i = 0; i < cl.allocated_client_slots; i++)
				{
					if (!strcmp(start, cl.players[i].name))
						break;
				}
				if (i == cl.allocated_client_slots)
				{

				}
				if (i != cl.allocated_client_slots)
				{
					cl.players[i].ping = atoi(pingstart);
				}
				cl_nqparseprint = CLNQPP_PINGS;
				return true;
			}
		}

		s = start;
	}

	if (!strcmp(s, "Client ping times:\n"))
	{
		cl_nqparseprint = CLNQPP_PINGS;
		return true;
	}

	return false;
}

void CLNQ_ParseServerMessage (void)
{
	const int	destsplit = 0;
	int			cmd;
	char		*s;
	int			i, j;

//	received_framecount = host_framecount;
//	cl.last_servermessage = realtime;
	CL_ClearProjectiles ();

//
// if recording demos, copy the message out
//
	if (cl_shownet.value == 1)
		Con_Printf ("%i ",net_message.cursize);
	else if (cl_shownet.value == 2)
		Con_Printf ("------------------\n");


	CL_ParseClientdata ();
//
// parse the message
//
	while (1)
	{
		if (msg_badread)
		{
			CL_DumpPacket();
			Host_EndGame ("CL_ParseServerMessage: Bad server message");
			break;
		}

		cmd = MSG_ReadByte ();

		if (cmd == -1)
		{
			msg_readcount++;	// so the EOM showner has the right value
			SHOWNET("END OF MESSAGE");
			break;
		}

		if (cmd & 128)
		{
			SHOWNET("fast update");
			CLNQ_ParseEntity(cmd&127);
			continue;
		}

		SHOWNET2(svc_nqstrings[cmd>(sizeof(svc_nqstrings)/sizeof(char*))?0:cmd], cmd);

	// other commands
		switch (cmd)
		{
		default:
			CL_DumpPacket();
			Host_EndGame ("CLNQ_ParseServerMessage: Illegible server message (%i)", cmd);
			return;

		case svc_nop:
//			Con_Printf ("svc_nop\n");
			break;

		case svc_print:
			s = MSG_ReadString ();

			if (*s == 1 || *s == 2)
				CL_ParsePrint(s+1, PRINT_CHAT);
			else if (CLNQ_ParseNQPrints(s))
				break;
			else
				CL_ParsePrint(s, PRINT_HIGH);
			break;

		case svc_disconnect:
			CL_Disconnect();
			return;

		case svc_centerprint:
			s = MSG_ReadString ();

#ifdef PLUGINS
			if (Plug_CenterPrintMessage(s, 0))
#endif
				SCR_CenterPrint (0, s, false);
			break;

		case svc_stufftext:
			s = MSG_ReadString ();
			if (*s == 1)
			{
				Con_DPrintf("Proquake: %s\n", s);
				CLNQ_ParseProQuakeMessage(s);
			}
			else
			{
				Con_DPrintf ("stufftext: %s\n", s);
				if (!strncmp(s, "cl_serverextension_download ", 14))
				{
					cl_dp_serverextension_download = true;
				}
				else if (!strncmp(s, "//svi ", 6))
				{
					Cmd_TokenizeString(s+2, false, false);
					Con_DPrintf("SERVERINFO: %s=%s\n", Cmd_Argv(1), Cmd_Argv(2));
					Info_SetValueForStarKey (cl.serverinfo, Cmd_Argv(1), Cmd_Argv(2), MAX_SERVERINFO_STRING);
					CL_CheckServerInfo();
				}
				else if (!strncmp(s, "\ncl_downloadbegin ", 17))
					CLDP_ParseDownloadBegin(s);
				else if (!strncmp(s, "\ncl_downloadfinished ", 17))
					CLDP_ParseDownloadFinished(s);
				else if (!strcmp(s, "\nstopdownload\n"))
				{
					if (cls.download)
						CL_DownloadFailed(cls.download->remotename, cls.download);
				}
				else if (!strncmp(s, "csqc_progname ", 14))
					COM_ParseOut(s+14, cl_dp_csqc_progsname, sizeof(cl_dp_csqc_progsname));
				else if (!strncmp(s, "csqc_progsize ", 14))
					cl_dp_csqc_progssize = atoi(s+14);
				else if (!strncmp(s, "csqc_progcrc ", 13))
					cl_dp_csqc_progscrc = atoi(s+13);
				else if (!strncmp(s, "cl_fullpitch ", 13) || !strncmp(s, "pq_fullpitch ", 13))
				{
					//
				}
				else
				{
					Cbuf_AddText (s, RESTRICT_SERVER);	//no cheating here...
				}
			}
			break;

		case svc_version:
			CLNQ_ParseProtoVersion();
			break;
		case svc_serverdata:
			Cbuf_Execute ();		// make sure any stuffed commands are done
			CLNQ_ParseServerData ();
			break;

		case svcdp_precache:
			CL_ParsePrecache();
			break;

		case svc_cdtrack:
			{
				unsigned int firsttrack;
				unsigned int looptrack;
				firsttrack = MSG_ReadByte ();
				looptrack = MSG_ReadByte ();
				Media_NumberedTrack (firsttrack, looptrack);
			}
			break;

		case svc_setview:
			i=MSGCL_ReadEntity();
			if (!cl.playerview[destsplit].viewentity)
			{
				if (!i || i > cl.allocated_client_slots)
					cl.playerview[destsplit].playernum = cl.allocated_client_slots;	//the mvd spectator slot.
				else
					cl.playerview[destsplit].playernum = (unsigned int)i-1;
			}
			cl.playerview[destsplit].viewentity = i;
			break;

		case svc_signonnum:
			i = MSG_ReadByte ();

			if (i <= cls.signon)
				Host_EndGame ("Received signon %i when at %i", i, cls.signon);
			cls.signon = i;
			CLNQ_SignonReply ();
			break;
		case svc_setpause:
			cl.paused = MSG_ReadByte ();
			if (cl.paused)
				CDAudio_Pause ();
			else
				CDAudio_Resume ();
			break;

		case svc_spawnstaticsound:
			CL_ParseStaticSound (false);
			break;

		case svc_spawnstatic:
			CL_ParseStatic (1);
			break;

		case svc_spawnbaseline:
			i = MSGCL_ReadEntity ();
			if (!CL_CheckBaselines(i))
				Host_EndGame("CLNQ_ParseServerMessage: svc_spawnbaseline failed with size %i", i);
			CL_ParseBaseline (cl_baselines + i);
			break;

		//PEXT_REPLACEMENTDELTAS
		case svcfte_updateentities:
			if (cls.signon == 4 - 1)
			{	// first update is the final signon stage
				cls.signon = 4;
				CLNQ_SignonReply ();
			}
			CLFTE_ParseEntities();
			break;
		case svcfte_spawnstatic2:
			CL_ParseStatic (2);
			break;
		case svcfte_spawnbaseline2:
			CL_ParseBaseline2 ();
			break;

		case svcfte_cgamepacket:
#ifdef HLCLIENT
			if (CLHL_ParseGamePacket())
				break;
#endif
#ifdef CSQC_DAT
			if (CSQC_ParseGamePacket())
				break;
#endif
			Con_Printf("Unable to parse gamecode packet\n");
			break;

		case svc_time:
			//fixme: move this stuff to a common place
//			cl.playerview[destsplit].oldfixangle = cl.playerview[destsplit].fixangle;
//			VectorCopy(cl.playerview[destsplit].fixangles, cl.playerview[destsplit].oldfixangles);
//			cl.playerview[destsplit].fixangle = false;
			if (cls.demoplayback)
			{
//				extern vec3_t demoangles;
//				cl.playerview[destsplit].fixangle = true;
//				VectorCopy(demoangles, cl.playerview[destsplit].fixangles);
			}

			cls.netchan.outgoing_sequence++;
			cls.netchan.incoming_sequence = cls.netchan.outgoing_sequence-1;
			cl.validsequence = cls.netchan.incoming_sequence;

			received_framecount = host_framecount;
			cl.last_servermessage = realtime;

			cl.oldgametime = cl.gametime;
			cl.oldgametimemark = cl.gametimemark;
			cl.gametime = MSG_ReadFloat();
			cl.gametimemark = realtime;

			{
				extern vec3_t demoangles;
				int fr = cls.netchan.incoming_sequence&UPDATE_MASK;
				if (cls.demoplayback)
				{
					cl.inframes[fr&UPDATE_MASK].packet_entities.fixangles[destsplit] = true;
					VectorCopy(demoangles, cl.inframes[fr&UPDATE_MASK].packet_entities.fixedangles[destsplit]);
				}
				else
					cl.inframes[fr&UPDATE_MASK].packet_entities.fixangles[destsplit] = false;
			}
			cl.inframes[cls.netchan.incoming_sequence&UPDATE_MASK].receivedtime = realtime;
			cl.inframes[cls.netchan.incoming_sequence&UPDATE_MASK].frameid = cls.netchan.incoming_sequence;

			if (CPNQ_IS_DP)
			{
				int n = cls.netchan.incoming_sequence&UPDATE_MASK, o = (cls.netchan.incoming_sequence-1)&UPDATE_MASK;
				cl.inframes[n].packet_entities.num_entities = cl.inframes[o].packet_entities.num_entities;
				if (cl.inframes[n].packet_entities.max_entities < cl.inframes[o].packet_entities.num_entities)
				{
					cl.inframes[n].packet_entities.max_entities = cl.inframes[o].packet_entities.max_entities;
					cl.inframes[n].packet_entities.entities = BZ_Realloc(cl.inframes[n].packet_entities.entities, sizeof(entity_state_t) *  cl.inframes[n].packet_entities.max_entities);
				}
				memcpy(cl.inframes[n].packet_entities.entities, cl.inframes[o].packet_entities.entities, sizeof(entity_state_t) * cl.inframes[o].packet_entities.num_entities);
				cl.inframes[n].packet_entities.servertime = cl.inframes[o].packet_entities.servertime;
			}
			else
			{
//				cl.inframes[(cls.netchan.incoming_sequence-1)&UPDATE_MASK].packet_entities = cl.frames[cls.netchan.incoming_sequence&UPDATE_MASK].packet_entities;
				cl.inframes[cl.validsequence&UPDATE_MASK].packet_entities.num_entities=0;
				cl.inframes[cl.validsequence&UPDATE_MASK].packet_entities.servertime = cl.gametime;
			}
			break;

		case svc_updatename:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				MSG_ReadString();
			else
			{
				strcpy(cl.players[i].name, MSG_ReadString());
				if (*cl.players[i].name)
					cl.players[i].userid = i+1;
				Info_SetValueForKey(cl.players[i].userinfo, "name", cl.players[i].name, sizeof(cl.players[i].userinfo));
			}
			break;

		case svc_updatefrags:
			Sbar_Changed ();
			i = MSG_ReadByte ();
			if (i >= MAX_CLIENTS)
				MSG_ReadShort();
			else
				cl.players[i].frags = MSG_ReadShort();
			break;
		case svc_updatecolors:
			{
				int a;
				i = MSG_ReadByte ();
				a = MSG_ReadByte ();
				if (i < cl.allocated_client_slots)
				{
					cl.players[i].rtopcolor = a&0x0f;
					cl.players[i].rbottomcolor = (a&0xf0)>>4;

					sprintf(cl.players[i].team, "%2d", cl.players[i].rbottomcolor);

					if (cls.state == ca_active)
						Skin_Find (&cl.players[i]);

					if (i == cl.playerview[destsplit].playernum)
						Skin_FlushPlayers();
					Sbar_Changed ();
					CL_NewTranslation (i);
				}
			}
			break;
		case svc_lightstyle:
			i = MSG_ReadByte ();
			if (i >= MAX_LIGHTSTYLES)
			{
				Con_Printf("svc_lightstyle: %i >= MAX_LIGHTSTYLES\n", i);
				MSG_ReadString();
				break;
			}
			R_UpdateLightStyle(i, MSG_ReadString(), 1, 1, 1);
			break;

		case svcnq_updatestatlong:
			i = MSG_ReadByte ();
			j = MSG_ReadLong ();
			CL_SetStatFloat (0, i, j);
			CL_SetStatInt (0, i, j);
			break;
		case svcdp_updatestatbyte:
			i = MSG_ReadByte ();
			j = MSG_ReadByte ();
			CL_SetStatFloat (0, i, j);
			CL_SetStatInt (0, i, j);
			break;
		case svcfte_updatestatstring:
			i = MSG_ReadByte();
			s = MSG_ReadString();
			CL_SetStatString (destsplit, i, s);
			break;
		case svcfte_updatestatfloat:
			i = MSG_ReadByte();
			{
			float f = MSG_ReadFloat();
			CL_SetStatInt (destsplit, i, f);
			CL_SetStatFloat (destsplit, i, f);
			}
			break;
		case svc_setangle:
			{
				inframe_t *inf = &cl.inframes[cls.netchan.incoming_sequence&UPDATE_MASK];
				inf->packet_entities.fixangles[destsplit] = true;
				for (i=0 ; i<3 ; i++)
					cl.playerview[destsplit].viewangles[i] = cl.playerview[destsplit].intermissionangles[i] = inf->packet_entities.fixedangles[destsplit][i] = MSG_ReadAngle ();
			}
			break;

		case svcnq_clientdata:
			CLNQ_ParseClientdata ();
			break;

		case svc_sound:
			CLNQ_ParseStartSoundPacket();
			break;
		case svc_stopsound:
			i = MSG_ReadShort();
			S_StopSound(i>>3, i&7);
			break;

		case svc_temp_entity:
			CL_ParseTEnt (true);
			break;

		case svc_particle:
			CLNQ_ParseParticleEffect ();
			break;

		case svc_killedmonster:
			cl.playerview[destsplit].stats[STAT_MONSTERS]++;
			cl.playerview[destsplit].statsf[STAT_MONSTERS]++;
			break;

		case svc_foundsecret:
			cl.playerview[destsplit].stats[STAT_SECRETS]++;
			cl.playerview[destsplit].statsf[STAT_SECRETS]++;
			break;

		case svc_intermission:
			if (!cl.intermission)
				TP_ExecTrigger ("f_mapend", false);
			cl.intermission = 1;
			cl.completed_time = cl.gametime;
			break;

		case svc_finale:
			cl.intermission = 2;
			cl.completed_time = cl.gametime;
			SCR_CenterPrint (0, MSG_ReadString (), false);
			break;

		case svc_cutscene:
			cl.intermission = 3;
			cl.completed_time = cl.gametime;
			SCR_CenterPrint (0, MSG_ReadString (), false);
			break;

		case svc_sellscreen:	//pantsie
			Cmd_ExecuteString ("help 0", RESTRICT_SERVER);
			break;

		case svc_damage:
			V_ParseDamage (&cl.playerview[destsplit]);
			break;

		case svcfitz_skybox:
			{
				extern cvar_t r_skyboxname;
				Cvar_Set(&r_skyboxname, MSG_ReadString());
			}
			break;
		case svcfitz_bf:
			Cmd_ExecuteString("bf", RESTRICT_SERVER);
			break;
		case svcfitz_fog:
			CL_ResetFog(0);
			cl.fog[0].density = MSG_ReadByte()/255.0f;
			cl.fog[0].colour[0] = MSG_ReadByte()/255.0f;
			cl.fog[0].colour[1] = MSG_ReadByte()/255.0f;
			cl.fog[0].colour[2] = MSG_ReadByte()/255.0f;
			cl.fog[0].time += ((unsigned short)MSG_ReadShort()) / 100.0;
			cl.fog_locked = !!cl.fog[0].density;
			break;
		case svcfitz_spawnbaseline2:
			i = MSGCL_ReadEntity ();
			if (!CL_CheckBaselines(i))
				Host_EndGame("CLNQ_ParseServerMessage: svcfitz_spawnbaseline2 failed with ent %i", i);
			CLFitz_ParseBaseline2 (cl_baselines + i);
			break;
		case svcfitz_spawnstatic2:
			CL_ParseStatic (3);
			break;
		case svcfitz_spawnstaticsound2:
			CL_ParseStaticSound(true);
			break;


		case svcnq_effect:
			CL_ParseEffect(false);
			break;
		case svcnq_effect2:
			CL_ParseEffect(true);
			break;

		case svcdp_entities:
			if (cls.signon == 4 - 1)
			{	// first update is the final signon stage
				cls.signon = 4;
				CLNQ_SignonReply ();
			}
			//well, it's really any protocol, but we're only going to support version 5.
			CLDP_ParseDarkPlaces5Entities();
			break;

		case svcdp_spawnstaticsound2:
			CL_ParseStaticSound(true);
			break;

#ifdef PEXT_CSQC
		case svcdp_csqcentities:
			CSQC_ParseEntities();
			break;
#endif

		case svcdp_downloaddata:
			CLDP_ParseDownloadData();
			break;

		case svcdp_trailparticles:
			CL_ParseTrailParticles();
			break;
		case svcdp_pointparticles:
			CL_ParsePointParticles(false);
			break;
		case svcdp_pointparticles1:
			CL_ParsePointParticles(true);
			break;
		}

	}
}
#endif

