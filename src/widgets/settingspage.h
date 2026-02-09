/*
** settingspage.h
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

#define RENDER_BACKENDS

class LauncherWindow;
class TextLabel;
class CheckboxLabel;
class ListView;
class Dropdown;
class PushButton;
struct FStartupSelectionInfo;

class SettingsPage : public Widget
{
public:
	SettingsPage(LauncherWindow* launcher, const FStartupSelectionInfo& info);
	void UpdateLanguage();
	void ApplyValues(const FStartupSelectionInfo& info);
	void SetValues(FStartupSelectionInfo& info) const;

private:
	void OnLanguageChanged(int i);
	void OnProfileChanged(int i);
	void OnProfileNew();
	void OnProfileDuplicate();
	void OnProfileDelete();
	void UpdateProfileList();
	void OnGeometryChanged() override;

	LauncherWindow* Launcher = nullptr;

	TextLabel* LangLabel = nullptr;
	TextLabel* GeneralLabel = nullptr;
	TextLabel* ExtrasLabel = nullptr;
	TextLabel* LoadLabel = nullptr;
	TextLabel* ProfileLabel = nullptr;
	CheckboxLabel* FullscreenCheckbox = nullptr;
	CheckboxLabel* DisableAutoloadCheckbox = nullptr;
	CheckboxLabel* DontAskAgainCheckbox = nullptr;
	CheckboxLabel* LightsCheckbox = nullptr;
	CheckboxLabel* BrightmapsCheckbox = nullptr;
	CheckboxLabel* WidescreenCheckbox = nullptr;
	CheckboxLabel* SupportWadsCheckbox = nullptr;
#ifdef RENDER_BACKENDS
	TextLabel* BackendLabel = nullptr;
	CheckboxLabel* VulkanCheckbox = nullptr;
	CheckboxLabel* OpenGLCheckbox = nullptr;
	CheckboxLabel* GLESCheckbox = nullptr;
#endif
	ListView* LangList = nullptr;
	Dropdown* LoadList = nullptr;
	Dropdown* ProfileList = nullptr;
	PushButton* ProfileNewButton = nullptr;
	PushButton* ProfileDuplicateButton = nullptr;
	PushButton* ProfileDeleteButton = nullptr;

	TArray<std::pair<FString, FString>> languages;
	bool hideLanguage = false;
	bool updatingProfiles = false;

	int ExtraWadFlags = 0;
};
