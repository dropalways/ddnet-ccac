// ChillerDragon 2020 - chillerbot ux

#include <engine/client/notifications.h>
#include <engine/config.h>
#include <engine/console.h>
#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/keys.h>
#include <engine/shared/config.h>
#include <engine/shared/json.h>
#include <engine/shared/protocol.h>
#include <engine/textrender.h>
#include <game/client/animstate.h>
#include <game/client/components/camera.h>
#include <game/client/components/chat.h>
#include <game/client/components/ccac/version.h>
#include <game/client/components/controls.h>
#include <game/client/components/menus.h>
#include <game/client/components/voting.h>
#include <game/client/gameclient.h>
#include <game/client/prediction/entities/character.h>
#include <game/client/race.h>
#include <game/client/render.h>
#include <game/generated/client_data.h>
#include <game/generated/protocol.h>
#include <game/version.h>

#include "chillerbotux.h"

void CChillerBotUX::OnRender()
{

		// if tabbing into tw and going afk set to inactive again over time
		if(m_AfkActivity && time_get() % 100 == 0)
			m_AfkActivity--;
	if(m_AfkTill)
	{
		char aBuf[128];
		str_format(aBuf, sizeof(aBuf), "(%d/%d)", m_AfkActivity, 200);
		SetComponentNoteShort("afk", aBuf);
	}
	m_ForceDir = 0;
	m_LastForceDir = m_ForceDir;
}

bool CChillerBotUX::OnSendChat(int Team, const char *pLine)
{
	char aTrimmedLine[512];
	str_copy(aTrimmedLine, pLine);

	int Length = 0;
	char *p = aTrimmedLine;
	char *pEnd = nullptr;
	while(*p)
	{
		char *pStrOld = p;
		int Code = str_utf8_decode((const char **)(&p));

		// check if unicode is not empty
		if(!str_utf8_isspace(Code))
		{
			pEnd = nullptr;
		}
		else if(pEnd == 0)
			pEnd = pStrOld;

		if(++Length >= 256)
		{
			*p = '\0';
			break;
		}
	}
	if(pEnd != nullptr)
		*pEnd = '\0';

	ReturnFromAfk(aTrimmedLine);

	int ClientId = m_pClient->m_aLocalIds[g_Config.m_ClDummy];
	if(m_pClient->m_ChatCommand.OnChatMsg(ClientId, Team, aTrimmedLine))
	{
		if(g_Config.m_CcacSilentCommands)
			return false;
	}

	return true;
}

bool CChillerBotUX::OnSnapInput(bool WouldSend, CNetObj_PlayerInput *pInput)
{
#if defined(CONF_CURSES_CLIENT)
	WouldSend = m_pClient->m_TerminalUI.OnSnapInput(WouldSend, pInput);
#endif

	// register your chillerbot-ux component below!

	return WouldSend;
}


inline bool CChillerBotUX::IsPlayerInfoAvailable(int ClientId) const
{
	const void *pPrevInfo = Client()->SnapFindItem(IClient::SNAP_PREV, NETOBJTYPE_PLAYERINFO, ClientId);
	const void *pInfo = Client()->SnapFindItem(IClient::SNAP_CURRENT, NETOBJTYPE_PLAYERINFO, ClientId);
	return pPrevInfo && pInfo;
}

void CChillerBotUX::RenderEnabledComponents()
{
	if(m_pClient->m_Menus.IsActive())
		return;
	if(m_pClient->m_Voting.IsVoting())
		return;
	int offset = 0;
	if(m_IsLeftSidedBroadcast && Client()->GameTick(g_Config.m_ClDummy) < m_BroadcastTick)
		offset = 2;
	for(auto &EnabledComponent : m_aEnabledComponents)
	{
		if(EnabledComponent.m_aName[0] == '\0')
			continue;
		float TwName = TextRender()->TextWidth(10.0f, EnabledComponent.m_aName, -1, -1);
		float TwNoteShort = 2.f;
		if(EnabledComponent.m_aNoteShort[0])
			TwNoteShort += TextRender()->TextWidth(10.0f, EnabledComponent.m_aNoteShort, -1, -1);
		Graphics()->DrawRect(4.0f, 60.f + offset * 15, TwName + TwNoteShort, 14.0f, ColorRGBA(0, 0, 0, 0.5f), IGraphics::CORNER_ALL, 3.0f);

		TextRender()->Text(5.0f, 60.f + offset * 15, 10.0f, EnabledComponent.m_aName, -1);
		TextRender()->Text(5.0f + TwName + 2.f, 60.f + offset * 15, 10.0f, EnabledComponent.m_aNoteShort, -1);
		TextRender()->Text(5.0f, 60.f + offset * 15 + 10, 4.0f, EnabledComponent.m_aNoteLong, -1);
		offset++;
	}
}

