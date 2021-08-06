从 loongson_module.c 开始，读取驱动信息：

    _X_EXPORT XF86ModuleData loongsonModuleData =
    {
        .vers = &VersRec,
        .setup = fnSetup,
        .teardown = NULL
    };

    VersRec:  驱动名称，xorg 版本信息，驱动的类型（输入/输出设备驱动，扩展驱动，xvideo驱动）
    fnSetup：   调用 xf86AddDriver 函数：xf86AddDriver(&I_LoongsonDrv, module, HaveDriverFuncs);
    I_LoongsonDrv：定义了驱动版本和名称，调用 Identify，LS_Probe，LS_AvailableOptions，DriverFunc，LS_PciProbe，LS_PlatformProbe 等

        // suijingfeng: I_ means Interface
        //
        _X_EXPORT DriverRec I_LoongsonDrv = {
            .driverVersion = 1,
            .driverName = "loongson",
            .Identify = Identify,
            .Probe = LS_Probe,
            .AvailableOptions = LS_AvailableOptions,
            .module = NULL,
            .refCount = 0,
            .driverFunc = DriverFunc,
            .supported_devices = loongson_device_match,
            .PciProbe = LS_PciProbe,
        #ifdef XSERVER_PLATFORM_BUS
            .platformProbe = LS_PlatformProbe,
        #endif
        };

        * Identify：定义了芯片的类型。
        * LS_Probe: 1>调用 xf86MatchDevice 函数，通过读取 conf 中的字符串来匹配驱动 section
                    2>调用 xf86FindOptionValue 函数:在 devSections[i]->options 列表中查找是否有 kmsdev 参数，获得的返回值传给 dev
                    3>调用 LS_OpenHW（dev）函数：获取 fd，加载 /dev/dri/card0
                    4>若成功读取 fd，调用 LS_CheckOutputs 函数--> drmModeGetResources 获取两个 drm_mode_card_res 结构体变量 res, counts，
                      一个 drmModeResPtr 结构体变量r，获取fbs，crtcs，connectors，encoders 的数量以及对应的 ptr，最大最小的width，height。
                      若使能 glamor 调用 drmGetCap。
        * DriverFunc：GET_REQUIRED_HW_INTERFACES  或  SUPPORTS_SERVER_FDS 
        * entity_num = xf86ClaimFbSlot(drv, 0, devSections[i], TRUE)
        * scrn = xf86ConfigFbEntity(scrn, 0, entity_num, NULL, NULL, NULL, NULL)
        * 若 scrn 值 ！=0 --> foundScreen = TRUE
        * 调用 LS_SetupScrnHooks(scrn, LS_Probe)：注册各种初始化等函数
            void LS_SetupScrnHooks(ScrnInfoPtr scrn, Bool (* pFnProbe)(DriverPtr, int))
                {
                    scrn->driverVersion = 1;
                    scrn->driverName = "loongson";
                    scrn->name = "loongson";

                    scrn->Probe = pFnProbe;
                    scrn->PreInit = PreInit;
                    scrn->ScreenInit = ScreenInit;
                    scrn->SwitchMode = SwitchMode;
                    scrn->AdjustFrame = AdjustFrame;
                    scrn->EnterVT = EnterVT;
                    scrn->LeaveVT = LeaveVT;
                    scrn->FreeScreen = FreeScreen;
                    scrn->ValidMode = ValidMode;
                }
        
        * LS_SetupEntity（ScrnInfoPtr scrn, int entity_num)  ？？？？？？？？


