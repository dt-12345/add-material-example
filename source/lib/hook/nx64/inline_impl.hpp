#pragma once

namespace exl::hook::nx64 {

    static constexpr int CtxGpOffset = 0x80;

    union GpRegister {
        u64 X;
        u32 W;
    };

    union GpRegisters {
        GpRegister m_Gp[31];
        struct {
            GpRegister _Gp[29];
            GpRegister m_Fp;
            GpRegister m_Lr;
        };
    };

    union FpRegister {
        double D;
        float S;
    };

    union FpRegisters {
        FpRegister m_Fp[16];
    };

    namespace impl {
        /* This type is only unioned with GpRegisters, so this is valid. */
        struct GpRegisterAccessorImpl {
            GpRegisters& Get() {
                return *reinterpret_cast<GpRegisters*>(this);
            }
        };

        struct GpRegisterAccessor64 : public GpRegisterAccessorImpl {
            u64& operator[](int index) {
                return Get().m_Gp[index].X;
            }
        };

        struct GpRegisterAccessor32 : public GpRegisterAccessorImpl {
            u32& operator[](int index) {
                return Get().m_Gp[index].W;
            }
        };

        struct FpRegisterAccessorImpl {
            FpRegisters& Get() {
                return *reinterpret_cast<FpRegisters*>(this);
            }
        };

        struct FpRegisterAccessor64 : public FpRegisterAccessorImpl {
            double& operator[](int index) {
                return Get().m_Fp[index].D;
            }
        };

        struct FpRegisterAccessor32 : public FpRegisterAccessorImpl {
            float& operator[](int index) {
                return Get().m_Fp[index].S;
            }
        };
    }

    struct InlineCtx {
        union {
            impl::FpRegisterAccessor64 D;
            impl::FpRegisterAccessor32 S;
            FpRegisters m_Fpr;
        };
        union {
            /* Accessors are union'd with the gprs so that they can be accessed directly. */
            impl::GpRegisterAccessor64 X;
            impl::GpRegisterAccessor32 W;
            GpRegisters m_Gpr;
        };
    };
    static_assert(offsetof(InlineCtx, m_Gpr) == CtxGpOffset);

    void InitializeInline();
}