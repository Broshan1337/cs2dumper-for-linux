// Generated using https://github.com/a2x/cs2-dumper
// 2026-10-03 10:13:17.305414941 UTC

pub const cs2_dumper = struct {
    pub const offsets = struct {
        // Module: libclient.so
        pub const libclient_so = struct {
            pub const dwEntityList: usize = 0x46BB380;
            pub const dwGameEntitySystem: usize = 0x4B8D8D0;
            pub const dwGameEntitySystem_highestEntityIndex: usize = 0x2120;
            pub const dwGlobalVars: usize = 0x46800F8;
            pub const dwGlowManager: usize = 0x4931AD8;
            pub const dwLocalPlayerController: usize = 0x4900798;
            pub const dwLocalPlayerPawn: usize = 0x4938858;
            pub const dwPlantedC4: usize = 0x47BEB48;
            pub const dwPrediction: usize = 0x4938710;
            pub const dwSensitivity: usize = 0x4936978;
            pub const dwSensitivity_sensitivity: usize = 0x58;
            pub const dwViewMatrix: usize = 0x493FE80;
            pub const dwViewRender: usize = 0x493FF90;
        };
        // Module: libengine2.so
        pub const libengine2_so = struct {
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
            pub const HUD_CONTEXT: usize = 0x6C6180;
            pub const MENU_CONTEXT: usize = 0x6C6160;
        };
    };
};
