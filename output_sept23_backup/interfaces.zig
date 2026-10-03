// Generated using https://github.com/a2x/cs2-dumper
// 2026-09-23 16:19:00.282623831 UTC

pub const cs2_dumper = struct {
    pub const interfaces = struct {
        // Module: libanimationsystem.so
        pub const libanimationsystem_so = struct {
            pub const AnimationSystemUtils_001: usize = 0x3A7FC0;
            pub const AnimationSystem_001: usize = 0x3A7CF0;
        };
        // Module: libclient.so
        pub const libclient_so = struct {
            pub const ClientToolsInfo_001: usize = 0x18EE6E0;
            pub const EmptyWorldService001_Client: usize = 0x13DE030;
            pub const GameClientExports001: usize = 0x18EDF80;
            pub const LegacyGameUI001: usize = 0x1B89350;
            pub const Source2Client002: usize = 0x18EE040;
            pub const Source2ClientConfig001: usize = 0x1388DC0;
            pub const Source2ClientPrediction001: usize = 0x1971ED0;
            pub const Source2ClientUI001: usize = 0x1AC8C70;
        };
        // Module: libengine2.so
        pub const libengine2_so = struct {
            pub const BenchmarkService001: usize = 0x3CD250;
            pub const BugBugService001: usize = 0x3C7B40;
            pub const BugService001: usize = 0x3C7AD0;
            pub const ClientServerEngineLoopService_001: usize = 0x3837D0;
            pub const ClientServerSharedHandleSystem001: usize = 0x34B8B0;
            pub const EngineGameUI001: usize = 0x5E5730;
            pub const EngineServiceMgr001: usize = 0x36F440;
            pub const GameEventSystemClientV001: usize = 0x375630;
            pub const GameEventSystemServerV001: usize = 0x375640;
            pub const GameResourceServiceClientV001: usize = 0x3CF430;
            pub const GameResourceServiceServerV001: usize = 0x3CF440;
            pub const GameUIService_001: usize = 0x3DA170;
            pub const HostStateMgr001: usize = 0x37C870;
            pub const INETSUPPORT_001: usize = 0x5998A0;
            pub const InputService_001: usize = 0x3DF680;
            pub const KeyValueCache001: usize = 0x37FF40;
            pub const MapListService_001: usize = 0x3FC560;
            pub const NetworkClientService_001: usize = 0x421900;
            pub const NetworkP2PService_001: usize = 0x4386B0;
            pub const NetworkServerService_001: usize = 0x402530;
            pub const NetworkService_001: usize = 0x4016C0;
            pub const RenderService_001: usize = 0x43E850;
            pub const ScreenshotService001: usize = 0x4423B0;
            pub const SimpleEngineLoopService_001: usize = 0x3A28F0;
            pub const SoundService_001: usize = 0x448470;
            pub const Source2EngineToClient001: usize = 0x4F5870;
            pub const Source2EngineToClientStringTable001: usize = 0x4B8E90;
            pub const Source2EngineToServer001: usize = 0x5260E0;
            pub const Source2EngineToServerStringTable001: usize = 0x501290;
            pub const SplitScreenService_001: usize = 0x452DC0;
            pub const StatsService_001: usize = 0x4571B0;
            pub const ToolService_001: usize = 0x45CD30;
            pub const VENGINE_GAMEUIFUNCS_VERSION005: usize = 0x5E4EF0;
            pub const VProfService_001: usize = 0x45E7D0;
        };
        // Module: libfilesystem_stdio.so
        pub const libfilesystem_stdio_so = struct {
            pub const VAsyncFileSystem2_001: usize = 0x11BFF0;
            pub const VFileSystem017: usize = 0x11BFE0;
        };
        // Module: libhost.so
        pub const libhost_so = struct {
            pub const DebugDrawQueueManager001: usize = 0x17B690;
            pub const GameModelInfo001: usize = 0x175150;
            pub const GameSystem2HostHook: usize = 0x1756F0;
            pub const HostUtils001: usize = 0x175C00;
            pub const PredictionDiffManager001: usize = 0x177260;
            pub const SaveRestoreDataVersion001: usize = 0x179DA0;
            pub const SinglePlayerSharedMemory001: usize = 0x17A090;
            pub const Source2Host001: usize = 0x17A8D0;
        };
        // Module: libinputsystem.so
        pub const libinputsystem_so = struct {
            pub const InputStackSystemVersion001: usize = 0x39AB0;
            pub const InputSystemVersion001: usize = 0x3B0E0;
        };
        // Module: liblocalize.so
        pub const liblocalize_so = struct {
            pub const Localize_001: usize = 0x38430;
        };
        // Module: libmatchmaking.so
        pub const libmatchmaking_so = struct {
            pub const GameTypes001: usize = 0x1AB4E0;
            pub const MATCHFRAMEWORK_001: usize = 0x2C0D80;
        };
        // Module: libmaterialsystem2.so
        pub const libmaterialsystem2_so = struct {
            pub const FontManager_001: usize = 0xD0E20;
            pub const MaterialUtils_001: usize = 0xBD760;
            pub const PostProcessingSystem_001: usize = 0xE7D50;
            pub const TextLayout_001: usize = 0xE4F00;
            pub const VMaterialSystem2_001: usize = 0x6FE10;
        };
        // Module: libmeshsystem.so
        pub const libmeshsystem_so = struct {
            pub const MeshSystem001: usize = 0x5A310;
        };
        // Module: libnetworksystem.so
        pub const libnetworksystem_so = struct {
            pub const FlattenedSerializersVersion001: usize = 0x258090;
            pub const NetworkMessagesVersion001: usize = 0x2AFAE0;
            pub const NetworkSystemVersion001: usize = 0x2CFE50;
            pub const SerializedEntitiesVersion001: usize = 0x2F03E0;
        };
        // Module: libpanorama.so
        pub const libpanorama_so = struct {
            pub const PanoramaUIEngine001: usize = 0x36E710;
        };
        // Module: libpanorama_text_pango.so
        pub const libpanorama_text_pango_so = struct {
            pub const PanoramaTextServices001: usize = 0x176B50;
        };
        // Module: libpanoramauiclient.so
        pub const libpanoramauiclient_so = struct {
            pub const PanoramaUIClient001: usize = 0x1A34E0;
        };
        // Module: libparticles.so
        pub const libparticles_so = struct {
            pub const ParticleSystemMgr003: usize = 0x2D1300;
        };
        // Module: libpulse_system.so
        pub const libpulse_system_so = struct {
            pub const IPulseSystem_001: usize = 0xB8AE0;
        };
        // Module: librendersystemvulkan.so
        pub const librendersystemvulkan_so = struct {
            pub const RenderDeviceMgr001: usize = 0x570540;
            pub const RenderUtils_001: usize = 0x4B6400;
        };
        // Module: libresourcesystem.so
        pub const libresourcesystem_so = struct {
            pub const ResourceSystem013: usize = 0x50D90;
        };
        // Module: libscenefilecache.so
        pub const libscenefilecache_so = struct {
            pub const ResponseRulesCache001: usize = 0x152CC0;
            pub const SceneFileCache002: usize = 0x150CD0;
        };
        // Module: libscenesystem.so
        pub const libscenesystem_so = struct {
            pub const RenderingPipelines_001: usize = 0x2558A0;
            pub const SceneSystem_002: usize = 0x2925C0;
            pub const SceneUtils_001: usize = 0x388480;
        };
        // Module: libserver.so
        pub const libserver_so = struct {
            pub const EmptyWorldService001_Server: usize = 0x1400DF0;
            pub const EntitySubclassUtilsV001: usize = 0xECAD80;
            pub const NavGameTest001: usize = 0x1D5E260;
            pub const ServerToolsInfo_001: usize = 0x1990540;
            pub const Source2GameClients001: usize = 0x1990530;
            pub const Source2GameDirector001: usize = 0xACE390;
            pub const Source2GameEntities001: usize = 0x19904C0;
            pub const Source2Server001: usize = 0x1990240;
            pub const Source2ServerConfig001: usize = 0x1344600;
            pub const customnavsystem001: usize = 0xD14A00;
        };
        // Module: libsoundsystem.so
        pub const libsoundsystem_so = struct {
            pub const SoundBugBugService001_Client: usize = 0x3FD110;
            pub const SoundOpSystem001: usize = 0x2F3F30;
            pub const SoundOpSystemEdit001: usize = 0x1B3890;
            pub const SoundSystem001: usize = 0x38F830;
            pub const VMixEditTool001: usize = 0x3CD1B0;
        };
        // Module: libsteamaudio.so
        pub const libsteamaudio_so = struct {
            pub const SteamAudio001: usize = 0x1CAC80;
        };
        // Module: libtier0.so
        pub const libtier0_so = struct {
            pub const TestScriptMgr001: usize = 0x24AB50;
            pub const VEngineCvar007: usize = 0x156520;
            pub const VProcessUtils002: usize = 0x2370F0;
            pub const VStringTokenSystem001: usize = 0x279490;
        };
        // Module: libv8system.so
        pub const libv8system_so = struct {
            pub const Source2V8System001: usize = 0x383D0;
        };
        // Module: libvphysics2.so
        pub const libvphysics2_so = struct {
            pub const VPhysics2_Interface_001: usize = 0x8B5D0;
        };
        // Module: libvscript.so
        pub const libvscript_so = struct {
            pub const VScriptManager010: usize = 0x638E0;
        };
        // Module: libworldrenderer.so
        pub const libworldrenderer_so = struct {
            pub const WorldRendererMgr001: usize = 0x1811F0;
        };
        // Module: steamclient.so
        pub const steamclient_so = struct {
            pub const CLIENTENGINE_INTERFACE_VERSION005: usize = 0x16154A0;
            pub const IVALIDATE001: usize = 0x1610E70;
            pub const SteamClient006: usize = 0x129FCF0;
            pub const SteamClient007: usize = 0x129FD00;
            pub const SteamClient008: usize = 0x129FD10;
            pub const SteamClient009: usize = 0x129FD20;
            pub const SteamClient010: usize = 0x129FD30;
            pub const SteamClient011: usize = 0x129FD40;
            pub const SteamClient012: usize = 0x129FD50;
            pub const SteamClient013: usize = 0x129FD60;
            pub const SteamClient014: usize = 0x129FD70;
            pub const SteamClient015: usize = 0x129FD80;
            pub const SteamClient016: usize = 0x129FDB0;
            pub const SteamClient017: usize = 0x129FDE0;
            pub const SteamClient018: usize = 0x129FE10;
            pub const SteamClient019: usize = 0x129FE40;
            pub const SteamClient020: usize = 0x129FE70;
            pub const SteamClient021: usize = 0x129FEA0;
            pub const SteamClient022: usize = 0x129FED0;
            pub const SteamClient023: usize = 0x129FF00;
            pub const p2pvoice002: usize = 0x1EF3B80;
            pub const p2pvoicesingleton002: usize = 0x1EEC2B0;
        };
    };
};
