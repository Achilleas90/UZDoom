/*
** doommenus.zs
**
**
**
**---------------------------------------------------------------------------
**
** Copyright 1993-1996 id Software
** Copyright 1999-2016 Marisa Heit
** Copyright 2006-2016 Christoph Oelckers
** Copyright 2017-2025 GZDoom Maintainers and Contributors
** Copyright 2025-2026 UZDoom Maintainers and Contributors
**
** SPDX-License-Identifier: GPL-3.0-or-later
**
**---------------------------------------------------------------------------
**
*/

class GameplayMenu : OptionMenu
{
	override void Drawer ()
	{
		Super.Drawer();

		String s = String.Format("dmflags = %d  dmflags2 = %d  dmflags3 = %d", dmflags, dmflags2, dmflags3);
		screen.DrawText (OptionFont(), OptionMenuSettings.mFontColorValue,
			(screen.GetWidth() - OptionWidth (s) * CleanXfac_1) / 2, 35 * CleanXfac_1, s,
			DTA_CleanNoMove_1, true);
	}
}

class CompatibilityMenu : OptionMenu
{
	override void Drawer ()
	{
		Super.Drawer();

		String s = String.Format("compatflags = %d  compatflags2 = %d", compatflags, compatflags2);
		screen.DrawText (OptionFont(), OptionMenuSettings.mFontColorValue,
			(screen.GetWidth() - OptionWidth (s) * CleanXfac_1) / 2, 35 * CleanXfac_1, s,
			DTA_CleanNoMove_1, true);
	}
}

class OptionsHubMenu : OptionMenu
{
	private Array<Name> mCategoryMenus;
	private Array<String> mCategoryLabels;
	private int mCategoryIndex;
	private int mNavTop;
	private int mNavLeft;
	private int mNavRight;
	private int mAdvancedToggleTop;
	private int mAdvancedToggleBottom;
	private int mAdvancedToggleLeft;
	private int mAdvancedToggleRight;
	private int mRightBackLeft;
	private int mRightBackTop;
	private int mRightBackRight;
	private int mRightBackBottom;
	private bool mLeftPaneFocused;
	private CVar mShowAdvanced;
	private Array<Name> mRightMenuStack;
	private Array<int> mRightSelectionStack;

	private void BuildCategories()
	{
		mCategoryMenus.Clear();
		mCategoryLabels.Clear();

		mCategoryMenus.Push('OptionsQuickMenu');
		mCategoryLabels.Push("$OPTMNU_QUICKSETTINGS");

		mCategoryMenus.Push('CustomizeControls');
		mCategoryLabels.Push("$OPTMNU_CONTROLS");
		mCategoryMenus.Push('NewPlayerMenu');
		mCategoryLabels.Push("$OPTMNU_PLAYER");
		mCategoryMenus.Push('GameplayOptions');
		mCategoryLabels.Push("$OPTMNU_GAMEPLAY");
		mCategoryMenus.Push('HUDOptions');
		mCategoryLabels.Push("$OPTMNU_HUD");
		mCategoryMenus.Push('MiscOptions');
		mCategoryLabels.Push("$OPTMNU_MISCELLANEOUS");
		mCategoryMenus.Push('SoundOptions');
		mCategoryLabels.Push("$OPTMNU_SOUND");
		mCategoryMenus.Push('AccessibilityOptions');
		mCategoryLabels.Push("$OPTMNU_ACCESSIBILITY");
		mCategoryMenus.Push('VideoOptions');
		mCategoryLabels.Push("$OPTMNU_DISPLAY");
		mCategoryMenus.Push('HapticsOptions');
		mCategoryLabels.Push("$OPTMNU_HAPTICS");
		mCategoryMenus.Push('OptionsSystemMenu');
		mCategoryLabels.Push("$OPTMNU_SYSTEM");
	}

	private int FirstSelectableForDesc(OptionMenuDescriptor desc)
	{
		for (int i = 0; i < desc.mItems.Size(); i++)
		{
			if (desc.mItems[i].Selectable() && desc.mItems[i].Visible())
				return i;
		}
		return -1;
	}

	private void ActivateCategory(int index)
	{
		if (index < 0 || index >= mCategoryMenus.Size()) return;

		mCategoryIndex = index;
		mRightMenuStack.Clear();
		mRightSelectionStack.Clear();
		OpenRightMenu(mCategoryMenus[index], -1);
	}

