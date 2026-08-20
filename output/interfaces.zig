// Generated using https://github.com/a2x/cs2-dumper
// 2026-08-20 19:49:07.583476765 UTC

pub const cs2_dumper = struct {
    pub const interfaces = struct {
        // Module: libanimationsystem.so
        pub const libanimationsystem_so = struct {
            pub const AnimationSystemUtils_001: usize = 0x3A77C0;
            pub const AnimationSystem_001: usize = 0x3A74F0;
        };
        // Module: libclient.so
        pub const libclient_so = struct {
            pub const ClientToolsInfo_001: usize = 0x1890950;
            pub const EmptyWorldService001_Client: usize = 0x139B040;
            pub const GameClientExports001: usize = 0x18901D0;
            pub const LegacyGameUI001: usize = 0x1B16570;
            pub const Source2Client002: usize = 0x1890290;
            pub const Source2ClientConfig001: usize = 0x1346550;
            pub const Source2ClientPrediction001: usize = 0x1911670;
            pub const Source2ClientUI001: usize = 0x1A5F650;
        };
        // Module: libengine2.so
        pub const libengine2_so = struct {
            pub const BenchmarkService001: usize = 0x3C2BF0;
            pub const BugBugService001: usize = 0x3BD4C0;
            pub const BugService001: usize = 0x3BD450;
            pub const ClientServerEngineLoopService_001: usize = 0x3788D0;
            pub const ClientServerSharedHandleSystem001: usize = 0x340180;
            pub const EngineGameUI001: usize = 0x5D6C60;
            pub const EngineServiceMgr001: usize = 0x3644D0;
            pub const GameEventSystemClientV001: usize = 0x36ABF0;
            pub const GameEventSystemServerV001: usize = 0x36AC00;
            pub const GameResourceServiceClientV001: usize = 0x3C4D30;
            pub const GameResourceServiceServerV001: usize = 0x3C4D40;
            pub const GameUIService_001: usize = 0x3CF9E0;
            pub const HostStateMgr001: usize = 0x371B10;
            pub const INETSUPPORT_001: usize = 0x58B100;
            pub const InputService_001: usize = 0x3D51B0;
            pub const KeyValueCache001: usize = 0x375190;
            pub const MapListService_001: usize = 0x3F2160;
            pub const NetworkClientService_001: usize = 0x414760;
            pub const NetworkP2PService_001: usize = 0x42AAB0;
            pub const NetworkServerService_001: usize = 0x3F8150;
            pub const NetworkService_001: usize = 0x3F72B0;
            pub const RenderService_001: usize = 0x430BC0;
            pub const ScreenshotService001: usize = 0x4349D0;
            pub const SimpleEngineLoopService_001: usize = 0x397EF0;
            pub const SoundService_001: usize = 0x43A670;
            pub const Source2EngineToClient001: usize = 0x4E5E60;
            pub const Source2EngineToClientStringTable001: usize = 0x4AAB50;
            pub const Source2EngineToServer001: usize = 0x516490;
            pub const Source2EngineToServerStringTable001: usize = 0x4F1800;
            pub const SplitScreenService_001: usize = 0x445230;
            pub const StatsService_001: usize = 0x449790;
            pub const ToolService_001: usize = 0x44F060;
            pub const VENGINE_GAMEUIFUNCS_VERSION005: usize = 0x5D64E0;
            pub const VProfService_001: usize = 0x450A70;
        };
        // Module: libfilesystem_stdio.so
        pub const libfilesystem_stdio_so = struct {
            pub const VAsyncFileSystem2_001: usize = 0x11C330;
            pub const VFileSystem017: usize = 0x11C320;
        };
        // Module: libhost.so
        pub const libhost_so = struct {
            pub const DebugDrawQueueManager001: usize = 0x179E70;
            pub const GameModelInfo001: usize = 0x173910;
            pub const GameSystem2HostHook: usize = 0x173E30;
            pub const HostUtils001: usize = 0x174340;
            pub const PredictionDiffManager001: usize = 0x175920;
            pub const SaveRestoreDataVersion001: usize = 0x178580;
            pub const SinglePlayerSharedMemory001: usize = 0x178870;
            pub const Source2Host001: usize = 0x1790B0;
        };
        // Module: libinputsystem.so
        pub const libinputsystem_so = struct {
            pub const InputStackSystemVersion001: usize = 0x39A40;
            pub const InputSystemVersion001: usize = 0x3AFD0;
        };
        // Module: liblocalize.so
        pub const liblocalize_so = struct {
            pub const Localize_001: usize = 0x38240;
        };
        // Module: libmatchmaking.so
        pub const libmatchmaking_so = struct {
            pub const GameTypes001: usize = 0x1AA240;
            pub const MATCHFRAMEWORK_001: usize = 0x2C01C0;
        };
        // Module: libmaterialsystem2.so
        pub const libmaterialsystem2_so = struct {
            pub const FontManager_001: usize = 0xD01E0;
            pub const MaterialUtils_001: usize = 0xBCDA0;
            pub const PostProcessingSystem_001: usize = 0xE6C70;
            pub const TextLayout_001: usize = 0xE40E0;
            pub const VMaterialSystem2_001: usize = 0x6F2E0;
        };
        // Module: libmeshsystem.so
        pub const libmeshsystem_so = struct {
            pub const MeshSystem001: usize = 0x596F0;
        };
        // Module: libnetworksystem.so
        pub const libnetworksystem_so = struct {
            pub const FlattenedSerializersVersion001: usize = 0x2510D0;
            pub const NetworkMessagesVersion001: usize = 0x2A4210;
            pub const NetworkSystemVersion001: usize = 0x2C4650;
            pub const SerializedEntitiesVersion001: usize = 0x2E4C60;
        };
        // Module: libpanorama.so
        pub const libpanorama_so = struct {
            pub const PanoramaUIEngine001: usize = 0x36C050;
        };
        // Module: libpanorama_text_pango.so
        pub const libpanorama_text_pango_so = struct {
            pub const PanoramaTextServices001: usize = 0x176900;
        };
        // Module: libpanoramauiclient.so
        pub const libpanoramauiclient_so = struct {
            pub const PanoramaUIClient001: usize = 0x1A8540;
        };
        // Module: libparticles.so
        pub const libparticles_so = struct {
            pub const ParticleSystemMgr003: usize = 0x2BB140;
        };
        // Module: libpulse_system.so
        pub const libpulse_system_so = struct {
            pub const IPulseSystem_001: usize = 0xBF730;
        };
        // Module: librendersystemvulkan.so
        pub const librendersystemvulkan_so = struct {
            pub const RenderDeviceMgr001: usize = 0x5673B0;
            pub const RenderUtils_001: usize = 0x4B2090;
        };
        // Module: libresourcesystem.so
        pub const libresourcesystem_so = struct {
            pub const ResourceSystem013: usize = 0x4F7F0;
        };
        // Module: libscenefilecache.so
        pub const libscenefilecache_so = struct {
            pub const ResponseRulesCache001: usize = 0x14C330;
            pub const SceneFileCache002: usize = 0x14A3E0;
        };
        // Module: libscenesystem.so
        pub const libscenesystem_so = struct {
            pub const RenderingPipelines_001: usize = 0x253C90;
            pub const SceneSystem_002: usize = 0x290970;
            pub const SceneUtils_001: usize = 0x384D70;
        };
        // Module: libschemasystem.so
        pub const libschemasystem_so = struct {
            pub const SchemaSystem_001: usize = 0x3A3B0;
        };
        // Module: libserver.so
        pub const libserver_so = struct {
            pub const EmptyWorldService001_Server: usize = 0x139F800;
            pub const EntitySubclassUtilsV001: usize = 0xEADDD0;
            pub const NavGameTest001: usize = 0x1C13AA0;
            pub const ServerToolsInfo_001: usize = 0x18D1700;
            pub const Source2GameClients001: usize = 0x18D16F0;
            pub const Source2GameDirector001: usize = 0xACF400;
            pub const Source2GameEntities001: usize = 0x18D1660;
            pub const Source2Server001: usize = 0x18D1410;
            pub const Source2ServerConfig001: usize = 0x12E98E0;
            pub const customnavsystem001: usize = 0xD094B0;
        };
        // Module: libsoundsystem.so
        pub const libsoundsystem_so = struct {
            pub const SoundBugBugService001_Client: usize = 0x42D3A0;
            pub const SoundOpSystem001: usize = 0x31B630;
            pub const SoundOpSystemEdit001: usize = 0x1E4FE0;
            pub const SoundSystem001: usize = 0x3B68B0;
            pub const VMixEditTool001: usize = 0x3FA040;
        };
        // Module: libsteamaudio.so
        pub const libsteamaudio_so = struct {
            pub const SteamAudio001: usize = 0x20A4B0;
        };
        // Module: libtier0.so
        pub const libtier0_so = struct {
            pub const TestScriptMgr001: usize = 0x246950;
            pub const VEngineCvar007: usize = 0x154160;
            pub const VProcessUtils002: usize = 0x235CF0;
            pub const VStringTokenSystem001: usize = 0x273F10;
        };
        // Module: libv8system.so
        pub const libv8system_so = struct {
            pub const Source2V8System001: usize = 0x37E20;
        };
        // Module: libvphysics2.so
        pub const libvphysics2_so = struct {
            pub const VPhysics2_Interface_001: usize = 0x8A5F0;
        };
        // Module: libvscript.so
        pub const libvscript_so = struct {
            pub const VScriptManager010: usize = 0x637A0;
        };
        // Module: libworldrenderer.so
        pub const libworldrenderer_so = struct {
            pub const WorldRendererMgr001: usize = 0x1807F0;
        };
        // Module: steamclient.so
        pub const steamclient_so = struct {
            pub const CLIENTENGINE_INTERFACE_VERSION005: usize = 0x15B3BC0;
            pub const IVALIDATE001: usize = 0x15AF8E0;
            pub const SteamClient006: usize = 0x123AF50;
            pub const SteamClient007: usize = 0x123AF60;
            pub const SteamClient008: usize = 0x123AF70;
            pub const SteamClient009: usize = 0x123AF80;
            pub const SteamClient010: usize = 0x123AF90;
            pub const SteamClient011: usize = 0x123AFA0;
            pub const SteamClient012: usize = 0x123AFB0;
            pub const SteamClient013: usize = 0x123AFC0;
            pub const SteamClient014: usize = 0x123AFD0;
            pub const SteamClient015: usize = 0x123AFE0;
            pub const SteamClient016: usize = 0x123AFF0;
            pub const SteamClient017: usize = 0x123B000;
            pub const SteamClient018: usize = 0x123B010;
            pub const SteamClient019: usize = 0x123B020;
            pub const SteamClient020: usize = 0x123B030;
            pub const SteamClient021: usize = 0x123B040;
            pub const SteamClient022: usize = 0x123B050;
            pub const SteamClient023: usize = 0x123B060;
            pub const p2pvoice002: usize = 0x1E64380;
            pub const p2pvoicesingleton002: usize = 0x1E5CB50;
        };
    };
};
