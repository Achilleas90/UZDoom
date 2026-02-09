/*
** i_interface.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2019-2025 GZDoom Maintainers and Contributors
** Copyright 2020 Christoph Oelckers
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "i_interface.h"
#include "st_start.h"
#include "gamestate.h"
#include "startupinfo.h"
#include "c_cvars.h"
#include "gstrings.h"
#include "version.h"
#include "m_argv.h"
#include "m_random.h"
#include "serializer.h"
#include "cmdlib.h"
#include <cstring>

static_assert(sizeof(void*) == 8,
	"Only LP64/LLP64 builds are officially supported. "
	"Please do not attempt to build for other platforms; "
	"even if the program succeeds in a MAP01 smoke test, "
	"there are e.g. known visual artifacts "
	"<https://forum.zdoom.org/viewtopic.php?f=7&t=75673> "
	"that lead to a bad user experience.");

// Some global engine variables taken out of the backend code.
FStartupScreen* StartWindow;
SystemCallbacks sysCallbacks;
FString endoomName;
bool batchrun;
float menuBlurAmount;

bool AppActive = true;
int chatmodeon;
gamestate_t 	gamestate = GS_STARTUP;
bool ToggleFullscreen;
int 			paused;
bool			pauseext;

FStartupInfo GameStartupInfo;

CVAR(Bool, vid_fps, false, 0)
CVAR(Bool, queryiwad, QUERYIWADDEFAULT, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, saveargs, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, savenetfile, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, savenetargs, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, defaultiwad, "", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, defaultargs, "", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, defaultnetiwad, "", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, defaultnetargs, "", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnetplayers, 8, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnethostport, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnetticdup, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnetgamemode, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, defaultnetaltdm, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, defaultnetaddress, "", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnetjoinport, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnetpage, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnethostteam, 255, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, defaultnetjointeam, 255, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, defaultnetextratic, false, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, defaultnetsavefile, "", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(String, startup_profiles, "", CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Int, startup_active_profile, 0, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)
CVAR(Bool, startup_show_quicksetup, true, CVAR_ARCHIVE | CVAR_GLOBALCONFIG)

EXTERN_CVAR(Bool, ui_generic)
EXTERN_CVAR(Int, vid_preferbackend)
EXTERN_CVAR(Bool, vid_fullscreen)

FSerializer& Serialize(FSerializer& arc, const char* key, FStartupProfile& p, FStartupProfile* def);

CUSTOM_CVAR(String, language, "auto", CVAR_ARCHIVE | CVAR_NOINITCALL | CVAR_GLOBALCONFIG)
{
	GStrings.UpdateLanguage(self);
	UpdateGenericUI(ui_generic);
	if (sysCallbacks.LanguageChanged) sysCallbacks.LanguageChanged(self);
}

// Some of this info has to be passed and managed from the front end since it's game-engine specific.
FStartupSelectionInfo::FStartupSelectionInfo(const TArray<WadStuff>& wads, FArgs& args, int startFlags) : Wads(&wads), Args(&args), DefaultStartFlags(startFlags)
{
	DefaultQueryIWAD = queryiwad;
	DefaultLanguage = language;
	DefaultBackend = vid_preferbackend;
	DefaultFullscreen = vid_fullscreen;
	ShowQuickSetup = startup_show_quicksetup;

	if (defaultiwad[0] != '\0')
	{
		for (int i = 0; i < wads.SSize(); ++i)
		{
			if (!wads[i].Name.CompareNoCase(defaultiwad))
			{
				DefaultIWAD = i;
				break;
			}
		}
	}
	DefaultArgs = defaultargs;
	bSaveArgs = saveargs;

	if (defaultnetiwad[0] != '\0')
	{
		for (int i = 0; i < wads.SSize(); ++i)
		{
			if (!wads[i].Name.CompareNoCase(defaultnetiwad))
			{
				DefaultNetIWAD = i;
				break;
			}
		}
	}
	DefaultNetArgs = defaultnetargs;
	DefaultNetPage = defaultnetpage;
	DefaultNetSaveFile = defaultnetsavefile;
	bSaveNetFile = savenetfile;
	bSaveNetArgs = savenetargs;

	DefaultNetPlayers = defaultnetplayers;
	DefaultNetHostPort = defaultnethostport;
	DefaultNetTicDup = defaultnetticdup;
	DefaultNetGameMode = defaultnetgamemode;
	DefaultNetAltDM = defaultnetaltdm;
	DefaultNetHostTeam = defaultnethostteam;
	DefaultNetExtraTic = defaultnetextratic;

	DefaultNetAddress = defaultnetaddress;
	DefaultNetJoinPort = defaultnetjoinport;
	DefaultNetJoinTeam = defaultnetjointeam;

	FStartupProfile legacy;
	legacy.Name = "Default";
	legacy.IWADName = wads[DefaultIWAD].Name;
	legacy.Args = DefaultArgs;
	legacy.SaveArgs = bSaveArgs;
	legacy.StartFlags = DefaultStartFlags;
	legacy.QueryIWAD = DefaultQueryIWAD;
	legacy.Language = DefaultLanguage;
	legacy.Backend = DefaultBackend;
	legacy.Fullscreen = DefaultFullscreen;
	legacy.FileLoadBehaviour = DefaultFileLoadBehaviour;
	legacy.NotifyNewRelease = notifyNewRelease;
	legacy.NetIWADName = wads[DefaultNetIWAD].Name;
	legacy.NetPage = DefaultNetPage;
	legacy.NetArgs = DefaultNetArgs;
	legacy.SaveNetFile = bSaveNetFile;
	legacy.SaveNetArgs = bSaveNetArgs;
	legacy.NetSaveFile = DefaultNetSaveFile;
	legacy.NetHostTeam = DefaultNetHostTeam;
	legacy.NetPlayers = DefaultNetPlayers;
	legacy.NetHostPort = DefaultNetHostPort;
	legacy.NetTicDup = DefaultNetTicDup;
	legacy.NetExtraTic = DefaultNetExtraTic;
	legacy.NetGameMode = DefaultNetGameMode;
	legacy.NetAltDM = DefaultNetAltDM;
	legacy.NetAddress = DefaultNetAddress;
	legacy.NetJoinPort = DefaultNetJoinPort;
	legacy.NetJoinTeam = DefaultNetJoinTeam;

	Profiles.Clear();
	if (startup_profiles[0] != '\0')
	{
		FSerializer arc;
		if (arc.OpenReader(startup_profiles, strlen(startup_profiles)))
		{
			arc("profiles", Profiles);
			arc("active", ActiveProfile);
			arc("recentjoin", RecentJoinEndpoints);
		}
	}
	if (Profiles.Size() == 0)
	{
		Profiles.Push(legacy);
		ActiveProfile = 0;
	}
	else
	{
		ActiveProfile = clamp<int>(startup_active_profile, 0, Profiles.Size() - 1);
	}
	ApplyProfile(ActiveProfile);
}

// Return whatever IWAD the user selected.
int FStartupSelectionInfo::SaveInfo()
{
	DefaultLanguage.StripLeftRight();

	DefaultArgs.StripLeftRight();

	DefaultNetArgs.StripLeftRight();
	AdditionalNetArgs.StripLeftRight();
	DefaultNetAddress.StripLeftRight();
	DefaultNetSaveFile.StripLeftRight();
	SaveActiveProfile();

	queryiwad = DefaultQueryIWAD;
	language = DefaultLanguage.GetChars();
	vid_fullscreen = DefaultFullscreen;
	startup_show_quicksetup = ShowQuickSetup;
	if (DefaultBackend != vid_preferbackend)
		vid_preferbackend = DefaultBackend;

	if (bNetStart)
	{
		savenetfile = bSaveNetFile;
		savenetargs = bSaveNetArgs;

		defaultnetiwad = (*Wads)[DefaultNetIWAD].Name.GetChars();
		defaultnetpage = DefaultNetPage;
		defaultnetsavefile = savenetfile ? DefaultNetSaveFile.GetChars() : "";
		defaultnetargs = savenetargs ? DefaultNetArgs.GetChars() : "";

		if (bHosting)
		{
			defaultnetplayers = DefaultNetPlayers;
			defaultnethostport = DefaultNetHostPort;
			defaultnetticdup = DefaultNetTicDup;
			defaultnetgamemode = DefaultNetGameMode;
			defaultnetaltdm = DefaultNetAltDM;
			defaultnethostteam = DefaultNetHostTeam;
			defaultnetextratic = DefaultNetExtraTic;
		}
		else
		{
			defaultnetaddress = DefaultNetAddress.GetChars();
			defaultnetjoinport = DefaultNetJoinPort;
			defaultnetjointeam = DefaultNetJoinTeam;
		}

		if (!DefaultNetArgs.IsEmpty())
			Args->AppendRawArgsString(DefaultNetArgs);
		if (!AdditionalNetArgs.IsEmpty())
			Args->AppendRawArgsString(AdditionalNetArgs);

		return DefaultNetIWAD;
	}

	defaultiwad = (*Wads)[DefaultIWAD].Name.GetChars();
	saveargs = bSaveArgs;
	defaultargs = saveargs ? DefaultArgs.GetChars() : "";

	if (!DefaultArgs.IsEmpty())
		Args->AppendRawArgsString(DefaultArgs);

	return DefaultIWAD;
}

static int FindIWADIndexByName(const TArray<WadStuff>* wads, const FString& name, int fallback)
{
	if (wads == nullptr || wads->Size() == 0) return 0;
	for (int i = 0; i < wads->SSize(); i++)
	{
		if (!(*wads)[i].Name.CompareNoCase(name)) return i;
	}
	return clamp<int>(fallback, 0, wads->SSize() - 1);
}

FSerializer& Serialize(FSerializer& arc, const char* key, FStartupProfile& p, FStartupProfile* def)
{
	if (arc.BeginObject(key))
	{
		arc("name", p.Name);
		arc("iwad", p.IWADName);
		arc("args", p.Args);
		arc("saveargs", p.SaveArgs);
		arc("startflags", p.StartFlags);
		arc("queryiwad", p.QueryIWAD);
		arc("language", p.Language);
		arc("backend", p.Backend);
		arc("fullscreen", p.Fullscreen);
		arc("fileloadbehaviour", p.FileLoadBehaviour);
		arc("notifynewrelease", p.NotifyNewRelease);
		arc("netiwad", p.NetIWADName);
		arc("netpage", p.NetPage);
		arc("netargs", p.NetArgs);
		arc("savenetfile", p.SaveNetFile);
		arc("savenetargs", p.SaveNetArgs);
		arc("netsavefile", p.NetSaveFile);
		arc("nethostteam", p.NetHostTeam);
		arc("netplayers", p.NetPlayers);
		arc("nethostport", p.NetHostPort);
		arc("netticdup", p.NetTicDup);
		arc("netextratic", p.NetExtraTic);
		arc("netgamemode", p.NetGameMode);
		arc("netaltdm", p.NetAltDM);
		arc("netaddress", p.NetAddress);
		arc("netjoinport", p.NetJoinPort);
		arc("netjointeam", p.NetJoinTeam);
		arc.EndObject();
	}
	return arc;
}

void FStartupSelectionInfo::ApplyProfile(int index)
{
	if (Profiles.Size() == 0) return;
	ActiveProfile = clamp<int>(index, 0, Profiles.Size() - 1);
	const auto& p = Profiles[ActiveProfile];

	DefaultIWAD = FindIWADIndexByName(Wads, p.IWADName, DefaultIWAD);
	DefaultArgs = p.Args;
	bSaveArgs = p.SaveArgs;
	DefaultStartFlags = p.StartFlags;
	DefaultQueryIWAD = p.QueryIWAD;
	DefaultLanguage = p.Language.IsEmpty() ? "auto" : p.Language;
	DefaultBackend = p.Backend;
	DefaultFullscreen = p.Fullscreen;
	DefaultFileLoadBehaviour = p.FileLoadBehaviour;
	notifyNewRelease = p.NotifyNewRelease;

	DefaultNetIWAD = FindIWADIndexByName(Wads, p.NetIWADName, DefaultNetIWAD);
	DefaultNetPage = p.NetPage;
	DefaultNetArgs = p.NetArgs;
	bSaveNetFile = p.SaveNetFile;
	bSaveNetArgs = p.SaveNetArgs;
	DefaultNetSaveFile = p.NetSaveFile;
	DefaultNetHostTeam = p.NetHostTeam;
	DefaultNetPlayers = p.NetPlayers;
	DefaultNetHostPort = p.NetHostPort;
	DefaultNetTicDup = p.NetTicDup;
	DefaultNetExtraTic = p.NetExtraTic;
	DefaultNetGameMode = p.NetGameMode;
	DefaultNetAltDM = p.NetAltDM;
	DefaultNetAddress = p.NetAddress;
	DefaultNetJoinPort = p.NetJoinPort;
	DefaultNetJoinTeam = p.NetJoinTeam;
}

void FStartupSelectionInfo::SaveActiveProfile()
{
	if (Profiles.Size() == 0) return;
	ActiveProfile = clamp<int>(ActiveProfile, 0, Profiles.Size() - 1);
	auto& p = Profiles[ActiveProfile];

	p.IWADName = (*Wads)[clamp<int>(DefaultIWAD, 0, Wads->SSize() - 1)].Name;
	p.Args = DefaultArgs;
	p.SaveArgs = bSaveArgs;
	p.StartFlags = DefaultStartFlags;
	p.QueryIWAD = DefaultQueryIWAD;
	p.Language = DefaultLanguage;
	p.Backend = DefaultBackend;
	p.Fullscreen = DefaultFullscreen;
	p.FileLoadBehaviour = DefaultFileLoadBehaviour;
	p.NotifyNewRelease = notifyNewRelease;

	p.NetIWADName = (*Wads)[clamp<int>(DefaultNetIWAD, 0, Wads->SSize() - 1)].Name;
	p.NetPage = DefaultNetPage;
	p.NetArgs = DefaultNetArgs;
	p.SaveNetFile = bSaveNetFile;
	p.SaveNetArgs = bSaveNetArgs;
	p.NetSaveFile = DefaultNetSaveFile;
	p.NetHostTeam = DefaultNetHostTeam;
	p.NetPlayers = DefaultNetPlayers;
	p.NetHostPort = DefaultNetHostPort;
	p.NetTicDup = DefaultNetTicDup;
	p.NetExtraTic = DefaultNetExtraTic;
	p.NetGameMode = DefaultNetGameMode;
	p.NetAltDM = DefaultNetAltDM;
	p.NetAddress = DefaultNetAddress;
	p.NetJoinPort = DefaultNetJoinPort;
	p.NetJoinTeam = DefaultNetJoinTeam;

	startup_active_profile = ActiveProfile;

	if (bNetStart && !bHosting)
	{
		FString endpoint = DefaultNetAddress;
		if (DefaultNetJoinPort > 0)
			endpoint.AppendFormat(":%d", DefaultNetJoinPort);
		endpoint.StripLeftRight();
		if (!endpoint.IsEmpty())
		{
			for (unsigned i = 0; i < RecentJoinEndpoints.Size(); i++)
			{
				if (!RecentJoinEndpoints[i].CompareNoCase(endpoint))
				{
					RecentJoinEndpoints.Delete(i);
					break;
				}
			}
			RecentJoinEndpoints.Insert(0, endpoint);
			while (RecentJoinEndpoints.Size() > 12)
				RecentJoinEndpoints.Delete(RecentJoinEndpoints.Size() - 1);
		}
	}

	FSerializer arc;
	if (arc.OpenWriter(false))
	{
		arc("profiles", Profiles);
		arc("active", ActiveProfile);
		arc("recentjoin", RecentJoinEndpoints);
		startup_profiles = arc.GetOutput();
	}
}

int FStartupSelectionInfo::DuplicateActiveProfile()
{
	if (Profiles.Size() == 0) return 0;
	SaveActiveProfile();
	auto copy = Profiles[ActiveProfile];
	copy.Name += " Copy";
	ActiveProfile = Profiles.Push(copy);
	SaveActiveProfile();
	return ActiveProfile;
}

bool FStartupSelectionInfo::DeleteActiveProfile()
{
	if (Profiles.Size() <= 1) return false;
	Profiles.Delete(ActiveProfile);
	ActiveProfile = clamp<int>(ActiveProfile, 0, Profiles.Size() - 1);
	ApplyProfile(ActiveProfile);
	SaveActiveProfile();
	return true;
}

FString FStartupSelectionInfo::BuildCommandPreview() const
{
	FString cmd;
	cmd.Format("uzdoom -iwad \"%s\"", (*Wads)[clamp<int>(bNetStart ? DefaultNetIWAD : DefaultIWAD, 0, Wads->SSize() - 1)].Name.GetChars());
	if (bNetStart)
	{
		if (bHosting)
		{
			cmd.AppendFormat(" -host %d", clamp<int>(DefaultNetPlayers, 1, 64));
			if (DefaultNetHostPort > 0) cmd.AppendFormat(" -port %d", DefaultNetHostPort);
			if (DefaultNetTicDup > 0) cmd.AppendFormat(" -dup %d", DefaultNetTicDup + 1);
			if (DefaultNetExtraTic) cmd += " -extratic";
		}
		else
		{
			if (!DefaultNetAddress.IsEmpty()) cmd.AppendFormat(" -join \"%s\"", DefaultNetAddress.GetChars());
			if (DefaultNetJoinPort > 0) cmd.AppendFormat(" -port %d", DefaultNetJoinPort);
		}
		if (!DefaultNetArgs.IsEmpty()) cmd.AppendFormat(" %s", DefaultNetArgs.GetChars());
		if (!AdditionalNetArgs.IsEmpty()) cmd.AppendFormat(" %s", AdditionalNetArgs.GetChars());
	}
	else if (!DefaultArgs.IsEmpty())
	{
		cmd.AppendFormat(" %s", DefaultArgs.GetChars());
	}
	return cmd;
}

bool FStartupSelectionInfo::ValidateSelection(FString& warning) const
{
	if (Wads == nullptr || Wads->Size() == 0)
	{
		warning = "No IWADs available.";
		return false;
	}
	const int iwadIndex = bNetStart ? DefaultNetIWAD : DefaultIWAD;
	if (iwadIndex < 0 || iwadIndex >= Wads->SSize())
	{
		warning = "Selected IWAD index is invalid.";
		return false;
	}
	if (bNetStart)
	{
		if (bHosting && (DefaultNetHostPort < 0 || DefaultNetHostPort > 65535))
		{
			warning = "Host port must be between 0 and 65535.";
			return false;
		}
		if (!bHosting)
		{
			if (DefaultNetAddress.IsEmpty())
			{
				warning = "Join address cannot be empty.";
				return false;
			}
			if (DefaultNetJoinPort < 0 || DefaultNetJoinPort > 65535)
			{
				warning = "Join port must be between 0 and 65535.";
				return false;
			}
		}
		if (!DefaultNetSaveFile.IsEmpty() && !FileExists(DefaultNetSaveFile))
		{
			warning = "Selected network save file does not exist.";
			return false;
		}
	}
	warning = {};
	return true;
}

FString GameUUID;
static FRandom pr_uuid("GameUUID");

FString GenerateUUID()
{
	FString uuid;
	uuid.AppendFormat("%08X-%04X-4%03X-9%03X-%08X%04X", pr_uuid.GenRand32(), pr_uuid(UINT16_MAX), pr_uuid(4095), pr_uuid(4095), pr_uuid.GenRand32(), pr_uuid(UINT16_MAX));
	return uuid;
}