	private bool OpenRightMenu(Name menuName, int preferredSelection)
	{
		let desc = OptionMenuDescriptor(MenuDescriptor.GetDescriptor(menuName));
		if (desc == null)
		{
			Menu.SetMenu(menuName, preferredSelection);
			return false;
		}

		mDesc = desc;
		if (preferredSelection >= 0 && preferredSelection < mDesc.mItems.Size())
		{
			mDesc.mSelectedItem = preferredSelection;
		}
		if (mDesc.mSelectedItem < 0 || mDesc.mSelectedItem >= mDesc.mItems.Size() ||
			!(mDesc.mItems[mDesc.mSelectedItem].Selectable() && mDesc.mItems[mDesc.mSelectedItem].Visible()))
		{
			mDesc.mSelectedItem = FirstSelectableForDesc(mDesc);
		}

		mDesc.CalcIndent();
		UpdateTooltip(GetSelectedTooltip());
		return true;
	}

	private bool OpenSelectedSubmenuInline()
	{
		if (mDesc == null || mDesc.mSelectedItem < 0 || mDesc.mSelectedItem >= mDesc.mItems.Size())
		{
			return false;
		}

		if (!(mDesc.mItems[mDesc.mSelectedItem] is "OptionMenuItemSubmenu"))
		{
			return false;
		}

		Name submenuName = mDesc.mItems[mDesc.mSelectedItem].GetAction();
		if (OptionMenuDescriptor(MenuDescriptor.GetDescriptor(submenuName)) == null)
		{
			return false;
		}

		mRightMenuStack.Push(mDesc.mMenuName);
		mRightSelectionStack.Push(mDesc.mSelectedItem);
		if (OpenRightMenu(submenuName, -1))
		{
			MenuSound("menu/advance");
			return true;
		}
		return false;
	}

	private bool GoBackRightMenu()
	{
		if (mRightMenuStack.Size() == 0 || mRightSelectionStack.Size() == 0)
		{
			return false;
		}

		Name previousMenu = mRightMenuStack[mRightMenuStack.Size() - 1];
		int previousSelection = mRightSelectionStack[mRightSelectionStack.Size() - 1];
		mRightMenuStack.Delete(mRightMenuStack.Size() - 1);
		mRightSelectionStack.Delete(mRightSelectionStack.Size() - 1);
		OpenRightMenu(previousMenu, previousSelection);
		MenuSound("menu/backup");
		return true;
	}

	override void Init(Menu parent, OptionMenuDescriptor desc)
	{
		Super.Init(parent, desc);

		BuildCategories();
		mCategoryIndex = 0;
		mLeftPaneFocused = true;
		mShowAdvanced = CVar.FindCVar("m_showadvanced");

		ActivateCategory(mCategoryIndex);
	}

	override int GetIndent()
	{
		int baseIndent = Super.GetIndent();
		// Center the right options pane within the free area to the right
		// of the category panel.
		return baseIndent + GetLeftPaneWidth() / 2;
	}

	private int GetLeftPaneWidth()
	{
		// Slightly narrower left pane to free up room for the options pane.
		return (screen.GetWidth() * 27) / 100;
	}

	private int GetCategoryRowHeight()
	{
		// Keep category rows more separated for readability.
		return OptionMenuSettings.mLinespacing * 2 * CleanYfac_1;
	}

	private String GetCategoryDisplayLabel(String labelKey)
	{
		String label = StringTable.Localize(labelKey);
		String lower = label.MakeLower();
		int len = lower.Length();

		// Display-only cleanup to avoid repetitive suffixes in the category list.
		if (len > 8 && lower.Mid(len - 8) == " options")
		{
			label = label.Left(len - 8);
		}
		else if (len > 6 && lower.Mid(len - 6) == " setup")
		{
			label = label.Left(len - 6);
		}
		else if (len > 10 && lower.Left(10) == "customize ")
		{
			label = label.Mid(10);
		}

		return label;
	}

	private int CountVisibleItems()
	{
		int count = 0;
		if (mDesc == null) return 0;

		for (int i = 0; i < mDesc.mItems.Size(); i++)
		{
			if (mDesc.mItems[i].Visible())
			{
				count++;
			}
		}
		return count;
	}

