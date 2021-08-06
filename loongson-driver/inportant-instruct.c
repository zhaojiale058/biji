1.重要的结构体：（modesettingRec, *modesettingPtr）

（1） modesettingRec, *modesettingPtr：

        typedef struct _modesettingRec {
        int fd;
        Bool fd_passed;

        int Chipset;
        EntityInfoPtr pEnt;

        Bool noAccel;
        CloseScreenProcPtr CloseScreen;
        CreateWindowProcPtr CreateWindow;

        CreateScreenResourcesProcPtr createScreenResources;
        ScreenBlockHandlerProcPtr BlockHandler;
        miPointerSpriteFuncPtr SpriteFuncs;
        void *driver;

        drmmode_rec drmmode;

        drmEventContext event_context;

        /**
        * Page flipping stuff.
        *  @{
        */
        Bool atomic_modeset;
        Bool pending_modeset;
        /** @} */

        DamagePtr damage;
        Bool dirty_enabled;

        uint32_t cursor_width, cursor_height;

        Bool has_queue_sequence;
        Bool tried_queue_sequence;

        Bool kms_has_modifiers;


        /* EXA API */
        ExaDriverPtr exaDrvPtr;

        /* shadow API */
        struct ShadowAPI {
            Bool (*Setup)(ScreenPtr);
            Bool (*Add)(ScreenPtr, PixmapPtr, ShadowUpdateProc, ShadowWindowProc, int, void *);
            void (*Remove)(ScreenPtr, PixmapPtr);
            void (*Update32to24)(ScreenPtr, shadowBufPtr);
            void (*UpdatePacked)(ScreenPtr, shadowBufPtr);
        } shadow;

    #ifdef GLAMOR_HAS_GBM
        /* glamor API */
        struct {
            Bool (*back_pixmap_from_fd)(PixmapPtr, int, CARD16, CARD16, CARD16,
                                        CARD8, CARD8);
            void (*block_handler)(ScreenPtr);
            void (*clear_pixmap)(PixmapPtr);
            Bool (*egl_create_textured_pixmap)(PixmapPtr, int, int);
            Bool (*egl_create_textured_pixmap_from_gbm_bo)(PixmapPtr,
                                                        struct gbm_bo *,
                                                        Bool);
            void (*egl_exchange_buffers)(PixmapPtr, PixmapPtr);
            struct gbm_device *(*egl_get_gbm_device)(ScreenPtr);
            Bool (*egl_init)(ScrnInfoPtr, int);
            void (*finish)(ScreenPtr);
            struct gbm_bo *(*gbm_bo_from_pixmap)(ScreenPtr, PixmapPtr);
            Bool (*init)(ScreenPtr, unsigned int);
            int (*name_from_pixmap)(PixmapPtr, CARD16 *, CARD32 *);
            void (*set_drawable_modifiers_func)(ScreenPtr,
                                                GetDrawableModifiersFuncPtr);
            int (*shareable_fd_from_pixmap)(ScreenPtr, PixmapPtr, CARD16 *,
                                            CARD32 *);
            Bool (*supports_pixmap_import_export)(ScreenPtr);
            XF86VideoAdaptorPtr (*xv_init)(ScreenPtr, int);
            const char *(*egl_get_driver_name)(ScreenPtr);
        } glamor;
    #endif
    } modesettingRec, *modesettingPtr;





 (2)  drmmode_rec, *drmmode_ptr

        typedef struct {
        int fd;
        unsigned fb_id;
        drmModeFBPtr mode_fb;
        int cpp;
        int kbpp;
        ScrnInfoPtr scrn;

        struct gbm_device *gbm;

    #ifdef HAVE_LIBUDEV
        struct udev_monitor *uevent_monitor;
        InputHandlerProc uevent_handler;
    #endif
        drmEventContext event_context;
        drmmode_bo front_bo;
        Bool sw_cursor;

        /* Broken-out options. */
        OptionInfoPtr Options;

        Bool glamor;
        Bool exa_enabled;
        enum ExaAccelType exa_acc_type;
        Bool shadow_enable;
        Bool shadow_enable2;


        /** Is Option "PageFlip" enabled? */
        Bool pageflip;
        Bool force_24_32;
        void *shadow_fb;
        void *shadow_fb2;
        /* SCREEN SPECIFIC_PRIVATE_KEYS */
        DevPrivateKeyRec pixmapPrivateKeyRec;
        DevScreenPrivateKeyRec spritePrivateKeyRec;
        /* Number of SW cursors currently visible on this screen */
        int sprites_visible;

        Bool reverse_prime_offload_mode;

        Bool is_secondary;

        PixmapPtr fbcon_pixmap;

    #ifdef DRI3
        char *dri3_device_name;
    #endif

        Bool dri2_flipping;
        Bool present_flipping;
        Bool flip_bo_import_failed;

        Bool dri2_enable;
        Bool present_enable;
    } drmmode_rec, *drmmode_ptr;






