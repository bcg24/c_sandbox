/*
 * sys_props.c — Implementation
 */
#include "sys_props.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ================================================================
 * Globals
 * ================================================================ */

CoolProp cp;
long     cp_errcode;
char     cp_msg[CP_MSG_LEN];

long IP_PQ  = 0;
long IP_HmP = 0;

ParamKey KEYS[N_KEYS] = {
    [K_T]     = {"T",               0},
    [K_P]     = {"P",               0},
    [K_H]     = {"Hmass",           0},
    [K_RHO]   = {"Dmass",           0},
    [K_CP]    = {"Cpmass",          0},
    [K_CV]    = {"Cvmass",          0},
    [K_MU]    = {"viscosity",       0},
    [K_K]     = {"conductivity",    0},
    [K_PR]    = {"Prandtl",         0},
    [K_S]     = {"Smass",           0},
    [K_Q]     = {"Q",               0},
    [K_SIGMA] = {"surface_tension", 0},
};

Sym SYMS[] = {
    {"AbstractState_factory",               (generic_func_ptr *)&cp.asFactory},
    {"AbstractState_free",                  (generic_func_ptr *)&cp.asFree},
    {"AbstractState_update",                (generic_func_ptr *)&cp.asUpdate},
    {"AbstractState_keyed_output",          (generic_func_ptr *)&cp.asKeyed},
    {"AbstractState_first_partial_deriv",   (generic_func_ptr *)&cp.asDeriv},
    {"AbstractState_first_saturation_deriv",(generic_func_ptr *)&cp.asDeriv2ph},
    {"get_param_index",                     (generic_func_ptr *)&cp.getParamIndex},
    {"get_input_pair_index",                (generic_func_ptr *)&cp.getInputPairIndex},
};

#define N_SYMS (sizeof(SYMS) / sizeof(SYMS[0]))

/* ================================================================
 * Stubs (replace with DLL on Windows)
 * ================================================================ */

static double stub_P_val;

static long stub_factory(const char *be, const char *fl,
                         long *err, char *msg, long len) {
    (void)be; (void)fl; (void)msg; (void)len;
    *err = 0; return 42;
}

static void stub_free(long h, long *err, char *msg, long len) {
    (void)h; (void)msg; (void)len; *err = 0;
}

static void stub_update(long h, long ip, double v1, double v2,
                        long *err, char *msg, long len) {
    (void)h; (void)ip; (void)v2; (void)msg; (void)len;
    stub_P_val = v1; *err = 0;
}

static double stub_keyed(long h, long param,
                         long *err, char *msg, long len) {
    (void)h; (void)msg; (void)len; *err = 0;
    return (double)param * 0.001 + stub_P_val * 0.01;
}

static double stub_deriv(long h, long of, long wrt, long cst,
                         long *err, char *msg, long len) {
    (void)h; (void)cst; (void)msg; (void)len; *err = 0;
    return (double)(of * 1000 + wrt) * 1e-6;
}

static double stub_deriv2ph(long h, long of, long wrt,
                            long *err, char *msg, long len) {
    (void)h; (void)msg; (void)len; *err = 0;
    return (double)(of * 1000 + wrt) * 1e-6;
}

static long stub_keys[]  = {100,101,102,103,104,105,106,107,108,109,110,111};
static const char *stub_names[] = {
    "T","P","Hmass","Dmass","Cpmass","Cvmass",
    "viscosity","conductivity","Prandtl","Smass","Q","surface_tension"
};
#define N_STUB (sizeof(stub_names)/sizeof(stub_names[0]))

static long stub_get_param_index(const char *p) {
    for (size_t i = 0; i < N_STUB; i++)
        if (strcmp(p, stub_names[i]) == 0) return stub_keys[i];
    fprintf(stderr, "WARNING: unknown param '%s'\n", p);
    return -1;
}

static long stub_get_input_pair_index(const char *p) {
    if (strcmp(p, "PQ_INPUTS") == 0)    return 200;
    if (strcmp(p, "HmassP_INPUTS") == 0) return 201;
    return -1;
}

