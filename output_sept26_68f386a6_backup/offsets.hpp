// Generated using https://github.com/a2x/cs2-dumper
// 2026-09-26 09:11:30.623489226 UTC

#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2_dumper {
    namespace offsets {
        // Module: libclient.so
        namespace libclient_so {
            constexpr std::ptrdiff_t dwEntityList = 0x46BCC00;
            constexpr std::ptrdiff_t dwGameEntitySystem = 0x4B8E950;
            constexpr std::ptrdiff_t dwGameEntitySystem_highestEntityIndex = 0x2120;
            constexpr std::ptrdiff_t dwGlobalVars = 0x4681778;
            constexpr std::ptrdiff_t dwGlowManager = 0x49333D8;
            constexpr std::ptrdiff_t dwLocalPlayerController = 0x4902098;
            constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x493A158;
            constexpr std::ptrdiff_t dwPlantedC4 = 0x47C0428;
            constexpr std::ptrdiff_t dwPrediction = 0x493A010;
            constexpr std::ptrdiff_t dwSensitivity = 0x4938278;
            constexpr std::ptrdiff_t dwSensitivity_sensitivity = 0x58;
            constexpr std::ptrdiff_t dwViewMatrix = 0x4941780;
            constexpr std::ptrdiff_t dwViewRender = 0x4941890;
        }
        // Module: libengine2.so
        namespace libengine2_so {
            constexpr std::ptrdiff_t dwNetworkGameClient = 0xA46F60;
            constexpr std::ptrdiff_t dwNetworkGameClient_clientTickCount = 0x3A8;
            constexpr std::ptrdiff_t dwNetworkGameClient_deltaTick = 0x3AC;
            constexpr std::ptrdiff_t dwNetworkGameClient_isBackgroundMap = 0x288;
            constexpr std::ptrdiff_t dwNetworkGameClient_localPlayer = 0x280;
            constexpr std::ptrdiff_t dwNetworkGameClient_maxClients = 0x240;
            constexpr std::ptrdiff_t dwNetworkGameClient_serverTickCount = 0x25C;
            constexpr std::ptrdiff_t dwNetworkGameClient_signOnState = 0x284;
            constexpr std::ptrdiff_t dwWindowHeight = 0xA00904;
            constexpr std::ptrdiff_t dwWindowWidth = 0xA00900;
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
            constexpr std::ptrdiff_t HUD_CONTEXT = 0x6C6000;
            constexpr std::ptrdiff_t MENU_CONTEXT = 0x6C5FE0;
        }
    }
}
