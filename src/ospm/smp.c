#include <ospm/smp.h>

spinlock_t smp_lock;

void start_ap() {
    #undef __MODULE__
    #define __MODULE__ "ap"

    init_cpu();
    init_apic();

    asm volatile("sti");

    for (;;) {
        asm volatile("hlt");
    }
}

void init_smp(struct stivale2_struct_tag_smp* smp) {
    #undef __MODULE__
    #define __MODULE__ "smp"

    LOG("Found %d CPU(s), using the %s:\n",
            smp->cpu_count,
            smp->flags & 1 ? "x2APIC" : "xAPIC");

    LOG("\t %-4s %-18s %-18s %-s\n",
            "LID",
            "Stack",
            "Address",
            "Type");

    for (uint64_t i = 0; i < smp->cpu_count; i++) {
        struct stivale2_smp_info cpu = smp->smp_info[i];
        int is_bsp = (cpu.lapic_id == smp->bsp_lapic_id);

        if (!is_bsp) {
            uint64_t* stack_base = kmalloc(STACK_SIZE);
            uint64_t* stack_top = (uint64_t *)((uint64_t)stack_base + STACK_SIZE);

            spinlock_lock(&smp_lock);

            smp->smp_info[i].target_stack = stack_top;
            smp->smp_info[i].goto_address = (uint64_t)start_ap;

            spinlock_release(&smp_lock);
        }

        LOG("-\t%-4lu %-#18lx %-#18lx %-4s\n",
                cpu.lapic_id,
                smp->smp_info[i].target_stack,
                smp->smp_info[i].goto_address,
                is_bsp ? "BSP" : "AP");
    }
}