void load_stubs(void) {
    cp.asFactory     = stub_factory;
    cp.asFree        = stub_free;
    cp.asUpdate      = stub_update;
    cp.asKeyed       = stub_keyed;
    cp.asDeriv       = stub_deriv;
    cp.asDeriv2ph    = stub_deriv2ph;
    cp.getParamIndex = stub_get_param_index;
    cp.getInputPairIndex = stub_get_input_pair_index;
    printf("Loaded %zu CoolProp function pointers (stubs)\n", N_SYMS);
}

/*
 * Windows DLL loader (uncomment and use instead of load_stubs):
 *
 * #include <windows.h>
 * int load_coolprop_dll(const char *dll_path) {
 *     HMODULE hLib = LoadLibrary(dll_path);
 *     if (!hLib) return -1;
 *     for (size_t i = 0; i < N_SYMS; i++) {
 *         FARPROC sym = GetProcAddress(hLib, SYMS[i].name);
 *         if (!sym) {
 *             fprintf(stderr, "Failed to load: %s\n", SYMS[i].name);
 *             FreeLibrary(hLib);
 *             return -1;
 *         }
 *         *SYMS[i].slot = (generic_func_ptr)sym;
 *     }
 *     printf("Loaded %zu CoolProp function pointers from DLL\n", N_SYMS);
 *     return 0;
 * }
 */

/* ================================================================
 * Key resolution
 * ================================================================ */

int resolve_keys(void) {
    int errors = 0;
    for (int i = 0; i < N_KEYS; i++) {
        KEYS[i].key = cp.getParamIndex(KEYS[i].name);
        if (KEYS[i].key < 0) {
            fprintf(stderr, "ERROR: failed to resolve '%s'\n", KEYS[i].name);
            errors++;
        }
    }
    IP_PQ  = cp.getInputPairIndex("PQ_INPUTS");
    IP_HmP = cp.getInputPairIndex("HmassP_INPUTS");
    if (IP_PQ < 0 || IP_HmP < 0) {
        fprintf(stderr, "ERROR: failed to resolve input pair keys\n");
        errors++;
    }
    printf("Resolved %d parameter keys + 2 input pairs", N_KEYS);
    if (errors) printf(" (%d errors)", errors);
    printf("\n");
    return errors;
}

/* ================================================================
 * Table builders
 * ================================================================ */

#define MAX_PB 32
#define MAX_DB 16

typedef struct { PropReq  e[MAX_PB]; size_t n; } PB;
typedef struct { DerivReq e[MAX_DB]; size_t n; } DB;

static void pb_clr(PB *b)                              { b->n = 0; }
static void db_clr(DB *b)                              { b->n = 0; }

static void pb_add(PB *b, enum KeyIdx ki, size_t off) {
    assert(b->n < MAX_PB);
    b->e[b->n++] = (PropReq){ .key_idx = ki, .offset = off };
}

static void db_partial(DB *b, enum KeyIdx of, enum KeyIdx wrt,
                       enum KeyIdx cst, size_t off) {
    assert(b->n < MAX_DB);
    b->e[b->n++] = (DerivReq){
        .type = DERIV_PARTIAL, .of_idx = of,
        .wrt_idx = wrt, .constant_idx = cst, .offset = off };
}

static void db_sat(DB *b, enum KeyIdx of, enum KeyIdx wrt, size_t off) {
    assert(b->n < MAX_DB);
    b->e[b->n++] = (DerivReq){
        .type = DERIV_SAT, .of_idx = of, .wrt_idx = wrt, .offset = off };
}

static void db_rhoH(DB *b, size_t dst, size_t rho, size_t h,
                     size_t dh, size_t drho) {
    assert(b->n < MAX_DB);
    b->e[b->n++] = (DerivReq){
        .type = DERIV_COMPUTED, .offset = dst,
        .rho_offset = rho, .h_offset = h,
        .dh_dP_offset = dh, .drho_dP_offset = drho };
}