	private void UpdateRightPaneVerticalLayout()
	{
		if (mDesc == null) return;

		ScreenArea box;
		GetTooltipArea(box);
		if (!DrawTooltips)
		{
			box.y = screen.GetHeight();
		}

		int top = 24 * CleanYfac_1;
		int bottom = box.y - OptionHeight() * CleanYfac_1;
		int available = bottom - top;
		int rowHeight = OptionMenuSettings.mLinespacing * CleanYfac_1;
		int itemCount = CountVisibleItems();
		int contentHeight = itemCount * rowHeight;
		int y = top;

		if (itemCount > 0 && contentHeight < available)
		{
			y = top + (available - contentHeight) / 2;
		}

		// Positive position skips caption drawing and places items at this Y.
		mDesc.mPosition = y;
	}

	private void DrawLeftPane()
	{
		mNavLeft = 0;
		mNavRight = GetLeftPaneWidth();
		int rowHeight = OptionMenuSettings.mLinespacing * CleanYfac_1;
		int categoryRowHeight = GetCategoryRowHeight();

		Screen.Dim(0u, 0.78, mNavLeft, 0, mNavRight, screen.GetHeight());
		Screen.DrawLineFrame(0xAA404040, mNavLeft, 0, mNavRight, screen.GetHeight(), CleanXfac_1);

		int toggleY = screen.GetHeight() - rowHeight * 2;
		mAdvancedToggleTop = toggleY - rowHeight / 2;
		mAdvancedToggleBottom = toggleY + rowHeight;
		mAdvancedToggleLeft = 0;
		mAdvancedToggleRight = mNavRight;

		int listBottom = toggleY - rowHeight;
		int totalListHeight = mCategoryMenus.Size() * categoryRowHeight;
		int availableHeight = listBottom;
		mNavTop = max(0, (availableHeight - totalListHeight) / 2);

		for (int i = 0; i < mCategoryMenus.Size(); i++)
		{
			int rowTop = mNavTop + i * categoryRowHeight;
			int textHeight = BigFont.GetHeight() * CleanYfac_1;
			int y = rowTop + max(0, (categoryRowHeight - textHeight) / 2);
			bool selected = i == mCategoryIndex;
			String itemLabel = GetCategoryDisplayLabel(mCategoryLabels[i]);
			int textWidth = BigFont.StringWidth(itemLabel, false) * CleanXfac_1;
			int x = mNavRight - (24 * CleanXfac_1) - textWidth;

			if (selected)
			{
				screen.DrawText(BigFont, OptionMenuSettings.mFontColorSelection, x, y, itemLabel,
					DTA_CleanNoMove_1, true, DTA_Localize, false);
			}
			else
			{
				screen.DrawText(BigFont, OptionMenuSettings.mFontColor, x, y, itemLabel,
					DTA_CleanNoMove_1, true, DTA_Localize, false);
			}
		}

		Screen.DrawLineFrame(0x66404040, 0, toggleY - rowHeight, mNavRight, rowHeight + 4, CleanXfac_1);
		DrawOptionText(12 * CleanXfac_1, toggleY, OptionMenuSettings.mFontColorHeader, "$OPTMNU_SHOWADVANCED");

		bool showAdvanced = mShowAdvanced && mShowAdvanced.GetBool();
		if (showAdvanced)
		{
			int onWidth = Menu.OptionWidth(StringTable.Localize("$OPTVAL_ON")) * CleanXfac_1;
			int onX = mNavRight - onWidth - 12 * CleanXfac_1;
			DrawOptionText(onX, toggleY, OptionMenuSettings.mFontColorSelection, "$OPTVAL_ON");
		}
		else
		{
			int offWidth = Menu.OptionWidth(StringTable.Localize("$OPTVAL_OFF")) * CleanXfac_1;
			int offX = mNavRight - offWidth - 12 * CleanXfac_1;
			DrawOptionText(offX, toggleY, OptionMenuSettings.mFontColorMore, "$OPTVAL_OFF");
		}
	}

	private void DrawRightBackButton()
	{
		if (mRightMenuStack.Size() == 0)
		{
			mRightBackLeft = mRightBackTop = mRightBackRight = mRightBackBottom = 0;
			return;
		}

		int pad = 12 * CleanXfac_1;
		int x = GetLeftPaneWidth() + 18 * CleanXfac_1;
		int y = 18 * CleanYfac_1;
		String backText = "< Back";
		int textW = Menu.OptionWidth(backText, false) * CleanXfac_1;
		int h = OptionMenuSettings.mLinespacing * CleanYfac_1;

		mRightBackLeft = x - pad;
		mRightBackTop = y - (h / 2);
		mRightBackRight = x + textW + pad;
		mRightBackBottom = y + h;

		Screen.Dim(0u, 0.18, mRightBackLeft, mRightBackTop, mRightBackRight, mRightBackBottom);
		Screen.DrawLineFrame(0x66404040, mRightBackLeft, mRightBackTop, mRightBackRight, mRightBackBottom, CleanXfac_1);
		DrawOptionText(x, y, OptionMenuSettings.mFontColorSelection, backText, false, false);
	}

