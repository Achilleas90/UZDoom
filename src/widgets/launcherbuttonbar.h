/*
** launcherbuttonbar.h
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

#pragma once

#include <zwidget/core/widget.h>

class LauncherWindow;
class PushButton;
class LineEdit;

class LauncherButtonbar : public Widget
{
public:
	LauncherButtonbar(LauncherWindow* parent);
	void UpdateLanguage();
	void ShowLaunchWarning(const std::string& warning);

	double GetPreferredHeight() const;

private:
	void OnGeometryChanged() override;
	void OnPlayButtonClicked();
	void OnExitButtonClicked();
	void OnCopyButtonClicked();

	LauncherWindow* GetLauncher() const;

	PushButton* PlayButton = nullptr;
	PushButton* ExitButton = nullptr;
	PushButton* CopyButton = nullptr;
	LineEdit* PreviewEdit = nullptr;
};