static PropReq *pb_fin(PB *b, size_t *n) {
    *n = b->n;
    if (!b->n) return NULL;
    PropReq *o = malloc(b->n * sizeof *o);
    memcpy(o, b->e, b->n * sizeof *o);
    return o;
}

static DerivReq *db_fin(DB *b, size_t *n) {
    *n = b->n;
    if (!b->n) return NULL;
    DerivReq *o = malloc(b->n * sizeof *o);
    memcpy(o, b->e, b->n * sizeof *o);
    return o;
}

/* ── Composite helpers ───────────────────────────────────────── */

static void db_drho_partials(DB *b, size_t dP_off, size_t dh_off) {
    db_partial(b, K_RHO, K_P, K_H,   dP_off);
    db_partial(b, K_RHO, K_H, K_P,   dh_off);
}

static void db_sat_set(DB *b,
                       size_t dh_dP, size_t drho_dP, size_t drhoH_dP,
                       size_t p_rho, size_t p_h) {
    db_sat(b, K_H,   K_P, dh_dP);
    db_sat(b, K_RHO, K_P, drho_dP);
    db_rhoH(b, drhoH_dP, p_rho, p_h, dh_dP, drho_dP);
}

static void pb_outlet(PB *b) {
    pb_add(b, K_H,   offsetof(OutletProps, h));
    pb_add(b, K_MU,  offsetof(OutletProps, mu));
    pb_add(b, K_RHO, offsetof(OutletProps, rho));
}

static void pb_bar6(PB *b, size_t T, size_t h, size_t mu,
                    size_t rho, size_t Pr, size_t Kk) {
    pb_add(b, K_T, T); pb_add(b, K_H, h); pb_add(b, K_MU, mu);
    pb_add(b, K_RHO, rho); pb_add(b, K_PR, Pr); pb_add(b, K_K, Kk);
}

/* ================================================================
 * State evaluation execution
 * ================================================================ */

static void state_eval_exec(StateEval *ev, long handle) {
    /* 1. CoolProp update */
    switch (ev->update_type) {
    case UPDATE_PH:
        cp.asUpdate(handle, IP_HmP, *ev->input_second, *ev->input_P,
                    &cp_errcode, cp_msg, CP_MSG_LEN);
        break;
    case UPDATE_PQ:
        cp.asUpdate(handle, IP_PQ, *ev->input_P, *ev->input_second,
                    &cp_errcode, cp_msg, CP_MSG_LEN);
        break;
    }
    if (cp_errcode) {
        fprintf(stderr, "[%s] update error %ld: %s\n",
                ev->label, cp_errcode, cp_msg);
        return;
    }

    /* 2. Standard properties */
    for (size_t i = 0; i < ev->n_props; i++) {
        double *f = (double *)((char *)ev->prop_dest + ev->prop_table[i].offset);
        *f = cp.asKeyed(handle, K(ev->prop_table[i].key_idx),
                        &cp_errcode, cp_msg, CP_MSG_LEN);
    }

    /* 3. Derivatives — pass 1: CoolProp fetches */
    for (size_t i = 0; i < ev->n_derivs; i++) {
        DerivReq *d = &ev->deriv_table[i];
        double *f = (double *)((char *)ev->deriv_dest + d->offset);
        switch (d->type) {
        case DERIV_PARTIAL:
            *f = cp.asDeriv(handle, K(d->of_idx), K(d->wrt_idx),
                            K(d->constant_idx),
                            &cp_errcode, cp_msg, CP_MSG_LEN);
            break;
        case DERIV_SAT:
            *f = cp.asDeriv2ph(handle, K(d->of_idx), K(d->wrt_idx),
                               &cp_errcode, cp_msg, CP_MSG_LEN);
            break;
        case DERIV_COMPUTED:
            break;
        }
    }

    /* 4. Derivatives — pass 2: computed (product rule) */
    for (size_t i = 0; i < ev->n_derivs; i++) {
        DerivReq *d = &ev->deriv_table[i];
        if (d->type != DERIV_COMPUTED) continue;
        double rho = *(double *)((char *)ev->prop_dest  + d->rho_offset);
        double h   = *(double *)((char *)ev->prop_dest  + d->h_offset);
        double dhP = *(double *)((char *)ev->deriv_dest + d->dh_dP_offset);
        double drP = *(double *)((char *)ev->deriv_dest + d->drho_dP_offset);
        *(double *)((char *)ev->deriv_dest + d->offset) = rho * dhP + h * drP;
    }
}

