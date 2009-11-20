/*
 * Secure Virtual Machine (SVM)
 *
 * Copyright (C) 2009, Udo Steinberg <udo@hypervisor.org>
 *
 * This file is part of the NOVA microhypervisor.
 *
 * NOVA is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOVA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 */

#pragma once

#include "buddy.h"
#include "compiler.h"
#include "utcb.h"
#include "types.h"

class Vmcb
{
    public:
        union {
            char pad[1024];
            struct {
                uint32      intercept_cr0;          // 0x0
                uint32      intercept_dr0;          // 0x4
                uint32      intercept_exc;          // 0x8
                uint32      intercept_cpu[2];       // 0xc
                uint32      reserved1[11];          // 0x14
                uint64      base_io;                // 0x40
                uint64      base_msr;               // 0x48
                uint64      tsc_offset;             // 0x50
                uint32      asid;                   // 0x58
                uint32      tlb_control;            // 0x5c
                uint64      int_control;            // 0x60
                uint64      int_shadow;             // 0x68
                uint64      exitcode;               // 0x70
                uint64      exitinfo1;              // 0x78
                uint64      exitinfo2;              // 0x80
                uint64      exitintinfo;            // 0x88
                uint64      npt_control;            // 0x90
                uint32      reserved2[4];           // 0x98
                uint64      inj_control;            // 0xa8
                uint64      npt_cr3;                // 0xb0
                uint64      lbr;                    // 0xb8
            };
        };

        Utcb_segment        es, cs, ss, ds, fs, gs, gdtr, ldtr, idtr, tr;
        char                reserved3[48];
        uint64              efer;
        char                reserved4[112];
        uint64              cr4, cr3, cr0, dr7, dr6, rflags, rip;
        char                reserved5[88];
        uint64              rsp;
        char                reserved6[24];
        uint64              rax, star, lstar, cstar, sfmask, kernel_gs_base;
        uint64              sysenter_cs, sysenter_esp, sysenter_eip, cr2, nrip;
        char                reserved7[24];
        uint64              g_pat;

        static Vmcb *       root        CPULOCAL;
        static unsigned     asid_ctr    CPULOCAL;

        static union svm_feature {
            uint32      val;
            struct {
                uint32  np      : 1,
                        lbr     : 1,
                        lock    : 1,
                        nrips   : 1;
            };
        } feature CPULOCAL;

        enum Ctrl0
        {
            CPU_INTR        = 1ul << 0,
            CPU_NMI         = 1ul << 1,
            CPU_INIT        = 1ul << 3,
            CPU_VINTR       = 1ul << 4,
            CPU_INVD        = 1ul << 22,
            CPU_HLT         = 1ul << 24,
            CPU_IO          = 1ul << 27,
            CPU_MSR         = 1ul << 28,
            CPU_SHUTDOWN    = 1ul << 31,
        };

        enum Ctrl1
        {
            CPU_VMLOAD      = 1ul << 2,
            CPU_VMSAVE      = 1ul << 3,
            CPU_CLGI        = 1ul << 5,
            CPU_SKINIT      = 1ul << 6,
        };

        static mword const force_ctrl0 =    CPU_INTR    |
                                            CPU_NMI     |
                                            CPU_INIT    |
                                            CPU_INVD    |
                                            CPU_HLT     |
                                            CPU_IO      |
                                            CPU_MSR     |
                                            CPU_SHUTDOWN;

        static mword const force_ctrl1 =    CPU_VMLOAD  |
                                            CPU_VMSAVE  |
                                            CPU_CLGI    |
                                            CPU_SKINIT;

        ALWAYS_INLINE
        static inline void *operator new (size_t)
        {
            return Buddy::allocator.alloc (2, Buddy::NOFILL);
        }

        Vmcb (mword);

        ALWAYS_INLINE
        inline Vmcb()
        {
            asm volatile ("vmsave" : : "a" (Buddy::ptr_to_phys (root = this)) : "memory");
        }

        static void init();
};