void CChillerBotUX::EnableComponent(const char *pComponent, const char *pNoteShort, const char *pNoteLong)
{
	// update
	for(auto &Component : m_aEnabledComponents)
	{
		if(str_comp(Component.m_aName, pComponent))
			continue;
		str_copy(Component.m_aName, pComponent, sizeof(Component.m_aName));
		if(pNoteShort)
			str_copy(Component.m_aNoteShort, pNoteShort, sizeof(Component.m_aNoteShort));
		if(pNoteLong)
			str_copy(Component.m_aNoteLong, pNoteLong, sizeof(Component.m_aNoteLong));
		return;
	}
	// insert
	for(auto &Component : m_aEnabledComponents)
	{
		if(Component.m_aName[0] != '\0')
			continue;
		str_copy(Component.m_aName, pComponent, sizeof(Component.m_aName));
		Component.m_aNoteShort[0] = '\0';
		Component.m_aNoteLong[0] = '\0';
		if(pNoteShort)
			str_copy(Component.m_aNoteShort, pNoteShort, sizeof(Component.m_aNoteShort));
		if(pNoteLong)
			str_copy(Component.m_aNoteLong, pNoteLong, sizeof(Component.m_aNoteLong));
		return;
	}
}

void CChillerBotUX::DisableComponent(const char *pComponent)
{
	// update
	for(auto &Component : m_aEnabledComponents)
	{
		if(str_comp(Component.m_aName, pComponent))
			continue;
		Component.m_aName[0] = '\0';
		return;
	}
}

bool CChillerBotUX::SetComponentNoteShort(const char *pComponent, const char *pNote)
{
	if(!pNote)
		return false;
	for(auto &Component : m_aEnabledComponents)
	{
		if(str_comp(Component.m_aName, pComponent))
			continue;
		str_copy(Component.m_aNoteShort, pNote, sizeof(Component.m_aNoteShort));
		return true;
	}
	return false;
}

bool CChillerBotUX::SetComponentNoteLong(const char *pComponent, const char *pNote)
{
	if(!pNote)
		return false;
	for(auto &Component : m_aEnabledComponents)
	{
		if(str_comp(Component.m_aName, pComponent))
			continue;
		str_copy(Component.m_aNoteLong, pNote, sizeof(Component.m_aNoteLong));
		return true;
	}
	return false;
}



bool CChillerBotUX::OnCursorMove(float x, float y, IInput::ECursorType CursorType)
{
	if(time_get() % 10 == 0)
		ReturnFromAfk();
	return false;
}


void CChillerBotUX::OnInit()
{

	m_AfkTill = 0;
	m_AfkActivity = 0;
	m_aAfkMessage[0] = '\0';

	for(auto &c : m_aEnabledComponents)
	{
		c.m_aName[0] = '\0';
		c.m_aNoteShort[0] = '\0';
		c.m_aNoteLong[0] = '\0';
	}
	UpdateComponents();
	m_aLastKiller[0][0] = '\0';
	m_aLastKiller[1][0] = '\0';
	m_aLastKillerTime[0][0] = '\0';
	m_aLastKillerTime[1][0] = '\0';
	m_BroadcastTick = 0;
	m_IsLeftSidedBroadcast = false;
	m_LastTile = -1;
}


void CChillerBotUX::UpdateComponents()
{
	// TODO: make this auto or nicer
	if(g_Config.m_CcacEnableAc)
		EnableComponent("war list");
	else
		DisableComponent("war list");
}

