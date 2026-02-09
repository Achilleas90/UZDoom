/*
** quicksetuppage.cpp
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

#include "quicksetuppage.h"
#include "launcherwindow.h"
#include "i_interface.h"
#include "gstrings.h"

#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/checkboxlabel/checkboxlabel.h>
#include <zwidget/widgets/dropdown/dropdown.h>

QuickSetupPage::QuickSetupPage(LauncherWindow* launcher, const FStartupSelectionInfo& info) : Widget(nullptr), Launcher(launcher)
{
	TitleLabel = new TextLabel(this);
	IntroLabel = new TextLabel(this);
	FullscreenCheckbox = new CheckboxLabel(this);
	AskIWADCheckbox = new CheckboxLabel(this);
	AutoloadCheckbox = new CheckboxLabel(this);
	BackendLabel = new TextLabel(this);
	BackendDropdown = new Dropdown(this);
	ShowAgainCheckbox = new CheckboxLabel(this);

	BackendDropdown->AddItem("");
	BackendDropdown->AddItem("");
	BackendDropdown->AddItem("");

	FullscreenCheckbox->FuncChanged = [this](bool) { Launcher->UpdatePlayButton(); };
	AskIWADCheckbox->FuncChanged = [this](bool) { Launcher->UpdatePlayButton(); };
	AutoloadCheckbox->FuncChanged = [this](bool) { Launcher->UpdatePlayButton(); };
	ShowAgainCheckbox->FuncChanged = [this](bool) { Launcher->UpdatePlayButton(); };
	BackendDropdown->OnChanged = [this](int) { Launcher->UpdatePlayButton(); };

	ApplyValues(info);
	UpdateLanguage();
}

void QuickSetupPage::UpdateLanguage()
{
	TitleLabel->SetText(GStrings.GetString("PICKER_QUICKSETUP_TITLE"));
	IntroLabel->SetText(GStrings.GetString("PICKER_QUICKSETUP_INTRO"));
	FullscreenCheckbox->SetText(GStrings.GetString("PICKER_QUICKSETUP_FULLSCREEN"));
	AskIWADCheckbox->SetText(GStrings.GetString("PICKER_QUICKSETUP_ASKIWAD"));
	AutoloadCheckbox->SetText(GStrings.GetString("PICKER_QUICKSETUP_AUTOLOAD"));
	BackendLabel->SetText(GStrings.GetString("PICKER_QUICKSETUP_BACKEND"));
	BackendDropdown->UpdateItem(GStrings.GetString("OPTVAL_OPENGL"), 0);
	BackendDropdown->UpdateItem(GStrings.GetString("OPTVAL_VULKAN"), 1);
	BackendDropdown->UpdateItem(GStrings.GetString("OPTVAL_OPENGLES"), 2);
	ShowAgainCheckbox->SetText(GStrings.GetString("PICKER_QUICKSETUP_SHOWAGAIN"));
}

void QuickSetupPage::ApplyValues(const FStartupSelectionInfo& info)
{
	FullscreenCheckbox->SetChecked(info.DefaultFullscreen);
	AskIWADCheckbox->SetChecked(info.DefaultQueryIWAD);
	AutoloadCheckbox->SetChecked((info.DefaultStartFlags & 1) == 0);
	BackendDropdown->SetSelectedItem(clamp<int>(info.DefaultBackend, 0, 2));
	ShowAgainCheckbox->SetChecked(info.ShowQuickSetup);
}

void QuickSetupPage::SetValues(FStartupSelectionInfo& info) const
{
	info.DefaultFullscreen = FullscreenCheckbox->GetChecked();
	info.DefaultQueryIWAD = AskIWADCheckbox->GetChecked();
	if (AutoloadCheckbox->GetChecked())
		info.DefaultStartFlags &= ~1;
	else
		info.DefaultStartFlags |= 1;
	info.DefaultBackend = clamp<int>(BackendDropdown->GetSelectedItem(), 0, 2);
	info.ShowQuickSetup = ShowAgainCheckbox->GetChecked();
}

void QuickSetupPage::OnGeometryChanged()
{
	double y = 0.0;
	const double w = GetWidth();

	TitleLabel->SetFrameGeometry(0.0, y, w, TitleLabel->GetPreferredHeight());
	y += TitleLabel->GetPreferredHeight() + 4.0;
	IntroLabel->SetFrameGeometry(0.0, y, w, IntroLabel->GetPreferredHeight());
	y += IntroLabel->GetPreferredHeight() + 10.0;

	FullscreenCheckbox->SetFrameGeometry(0.0, y, w, FullscreenCheckbox->GetPreferredHeight());
	y += FullscreenCheckbox->GetPreferredHeight();
	AskIWADCheckbox->SetFrameGeometry(0.0, y, w, AskIWADCheckbox->GetPreferredHeight());
	y += AskIWADCheckbox->GetPreferredHeight();
	AutoloadCheckbox->SetFrameGeometry(0.0, y, w, AutoloadCheckbox->GetPreferredHeight());
	y += AutoloadCheckbox->GetPreferredHeight() + 6.0;

	BackendLabel->SetFrameGeometry(0.0, y + (BackendDropdown->GetPreferredHeight() - BackendLabel->GetPreferredHeight()) * 0.5, 150.0, BackendLabel->GetPreferredHeight());
	BackendDropdown->SetFrameGeometry(154.0, y, 180.0, BackendDropdown->GetPreferredHeight());
	y += BackendDropdown->GetPreferredHeight() + 6.0;

	ShowAgainCheckbox->SetFrameGeometry(0.0, y, w, ShowAgainCheckbox->GetPreferredHeight());
}
