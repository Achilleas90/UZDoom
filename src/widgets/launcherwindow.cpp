/*
** launcherwindow.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2024 Magnus Norddahl
** Copyright 2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include <zwidget/core/resourcedata.h>
#include <zwidget/widgets/tabwidget/tabwidget.h>
#include <zwidget/window/window.h>

#include "aboutpage.h"
#include "gstrings.h"
#include "i_interface.h"
#include "launcherbanner.h"
#include "launcherbuttonbar.h"
#include "launcherwindow.h"
#include "networkpage.h"
#include "playgamepage.h"
#include "quicksetuppage.h"
#include "releasepage.h"
#include "settingspage.h"
#include "version.h"

bool LauncherWindow::ExecModal(FStartupSelectionInfo& info)
{
	Size screenSize = GetScreenSize();
	double windowWidth = 615.0;
	double windowHeight = 700.0;

	auto launcher = std::make_unique<LauncherWindow>(info);
	launcher->SetFrameGeometry((screenSize.width - windowWidth) * 0.5, (screenSize.height - windowHeight) * 0.5, windowWidth, windowHeight);
	launcher->Show();

	DisplayWindow::RunLoop();

	return launcher->ExecResult;
}

LauncherWindow::LauncherWindow(FStartupSelectionInfo& info) : Widget(nullptr, WidgetType::Window), Info(&info)
{
	SetWindowTitle(GAMENAME);

	Banner = new LauncherBanner(this);
	Pages = new TabWidget(this);
	Buttonbar = new LauncherButtonbar(this);

	bool releasenotes = info.isNewRelease && info.notifyNewRelease;

	PlayGame = new PlayGamePage(this, info);
	Settings = new SettingsPage(this, info);
	Network = new NetworkPage(this, info);
	About = new AboutPage(this, info);
	if (info.ShowQuickSetup)
	{
		QuickSetup = new QuickSetupPage(this, info);
		Pages->AddTab(QuickSetup, GStrings.GetString("PICKER_TAB_QUICKSETUP"));
	}

	if (releasenotes)
	{
		Release = new ReleasePage(this, info);
		Pages->AddTab(Release, "Release Notes");
	}

	Pages->AddTab(PlayGame, "Play");
	Pages->AddTab(Settings, "Settings");
	Pages->AddTab(Network, "Multiplayer");
	Pages->AddTab(About, "About");

	Network->InitializeTabs(info);

	UpdateLanguage();

	Pages->SetCurrentIndex(0);
	Pages->GetCurrentWidget()->SetFocus();
}

void LauncherWindow::UpdatePlayButton()
{
	Buttonbar->UpdateLanguage();
}

bool LauncherWindow::IsInMultiplayer() const
{
	return Pages->GetCurrentIndex() >= 0 ? Pages->GetCurrentWidget() == Network : false;
}

bool LauncherWindow::IsHosting() const
{
	return IsInMultiplayer() && Network->IsInHost();
}

void LauncherWindow::Start()
{
	FString warning;
	if (!GetLaunchWarning(warning))
	{
		Buttonbar->ShowLaunchWarning(warning.GetChars());
		return;
	}

	Info->bNetStart = IsInMultiplayer();

	CaptureInfoFromPages();

	if (Release)
		Release->SetValues(*Info);

	ExecResult = true;
	DisplayWindow::ExitLoop();
}

void LauncherWindow::Exit()
{
	ExecResult = false;
	DisplayWindow::ExitLoop();
}

void LauncherWindow::UpdateLanguage()
{
	Pages->SetTabText(PlayGame, GStrings.GetString("PICKER_TAB_PLAY"));
	Pages->SetTabText(Settings, GStrings.GetString("OPTMNU_TITLE"));
	Pages->SetTabText(Network, GStrings.GetString("PICKER_TAB_MULTI"));
	Pages->SetTabText(About, GStrings.GetString("PICKER_TAB_ABOUT"));
	if (QuickSetup) Pages->SetTabText(QuickSetup, GStrings.GetString("PICKER_TAB_QUICKSETUP"));
	PlayGame->UpdateLanguage();
	Settings->UpdateLanguage();
	Network->UpdateLanguage();
	About->UpdateLanguage();
	if (QuickSetup) QuickSetup->UpdateLanguage();
	if (Release)
	{
		Pages->SetTabText(Release, GStrings.GetString("PICKER_TAB_RELEASE"));
		Release->UpdateLanguage();
	}
	Buttonbar->UpdateLanguage();
}

void LauncherWindow::CaptureInfoFromPages()
{
	Info->bNetStart = IsInMultiplayer();
	if (QuickSetup)
	{
		QuickSetup->SetValues(*Info);
	}
	Settings->SetValues(*Info);
	if (Info->bNetStart)
		Network->SetValues(*Info);
	else
		PlayGame->SetValues(*Info);
}

void LauncherWindow::ApplyInfoToPages()
{
	PlayGame->ApplyValues(*Info);
	Settings->ApplyValues(*Info);
	Network->ApplyValues(*Info);
	if (QuickSetup) QuickSetup->ApplyValues(*Info);
	Buttonbar->UpdateLanguage();
}

void LauncherWindow::ApplyProfile(int index)
{
	CaptureInfoFromPages();
	Info->ApplyProfile(index);
	ApplyInfoToPages();
}

void LauncherWindow::DuplicateProfile()
{
	CaptureInfoFromPages();
	Info->DuplicateActiveProfile();
	ApplyInfoToPages();
}

void LauncherWindow::NewProfile()
{
	CaptureInfoFromPages();
	FStartupProfile p = Info->Profiles[Info->ActiveProfile];
	p.Name = "New Profile";
	Info->ActiveProfile = Info->Profiles.Push(p);
	Info->SaveActiveProfile();
	ApplyInfoToPages();
}

void LauncherWindow::DeleteProfile()
{
	CaptureInfoFromPages();
	Info->DeleteActiveProfile();
	ApplyInfoToPages();
}

FString LauncherWindow::GetCommandPreview()
{
	Info->bNetStart = IsInMultiplayer();
	CaptureInfoFromPages();
	return Info->BuildCommandPreview();
}

bool LauncherWindow::GetLaunchWarning(FString& warning)
{
	Info->bNetStart = IsInMultiplayer();
	CaptureInfoFromPages();
	return Info->ValidateSelection(warning);
}

void LauncherWindow::OnClose()
{
	Exit();
}

void LauncherWindow::OnGeometryChanged()
{
	double top = 0.0;
	double bottom = GetHeight();

	Banner->SetFrameGeometry(0.0, top, GetWidth(), Banner->GetPreferredHeight());
	top += Banner->GetPreferredHeight();

	bottom -= Buttonbar->GetPreferredHeight();
	Buttonbar->SetFrameGeometry(0.0, bottom, GetWidth(), Buttonbar->GetPreferredHeight());

	Pages->SetFrameGeometry(0.0, top, GetWidth(), std::max(bottom - top, 0.0));
}
