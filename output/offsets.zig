// Generated using https://github.com/a2x/cs2-dumper
// 2026-10-06 19:38:53.279116825 UTC

pub const cs2_dumper = struct {
    pub const offsets = struct {
        // Module: libclient.so
        pub const libclient_so = struct {
            pub const dwCSGOInput: usize = 0x4951AC0;
            pub const dwEntityList: usize = 0x46BEB00;
            pub const dwGameEntitySystem: usize = 0x4B91050;
            pub const dwGameEntitySystem_highestEntityIndex: usize = 0x2120;
            pub const dwGameRules: usize = 0x493ABF0;
            pub const dwGlobalVars: usize = 0x4683858;
            pub const dwGlowManager: usize = 0x4935258;
            pub const dwLocalPlayerController: usize = 0x4903F18;
            pub const dwLocalPlayerPawn: usize = 0x493BFD8;
            pub const dwPlantedC4: usize = 0x47C22C8;
            pub const dwPrediction: usize = 0x493BE90;
            pub const dwSensitivity: usize = 0x493A0F8;
            pub const dwSensitivity_sensitivity: usize = 0x58;
            pub const dwViewMatrix: usize = 0x4943600;
            pub const dwViewRender: usize = 0x4943710;
        };
        // Module: libengine2.so
        pub const libengine2_so = struct {
            pub const dwBuildNumber: usize = 0x9F60FC;
            pub const dwNetworkGameClient: usize = 0xA477C0;
            pub const dwNetworkGameClient_clientTickCount: usize = 0x3A8;
            pub const dwNetworkGameClient_deltaTick: usize = 0x3AC;
            pub const dwNetworkGameClient_isBackgroundMap: usize = 0x288;
            pub const dwNetworkGameClient_localPlayer: usize = 0x280;
            pub const dwNetworkGameClient_maxClients: usize = 0x240;
            pub const dwNetworkGameClient_serverTickCount: usize = 0x25C;
            pub const dwNetworkGameClient_signOnState: usize = 0x284;
            pub const dwWindowHeight: usize = 0xA01164;
            pub const dwWindowWidth: usize = 0xA01160;
        };
        // Module: libinputsystem.so
        pub const libinputsystem_so = struct {
            pub const dwInputSystem: usize = 0x7FCA0;
        };
        // Module: libmatchmaking.so
        pub const libmatchmaking_so = struct {
            pub const dwGameTypes: usize = 0x39D2E0;
            pub const dwGameTypes_mapName: usize = 0x39D400;
        };
        // Module: libpanorama.so
        pub const libpanorama_so = struct {
            pub const HUD_CONTEXT: usize = 0x6C9100;
            pub const MENU_CONTEXT: usize = 0x6C90E0;
        };
    };
};
