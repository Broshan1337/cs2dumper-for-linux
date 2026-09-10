// Generated using https://github.com/a2x/cs2-dumper
// 2026-09-10 13:49:10.424194871 UTC

pub const cs2_dumper = struct {
    pub const offsets = struct {
        // Module: libclient.so
        pub const libclient_so = struct {
            pub const dwCSGOInput: usize = 0x45A0858;
            pub const dwEntityList: usize = 0x45D8700;
            pub const dwGameEntitySystem: usize = 0x4A72350;
            pub const dwGameEntitySystem_highestEntityIndex: usize = 0x20A0;
            pub const dwGameRules: usize = 0x489BB14;
            pub const dwGlobalVars: usize = 0x45953F8;
            pub const dwGlowManager: usize = 0x4828938;
            pub const dwLocalPlayerController: usize = 0x47FB7B8;
            pub const dwLocalPlayerPawn: usize = 0x482E7E8;
            pub const dwPlantedC4: usize = 0x47EA000;
            pub const dwPrediction: usize = 0x482E6A0;
            pub const dwSensitivity: usize = 0x482CAD8;
            pub const dwSensitivity_sensitivity: usize = 0x58;
            pub const dwViewAngles: usize = 0x45A0DA0;
            pub const dwViewMatrix: usize = 0x4835880;
            pub const dwViewRender: usize = 0x4835990;
        };
        // Module: libengine2.so
        pub const libengine2_so = struct {
            pub const dwBuildNumber: usize = 0x9DCF54;
            pub const dwNetworkGameClient: usize = 0xA2E380;
            pub const dwNetworkGameClient_clientTickCount: usize = 0x388;
            pub const dwNetworkGameClient_deltaTick: usize = 0x38C;
            pub const dwNetworkGameClient_isBackgroundMap: usize = 0x288;
            pub const dwNetworkGameClient_localPlayer: usize = 0x280;
            pub const dwNetworkGameClient_maxClients: usize = 0x240;
            pub const dwNetworkGameClient_serverTickCount: usize = 0x25C;
            pub const dwNetworkGameClient_signOnState: usize = 0x284;
            pub const dwWindowHeight: usize = 0x9E7CC4;
            pub const dwWindowWidth: usize = 0x9E7CC0;
        };
        // Module: libinputsystem.so
        pub const libinputsystem_so = struct {
            pub const dwInputSystem: usize = 0x7FBC0;
        };
        // Module: libmatchmaking.so
        pub const libmatchmaking_so = struct {
            pub const dwGameTypes: usize = 0x39C480;
            pub const dwGameTypes_mapName: usize = 0x39C5A0;
        };
        // Module: libpanorama.so
        pub const libpanorama_so = struct {
            pub const HUD_CONTEXT: usize = 0x67CCE0;
            pub const MENU_CONTEXT: usize = 0x67CCC0;
        };
    };
};
