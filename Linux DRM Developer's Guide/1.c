IRQ Registration：drm core 通过 drm_irq_install 和 drm_irq_uninstall 进行 IRQ 的注册与取消，这个功能只支持每台设备单中断，
                    如果设备产生多中断，需要手动处理。
Managed IRQ Registration：