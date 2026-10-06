// Generated using https://github.com/a2x/cs2-dumper
// 2026-10-06 19:38:53.279116825 UTC

#![allow(non_upper_case_globals, unused)]

pub mod cs2_dumper {
    pub mod interfaces {
        // Module: libanimationsystem.so
        pub mod libanimationsystem_so {
            pub const AnimationSystemUtils_001: usize = 0x3A7FC0;
            pub const AnimationSystem_001: usize = 0x3A7CF0;
        }
        // Module: libclient.so
        pub mod libclient_so {
            pub const ClientToolsInfo_001: usize = 0x18ED3E0;
            pub const EmptyWorldService001_Client: usize = 0x13D9FB0;
            pub const GameClientExports001: usize = 0x18ECC80;
            pub const LegacyGameUI001: usize = 0x1B89080;
            pub const Source2Client002: usize = 0x18ECD40;
            pub const Source2ClientConfig001: usize = 0x1384D40;
            pub const Source2ClientPrediction001: usize = 0x1970BD0;
            pub const Source2ClientUI001: usize = 0x1AC78F0;
        }
        // Module: libengine2.so
        pub mod libengine2_so {
            pub const BenchmarkService001: usize = 0x3CDC10;
            pub const BugBugService001: usize = 0x3C8500;
            pub const BugService001: usize = 0x3C8490;
            pub const ClientServerEngineLoopService_001: usize = 0x384190;
            pub const ClientServerSharedHandleSystem001: usize = 0x34C250;
            pub const EngineGameUI001: usize = 0x5E6530;
            pub const EngineServiceMgr001: usize = 0x36FE00;
            pub const GameEventSystemClientV001: usize = 0x375FF0;
            pub const GameEventSystemServerV001: usize = 0x376000;
            pub const GameResourceServiceClientV001: usize = 0x3CFDF0;
            pub const GameResourceServiceServerV001: usize = 0x3CFE00;
            pub const GameUIService_001: usize = 0x3DAB30;
            pub const HostStateMgr001: usize = 0x37D230;
            pub const INETSUPPORT_001: usize = 0x59A6A0;
            pub const InputService_001: usize = 0x3E0040;
            pub const KeyValueCache001: usize = 0x380900;
            pub const MapListService_001: usize = 0x3FCF20;
            pub const NetworkClientService_001: usize = 0x4222C0;
            pub const NetworkP2PService_001: usize = 0x439070;
            pub const NetworkServerService_001: usize = 0x402EF0;
            pub const NetworkService_001: usize = 0x402080;
            pub const RenderService_001: usize = 0x43F210;
            pub const ScreenshotService001: usize = 0x442D70;
            pub const SimpleEngineLoopService_001: usize = 0x3A32B0;
            pub const SoundService_001: usize = 0x448E30;
            pub const Source2EngineToClient001: usize = 0x4F6660;
            pub const Source2EngineToClientStringTable001: usize = 0x4B9850;
            pub const Source2EngineToServer001: usize = 0x526EE0;
            pub const Source2EngineToServerStringTable001: usize = 0x502090;
            pub const SplitScreenService_001: usize = 0x453780;
            pub const StatsService_001: usize = 0x457B70;
            pub const ToolService_001: usize = 0x45D6F0;
            pub const VENGINE_GAMEUIFUNCS_VERSION005: usize = 0x5E5CF0;
            pub const VProfService_001: usize = 0x45F190;
        }
        // Module: libfilesystem_stdio.so
        pub mod libfilesystem_stdio_so {
            pub const VAsyncFileSystem2_001: usize = 0x11BFF0;
            pub const VFileSystem017: usize = 0x11BFE0;
        }
        // Module: libhost.so
        pub mod libhost_so {
            pub const DebugDrawQueueManager001: usize = 0x17B690;
            pub const GameModelInfo001: usize = 0x175150;
            pub const GameSystem2HostHook: usize = 0x1756F0;
            pub const HostUtils001: usize = 0x175C00;
            pub const PredictionDiffManager001: usize = 0x177260;
            pub const SaveRestoreDataVersion001: usize = 0x179DA0;
            pub const SinglePlayerSharedMemory001: usize = 0x17A090;
            pub const Source2Host001: usize = 0x17A8D0;
        }
        // Module: libinputsystem.so
        pub mod libinputsystem_so {
            pub const InputStackSystemVersion001: usize = 0x39AB0;
            pub const InputSystemVersion001: usize = 0x3B0E0;
        }
        // Module: liblocalize.so
        pub mod liblocalize_so {
            pub const Localize_001: usize = 0x38430;
        }
        // Module: libmatchmaking.so
        pub mod libmatchmaking_so {
            pub const GameTypes001: usize = 0x1AB4E0;
            pub const MATCHFRAMEWORK_001: usize = 0x2C0D80;
        }
        // Module: libmaterialsystem2.so
        pub mod libmaterialsystem2_so {
            pub const FontManager_001: usize = 0xD0E60;
            pub const MaterialUtils_001: usize = 0xBD7A0;
            pub const PostProcessingSystem_001: usize = 0xE7D90;
            pub const TextLayout_001: usize = 0xE4F40;
            pub const VMaterialSystem2_001: usize = 0x6FE50;
        }
        // Module: libmeshsystem.so
        pub mod libmeshsystem_so {
            pub const MeshSystem001: usize = 0x5A310;
        }
        // Module: libnetworksystem.so
        pub mod libnetworksystem_so {
            pub const FlattenedSerializersVersion001: usize = 0x258F10;
            pub const NetworkMessagesVersion001: usize = 0x2B1A60;
            pub const NetworkSystemVersion001: usize = 0x2D1ED0;
            pub const SerializedEntitiesVersion001: usize = 0x2F2460;
        }
        // Module: libpanorama.so
        pub mod libpanorama_so {
            pub const PanoramaUIEngine001: usize = 0x3799D0;
        }
        // Module: libpanorama_text_pango.so
        pub mod libpanorama_text_pango_so {
            pub const PanoramaTextServices001: usize = 0x176B50;
        }
        // Module: libpanoramauiclient.so
        pub mod libpanoramauiclient_so {
            pub const PanoramaUIClient001: usize = 0x1A5220;
        }
        // Module: libparticles.so
        pub mod libparticles_so {
            pub const ParticleSystemMgr003: usize = 0x2D1300;
        }
        // Module: libpulse_system.so
        pub mod libpulse_system_so {
            pub const IPulseSystem_001: usize = 0xB8AE0;
        }
        // Module: librendersystemvulkan.so
        pub mod librendersystemvulkan_so {
            pub const RenderDeviceMgr001: usize = 0x56FAB0;
            pub const RenderUtils_001: usize = 0x4B6200;
        }
        // Module: libresourcesystem.so
        pub mod libresourcesystem_so {
            pub const ResourceSystem013: usize = 0x50D90;
        }
        // Module: libscenefilecache.so
        pub mod libscenefilecache_so {
            pub const ResponseRulesCache001: usize = 0x152CC0;
            pub const SceneFileCache002: usize = 0x150CD0;
        }
        // Module: libscenesystem.so
        pub mod libscenesystem_so {
            pub const RenderingPipelines_001: usize = 0x255860;
            pub const SceneSystem_002: usize = 0x292580;
            pub const SceneUtils_001: usize = 0x388440;
        }
        // Module: libserver.so
        pub mod libserver_so {
            pub const EmptyWorldService001_Server: usize = 0x13FC970;
            pub const EntitySubclassUtilsV001: usize = 0xEC9B00;
            pub const NavGameTest001: usize = 0x1D5B8E0;
            pub const ServerToolsInfo_001: usize = 0x198DB40;
            pub const Source2GameClients001: usize = 0x198DB30;
            pub const Source2GameDirector001: usize = 0xACD110;
            pub const Source2GameEntities001: usize = 0x198DAC0;
            pub const Source2Server001: usize = 0x198D840;
            pub const Source2ServerConfig001: usize = 0x1340180;
            pub const customnavsystem001: usize = 0xD13780;
        }
        // Module: libsoundsystem.so
        pub mod libsoundsystem_so {
            pub const SoundBugBugService001_Client: usize = 0x3FD110;
            pub const SoundOpSystem001: usize = 0x2F3F30;
            pub const SoundOpSystemEdit001: usize = 0x1B3890;
            pub const SoundSystem001: usize = 0x38F830;
            pub const VMixEditTool001: usize = 0x3CD1B0;
        }
        // Module: libsteamaudio.so
        pub mod libsteamaudio_so {
            pub const SteamAudio001: usize = 0x1CAC80;
        }
        // Module: libtier0.so
        pub mod libtier0_so {
            pub const TestScriptMgr001: usize = 0x24AB50;
            pub const VEngineCvar007: usize = 0x156520;
            pub const VProcessUtils002: usize = 0x2370F0;
            pub const VStringTokenSystem001: usize = 0x279490;
        }
        // Module: libv8system.so
        pub mod libv8system_so {
            pub const Source2V8System001: usize = 0x383D0;
        }
        // Module: libvphysics2.so
        pub mod libvphysics2_so {
            pub const VPhysics2_Interface_001: usize = 0x8B5D0;
        }
        // Module: libvscript.so
        pub mod libvscript_so {
            pub const VScriptManager010: usize = 0x638E0;
        }
        // Module: libworldrenderer.so
        pub mod libworldrenderer_so {
            pub const WorldRendererMgr001: usize = 0x181370;
        }
        // Module: steamclient.so
        pub mod steamclient_so {
            pub const CLIENTENGINE_INTERFACE_VERSION005: usize = 0x1617530;
            pub const IVALIDATE001: usize = 0x1612F00;
            pub const SteamClient006: usize = 0x12A06D0;
            pub const SteamClient007: usize = 0x12A06E0;
            pub const SteamClient008: usize = 0x12A06F0;
            pub const SteamClient009: usize = 0x12A0700;
            pub const SteamClient010: usize = 0x12A0710;
            pub const SteamClient011: usize = 0x12A0720;
            pub const SteamClient012: usize = 0x12A0730;
            pub const SteamClient013: usize = 0x12A0740;
            pub const SteamClient014: usize = 0x12A0750;
            pub const SteamClient015: usize = 0x12A0760;
            pub const SteamClient016: usize = 0x12A0790;
            pub const SteamClient017: usize = 0x12A07C0;
            pub const SteamClient018: usize = 0x12A07F0;
            pub const SteamClient019: usize = 0x12A0820;
            pub const SteamClient020: usize = 0x12A0850;
            pub const SteamClient021: usize = 0x12A0880;
            pub const SteamClient022: usize = 0x12A08B0;
            pub const SteamClient023: usize = 0x12A08E0;
            pub const p2pvoice002: usize = 0x1EF33A0;
            pub const p2pvoicesingleton002: usize = 0x1EEBAD0;
        }
    }
}