/* ================================================================
 * System evaluator
 * ================================================================ */

static void syseval_add(SysEvaluator *se, StateEval ev) {
    assert(se->n_evals < MAX_EVALS);
    se->evals[se->n_evals++] = ev;
}

void syseval_exec_all(SysEvaluator *se) {
    for (size_t i = 0; i < se->n_evals; i++)
        state_eval_exec(&se->evals[i], se->handle);
}

void syseval_free_tables(SysEvaluator *se) {
    for (size_t i = 0; i < se->n_evals; i++) {
        free(se->evals[i].prop_table);
        free(se->evals[i].deriv_table);
    }
}

/* ================================================================
 * Build evaluation schedule
 * ================================================================ */

/* Macro to reduce boilerplate for syseval_add */
#define EVAL_PH(lbl, pP, pH, pd, pt, np_, dd, dt, nd_) \
    syseval_add(se, (StateEval){ \
        .update_type = UPDATE_PH, .input_P = pP, .input_second = pH, \
        .prop_dest = pd, .prop_table = pt, .n_props = np_, \
        .deriv_dest = dd, .deriv_table = dt, .n_derivs = nd_, \
        .label = lbl })

#define EVAL_PQ(lbl, pP, pQ, pd, pt, np_, dd, dt, nd_) \
    syseval_add(se, (StateEval){ \
        .update_type = UPDATE_PQ, .input_P = pP, .input_second = pQ, \
        .prop_dest = pd, .prop_table = pt, .n_props = np_, \
        .deriv_dest = dd, .deriv_table = dt, .n_derivs = nd_, \
        .label = lbl })

