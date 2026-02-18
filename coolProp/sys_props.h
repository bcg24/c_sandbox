/*
 * sys_props.h — System-level refrigerant property management
 */
#ifndef SYS_PROPS_H
#define SYS_PROPS_H

#include <stddef.h>

#define CP_MSG_LEN 512

/* ── CoolProp function pointer typedefs (match DLL C API) ────── */

typedef long   (*AbstractState_factory_func)(const char*, const char*, long*, char*, long);
typedef void   (*AbstractState_free_func)(long, long*, char*, long);
typedef void   (*AbstractState_update_func)(long, long, double, double, long*, char*, long);
typedef double (*AbstractState_keyed_output_func)(long, long, long*, char*, long);
typedef double (*AbstractState_first_partial_deriv_func)(long, long, long, long, long*, char*, long);
typedef double (*AbstractState_first_two_phase_deriv_func)(long, long, long, long*, char*, long);
typedef long   (*get_param_index_func)(const char*);
typedef long   (*get_input_pair_index_func)(const char*);

typedef struct CoolProp {
    AbstractState_factory_func               asFactory;
    AbstractState_free_func                  asFree;
    AbstractState_update_func                asUpdate;
    AbstractState_keyed_output_func          asKeyed;
    AbstractState_first_partial_deriv_func   asDeriv;
    AbstractState_first_two_phase_deriv_func asDeriv2ph;
    get_param_index_func                     getParamIndex;
    get_input_pair_index_func                getInputPairIndex;
} CoolProp;

extern CoolProp cp;
extern long     cp_errcode;
extern char     cp_msg[CP_MSG_LEN];

/* ── DLL symbol loader ───────────────────────────────────────── */

typedef void (*generic_func_ptr)(void);

typedef struct {
    const char       *name;
    generic_func_ptr *slot;
} Sym;

/* ── Parameter key registry ──────────────────────────────────── */

typedef struct { const char *name; long key; } ParamKey;

enum KeyIdx {
    K_T = 0, K_P, K_H, K_RHO, K_CP, K_CV,
    K_MU, K_K, K_PR, K_S, K_Q, K_SIGMA,
    N_KEYS
};

extern ParamKey KEYS[N_KEYS];
extern long IP_PQ, IP_HmP;

static inline long K(enum KeyIdx i) { return KEYS[i].key; }
int resolve_keys(void);

/* ── Property structs (per-component, bespoke) ───────────────── */

typedef struct { double P, h; }             AccProps;
typedef struct { double drho_dP, drho_dh; } AccDerivs;

typedef struct { double P, h, mu, rho; }    Node1Props;
typedef struct { double drho_dP, drho_dh; } Node1Derivs;

typedef struct { double T, P, h, cp, cv, rho; struct { double T; } sat; } Node2Props;
typedef struct { double drho_dP, drho_dh; } Node2Derivs;

typedef struct { double P, h, T, rho; struct { double T; } sat; } Node3Props;
typedef struct { double drho_dP, drho_dh; } Node3Derivs;

typedef struct { double h, mu, rho; } OutletProps;

typedef struct { double T, h, mu, rho, Pr, K; }     CondBarProps;
typedef struct { double drho_dP, drho_dh; }          CondBarDerivs;
typedef struct { double h, rho, mu; }                CondSatPhaseProps;
typedef struct { double sigma; }                      CondSatVapExtra;
typedef struct { double K, Pr; }                      CondSatLiqExtra;
typedef struct { double dh_dP, drho_dP, drhoH_dP; } CondSatPhaseDerivs;

typedef struct {
    struct { CondBarProps props; CondBarDerivs derivs; } bar1;
    struct { CondBarProps props; CondBarDerivs derivs; } bar3;
    struct {
        double T;
        struct { CondSatPhaseProps props; CondSatVapExtra extra; CondSatPhaseDerivs derivs; } vap;
        struct { CondSatPhaseProps props; CondSatLiqExtra extra; CondSatPhaseDerivs derivs; } liq;
    } sat;
    OutletProps out;
} CondProps;

typedef struct { double T, h, mu, rho, Pr, K; }         EvapBarProps;
typedef struct { double drho_dP, drho_dh; }              EvapBarDerivs;
typedef struct { double rho, h, mu; }                    EvapSatPhaseProps;
typedef struct { double Pr, K; }                          EvapSatLiqExtra;
typedef struct { double drhoH_dP, drho_dP, dh_dP; }     EvapSatPhaseDerivs;

typedef struct {
    struct { EvapBarProps props; EvapBarDerivs derivs; } bar2;
    struct {
        double T;
        struct { EvapSatPhaseProps props; EvapSatPhaseDerivs derivs; } vap;
        struct { EvapSatPhaseProps props; EvapSatLiqExtra extra; EvapSatPhaseDerivs derivs; } liq;
    } sat;
    OutletProps out;
} EvapProps;

typedef struct {
    struct { AccProps   props; AccDerivs   derivs; } acc;
    struct { Node1Props props; Node1Derivs derivs; } node1;
    struct { Node2Props props; Node2Derivs derivs; } node2;
    struct { Node3Props props; Node3Derivs derivs; } node3;
    CondProps cond;
    EvapProps evap;
    struct { OutletProps out; } lcc;
    struct { OutletProps out; } chil;
} SysRef;

/* ── Evaluation machinery ────────────────────────────────────── */

typedef struct { enum KeyIdx key_idx; size_t offset; } PropReq;

typedef enum { DERIV_PARTIAL, DERIV_SAT, DERIV_COMPUTED } DerivType;

typedef struct {
    DerivType   type;
    enum KeyIdx of_idx, wrt_idx, constant_idx;
    size_t      offset;
    size_t      rho_offset, h_offset;           /* in prop_dest */
    size_t      dh_dP_offset, drho_dP_offset;   /* in deriv_dest */
} DerivReq;

typedef enum { UPDATE_PH, UPDATE_PQ } UpdateType;

typedef struct {
    UpdateType  update_type;
    double     *input_P, *input_second;
    void       *prop_dest;   PropReq  *prop_table;  size_t n_props;
    void       *deriv_dest;  DerivReq *deriv_table;  size_t n_derivs;
    const char *label;
} StateEval;

#define MAX_EVALS 32

typedef struct {
    StateEval evals[MAX_EVALS];
    size_t    n_evals;
    long      handle;
} SysEvaluator;

typedef struct {
    double P_acc,   h_acc;
    double P_node1, h_node1;
    double P_node2, h_node2;
    double P_node3, h_node3;
    double P_cond,  h_bar1, h_bar3, h_out_cond;
    double P_evap,  h_bar2, h_out_evap;
    double P_lcc,   h_out_lcc;
    double P_chil,  h_out_chil;
    double Q_0, Q_05, Q_1;
} SysInputs;

/* ── Public API ──────────────────────────────────────────────── */

void load_stubs(void);
void build_system(SysEvaluator *se, SysRef *ref, SysInputs *inp);
void syseval_exec_all(SysEvaluator *se);
void syseval_free_tables(SysEvaluator *se);
void print_sys_ref(const SysRef *ref);

#endif