(3)msPixmapPrivRec, *msPixmapPrivPtr:

        /* OUTPUT SLAVE SUPPORT */
        typedef struct _msPixmapPriv {
            uint32_t fb_id;
            struct dumb_bo *backing_bo; /* if this pixmap is backed by a dumb bo */
            /* OUTPUT SLAVE SUPPORT */
            DamagePtr slave_damage;

            /** Sink fields for flipping shared pixmaps */
            int flip_seq; /* seq of current page flip event handler */
            Bool wait_for_damage; /* if we have requested damage notification from source */

            /** Source fields for flipping shared pixmaps */
            Bool defer_dirty_update; /* if we want to manually update */
            PixmapDirtyUpdatePtr dirty; /* cached dirty ent to avoid searching list */
            DrawablePtr slave_src; /* if we exported shared pixmap, dirty tracking src */
            Bool notify_on_damage; /* if sink has requested damage notification */
        } msPixmapPrivRec, *msPixmapPrivPtr;




(4)PictureScreenRec, *PictureScreenPtr:

        typedef struct _PictureScreen {
            PictFormatPtr formats;
            PictFormatPtr fallback;
            int nformats;

            CreatePictureProcPtr CreatePicture;
            DestroyPictureProcPtr DestroyPicture;
            ChangePictureClipProcPtr ChangePictureClip;
            DestroyPictureClipProcPtr DestroyPictureClip;

            ChangePictureProcPtr ChangePicture;
            ValidatePictureProcPtr ValidatePicture;

            CompositeProcPtr Composite;
            GlyphsProcPtr Glyphs;       /* unused */
            CompositeRectsProcPtr CompositeRects;

            DestroyWindowProcPtr DestroyWindow;
            CloseScreenProcPtr CloseScreen;

            StoreColorsProcPtr StoreColors;

            InitIndexedProcPtr InitIndexed;
            CloseIndexedProcPtr CloseIndexed;
            UpdateIndexedProcPtr UpdateIndexed;

            int subpixel;

            PictFilterPtr filters;
            int nfilters;
            PictFilterAliasPtr filterAliases;
            int nfilterAliases;

            /**
            * Called immediately after a picture's transform is changed through the
            * SetPictureTransform request.  Not called for source-only pictures.
            */
            ChangePictureTransformProcPtr ChangePictureTransform;

            /**
            * Called immediately after a picture's transform is changed through the
            * SetPictureFilter request.  Not called for source-only pictures.
            */
            ChangePictureFilterProcPtr ChangePictureFilter;

            DestroyPictureFilterProcPtr DestroyPictureFilter;

            TrapezoidsProcPtr Trapezoids;
            TrianglesProcPtr Triangles;

            RasterizeTrapezoidProcPtr RasterizeTrapezoid;

            AddTrianglesProcPtr AddTriangles;

            AddTrapsProcPtr AddTraps;

            RealizeGlyphProcPtr RealizeGlyph;
            UnrealizeGlyphProcPtr UnrealizeGlyph;

        #define PICTURE_SCREEN_VERSION 2
            TriStripProcPtr TriStrip;
            TriFanProcPtr TriFan;
        } PictureScreenRec, *PictureScreenPtr






(5)xf86CursorInfoRec, *xf86CursorInfoPtr:

        typedef struct _xf86CursorInfoRec {
            ScrnInfoPtr pScrn;
            int Flags;
            int MaxWidth;
            int MaxHeight;
            void (*SetCursorColors) (ScrnInfoPtr pScrn, int bg, int fg);
            void (*SetCursorPosition) (ScrnInfoPtr pScrn, int x, int y);
            void (*LoadCursorImage) (ScrnInfoPtr pScrn, unsigned char *bits);
            Bool (*LoadCursorImageCheck) (ScrnInfoPtr pScrn, unsigned char *bits);
            void (*HideCursor) (ScrnInfoPtr pScrn);
            void (*ShowCursor) (ScrnInfoPtr pScrn);
            Bool (*ShowCursorCheck) (ScrnInfoPtr pScrn);
            unsigned char *(*RealizeCursor) (struct _xf86CursorInfoRec *, CursorPtr);
            Bool (*UseHWCursor) (ScreenPtr, CursorPtr);

            Bool (*UseHWCursorARGB) (ScreenPtr, CursorPtr);
            void (*LoadCursorARGB) (ScrnInfoPtr, CursorPtr);
            Bool (*LoadCursorARGBCheck) (ScrnInfoPtr, CursorPtr);

        } xf86CursorInfoRec, *xf86CursorInfoPtr;