void build_system(SysEvaluator *se, SysRef *ref, SysInputs *inp) {
    PB pb; DB db;
    size_t np, nd;
    memset(se, 0, sizeof(*se));

    /* ── ACC ─────────────────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_P, offsetof(AccProps, P));
    pb_add(&pb, K_H, offsetof(AccProps, h));
    db_clr(&db);
    db_drho_partials(&db, offsetof(AccDerivs, drho_dP),
                          offsetof(AccDerivs, drho_dh));
    EVAL_PH("acc", &inp->P_acc, &inp->h_acc,
            &ref->acc.props,  pb_fin(&pb, &np), np,
            &ref->acc.derivs, db_fin(&db, &nd), nd);

    /* ── NODE1 ───────────────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_P,   offsetof(Node1Props, P));
    pb_add(&pb, K_H,   offsetof(Node1Props, h));
    pb_add(&pb, K_MU,  offsetof(Node1Props, mu));
    pb_add(&pb, K_RHO, offsetof(Node1Props, rho));
    db_clr(&db);
    db_drho_partials(&db, offsetof(Node1Derivs, drho_dP),
                          offsetof(Node1Derivs, drho_dh));
    EVAL_PH("node1", &inp->P_node1, &inp->h_node1,
            &ref->node1.props,  pb_fin(&pb, &np), np,
            &ref->node1.derivs, db_fin(&db, &nd), nd);

    /* ── NODE2 main ──────────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_T,   offsetof(Node2Props, T));
    pb_add(&pb, K_P,   offsetof(Node2Props, P));
    pb_add(&pb, K_H,   offsetof(Node2Props, h));
    pb_add(&pb, K_CP,  offsetof(Node2Props, cp));
    pb_add(&pb, K_CV,  offsetof(Node2Props, cv));
    pb_add(&pb, K_RHO, offsetof(Node2Props, rho));
    db_clr(&db);
    db_drho_partials(&db, offsetof(Node2Derivs, drho_dP),
                          offsetof(Node2Derivs, drho_dh));
    EVAL_PH("node2", &inp->P_node2, &inp->h_node2,
            &ref->node2.props,  pb_fin(&pb, &np), np,
            &ref->node2.derivs, db_fin(&db, &nd), nd);

    /* NODE2 sat.T */
    pb_clr(&pb);
    pb_add(&pb, K_T, offsetof(Node2Props, sat.T));
    EVAL_PQ("node2 sat", &inp->P_node2, &inp->Q_05,
            &ref->node2.props, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── NODE3 main ──────────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_P,   offsetof(Node3Props, P));
    pb_add(&pb, K_H,   offsetof(Node3Props, h));
    pb_add(&pb, K_T,   offsetof(Node3Props, T));
    pb_add(&pb, K_RHO, offsetof(Node3Props, rho));
    db_clr(&db);
    db_drho_partials(&db, offsetof(Node3Derivs, drho_dP),
                          offsetof(Node3Derivs, drho_dh));
    EVAL_PH("node3", &inp->P_node3, &inp->h_node3,
            &ref->node3.props,  pb_fin(&pb, &np), np,
            &ref->node3.derivs, db_fin(&db, &nd), nd);

    /* NODE3 sat.T */
    pb_clr(&pb);
    pb_add(&pb, K_T, offsetof(Node3Props, sat.T));
    EVAL_PQ("node3 sat", &inp->P_node3, &inp->Q_05,
            &ref->node3.props, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── COND bar1 ───────────────────────────────────────────── */
    pb_clr(&pb);
    pb_bar6(&pb,
        offsetof(CondBarProps, T),   offsetof(CondBarProps, h),
        offsetof(CondBarProps, mu),  offsetof(CondBarProps, rho),
        offsetof(CondBarProps, Pr),  offsetof(CondBarProps, K));
    db_clr(&db);
    db_drho_partials(&db, offsetof(CondBarDerivs, drho_dP),
                          offsetof(CondBarDerivs, drho_dh));
    EVAL_PH("cond bar1", &inp->P_cond, &inp->h_bar1,
            &ref->cond.bar1.props,  pb_fin(&pb, &np), np,
            &ref->cond.bar1.derivs, db_fin(&db, &nd), nd);

    /* ── COND bar3 ───────────────────────────────────────────── */
    pb_clr(&pb);
    pb_bar6(&pb,
        offsetof(CondBarProps, T),   offsetof(CondBarProps, h),
        offsetof(CondBarProps, mu),  offsetof(CondBarProps, rho),
        offsetof(CondBarProps, Pr),  offsetof(CondBarProps, K));
    db_clr(&db);
    db_drho_partials(&db, offsetof(CondBarDerivs, drho_dP),
                          offsetof(CondBarDerivs, drho_dh));
    EVAL_PH("cond bar3", &inp->P_cond, &inp->h_bar3,
            &ref->cond.bar3.props,  pb_fin(&pb, &np), np,
            &ref->cond.bar3.derivs, db_fin(&db, &nd), nd);

    /* ── COND sat.T ──────────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_T, offsetof(CondProps, sat.T));
    EVAL_PQ("cond sat.T", &inp->P_cond, &inp->Q_05,
            &ref->cond, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── COND sat vapor ──────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_H,   offsetof(CondSatPhaseProps, h));
    pb_add(&pb, K_RHO, offsetof(CondSatPhaseProps, rho));
    pb_add(&pb, K_MU,  offsetof(CondSatPhaseProps, mu));
    db_clr(&db);
    db_sat_set(&db,
        offsetof(CondSatPhaseDerivs, dh_dP),
        offsetof(CondSatPhaseDerivs, drho_dP),
        offsetof(CondSatPhaseDerivs, drhoH_dP),
        offsetof(CondSatPhaseProps, rho),
        offsetof(CondSatPhaseProps, h));
    EVAL_PQ("cond sat vap", &inp->P_cond, &inp->Q_1,
            &ref->cond.sat.vap.props,  pb_fin(&pb, &np), np,
            &ref->cond.sat.vap.derivs, db_fin(&db, &nd), nd);

    /* COND sat vap extra (sigma) — redundant update, optimize later */
    pb_clr(&pb);
    pb_add(&pb, K_SIGMA, offsetof(CondSatVapExtra, sigma));
    EVAL_PQ("cond sat vap extra", &inp->P_cond, &inp->Q_1,
            &ref->cond.sat.vap.extra, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── COND sat liquid ─────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_H,   offsetof(CondSatPhaseProps, h));
    pb_add(&pb, K_RHO, offsetof(CondSatPhaseProps, rho));
    pb_add(&pb, K_MU,  offsetof(CondSatPhaseProps, mu));
    db_clr(&db);
    db_sat_set(&db,
        offsetof(CondSatPhaseDerivs, dh_dP),
        offsetof(CondSatPhaseDerivs, drho_dP),
        offsetof(CondSatPhaseDerivs, drhoH_dP),
        offsetof(CondSatPhaseProps, rho),
        offsetof(CondSatPhaseProps, h));
    EVAL_PQ("cond sat liq", &inp->P_cond, &inp->Q_0,
            &ref->cond.sat.liq.props,  pb_fin(&pb, &np), np,
            &ref->cond.sat.liq.derivs, db_fin(&db, &nd), nd);

    /* COND sat liq extra (K, Pr) */
    pb_clr(&pb);
    pb_add(&pb, K_K,  offsetof(CondSatLiqExtra, K));
    pb_add(&pb, K_PR, offsetof(CondSatLiqExtra, Pr));
    EVAL_PQ("cond sat liq extra", &inp->P_cond, &inp->Q_0,
            &ref->cond.sat.liq.extra, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── COND outlet ─────────────────────────────────────────── */
    pb_clr(&pb);
    pb_outlet(&pb);
    EVAL_PH("cond out", &inp->P_cond, &inp->h_out_cond,
            &ref->cond.out, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── EVAP bar2 ───────────────────────────────────────────── */
    pb_clr(&pb);
    pb_bar6(&pb,
        offsetof(EvapBarProps, T),   offsetof(EvapBarProps, h),
        offsetof(EvapBarProps, mu),  offsetof(EvapBarProps, rho),
        offsetof(EvapBarProps, Pr),  offsetof(EvapBarProps, K));
    db_clr(&db);
    db_drho_partials(&db, offsetof(EvapBarDerivs, drho_dP),
                          offsetof(EvapBarDerivs, drho_dh));
    EVAL_PH("evap bar2", &inp->P_evap, &inp->h_bar2,
            &ref->evap.bar2.props,  pb_fin(&pb, &np), np,
            &ref->evap.bar2.derivs, db_fin(&db, &nd), nd);

    /* ── EVAP sat.T ──────────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_T, offsetof(EvapProps, sat.T));
    EVAL_PQ("evap sat.T", &inp->P_evap, &inp->Q_05,
            &ref->evap, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── EVAP sat vapor ──────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_RHO, offsetof(EvapSatPhaseProps, rho));
    pb_add(&pb, K_H,   offsetof(EvapSatPhaseProps, h));
    db_clr(&db);
    db_sat_set(&db,
        offsetof(EvapSatPhaseDerivs, dh_dP),
        offsetof(EvapSatPhaseDerivs, drho_dP),
        offsetof(EvapSatPhaseDerivs, drhoH_dP),
        offsetof(EvapSatPhaseProps, rho),
        offsetof(EvapSatPhaseProps, h));
    EVAL_PQ("evap sat vap", &inp->P_evap, &inp->Q_1,
            &ref->evap.sat.vap.props,  pb_fin(&pb, &np), np,
            &ref->evap.sat.vap.derivs, db_fin(&db, &nd), nd);

    /* ── EVAP sat liquid ─────────────────────────────────────── */
    pb_clr(&pb);
    pb_add(&pb, K_RHO, offsetof(EvapSatPhaseProps, rho));
    pb_add(&pb, K_H,   offsetof(EvapSatPhaseProps, h));
    pb_add(&pb, K_MU,  offsetof(EvapSatPhaseProps, mu));
    db_clr(&db);
    db_sat_set(&db,
        offsetof(EvapSatPhaseDerivs, dh_dP),
        offsetof(EvapSatPhaseDerivs, drho_dP),
        offsetof(EvapSatPhaseDerivs, drhoH_dP),
        offsetof(EvapSatPhaseProps, rho),
        offsetof(EvapSatPhaseProps, h));
    EVAL_PQ("evap sat liq", &inp->P_evap, &inp->Q_0,
            &ref->evap.sat.liq.props,  pb_fin(&pb, &np), np,
            &ref->evap.sat.liq.derivs, db_fin(&db, &nd), nd);

    /* EVAP sat liq extra (Pr, K) */
    pb_clr(&pb);
    pb_add(&pb, K_PR, offsetof(EvapSatLiqExtra, Pr));
    pb_add(&pb, K_K,  offsetof(EvapSatLiqExtra, K));
    EVAL_PQ("evap sat liq extra", &inp->P_evap, &inp->Q_0,
            &ref->evap.sat.liq.extra, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── EVAP outlet ─────────────────────────────────────────── */
    pb_clr(&pb);
    pb_outlet(&pb);
    EVAL_PH("evap out", &inp->P_evap, &inp->h_out_evap,
            &ref->evap.out, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── LCC outlet (WIP) ────────────────────────────────────── */
    pb_clr(&pb);
    pb_outlet(&pb);
    EVAL_PH("lcc out", &inp->P_lcc, &inp->h_out_lcc,
            &ref->lcc.out, pb_fin(&pb, &np), np,
            NULL, NULL, 0);

    /* ── CHILLER outlet (WIP) ────────────────────────────────── */
    pb_clr(&pb);
    pb_outlet(&pb);
    EVAL_PH("chil out", &inp->P_chil, &inp->h_out_chil,
            &ref->chil.out, pb_fin(&pb, &np), np,
            NULL, NULL, 0);
}

/* ================================================================
 * Debug printer
 * ================================================================ */

#define P2(l, v) printf("  %-18s = %12.6f\n", l, v)
#define P9(l, v) printf("  %-18s = %15.9f\n", l, v)

static void hdr(const char *t) {
    printf("\n========================================\n  %s\n"
           "========================================\n", t);
}

void print_sys_ref(const SysRef *r) {
    hdr("ACC");
    P2("P", r->acc.props.P); P2("h", r->acc.props.h);
    P9("drho/dP", r->acc.derivs.drho_dP);
    P9("drho/dh", r->acc.derivs.drho_dh);

    hdr("NODE1");
    P2("P", r->node1.props.P); P2("h", r->node1.props.h);
    P2("mu", r->node1.props.mu); P2("rho", r->node1.props.rho);
    P9("drho/dP", r->node1.derivs.drho_dP);
    P9("drho/dh", r->node1.derivs.drho_dh);

    hdr("NODE2");
    P2("T", r->node2.props.T); P2("P", r->node2.props.P);
    P2("h", r->node2.props.h); P2("cp", r->node2.props.cp);
    P2("cv", r->node2.props.cv); P2("rho", r->node2.props.rho);
    P9("drho/dP", r->node2.derivs.drho_dP);
    P9("drho/dh", r->node2.derivs.drho_dh);
    P2("sat.T", r->node2.props.sat.T);

    hdr("NODE3");
    P2("P", r->node3.props.P); P2("h", r->node3.props.h);
    P2("T", r->node3.props.T); P2("rho", r->node3.props.rho);
    P9("drho/dP", r->node3.derivs.drho_dP);
    P9("drho/dh", r->node3.derivs.drho_dh);
    P2("sat.T", r->node3.props.sat.T);

    hdr("COND bar1");
    P2("T", r->cond.bar1.props.T); P2("h", r->cond.bar1.props.h);
    P2("mu", r->cond.bar1.props.mu); P2("rho", r->cond.bar1.props.rho);
    P2("Pr", r->cond.bar1.props.Pr); P2("K", r->cond.bar1.props.K);
    P9("drho/dP", r->cond.bar1.derivs.drho_dP);
    P9("drho/dh", r->cond.bar1.derivs.drho_dh);

    hdr("COND bar3");
    P2("T", r->cond.bar3.props.T); P2("h", r->cond.bar3.props.h);
    P2("mu", r->cond.bar3.props.mu); P2("rho", r->cond.bar3.props.rho);
    P2("Pr", r->cond.bar3.props.Pr); P2("K", r->cond.bar3.props.K);
    P9("drho/dP", r->cond.bar3.derivs.drho_dP);
    P9("drho/dh", r->cond.bar3.derivs.drho_dh);

    hdr("COND sat");
    P2("T", r->cond.sat.T);
    P2("vap.h", r->cond.sat.vap.props.h);
    P2("vap.rho", r->cond.sat.vap.props.rho);
    P2("vap.mu", r->cond.sat.vap.props.mu);
    P2("vap.sigma", r->cond.sat.vap.extra.sigma);
    P9("vap.dh/dP", r->cond.sat.vap.derivs.dh_dP);
    P9("vap.drho/dP", r->cond.sat.vap.derivs.drho_dP);
    P9("vap.d(rhoH)/dP", r->cond.sat.vap.derivs.drhoH_dP);
    P2("liq.h", r->cond.sat.liq.props.h);
    P2("liq.rho", r->cond.sat.liq.props.rho);
    P2("liq.mu", r->cond.sat.liq.props.mu);
    P2("liq.K", r->cond.sat.liq.extra.K);
    P2("liq.Pr", r->cond.sat.liq.extra.Pr);
    P9("liq.dh/dP", r->cond.sat.liq.derivs.dh_dP);
    P9("liq.drho/dP", r->cond.sat.liq.derivs.drho_dP);
    P9("liq.d(rhoH)/dP", r->cond.sat.liq.derivs.drhoH_dP);

    hdr("COND out");
    P2("h", r->cond.out.h); P2("mu", r->cond.out.mu);
    P2("rho", r->cond.out.rho);

    hdr("EVAP bar2");
    P2("T", r->evap.bar2.props.T); P2("h", r->evap.bar2.props.h);
    P2("mu", r->evap.bar2.props.mu); P2("rho", r->evap.bar2.props.rho);
    P2("Pr", r->evap.bar2.props.Pr); P2("K", r->evap.bar2.props.K);
    P9("drho/dP", r->evap.bar2.derivs.drho_dP);
    P9("drho/dh", r->evap.bar2.derivs.drho_dh);

    hdr("EVAP sat");
    P2("T", r->evap.sat.T);
    P2("vap.rho", r->evap.sat.vap.props.rho);
    P2("vap.h", r->evap.sat.vap.props.h);
    P9("vap.d(rhoH)/dP", r->evap.sat.vap.derivs.drhoH_dP);
    P9("vap.drho/dP", r->evap.sat.vap.derivs.drho_dP);
    P9("vap.dh/dP", r->evap.sat.vap.derivs.dh_dP);
    P2("liq.rho", r->evap.sat.liq.props.rho);
    P2("liq.h", r->evap.sat.liq.props.h);
    P2("liq.mu", r->evap.sat.liq.props.mu);
    P9("liq.d(rhoH)/dP", r->evap.sat.liq.derivs.drhoH_dP);
    P9("liq.drho/dP", r->evap.sat.liq.derivs.drho_dP);
    P2("liq.Pr", r->evap.sat.liq.extra.Pr);
    P2("liq.K", r->evap.sat.liq.extra.K);

    hdr("EVAP out");
    P2("h", r->evap.out.h); P2("mu", r->evap.out.mu);
    P2("rho", r->evap.out.rho);

    hdr("LCC out");
    P2("h", r->lcc.out.h); P2("mu", r->lcc.out.mu);
    P2("rho", r->lcc.out.rho);

    hdr("CHIL out");
    P2("h", r->chil.out.h); P2("mu", r->chil.out.mu);
    P2("rho", r->chil.out.rho);
}
