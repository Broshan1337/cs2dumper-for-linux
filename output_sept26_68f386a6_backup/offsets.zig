// Generated using https://github.com/a2x/cs2-dumper
// 2026-09-26 09:11:30.623489226 UTC

pub const cs2_dumper = struct {
    pub const offsets = struct {
        // Module: libclient.so
        pub const libclient_so = struct {
            pub const dwEntityList: usize = 0x46BCC00;
            pub const dwGameEntitySystem: usize = 0x4B8E950;
            pub const dwGameEntitySystem_highestEntityIndex: usize = 0x2120;
            pub const dwGlobalVars: usize = 0x4681778;
            pub const dwGlowManager: usize = 0x49333D8;
            pub const dwLocalPlayerController: usize = 0x4902098;
            pub const dwLocalPlayerPawn: usize = 0x493A158;
            pub const dwPlantedC4: usize = 0x47C0428;
            pub const dwPrediction: usize = 0x493A010;
            pub const dwSensitivity: usize = 0x4938278;
            pub const dwSensitivity_sensitivity: usize = 0x58;
            pub const dwViewMatrix: usize = 0x4941780;
            pub const dwViewRender: usize = 0x4941890;
        };
        // Module: libengine2.so
        pub const libengine2_so = struct {
            pub const dwNetworkGameClient: usize = 0xA46F60;
            pub const dwNetworkGameClient_clientTickCount: usize = 0x3A8;
            pub const dwNetworkGameClient_deltaTick: usize = 0x3AC;
            pub const dwNetworkGameClient_isBackgroundMap: usize = 0x288;
            pub const dwNetworkGameClient_localPlayer: usize = 0x280;
            pub const dwNetworkGameClient_maxClients: usize = 0x240;
            pub const dwNetworkGameClient_serverTickCount: usize = 0x25C;
            pub const dwNetworkGameClient_signOnState: usize = 0x284;
            pub const dwWindowHeight: usize = 0xA00904;
            pub const dwWindowWidth: usize = 0xA00900;
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
            pub const HUD_CONTEXT: usize = 0x6C6000;
            pub const MENU_CONTEXT: usize = 0x6C5FE0;
        };
    };
};
