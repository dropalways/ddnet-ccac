// ChillerDragon 2021 - chillerbot ux

#include <engine/config.h>
#include <engine/shared/linereader.h>
#include <engine/textrender.h>
#include <game/client/gameclient.h>

#include <base/system.h>

#include "warlist.h"

void CWarList::AddSimpleWar(const char *pName)
{
	if(!pName || pName[0] == '\0')
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: missing argument <name>");
		return;
	}
	if(!Storage()->CreateFolder("ccac/bot", IStorage::TYPE_SAVE))
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: failed to create bot folder");
		return;
	}
	AddWar("bot", pName);
}

void CWarList::RemoveSimpleWar(const char *pName)
{
	char aBuf[512];
	if(!RemoveWarNameFromVector("ccac/bot", pName))
	{
		str_format(aBuf, sizeof(aBuf), "Name '%s' not found in the bot list", pName);
		m_pClient->m_Chat.AddLine(-2, 0, aBuf);
		return;
	}
	if(!WriteWarNames("ccac/bot"))
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: failed to write bot names");
	}
	str_format(aBuf, sizeof(aBuf), "Removed '%s' from the bot list", pName);
	m_pClient->m_Chat.AddLine(-2, 0, aBuf);
	ReloadList();
}

void CWarList::AddSimpleTeam(const char *pName)
{
	if(!pName || pName[0] == '\0')
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: missing argument <name>");
		return;
	}
	if(!Storage()->CreateFolder("ccac/clean", IStorage::TYPE_SAVE))
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: failed to create clean folder");
		return;
	}

	AddTeam("clean", pName);
}

void CWarList::RemoveSimpleTeam(const char *pName)
{
	char aBuf[512];
	if(!RemoveTeamNameFromVector("ccac/clean", pName))
	{
		str_format(aBuf, sizeof(aBuf), "Name '%s' not found in the clean list", pName);
		m_pClient->m_Chat.AddLine(-2, 0, aBuf);
		return;
	}
	if(!WriteTeamNames("ccac/clean"))
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: failed to write clean names");
	}
	str_format(aBuf, sizeof(aBuf), "Removed '%s' from the clean list", pName);
	m_pClient->m_Chat.AddLine(-2, 0, aBuf);
	ReloadList();
}
void CWarList::AddSimpleSus(const char *pName)
{
	if(!pName || pName[0] == '\0')
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: missing argument <name>");
		return;
	}
	if(!Storage()->CreateFolder("ccac/sus", IStorage::TYPE_SAVE))
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: failed to create sus folder");
		return;
	}

	AddSus("sus", pName);
}

void CWarList::RemoveSimpleSus(const char *pName)
{
	char aBuf[512];
	if(!RemoveSusNameFromVector("ccac/sus", pName))
	{
		str_format(aBuf, sizeof(aBuf), "Name '%s' not found in the sus list", pName);
		m_pClient->m_Chat.AddLine(-2, 0, aBuf);
		return;
	}
	if(!WriteSusNames("ccac/sus"))
	{
		m_pClient->m_Chat.AddLine(-2, 0, "Error: failed to write sus names");
	}
	str_format(aBuf, sizeof(aBuf), "Removed '%s' from the sus list", pName);
	m_pClient->m_Chat.AddLine(-2, 0, aBuf);
	ReloadList();
}
bool CWarList::OnChatCmdSimple(char Prefix, int ClientId, int Team, const char *pCmd, int NumArgs, const char **ppArgs, const char *pRawArgLine)
{
	if(!str_comp(pCmd, "help"))
	{
		m_pClient->m_Chat.AddLine(-2, 0, "=== command help ===");
		m_pClient->m_Chat.AddLine(-2, 0, "!b <name>  - mark someone as a botter");
		m_pClient->m_Chat.AddLine(-2, 0, "!nb <name>");
		m_pClient->m_Chat.AddLine(-2, 0, "!c <name>  - mark someone as clean");
		m_pClient->m_Chat.AddLine(-2, 0, "!nc <name>");
		m_pClient->m_Chat.AddLine(-2, 0, "!s <name>  - mark someone as sus");
		m_pClient->m_Chat.AddLine(-2, 0, "!ns <name>");
		// m_pClient->m_Chat.AddLine(-2, 0, "!search <name>");
	}
	else if(!str_comp(pCmd, "b") || !str_comp(pCmd, "addwar")) // "war <name>"
	{
		AddSimpleWar(pRawArgLine);
		return true;
	}
	else if(!str_comp(pCmd, "c") || !str_comp(pCmd, "addteam")) // "team <name>"
	{
		AddSimpleTeam(pRawArgLine);
		return true;
	}
	else if(!str_comp(pCmd, "nb") || !str_comp(pCmd, "unwar") || !str_comp(pCmd, "peace")) // "delwar <name>"
	{
		RemoveSimpleWar(pRawArgLine);
		return true;
	}
	else if(!str_comp(pCmd, "nc") || !str_comp(pCmd, "unteam") || !str_comp(pCmd, "unfriend")) // "delteam <name>"
	{
		RemoveSimpleTeam(pRawArgLine);
		return true;
	}
	else if(!str_comp(pCmd, "s") || !str_comp(pCmd, "sus") || !str_comp(pCmd, "unsure")) // "delwar <name>"
	{
		AddSimpleSus(pRawArgLine);
		return true;
	}
	else if(!str_comp(pCmd, "ns") || !str_comp(pCmd, "unsus") || !str_comp(pCmd, "sure")) // "delteam <name>"
	{
		RemoveSimpleSus(pRawArgLine);
		return true;
	}
	else
	{
		return false;
	}
	return true;
}