	override void Drawer()
	{
		UpdateRightPaneVerticalLayout();
		Super.Drawer();
		DrawRightBackButton();
		DrawLeftPane();
	}

	override bool MenuEvent(int mkey, bool fromcontroller)
	{
		if (mLeftPaneFocused)
		{
			switch (mkey)
			{
			case Menu.MKEY_Up:
				mCategoryIndex = (mCategoryIndex + mCategoryMenus.Size() - 1) % mCategoryMenus.Size();
				ActivateCategory(mCategoryIndex);
				MenuSound("menu/cursor");
				return true;
			case Menu.MKEY_Down:
				mCategoryIndex = (mCategoryIndex + 1) % mCategoryMenus.Size();
				ActivateCategory(mCategoryIndex);
				MenuSound("menu/cursor");
				return true;
			case Menu.MKEY_Right:
			case Menu.MKEY_Enter:
				mLeftPaneFocused = false;
				return true;
			default:
				return Super.MenuEvent(mkey, fromcontroller);
			}
		}

		if (mkey == Menu.MKEY_Left)
		{
			mLeftPaneFocused = true;
			return true;
		}
		if (mkey == Menu.MKEY_Back)
		{
			if (GoBackRightMenu())
			{
				return true;
			}
		}
		if (mkey == Menu.MKEY_Enter)
		{
			if (OpenSelectedSubmenuInline())
			{
				return true;
			}
		}

		return Super.MenuEvent(mkey, fromcontroller);
	}

	override bool MouseEvent(int type, int x, int y)
	{
		int paneRight = GetLeftPaneWidth();
		if (x < paneRight)
		{
			if (y >= mAdvancedToggleTop && y <= mAdvancedToggleBottom)
			{
				if (type == Menu.MOUSE_Release && mShowAdvanced)
				{
					mShowAdvanced.SetBool(!mShowAdvanced.GetBool());
					ActivateCategory(mCategoryIndex);
					MenuSound("menu/change");
				}
				return true;
			}

			int categoryRowHeight = GetCategoryRowHeight();
			int index = (y - mNavTop) / categoryRowHeight;
			if (index >= 0 && index < mCategoryMenus.Size())
			{
				if (type == Menu.MOUSE_Click || type == Menu.MOUSE_Release)
				{
					mCategoryIndex = index;
					ActivateCategory(mCategoryIndex);
					mLeftPaneFocused = true;
					MenuSound("menu/cursor");
				}
			}
			// Always consume left-pane mouse events so they don't trigger
			// hover/selection changes in the right options pane.
			return true;
		}

		if (mRightMenuStack.Size() > 0 &&
			x >= mRightBackLeft && x <= mRightBackRight &&
			y >= mRightBackTop && y <= mRightBackBottom)
		{
			if (type == Menu.MOUSE_Release)
			{
				GoBackRightMenu();
			}
			return true;
		}

		mLeftPaneFocused = false;
		return Super.MouseEvent(type, x, y);
	}
}

//=============================================================================
//
// Placeholder classes for overhauled video mode menu. Do not use!
// Their sole purpose is to support mods with full copy of embedded MENUDEF
//
//=============================================================================

class OptionMenuItemScreenResolution : OptionMenuItem
{
	String mResTexts[3];
	int mSelection;
	int mHighlight;
	int mMaxValid;

	enum EValues
	{
		SRL_INDEX = 0x30000,
		SRL_SELECTION = 0x30003,
		SRL_HIGHLIGHT = 0x30004,
	};

	OptionMenuItemScreenResolution Init(String command)
	{
		return self;
	}

	override bool Selectable()
	{
		return false;
	}
}

class VideoModeMenu : OptionMenu
{
	static bool SetSelectedSize()
	{
		return false;
	}
}

class DoomMenuDelegate : MenuDelegateBase
{
	override void PlaySound(Name snd)
	{
		String s = snd;
		S_StartSound (s, CHAN_VOICE, CHANF_UI, snd_menuvolume); 	
	}
} 