void CChillerBotUX::OnConsoleInit()
{
	Console()->Register("afk", "?i[minutes]?r[message]", CFGFLAG_CLIENT, ConAfk, this, "Activate afk mode (auto chat respond)");
	Console()->Register("load_map", "s[file]", CFGFLAG_CLIENT, ConLoadMap, this, "Load mapfile");
	Console()->Register("dump_players", "?s[search]", CFGFLAG_CLIENT, ConDumpPlayers, this, "Prints players to console");
	Console()->Register("force_quit", "", CFGFLAG_CLIENT, ConForceQuit, this, "Forces a dirty client quit all data will be lost");

	Console()->Chain("cl_show_last_ping", ConchainShowLastPing, this);
}

void CChillerBotUX::ConForceQuit(IConsole::IResult *pResult, void *pUserData)
{
	exit(0);
}

void CChillerBotUX::ConDumpPlayers(IConsole::IResult *pResult, void *pUserData)
{
	CChillerBotUX *pSelf = (CChillerBotUX *)pUserData;
	pSelf->DumpPlayers(pResult->GetString(0));
}

void CChillerBotUX::ConchainChillerbotHud(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CChillerBotUX *pSelf = (CChillerBotUX *)pUserData;
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->GetInteger(0))
		pSelf->EnableComponent("chillerbot hud");
	else
		pSelf->DisableComponent("chillerbot hud");
	pSelf->UpdateComponents();
}

void CChillerBotUX::ConchainWeaponHud(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CChillerBotUX *pSelf = (CChillerBotUX *)pUserData;
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->GetInteger(0))
		pSelf->EnableComponent("weapon hud");
	else
		pSelf->DisableComponent("weapon hud");
	pSelf->UpdateComponents();
}

void CChillerBotUX::ConchainAutoReply(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CChillerBotUX *pSelf = (CChillerBotUX *)pUserData;
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->GetInteger(0))
		pSelf->EnableComponent("auto reply");
	else
		pSelf->DisableComponent("auto reply");
}

void CChillerBotUX::ConchainFinishRename(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CChillerBotUX *pSelf = (CChillerBotUX *)pUserData;
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->GetInteger(0))
		pSelf->EnableComponent("finish rename");
	else
		pSelf->DisableComponent("finish rename");
}

void CChillerBotUX::ConchainShowLastPing(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData)
{
	CChillerBotUX *pSelf = (CChillerBotUX *)pUserData;
	pfnCallback(pResult, pCallbackUserData);
	if(pResult->GetInteger(0))
		pSelf->EnableComponent("last ping");
	else
		pSelf->DisableComponent("last ping");
}


void CChillerBotUX::ConAfk(IConsole::IResult *pResult, void *pUserData)
{
	((CChillerBotUX *)pUserData)->GoAfk(pResult->NumArguments() ? pResult->GetInteger(0) : -1, pResult->GetString(1));
}

