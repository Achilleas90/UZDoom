/*
** quicksetuppage.h
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

#pragma once

#include <zwidget/core/widget.h>

class LauncherWindow;
class TextLabel;
class CheckboxLabel;
class Dropdown;
struct FStartupSelectionInfo;

class QuickSetupPage : public Widget
{
public:
	QuickSetupPage(LauncherWindow* launcher, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void ApplyValues(const FStartupSelectionInfo& info);
	void SetValues(FStartupSelectionInfo& info) const;

private:
	void OnGeometryChanged() override;

	LauncherWindow* Launcher = nullptr;
	TextLabel* TitleLabel = nullptr;
	TextLabel* IntroLabel = nullptr;
	CheckboxLabel* FullscreenCheckbox = nullptr;
	CheckboxLabel* AskIWADCheckbox = nullptr;
	CheckboxLabel* AutoloadCheckbox = nullptr;
	TextLabel* BackendLabel = nullptr;
	Dropdown* BackendDropdown = nullptr;
	CheckboxLabel* ShowAgainCheckbox = nullptr;
};

