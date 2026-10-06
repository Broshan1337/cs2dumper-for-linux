// Generated using https://github.com/a2x/cs2-dumper
// 2026-10-06 19:38:53.279116825 UTC

#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2_dumper {
    namespace offsets {
        // Module: libclient.so
        namespace libclient_so {
            constexpr std::ptrdiff_t dwCSGOInput = 0x4951AC0;
            constexpr std::ptrdiff_t dwEntityList = 0x46BEB00;
            constexpr std::ptrdiff_t dwGameEntitySystem = 0x4B91050;
            constexpr std::ptrdiff_t dwGameEntitySystem_highestEntityIndex = 0x2120;
            constexpr std::ptrdiff_t dwGameRules = 0x493ABF0;
            constexpr std::ptrdiff_t dwGlobalVars = 0x4683858;
            constexpr std::ptrdiff_t dwGlowManager = 0x4935258;
            constexpr std::ptrdiff_t dwLocalPlayerController = 0x4903F18;
            constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x493BFD8;
            constexpr std::ptrdiff_t dwPlantedC4 = 0x47C22C8;
            constexpr std::ptrdiff_t dwPrediction = 0x493BE90;
            constexpr std::ptrdiff_t dwSensitivity = 0x493A0F8;
            constexpr std::ptrdiff_t dwSensitivity_sensitivity = 0x58;
            constexpr std::ptrdiff_t dwViewMatrix = 0x4943600;
            constexpr std::ptrdiff_t dwViewRender = 0x4943710;
        }
        // Module: libengine2.so
        namespace libengine2_so {
            constexpr std::ptrdiff_t dwBuildNumber = 0x9F60FC;
            constexpr std::ptrdiff_t dwNetworkGameClient = 0xA477C0;
            constexpr std::ptrdiff_t dwNetworkGameClient_clientTickCount = 0x3A8;
            constexpr std::ptrdiff_t dwNetworkGameClient_deltaTick = 0x3AC;
            constexpr std::ptrdiff_t dwNetworkGameClient_isBackgroundMap = 0x288;
            constexpr std::ptrdiff_t dwNetworkGameClient_localPlayer = 0x280;
            constexpr std::ptrdiff_t dwNetworkGameClient_maxClients = 0x240;
            constexpr std::ptrdiff_t dwNetworkGameClient_serverTickCount = 0x25C;
            constexpr std::ptrdiff_t dwNetworkGameClient_signOnState = 0x284;
            constexpr std::ptrdiff_t dwWindowHeight = 0xA01164;
            constexpr std::ptrdiff_t dwWindowWidth = 0xA01160;
        }
        // Module: libinputsystem.so
        namespace libinputsystem_so {
            constexpr std::ptrdiff_t dwInputSystem = 0x7FCA0;
        }
        // Module: libmatchmaking.so
        namespace libmatchmaking_so {
            constexpr std::ptrdiff_t dwGameTypes = 0x39D2E0;
            constexpr std::ptrdiff_t dwGameTypes_mapName = 0x39D400;
        }
        // Module: libpanorama.so
        namespace libpanorama_so {
            constexpr std::ptrdiff_t HUD_CONTEXT = 0x6C9100;
            constexpr std::ptrdiff_t MENU_CONTEXT = 0x6C90E0;
        }
    }
}