void CChillerBotUX::DumpPlayers(const char *pSearch)
{
	char aBuf[128];
	char aTime[128];
	char aLine[512];
	int OldDDTeam = -1;
	dbg_msg("dump_players", "+----------+--+----------------+----------------+---+-------+");
	dbg_msg("dump_players", "|score     |id|name            |clan            |lat|team   |");
	dbg_msg("dump_players", "+----------+--+----------------+----------------+---+-------+");
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		const CNetObj_PlayerInfo *pInfo = m_pClient->m_Snap.m_apInfoByDDTeamScore[i];
		if(!pInfo)
			continue;

		bool IsMatch = !(pSearch && pSearch[0] != 0);
		aLine[0] = '\0';
		// score
		if(m_pClient->m_GameInfo.m_TimeScore)
		{
			if(pInfo->m_Score == -9999)
				str_format(aBuf, sizeof(aBuf), "|%10s|", " ");
			else
			{
				str_time((int64_t)abs(pInfo->m_Score) * 100, TIME_HOURS, aTime, sizeof(aTime));
				str_format(aBuf, sizeof(aBuf), "|%10s|", aTime);
			}
		}
		else
			str_format(aBuf, sizeof(aBuf), "|%10d|", clamp(pInfo->m_Score, -999, 99999));
		str_append(aLine, aBuf, sizeof(aLine));

		// id | name
		if(pSearch && pSearch[0] != 0)
			if(str_find_nocase(m_pClient->m_aClients[pInfo->m_ClientId].m_aName, pSearch))
				IsMatch = true;
		str_format(aBuf, sizeof(aBuf), "%2d|%16s|", pInfo->m_ClientId, m_pClient->m_aClients[pInfo->m_ClientId].m_aName);
		str_append(aLine, aBuf, sizeof(aLine));

		// clan
		if(pSearch && pSearch[0] != 0)
			if(str_find_nocase(m_pClient->m_aClients[pInfo->m_ClientId].m_aClan, pSearch))
				IsMatch = true;
		str_format(aBuf, sizeof(aBuf), "%16s|", m_pClient->m_aClients[pInfo->m_ClientId].m_aClan);
		str_append(aLine, aBuf, sizeof(aLine));

		// ping
		str_format(aBuf, sizeof(aBuf), "%3d|", clamp(pInfo->m_Latency, 0, 999));
		str_append(aLine, aBuf, sizeof(aLine));

		// team
		int DDTeam = m_pClient->m_Teams.Team(pInfo->m_ClientId);
		int NextDDTeam = 0;

		for(int j = i + 1; j < MAX_CLIENTS; j++)
		{
			const CNetObj_PlayerInfo *pInfo2 = m_pClient->m_Snap.m_apInfoByDDTeamScore[j];

			if(!pInfo2)
				continue;

			NextDDTeam = m_pClient->m_Teams.Team(pInfo2->m_ClientId);
			break;
		}

		if(OldDDTeam == -1)
		{
			for(int j = i - 1; j >= 0; j--)
			{
				const CNetObj_PlayerInfo *pInfo2 = m_pClient->m_Snap.m_apInfoByDDTeamScore[j];

				if(!pInfo2)
					continue;

				OldDDTeam = m_pClient->m_Teams.Team(pInfo2->m_ClientId);
				break;
			}
		}

		if(DDTeam != TEAM_FLOCK)
		{
			if(NextDDTeam != DDTeam)
			{
				if(m_pClient->m_Snap.m_aTeamSize[0] > 8)
					str_format(aBuf, sizeof(aBuf), "%7d|", DDTeam);
				else
					str_format(aBuf, sizeof(aBuf), "Team %2d|", DDTeam);
			}
			else
				str_format(aBuf, sizeof(aBuf), "%7s|", " ");
			str_append(aLine, aBuf, sizeof(aLine));
			if(OldDDTeam != DDTeam)
				dbg_msg("dump_players", "+----------+--+----------------+----------------+---+-------/");
			if(NextDDTeam != DDTeam)
				dbg_msg("dump_players", "+----------+--+----------------+----------------+---+-------\\");
		}

		OldDDTeam = DDTeam;
		if(IsMatch)
			dbg_msg("dump_players", "%s", aLine);
	}
	dbg_msg("dump_players", "+----------+--+----------------+----------------+---+-------+");
}

void CChillerBotUX::ConLoadMap(IConsole::IResult *pResult, void *pUserData)
{
	CChillerBotUX *pSelf = (CChillerBotUX *)pUserData;
	pSelf->m_pClient->Client()->ChillerBotLoadMap(pResult->GetString(0));
}


