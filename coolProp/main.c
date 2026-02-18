/*
 * main.c — Demo: load CoolProp, resolve keys, build schedule, execute
 */
#include "sys_props.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    SysRef       ref;
    SysInputs    inp;
    SysEvaluator se;

    memset(&ref, 0, sizeof(ref));

    /* ── Step 1: Load CoolProp function pointers ─────────────── */
    /*
     * On Windows replace with:
     *   if (load_coolprop_dll("CoolProp.dll") != 0) {
     *       fprintf(stderr, "Failed to load CoolProp DLL\n");
     *       return 1;
     *   }
     */
    load_stubs();

    /* ── Step 2: Resolve parameter keys from string names ────── */
    if (resolve_keys() != 0) {
        fprintf(stderr, "Key resolution failed\n");
        return 1;
    }

    /* ── Step 3: Create abstract state handle ────────────────── */
    long handle = cp.asFactory("HEOS", "R134a",
                               &cp_errcode, cp_msg, CP_MSG_LEN);
    if (cp_errcode) {
        fprintf(stderr, "Factory error: %s\n", cp_msg);
        return 1;
    }
    printf("Abstract state handle: %ld\n", handle);

    /* ── Step 4: Set up operating point ──────────────────────── */
    inp = (SysInputs){
        .P_acc   = 800000,  .h_acc   = 250000,
        .P_node1 = 800000,  .h_node1 = 260000,
        .P_node2 = 750000,  .h_node2 = 270000,
        .P_node3 = 300000,  .h_node3 = 420000,
        .P_cond  = 800000,  .h_bar1  = 280000, .h_bar3 = 240000,
                             .h_out_cond = 230000,
        .P_evap  = 300000,  .h_bar2  = 400000,
                             .h_out_evap = 430000,
        .P_lcc   = 750000,  .h_out_lcc  = 245000,
        .P_chil  = 300000,  .h_out_chil = 410000,
        .Q_0 = 0.0, .Q_05 = 0.5, .Q_1 = 1.0,
    };

    /* ── Step 5: Build evaluation schedule (once at init) ───── */
    build_system(&se, &ref, &inp);
    se.handle = handle;
    printf("Built %zu state evaluations\n", se.n_evals);
    printf("SysRef size: %zu bytes\n\n", sizeof(SysRef));

    /* ── Step 6: Execute (this is the 200 Hz hot path) ──────── */
    syseval_exec_all(&se);

    /* ── Step 7: Print results ───────────────────────────────── */
    print_sys_ref(&ref);

    /* ── Cleanup ─────────────────────────────────────────────── */
    syseval_free_tables(&se);
    cp.asFree(handle, &cp_errcode, cp_msg, CP_MSG_LEN);

    printf("\nDone.\n");
    return 0;
}
