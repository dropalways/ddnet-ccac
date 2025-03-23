#ifndef GAME_CLIENT_COMPONENTS_CHILLERBOT_CHILLERBOTUX_H
#define GAME_CLIENT_COMPONENTS_CHILLERBOT_CHILLERBOTUX_H

#include <engine/client.h>
#include <game/client/component.h>
#include <game/client/render.h>
#include <game/mapitems.h>

#include <engine/console.h>

#include <engine/shared/http.h>

#define MAX_COMPONENT_LEN 16
#define MAX_COMPONENTS_ENABLED 8

class CChillerBotUX : public CComponent
{
	class CChatHelper *m_pChatHelper;

	enum
	{
		STATE_DONE,
		STATE_WANTREFRESH,
		STATE_REFRESHING,
	};
	std::shared_ptr<CHttpRequest> m_pAliveGet = nullptr;
	int64_t m_NextHeartbeat;
	int64_t m_AfkTill;

	bool m_IsNearFinish;

	char m_aAfkMessage[32];
	char m_aLastAfkPing[2048];
	char m_aLastKiller[2][32];
	char m_aLastKillerTime[2][32];

	struct CUiComponent
	{
		char m_aName[MAX_COMPONENT_LEN];
		char m_aNoteShort[16];
		char m_aNoteLong[2048];
	};
	CUiComponent m_aEnabledComponents[MAX_COMPONENTS_ENABLED];

	struct CKillMsg
	{
		int m_Weapon;

		int m_VictimId;
		int m_VictimTeam;
		int m_VictimDDTeam;
		char m_aVictimName[64];
		CTeeRenderInfo m_VictimRenderInfo;

		int m_KillerId;
		int m_KillerTeam;
		char m_aKillerName[64];
		CTeeRenderInfo m_KillerRenderInfo;

		int m_ModeSpecial; // for CTF, if the guy is carrying a flag for example
		int m_Tick;
		int m_FlagCarrierBlue;
	};

	bool m_MouseModeAbs;
	int m_AfkActivity;
	int m_CampClick;
	int m_ForceDir;
	int m_LastForceDir;
	int64_t m_LastNotification;
	int64_t m_NextSkinSteal;
	// used for notifications when tabbed out
	int m_LastTile = -1;

	void SaveSkins();
	void RestoreSkins();

	// broadcasts
	char m_aBroadcastText[1024];
	int m_BroadcastTick;
	bool m_IsLeftSidedBroadcast;

	bool IsPlayerInfoAvailable(int ClientId) const;

	void OnChatMessage(int ClientId, int Team, const char *pMsg);
	void GoAfk(int Minutes, const char *pMsg = 0);
	void CampHackTick();
	void MapScreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup, float Zoom);
	void RenderEnabledComponents();
	void DumpPlayers(const char *pSearch = 0);

	// helpers
	int CountOnlinePlayers();

	virtual void OnRender() override;
	virtual void OnMessage(int MsgType, void *pRawMsg) override;
	virtual void OnConsoleInit() override;
	virtual void OnInit() override;
	virtual bool OnCursorMove(float x, float y, IInput::ECursorType CursorType) override;

	static void ConPlaytime(IConsole::IResult *pResult, void *pUserData);
	static void ConAfk(IConsole::IResult *pResult, void *pUserData);
	static void ConCampHack(IConsole::IResult *pResult, void *pUserData);
	static void ConCampHackAbs(IConsole::IResult *pResult, void *pUserData);
	static void ConUnCampHack(IConsole::IResult *pResult, void *pUserData);
	static void ConLoadMap(IConsole::IResult *pResult, void *pUserData);
	static void ConDumpPlayers(IConsole::IResult *pResult, void *pUserData);
	static void ConForceQuit(IConsole::IResult *pResult, void *pUserData);

	static void ConchainCampHack(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainChillerbotHud(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainWeaponHud(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainAutoReply(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainFinishRename(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainShowLastKiller(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainShowLastPing(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainSkinStealer(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

public:
	virtual int Sizeof() const override { return sizeof(*this); }
	int m_IgnoreChatAfk;

	// return false to drop the message
	bool OnSendChat(int Team, const char *pLine);

	// return true to send the input
	// is called at the end of ddnet's SnapInput
	// `WouldSend` holds the value if the ddnet client wants to send the input
	// but the return value is what is actually used
	//
	// `pInput` is what the ddnet client wants to send
	// but it can also be altered
	//
	// Only called from the active tee never for the dummy
	// if you want to set the dummy input can manually set it using:
	//
	// ```C++
	// m_pClient->m_Controls.m_aInputData[!g_Config.m_ClDummy]
	// ```
	bool OnSnapInput(bool WouldSend, CNetObj_PlayerInput *pInput);

	void ReturnFromAfk(const char *pChatMessage = 0);
	int64_t GetAfkTime() { return m_AfkTill; }
	const char *GetAfkMessage() { return m_aAfkMessage; }
	int GetAfkActivity() { return m_AfkActivity; }
	bool IsAfk() { return GetAfkTime() && GetAfkActivity() < 25; }

	void EnableComponent(const char *pComponent, const char *pNoteShort = 0, const char *pNoteLong = 0);
	void DisableComponent(const char *pComponent);
	bool SetComponentNoteShort(const char *pComponent, const char *pNoteShort = 0);
	bool SetComponentNoteLong(const char *pComponent, const char *pNoteLong = 0);
	void UpdateComponents();

	int GetTotalJumps();
	int GetUnusedJumps();
	int GetPlayTimeHours() const;
};

#endif