void CChillerBotUX::OnMessage(int MsgType, void *pRawMsg)
{
	if(MsgType == NETMSGTYPE_SV_KILLMSG)
	{
		CNetMsg_Sv_KillMsg *pMsg = (CNetMsg_Sv_KillMsg *)pRawMsg;

		// unpack messages
		CKillMsg Kill;
		Kill.m_aVictimName[0] = '\0';
		Kill.m_aKillerName[0] = '\0';

		Kill.m_VictimId = pMsg->m_Victim;
		if(Kill.m_VictimId >= 0 && Kill.m_VictimId < MAX_CLIENTS)
		{
			Kill.m_VictimTeam = m_pClient->m_aClients[Kill.m_VictimId].m_Team;
			Kill.m_VictimDDTeam = m_pClient->m_Teams.Team(Kill.m_VictimId);
			str_copy(Kill.m_aVictimName, m_pClient->m_aClients[Kill.m_VictimId].m_aName, sizeof(Kill.m_aVictimName));
			Kill.m_VictimRenderInfo = m_pClient->m_aClients[Kill.m_VictimId].m_RenderInfo;
		}

		Kill.m_KillerId = pMsg->m_Killer;
		if(Kill.m_KillerId >= 0 && Kill.m_KillerId < MAX_CLIENTS)
		{
			Kill.m_KillerTeam = m_pClient->m_aClients[Kill.m_KillerId].m_Team;
			str_copy(Kill.m_aKillerName, m_pClient->m_aClients[Kill.m_KillerId].m_aName, sizeof(Kill.m_aKillerName));
			Kill.m_KillerRenderInfo = m_pClient->m_aClients[Kill.m_KillerId].m_RenderInfo;
		}

		Kill.m_Weapon = pMsg->m_Weapon;
		Kill.m_ModeSpecial = pMsg->m_ModeSpecial;
		Kill.m_Tick = Client()->GameTick(g_Config.m_ClDummy);

		Kill.m_FlagCarrierBlue = m_pClient->m_Snap.m_pGameDataObj ? m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue : -1;

		bool KillMsgValid = (Kill.m_VictimRenderInfo.m_CustomColoredSkin && Kill.m_VictimRenderInfo.m_ColorableRenderSkin.m_Body.IsValid()) || (!Kill.m_VictimRenderInfo.m_CustomColoredSkin && Kill.m_VictimRenderInfo.m_OriginalRenderSkin.m_Body.IsValid());
		// if killer != victim, killer must be valid too
		KillMsgValid &= Kill.m_KillerId == Kill.m_VictimId || ((Kill.m_KillerRenderInfo.m_CustomColoredSkin && Kill.m_KillerRenderInfo.m_ColorableRenderSkin.m_Body.IsValid()) || (!Kill.m_KillerRenderInfo.m_CustomColoredSkin && Kill.m_KillerRenderInfo.m_OriginalRenderSkin.m_Body.IsValid()));
		if(KillMsgValid && Kill.m_KillerId != Kill.m_VictimId)
		{
			for(int i = 0; i < 2; i++)
			{
				if(m_pClient->m_aLocalIds[i] != Kill.m_VictimId)
					continue;

				str_copy(m_aLastKiller[i], Kill.m_aKillerName, sizeof(m_aLastKiller[i]));
				char aBuf[512];
				str_timestamp_format(m_aLastKillerTime[i], sizeof(m_aLastKillerTime[i]), "%H:%M");
				str_format(
					aBuf,
					sizeof(aBuf),
					"[%s]main: %s [%s]dummy: %s",
					m_aLastKillerTime[0],
					m_aLastKiller[0],
					m_aLastKillerTime[1],
					m_aLastKiller[1]);
				SetComponentNoteLong("last killer", aBuf);
			}
		}
	}
	else if(MsgType == NETMSGTYPE_SV_BROADCAST)
	{
		CNetMsg_Sv_Broadcast *pMsg = (CNetMsg_Sv_Broadcast *)pRawMsg;
		str_copy(m_aBroadcastText, pMsg->m_pMessage, sizeof(m_aBroadcastText));
		m_BroadcastTick = Client()->GameTick(g_Config.m_ClDummy) + Client()->GameTickSpeed() * 10;
		m_IsLeftSidedBroadcast = str_find(m_aBroadcastText, "                                ") != NULL;
	}
/*	else if(MsgType == NETMSGTYPE_SV_MODIFYTILE)
	{
		CNetMsg_Sv_ModifyTile *pMsg = (CNetMsg_Sv_ModifyTile *)pRawMsg;
		Collision()->ModifyTile(pMsg->m_X, pMsg->m_Y, pMsg->m_Group, pMsg->m_Layer, pMsg->m_Index, pMsg->m_Flags);

		// fixes tiles not being updated if the gpu supports tile buffering
		GameClient()->m_MapLayersForeground.RefreshTileBuffers([&]() {});
		GameClient()->m_MapLayersBackground.RefreshTileBuffers([&]() {});

		// fixes skipped tiles that were empty
		Collision()->Layers()->Init(Kernel()->RequestInterface<IMap>(), false);

		// doesnt help
		// Collision()->Init(Layers());
	}*/
}

