/*
** launcherbuttonbar.cpp
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

#include "launcherbuttonbar.h"
#include "launcherwindow.h"
#include "gstrings.h"
#include "g_input.h"
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/lineedit/lineedit.h>

LauncherButtonbar::LauncherButtonbar(LauncherWindow* parent) : Widget(parent)
{
	PlayButton = new PushButton(this);
	ExitButton = new PushButton(this);
	CopyButton = new PushButton(this);
	PreviewEdit = new LineEdit(this);
	PreviewEdit->SetReadOnly(true);
	PreviewEdit->SetSelectAllOnFocusGain(false);

	PlayButton->OnClick = [=]() { OnPlayButtonClicked(); };
	ExitButton->OnClick = [=]() { OnExitButtonClicked(); };
	CopyButton->OnClick = [=]() { OnCopyButtonClicked(); };
}

void LauncherButtonbar::UpdateLanguage()
{
	auto launcher = GetLauncher();
	if (!launcher->IsInMultiplayer())
		PlayButton->SetText(GStrings.GetString("PICKER_PLAY"));
	else if (launcher->IsHosting())
		PlayButton->SetText(GStrings.GetString("PICKER_PLAYHOST"));
	else
		PlayButton->SetText(GStrings.GetString("PICKER_PLAYJOIN"));

	ExitButton->SetText(GStrings.GetString("PICKER_EXIT"));
	CopyButton->SetText(GStrings.GetString("ACTION_COPYTOCLIPBOARD"));
	PreviewEdit->SetText(launcher->GetCommandPreview().GetChars());
}

double LauncherButtonbar::GetPreferredHeight() const
{
	return 52.0 + std::max(PlayButton->GetPreferredHeight(), ExitButton->GetPreferredHeight());
}

void LauncherButtonbar::OnGeometryChanged()
{
	const double y = 10.0;
	PlayButton->SetFrameGeometry(20.0, y, 120.0, PlayButton->GetPreferredHeight());
	ExitButton->SetFrameGeometry(GetWidth() - 20.0 - 120.0, y, 120.0, PlayButton->GetPreferredHeight());

	const double inputY = y + PlayButton->GetPreferredHeight() + 8.0;
	const double copyWidth = 170.0;
	PreviewEdit->SetFrameGeometry(20.0, inputY, std::max(50.0, GetWidth() - 60.0 - copyWidth), 24.0);
	CopyButton->SetFrameGeometry(GetWidth() - 20.0 - copyWidth, inputY, copyWidth, 24.0);
}

void LauncherButtonbar::OnPlayButtonClicked()
{
	FString warning;
	if (!GetLauncher()->GetLaunchWarning(warning))
	{
		ShowLaunchWarning(warning.GetChars());
		return;
	}
	GetLauncher()->Start();
}

void LauncherButtonbar::OnExitButtonClicked()
{
	GetLauncher()->Exit();
}

void LauncherButtonbar::OnCopyButtonClicked()
{
	I_PutInClipboard(GetLauncher()->GetCommandPreview().GetChars());
}

void LauncherButtonbar::ShowLaunchWarning(const std::string& warning)
{
	PreviewEdit->SetText(warning);
}

LauncherWindow* LauncherButtonbar::GetLauncher() const
{
	return static_cast<LauncherWindow*>(Parent());
}
