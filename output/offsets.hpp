// Generated using https://github.com/a2x/cs2-dumper
// 2026-08-25 09:11:58.966983803 UTC

#pragma once

#include <cstddef>
#include <cstdint>

namespace cs2_dumper {
    namespace offsets {
        // Module: libclient.so
        namespace libclient_so {
            constexpr std::ptrdiff_t dwCSGOInput = 0x45996D8;
            constexpr std::ptrdiff_t dwEntityList = 0x45D1580;
            constexpr std::ptrdiff_t dwGameEntitySystem = 0x4A6AD10;
            constexpr std::ptrdiff_t dwGameEntitySystem_highestEntityIndex = 0x20A0;
            constexpr std::ptrdiff_t dwGameRules = 0x48944D4;
            constexpr std::ptrdiff_t dwGlobalVars = 0x458E278;
            constexpr std::ptrdiff_t dwGlowManager = 0x4821338;
            constexpr std::ptrdiff_t dwLocalPlayerController = 0x47F41D8;
            constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x48271E8;
            constexpr std::ptrdiff_t dwPlantedC4 = 0x47E29E0;
            constexpr std::ptrdiff_t dwPrediction = 0x48270A0;
            constexpr std::ptrdiff_t dwSensitivity = 0x48254D8;
            constexpr std::ptrdiff_t dwSensitivity_sensitivity = 0x58;
            constexpr std::ptrdiff_t dwViewAngles = 0x4599C20;
            constexpr std::ptrdiff_t dwViewMatrix = 0x482E280;
            constexpr std::ptrdiff_t dwViewRender = 0x482E390;
        }
        // Module: libengine2.so
        namespace libengine2_so {
            constexpr std::ptrdiff_t dwBuildNumber = 0x9D9874;
            constexpr std::ptrdiff_t dwNetworkGameClient = 0xA2AC80;
            constexpr std::ptrdiff_t dwNetworkGameClient_clientTickCount = 0x388;
            constexpr std::ptrdiff_t dwNetworkGameClient_deltaTick = 0x38C;
            constexpr std::ptrdiff_t dwNetworkGameClient_isBackgroundMap = 0x288;
            constexpr std::ptrdiff_t dwNetworkGameClient_localPlayer = 0x280;
            constexpr std::ptrdiff_t dwNetworkGameClient_maxClients = 0x240;
            constexpr std::ptrdiff_t dwNetworkGameClient_serverTickCount = 0x25C;
            constexpr std::ptrdiff_t dwNetworkGameClient_signOnState = 0x284;
            constexpr std::ptrdiff_t dwWindowHeight = 0x9E45E4;
            constexpr std::ptrdiff_t dwWindowWidth = 0x9E45E0;
        }
        // Module: libinputsystem.so
        namespace libinputsystem_so {
            constexpr std::ptrdiff_t dwInputSystem = 0x7FBC0;
        }
        // Module: libmatchmaking.so
        namespace libmatchmaking_so {
            constexpr std::ptrdiff_t dwGameTypes = 0x39C480;
            constexpr std::ptrdiff_t dwGameTypes_mapName = 0x39C5A0;
        }
        // Module: libpanorama.so
        namespace libpanorama_so {
            constexpr std::ptrdiff_t HUD_CONTEXT = 0x67CA40;
            constexpr std::ptrdiff_t MENU_CONTEXT = 0x67CA20;
        }
    }
}