void CChillerBotUX::GoAfk(int Minutes, const char *pMsg)
{
	if(pMsg)
	{
		str_copy(m_aAfkMessage, pMsg, sizeof(m_aAfkMessage));
		if((unsigned long)str_length(pMsg) > sizeof(m_aAfkMessage))
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "error: afk message too long %d/%lu", str_length(pMsg), sizeof(m_aAfkMessage));
			Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "console", aBuf);
			return;
		}
	}
	m_AfkTill = time_get() + time_freq() * 60 * Minutes;
	m_AfkActivity = 0;
	EnableComponent("afk");
}

void CChillerBotUX::ReturnFromAfk(const char *pChatMessage)
{
	if(!m_AfkTill)
		return;
	if(pChatMessage && pChatMessage[0] != '/')
	{
		if(m_IgnoreChatAfk > 0)
			m_IgnoreChatAfk--;
		else
			m_AfkActivity += 400;
	}
	m_AfkActivity++;
	if(m_AfkActivity < 200)
		return;
	m_pClient->m_Chat.AddLine(-2, 0, "[chillerbot-ux] welcome back :)");
	m_AfkTill = 0;
	DisableComponent("afk");
}

int CChillerBotUX::CountOnlinePlayers()
{
	// Code from scoreboard. There is probably a better way to do this
	int Num = 0;
	for(const auto *pInfo : m_pClient->m_Snap.m_apInfoByDDTeamScore)
		if(pInfo)
			Num++;
	return Num;
}

int CChillerBotUX::GetTotalJumps()
{
	int ClientId = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
	CCharacterCore *pCharacter = &m_pClient->m_aClients[ClientId].m_Predicted;
	if(m_pClient->m_Snap.m_aCharacters[ClientId].m_HasExtendedDisplayInfo)
		return maximum(minimum(abs(pCharacter->m_Jumps), 10), 0);
	else
		return abs(m_pClient->m_Snap.m_aCharacters[ClientId].m_ExtendedData.m_Jumps);
}

int CChillerBotUX::GetUnusedJumps()
{
	int ClientId = GameClient()->m_aLocalIds[g_Config.m_ClDummy];
	CCharacterCore *pCharacter = &m_pClient->m_aClients[ClientId].m_Predicted;
	int TotalJumpsToDisplay = 0, AvailableJumpsToDisplay = 0;
	if(m_pClient->m_Snap.m_aCharacters[ClientId].m_HasExtendedDisplayInfo)
	{
		bool Grounded = false;
		if(Collision()->CheckPoint(pCharacter->m_Pos.x + CCharacterCore::PhysicalSize() / 2,
			   pCharacter->m_Pos.y + CCharacterCore::PhysicalSize() / 2 + 5))
		{
			Grounded = true;
		}
		if(Collision()->CheckPoint(pCharacter->m_Pos.x - CCharacterCore::PhysicalSize() / 2,
			   pCharacter->m_Pos.y + CCharacterCore::PhysicalSize() / 2 + 5))
		{
			Grounded = true;
		}

		int UsedJumps = pCharacter->m_JumpedTotal;
		if(pCharacter->m_Jumps > 1)
		{
			UsedJumps += !Grounded;
		}
		else if(pCharacter->m_Jumps == 1)
		{
			// If the player has only one jump, each jump is the last one
			UsedJumps = pCharacter->m_Jumped & 2;
		}
		else if(pCharacter->m_Jumps == -1)
		{
			// The player has only one ground jump
			UsedJumps = !Grounded;
		}

		if(pCharacter->m_EndlessJump && UsedJumps >= abs(pCharacter->m_Jumps))
		{
			UsedJumps = abs(pCharacter->m_Jumps) - 1;
		}

		int UnusedJumps = abs(pCharacter->m_Jumps) - UsedJumps;
		if(!(pCharacter->m_Jumped & 2) && UnusedJumps <= 0)
		{
			// In some edge cases when the player just got another number of jumps, UnusedJumps is not correct
			UnusedJumps = 1;
		}
		TotalJumpsToDisplay = maximum(minimum(abs(pCharacter->m_Jumps), 10), 0);
		AvailableJumpsToDisplay = maximum(minimum(UnusedJumps, TotalJumpsToDisplay), 0);
	}
	else
	{
		AvailableJumpsToDisplay = abs(m_pClient->m_Snap.m_aCharacters[ClientId].m_ExtendedData.m_Jumps);
	}
	return AvailableJumpsToDisplay;
}
