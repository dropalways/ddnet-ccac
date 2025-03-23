#include <base/log.h>
#include <base/math.h>
#include <base/system.h>

#include <engine/graphics.h>
#include <engine/shared/config.h>
#include <engine/shared/linereader.h>
#include <engine/shared/localization.h>
#include <engine/shared/protocol7.h>
#include <engine/storage.h>
#include <engine/textrender.h>
#include <engine/updater.h>

#include <game/generated/protocol.h>

#include <game/client/animstate.h>
#include <game/client/components/chat.h>
#include <game/client/components/menu_background.h>
#include <game/client/components/sounds.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/skin.h>
#include <game/client/ui.h>
#include <game/client/ui_listbox.h>
#include <game/client/ui_scrollregion.h>
#include <game/localization.h>

#include "menus.h"

using namespace FontIcons;
using namespace std::chrono_literals;

void CMenus::RenderSettingsCCAC(CUIRect MainView)
{

	CUIRect Button;
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_CcacEnableAc, Localize("Enable anticheat features"), g_Config.m_CcacEnableAc, &Button))
	{
		g_Config.m_CcacEnableAc ^= 1;
	}

	if(!g_Config.m_CcacEnableAc)
		return;
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_CcacSilentCommands, Localize("Toggle silent chat commands"), g_Config.m_CcacSilentCommands, &Button))
	{
		g_Config.m_CcacSilentCommands ^= 1;
	}
	MainView.HSplitTop(20.0f, &Button, &MainView);
	if(DoButton_CheckBox(&g_Config.m_CcacMarkClean, Localize("Mark clean players"), g_Config.m_CcacMarkClean, &Button))
	{
		g_Config.m_CcacMarkClean ^= 1;
	}
	ColorRGBA BotDefault(0.72f, 0.23f, 0.23f, 1.0f);
	ColorRGBA CleanDefault(0.68f, 1.0f, 0.68f, 1.0f);
	ColorRGBA SusDefault(0.81f, 0.68f, 0.68f, 1.0f);
	static CButtonContainer s_BottingPlayerColor, s_CleanPlayerColor, s_SusPlayerColor;
    const float ColorPickerLineSize = 25.0f;
    const float ColorPickerLabelSize = 13.0f;
    const float ColorPickerLineSpacing = 5.0f;
	DoLine_ColorPicker(&s_BottingPlayerColor, ColorPickerLineSize, ColorPickerLabelSize, ColorPickerLineSpacing, &MainView, Localize("Botting player color"), &g_Config.m_CcacBottingPlayerColor, BotDefault, false);
    DoLine_ColorPicker(&s_SusPlayerColor, ColorPickerLineSize, ColorPickerLabelSize, ColorPickerLineSpacing, &MainView, Localize("Unsure player color"), &g_Config.m_CcacSusPlayerColor, SusDefault, false);
	DoLine_ColorPicker(&s_CleanPlayerColor, ColorPickerLineSize, ColorPickerLabelSize, ColorPickerLineSpacing, &MainView, Localize("Clean player color"), &g_Config.m_CcacCleanPlayerColor, CleanDefault, false);

}