（1）PreInit：1> LS_AllocDriverPrivate(pScrn):申请DriverPrivate
             2> ms->pEnt = xf86GetEntityInfo(pScrn->entityList[0])获取实体信息。
             3> Primary init
             4> ms_get_drm_master_fd(pScrn)  ??????
             5> LS_CheckOutputs(ms->fd, &connector_count) 获取 connector_count 数量
             6> InitDefaultVisual(pScrn):--> drmmode_get_default_bpp 创建一个临时 bo 来获取默认的 bpp 和 depth。设置 rgb 的 weight 和 mask。
                xf86SetDefaultVisual
             7> LS_ProcessOptions(pScrn, &ms->drmmode.Options):搜集 options-->传到 ms->drmmode.Options 中并使用。
             8> LS_GetCursorDimK(pScrn):从 ms->drmmode.Options 中读取 option，并设置 cursor 的 width，height，SWcursor or HWcursor
             9> LS_PrepareDebug(pScrn):从 conf 中读取配置，是否使能 debug 功能
             10> ret = drmGetCap(ms->fd, DRM_CAP_PRIME, &value)：获取 value 和 cap 值，判断底层是否支持prime。
             11> try_enable_glamor(pScrn): 通过读取 ms->drmmode.Options 中信息进行 conf 中 glamor 字符匹配，因为暂未支持 glamor 
                直接写死为false。load_glamor(pScrn)：加载 glamor 模块，加载各种函数的标志位？？？？？？
             12> try_enable_exa(pScrn)：是否使能 exa，若在未支持 glamor 的情况下， 支持 prime 下，可加载 exa，若未支持 prime 直接回滚到 shadow。
                若支持 prime，先从 conf 中读取是否使能 exa,若使能，加载 exa 模块，默认选择 fake—exa 加速方式
             13> LS_TryEnableShadow(pScrn):从 conf 中读取是否使能 shadow ，从 conf 中检测是否使能 doubleshadow， 查看驱动的版本信息？？？？？
             14> xf86LoadSubModule(pScrn, "fb"):加载 fb 子模块。
             15> 使能 pageflip
             16> 获取 pScrn->capabilities 的值
             17> 是否使能 atomic 
             18> ms->kms_has_modifiers  ??????
             19> drmmode_pre_init(pScrn, &ms->drmmode, pScrn->bitsPerPixel / 8):检测 dumb capability; xf86CrtcConfigInit;
                 获取fbs，crtcs，connectors，encoders 的数量以及对应的 ptr，最大最小的width，height。
                 设置克隆屏等一些信息。（可多仔细看看）
             20> 设置 gamma 校验值
             21> xf86SetDpi(pScrn, 0, 0):设置显示信息（可多仔细看看）
             22> LS_ShadowLoadAPI(pScrn):加载 shadow 模块。

(2）ScreenInit：   1> SetMaster(pScrn):???????
                  2> ms->drmmode.gbm = ms->glamor.egl_get_gbm_device(pScreen)
                  3> drmmode_create_initial_bos(pScrn, &ms->drmmode):创建 front_bo 和 cursor_bo
                  4> ms->drmmode.shadow_enable = LS_ShadowAllocFB(pScrn):为 shadow_fb 申请内存
                  5> miClearVisualTypes(): Reset the visual list ???????
                  6> miSetVisualTypes(pScrn->depth,miGetDefaultVisualMask(pScrn->depth),pScrn->rgbBits, pScrn->defaultVisual)
                  7> miSetPixmapDepths:
                  8> dixRegisterScreenSpecificPrivateKey(pScreen, &ms->drmmode.pixmapPrivateKeyRec, 
                    PRIVATE_PIXMAP,sizeof(msPixmapPrivRec)):设置 screen 中 size，offset，type 等值。
                  9> fbScreenInit(pScreen, NULL,pScrn->virtualX, pScrn->virtualY,
                      pScrn->xDpi, pScrn->yDpi,pScrn->displayWidth, pScrn->bitsPerPixel)-->fbSetupScreen：调用各种 screen 的注册函数。
                      获取 pitch 
                  10> Fixup RGB ordering
                  11> fbPictureInit(pScreen, NULL, 0)
                  12> drmmode_init(pScrn, &ms->drmmode)：初始化 glamor
                  13> ms->shadow.Setup(pScreen): 初始化 shadow
                  14> ms->createScreenResources = pScreen->CreateScreenResources;
                      pScreen->CreateScreenResources = CreateScreenResources;???????????????
                  15> xf86SetBlackWhitePixels(pScreen):设置黑白像素
                  16> xf86SetBackingStore(pScreen):Initialize backing store
                  17> xf86SetSilkenMouse(pScreen):Enable cursor position updates by mouse signal handler
                  18> miDCInitialize(pScreen, xf86GetPointerScreenFuncs())
                  19> if (ms->drmmode.pageflip)
                        {
                            miPointerScreenPtr PointPriv =
                                dixLookupPrivate(&pScreen->devPrivates, miPointerScreenKey);

                            if (!dixRegisterScreenPrivateKey(&ms->drmmode.spritePrivateKeyRec,
                                                pScreen, PRIVATE_DEVICE, sizeof(msSpritePrivRec)))
                            {
                                return FALSE;
                            }

                            ms->SpriteFuncs = PointPriv->spriteFuncs;
                            PointPriv->spriteFuncs = &drmmode_sprite_funcs;
                        }
                  20> xf86_cursors_init(pScreen, ms->cursor_width, ms->cursor_height,
                          HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_64 |
                          HARDWARE_CURSOR_UPDATE_UNHIDDEN | HARDWARE_CURSOR_ARGB):创建并初始化光标，设置光标大小，注册函数，设置隐藏光标。
                  21> 