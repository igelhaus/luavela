/*
 * Bytecode interpreter implemented in C.
 * Copyright (C) 2015-2019 IPONWEB Ltd. See Copyright Notice in COPYRIGHT
 */

#ifndef NDEBUG
#define NDEBUG
#endif

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "lj_bc.h"
#include "lj_bcins.h"
#include "lj_frame.h"
#include "lj_gc.h"
#include "lj_obj.h"
#include "lj_tab.h"
#include "uj_lib.h"
#include "uj_meta.h"

#define vm_assert(x) ((x)? (void)0 : abort())

/* FIXME: Align with x2 API from lj_bc.h */

#define vm_raw_ra(ins) (((ins) >> 8) & 0xff)
#define vm_raw_rb(ins) ((ins) >> 24)
#define vm_raw_rc(ins) (((ins) >> 16) & 0xff)
#define vm_raw_rd(ins) (((ins) >> 16))

#define vm_index_ra(ins) (vm_raw_ra(ins) >> 1)

#define vm_slot_ra(base, ins) \
	(TValue *)((char *)(base) + (ptrdiff_t)vm_raw_ra(ins) * sizeof(TValue) / 2)
#define vm_slot_rb(base, ins) \
	(TValue *)((char *)(base) + (ptrdiff_t)vm_raw_rb(ins) * sizeof(TValue) / 2)
#define vm_slot_rc(base, ins) \
	(TValue *)((char *)(base) + (ptrdiff_t)vm_raw_rc(ins) * sizeof(TValue) / 2)
#define vm_slot_rd(base, ins) \
	(TValue *)((char *)(base) + (ptrdiff_t)vm_raw_rd(ins) * sizeof(TValue) / 2)
#define vm_kbase_gco(kbase, kindex) \
	((GCobj *)*((GCobj **)(kbase) + (int16_t)~(uint16_t)(kindex)))

/* FIXME: Apply base2func everywhere */
#define base2func(base) ((((base) - 1)->fr.func)->fn)
#define pc2proto(pc) ((const GCproto *)(pc) - 1)

struct vm_frame {
	lua_State *L;
	int64_t nargs;
	int64_t nres1;
	void *kbase;
};

/* NB! For development purposes only: */

typedef void *(* bc_handler)(BCIns *, TValue *, struct vm_frame *, BCIns);

#define HANDLER_SIGNATURE \
	BCIns *pc, TValue *base, struct vm_frame *vmf, BCIns ins

#define HANDLER_ARGUMENTS pc, base, vmf, ins

#define vm_next(pc, base, vmf) \
	(dispatch[bc_op(*(pc))])((pc) + 1, (base), (vmf), *(pc))

/* NB! For development purposes only: */
#define DISPATCH() \
	return vm_next(pc, base, vmf)

/* These ones piggyback nargs1 from CALL* to the prologue */

#define vm_next_call(pc, base, vmf, ins) \
	(dispatch[bc_op(*(pc))])((pc) + 1, (base), (vmf), *(pc) | ((ins) & 0x00ff0000))

#define DISPATCH_CALL() \
	return vm_next_call(pc, base, vmf, ins)

static void *uj_BC_NYI(HANDLER_SIGNATURE);
static void *uj_BC_MOV(HANDLER_SIGNATURE);
static void *uj_BC_NOT(HANDLER_SIGNATURE);
static void *uj_BC_UNM(HANDLER_SIGNATURE);
static void *uj_BC_LEN(HANDLER_SIGNATURE);
static void *uj_BC_ADD(HANDLER_SIGNATURE);
static void *uj_BC_SUB(HANDLER_SIGNATURE);
static void *uj_BC_MUL(HANDLER_SIGNATURE);
static void *uj_BC_DIV(HANDLER_SIGNATURE);
static void *uj_BC_MOD(HANDLER_SIGNATURE);
static void *uj_BC_POW(HANDLER_SIGNATURE);
static void *uj_BC_CAT(HANDLER_SIGNATURE);
static void *uj_BC_KSTR(HANDLER_SIGNATURE);
static void *uj_BC_KCDATA(HANDLER_SIGNATURE);
static void *uj_BC_KSHORT(HANDLER_SIGNATURE);
static void *uj_BC_KNUM(HANDLER_SIGNATURE);
static void *uj_BC_KPRI(HANDLER_SIGNATURE);
static void *uj_BC_KNIL(HANDLER_SIGNATURE);
static void *uj_BC_GGET(HANDLER_SIGNATURE);
static void *uj_BC_CALL(HANDLER_SIGNATURE);
static void *uj_BC_RET0(HANDLER_SIGNATURE);
static void *uj_BC_IFUNCF(HANDLER_SIGNATURE);
static void *uj_BC_FUNCC(HANDLER_SIGNATURE);
static void *uj_BC_HOTCNT(HANDLER_SIGNATURE);
static void *uj_BC_FORI(HANDLER_SIGNATURE);
static void *uj_BC_FORL(HANDLER_SIGNATURE);

static const bc_handler dispatch[] = {
	uj_BC_NYI, /* 0x00 ISLT */
	uj_BC_NYI, /* 0x01 ISGE */
	uj_BC_NYI, /* 0x02 ISLE */
	uj_BC_NYI, /* 0x03 ISGT */
	uj_BC_NYI, /* 0x04 ISEQV */
	uj_BC_NYI, /* 0x05 ISNEV */
	uj_BC_NYI, /* 0x06 ISEQS */
	uj_BC_NYI, /* 0x07 ISNES */
	uj_BC_NYI, /* 0x08 ISEQN */
	uj_BC_NYI, /* 0x09 ISNEN */
	uj_BC_NYI, /* 0x0a ISEQP */
	uj_BC_NYI, /* 0x0b ISNEP */
	uj_BC_NYI, /* 0x0c ISTC */
	uj_BC_NYI, /* 0x0d ISFC */
	uj_BC_NYI, /* 0x0e IST */
	uj_BC_NYI, /* 0x0f ISF */
	uj_BC_MOV, /* 0x10 MOV */
	uj_BC_NOT, /* 0x11 NOT */
	uj_BC_UNM, /* 0x12 UNM */
	uj_BC_LEN, /* 0x13 LEN */
	uj_BC_ADD, /* 0x14 ADD */
	uj_BC_SUB, /* 0x15 SUB */
	uj_BC_MUL, /* 0x16 MUL */
	uj_BC_DIV, /* 0x17 DIV */
	uj_BC_MOD, /* 0x18 MOD */
	uj_BC_POW, /* 0x19 POW */
	uj_BC_CAT, /* 0x1a CAT */
	uj_BC_KSTR, /* 0x1b KSTR */
	uj_BC_KCDATA, /* 0x1c KCDATA */
	uj_BC_KSHORT, /* 0x1d KSHORT */
	uj_BC_KNUM, /* 0x1e KNUM */
	uj_BC_KPRI, /* 0x1f KPRI */
	uj_BC_KNIL, /* 0x20 KNIL */
	uj_BC_NYI, /* 0x21 UGET */
	uj_BC_NYI, /* 0x22 USETV */
	uj_BC_NYI, /* 0x23 USETS */
	uj_BC_NYI, /* 0x24 USETN */
	uj_BC_NYI, /* 0x25 USETP */
	uj_BC_NYI, /* 0x26 UCLO */
	uj_BC_NYI, /* 0x27 FNEW */
	uj_BC_NYI, /* 0x28 TNEW */
	uj_BC_NYI, /* 0x29 TDUP */
	uj_BC_GGET, /* 0x2a GGET */
	uj_BC_NYI, /* 0x2b GSET */
	uj_BC_NYI, /* 0x2c TGETV */
	uj_BC_NYI, /* 0x2d TGETS */
	uj_BC_NYI, /* 0x2e TGETB */
	uj_BC_NYI, /* 0x2f TSETV */
	uj_BC_NYI, /* 0x30 TSETS */
	uj_BC_NYI, /* 0x31 TSETB */
	uj_BC_NYI, /* 0x32 TSETM */
	uj_BC_NYI, /* 0x33 CALLM */
	uj_BC_CALL, /* 0x34 CALL */
	uj_BC_NYI, /* 0x35 CALLMT */
	uj_BC_NYI, /* 0x36 CALLT */
	uj_BC_NYI, /* 0x37 ITERC */
	uj_BC_NYI, /* 0x38 ITERN */
	uj_BC_NYI, /* 0x39 VARG */
	uj_BC_NYI, /* 0x3a ISNEXT */
	uj_BC_NYI, /* 0x3b RETM */
	uj_BC_NYI, /* 0x3c RET */
	uj_BC_RET0, /* 0x3d RET0 */
	uj_BC_NYI, /* 0x3e RET1 */
	uj_BC_HOTCNT, /* 0x3f HOTCNT */
	uj_BC_NYI, /* 0x40 COVERG */
	uj_BC_FORI, /* 0x41 FORI */
	uj_BC_NYI, /* 0x42 JFORI */
	uj_BC_FORL, /* 0x43 FORL */
	uj_BC_FORL, /* 0x44 IFORL */ /* FIXME */
	uj_BC_FORL, /* 0x45 JFORL */ /* FIXME */
	uj_BC_NYI, /* 0x46 ITERL */
	uj_BC_NYI, /* 0x47 IITERL */
	uj_BC_NYI, /* 0x48 JITERL */
	uj_BC_NYI, /* 0x49 ITRNL */
	uj_BC_NYI, /* 0x4a IITRNL */
	uj_BC_NYI, /* 0x4b JITRNL */
	uj_BC_NYI, /* 0x4c LOOP */
	uj_BC_NYI, /* 0x4d ILOOP */
	uj_BC_NYI, /* 0x4e JLOOP */
	uj_BC_NYI, /* 0x4f JMP */
	uj_BC_IFUNCF, /* 0x50 FUNCF */ /* FIXME */
	uj_BC_IFUNCF, /* 0x51 IFUNCF */
	uj_BC_NYI, /* 0x52 JFUNCF */
	uj_BC_NYI, /* 0x53 FUNCV */
	uj_BC_NYI, /* 0x54 IFUNCV */
	uj_BC_NYI, /* 0x55 JFUNCV */
	uj_BC_FUNCC, /* 0x56 FUNCC */
	uj_BC_NYI, /* 0x57 FUNCCW */
};

void uj_vm_call(lua_State *L, int nargs, int nres)
{
	const GCfunc *fn;
	const GCproto *pt;
	struct vm_frame vmf = {0};

	fn = uj_lib_checkfunc(L, 1);

	vm_assert(isluafunc(fn));
	pt = funcproto(fn);

	vmf.L = L;
	vmf.nargs = nargs;
	vmf.nres1 = nres + 1;
	/* NB! kbase will be set up in the prologue */

	L->base->fr.tp.ftsz = (int64_t)(FRAME_C | sizeof(TValue));

	vm_next(proto_bc(pt), L->base + 1, &vmf);
}

static void *vm_return(HANDLER_SIGNATURE)
{
	int8_t ra = vm_index_ra(ins);
	int8_t rd = vm_raw_rd(ins);
	lua_State *L = vmf->L;

	for (ptrdiff_t i = -1; i < (ptrdiff_t)rd - 2; i++)
		*(base + i) = *(base + i + ra + 1);

	if (vmf->nres1 == 0 || vmf->nres1 > rd) { /* 0 == LUA_MULTRET + 1 */
		/* NYI: More results wanted. Check stack size and fill up results with nil. */
		vm_assert(0);
	}

	/* Set stack top: */
	L->top = base + vmf->nres1 - 2;

	/* Set stack base: */
	L->base = (TValue *)((char *)base - ((uint64_t)(uintptr_t)pc & (uint64_t)(-8)));

	/* NYI: Restore L->cframe */
	/* NYI: Set exit code */
	/* NYI: set_vmstate INTERP */
	/* NYI: restore_vmstate */

	return NULL;
}

static void *uj_BC_NYI(HANDLER_SIGNATURE)
{
	UNUSED(ins);
	UNUSED(pc);
	UNUSED(base);
	UNUSED(vmf);

	vm_assert(0);
	return NULL;
}

static void *uj_BC_MOV(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *src = vm_slot_rd(base, ins);

	*dst = *src;

	DISPATCH();
}

static void *uj_BC_NOT(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rd(base, ins);

	setboolV(dst, !tvistruecond(op1));

	DISPATCH();
}

static void *uj_BC_UNM(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rd(base, ins);

	if (LJ_UNLIKELY(!tvisnum(op1))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	setnumV(dst, -numV(op1));

	DISPATCH();
}

static void *uj_BC_LEN(HANDLER_SIGNATURE)
{
UJ_PEDANTIC_OFF

	static void *objlen[] = {
		[~LJ_TSTR] = &&gcstrlen,
		[~LJ_TTAB] = &&gctablen,
	};

	/*
	 * XXX Fragile: Since metacall is not yet implemented we
	 * are free to obtain guest slots pointer at the begining.
	 * However metamethod can trigger guest stack reallocation
	 * that invalidates dst and obj values.
	 */
	TValue *dst = vm_slot_ra(base, ins);
	TValue *obj = vm_slot_rd(base, ins);


	if (LJ_UNLIKELY(!tvisstr(obj) && !tvistab(obj))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	goto *objlen[~gettag(obj)];

gcstrlen:
	setnumV(dst, strV(obj)->len);
	DISPATCH();

gctablen:
	setnumV(dst, lj_tab_len(tabV(obj)));
	DISPATCH();

UJ_PEDANTIC_ON
}

static void *uj_BC_ADD(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rb(base, ins);
	TValue *op2 = vm_slot_rc(base, ins);

	if (LJ_UNLIKELY(!tvisnum(op1) || !tvisnum(op2))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	setnumV(dst, numV(op1) + numV(op2));
	DISPATCH();
}

static void *uj_BC_SUB(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rb(base, ins);
	TValue *op2 = vm_slot_rc(base, ins);

	if (LJ_UNLIKELY(!tvisnum(op1) || !tvisnum(op2))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	setnumV(dst, numV(op1) - numV(op2));
	DISPATCH();
}

static void *uj_BC_MUL(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rb(base, ins);
	TValue *op2 = vm_slot_rc(base, ins);

	if (LJ_UNLIKELY(!tvisnum(op1) || !tvisnum(op2))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	setnumV(dst, numV(op1) * numV(op2));
	DISPATCH();
}

static void *uj_BC_DIV(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rb(base, ins);
	TValue *op2 = vm_slot_rc(base, ins);

	if (LJ_UNLIKELY(!tvisnum(op1) || !tvisnum(op2))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	setnumV(dst, numV(op1) / numV(op2));
	DISPATCH();
}

static LJ_AINLINE double uj_vm_mod(double a, double b)
{
	/* according to Lua Reference */
	return a - floor(a / b) * b;
}

static void *uj_BC_MOD(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rb(base, ins);
	TValue *op2 = vm_slot_rc(base, ins);

	if (LJ_UNLIKELY(!tvisnum(op1) || !tvisnum(op2))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	setnumV(dst, uj_vm_mod(numV(op1), numV(op2)));
	DISPATCH();
}

static LJ_AINLINE double uj_vm_pow(double x, double exp)
{
	return pow(x, exp);
}

static void *uj_BC_POW(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *op1 = vm_slot_rb(base, ins);
	TValue *op2 = vm_slot_rc(base, ins);

	if (LJ_UNLIKELY(!tvisnum(op1) || !tvisnum(op2))) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	setnumV(dst, uj_vm_pow(numV(op1), numV(op2)));
	DISPATCH();
}

static void *uj_BC_CAT(HANDLER_SIGNATURE)
{
	/*
	 * XXX Fragile: Since metacall is not yet implemented we
	 * are free to obtain guest slots pointer at the begining.
	 * However metamethod can trigger guest stack reallocation
	 * that invalidates dst and obj values.
	 */
	TValue *dst = vm_slot_ra(base, ins);
	TValue *bottom = vm_slot_rb(base, ins);
	TValue *top = vm_slot_rc(base, ins);

	if (uj_meta_cat(vmf->L, bottom, top)) {
		vm_assert(0); /* FIXME: Implement metacall */
	}

	lj_gc_check_fixtop(vmf->L);

	*dst = *bottom;

	DISPATCH();
}

static void *uj_BC_KSTR(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	GCstr *str = (GCstr *)vm_kbase_gco(vmf->kbase, vm_raw_rd(ins));

	setstrV(vmf->L, dst, str);

	DISPATCH();
}

static void *uj_BC_KCDATA(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	GCcdata *cdata = (GCcdata *)vm_kbase_gco(vmf->kbase, vm_raw_rd(ins));

	setcdataV(vmf->L, dst, cdata);

	DISPATCH();
}

static void *uj_BC_KSHORT(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	int16_t i16 = (int16_t)vm_raw_rd(ins);

	setnumV(dst, (double)i16);

	DISPATCH();
}

static void *uj_BC_KNUM(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	TValue *src = vm_slot_rd(vmf->kbase, ins);

	*dst = *src;

	DISPATCH();
}

static void *uj_BC_KPRI(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	uint32_t tag = ~vm_raw_rd(ins);

	settag(dst, tag);

	DISPATCH();
}

static void *uj_BC_KNIL(HANDLER_SIGNATURE)
{
	TValue *begin = vm_slot_ra(base, ins);
	TValue *end = vm_slot_rd(base, ins);

	for (; begin <= end; begin++)
		setnilV(begin);

	DISPATCH();
}

static void *uj_BC_GGET(HANDLER_SIGNATURE)
{
	GCtab *tab = base2func(base).l.env;
	GCstr *str = (GCstr *)vm_kbase_gco(vmf->kbase, vm_raw_rd(ins));
	Node *n = &(tab->node[tab->hmask & str->hash]);

	do {
		TValue *dst;

		if (tvisstr(&n->key) && strV(&n->key) == str) {
			if (tvisnil(&n->val))
				goto key_not_found;

			dst = vm_slot_ra(base, ins);
			*dst = n->val;
			DISPATCH();
		}
	} while ((n = n->next));

key_not_found:
	vm_assert(0);
	DISPATCH();
	/* NYI: Implement nil handling, metamethod lookup, etc. */
}

static void *uj_BC_CALL(HANDLER_SIGNATURE)
{
	TValue *fn = vm_slot_ra(base, ins);

	/* NYI: set_vmstate INTERP */

	if (LJ_UNLIKELY(!tvisfunc(fn)))
		vm_assert(0); /* NYI: metacall */

	base = fn + 1;
	fn->fr.tp.pcr = pc;
	pc = ((GCfuncL *)fn->gcr)->pc;

	DISPATCH_CALL();
}

static void *uj_BC_HOTCNT(HANDLER_SIGNATURE)
{
	/* NYI: Payload */
	DISPATCH();
}

static void *uj_BC_FORI(HANDLER_SIGNATURE)
{
UJ_PEDANTIC_OFF

	static void *comparator[] = {
		&&positive_step,
		&&non_positive_step
	};
	TValue *idx;
	lua_Number i, stop;

	/* NYI: checktimeout */

	idx = vm_slot_ra(base, ins);

	if (LJ_UNLIKELY(!tvisnum(idx)))
		vm_assert(0); /* NYI: vmeta_for */

	if (LJ_UNLIKELY(!tvisnum(idx + 1)))
		vm_assert(0); /* NYI: vmeta_for */

	if (LJ_UNLIKELY(!tvisnum(idx + 2)))
		vm_assert(0); /* NYI: vmeta_for */

	i = numV(idx);
	setnumV(idx + 3, i);
	stop = numV(idx + 1);

	goto *comparator[rawV(idx + 2) >> 63];

positive_step:
	if (stop < i)
		pc += (int16_t)vm_raw_rd(ins) - BCBIAS_J;
	DISPATCH();

non_positive_step:
	if (stop >= i)
		pc += (int16_t)vm_raw_rd(ins) - BCBIAS_J;
	DISPATCH();

UJ_PEDANTIC_OFF
}

static void *uj_BC_FORL(HANDLER_SIGNATURE)
{
UJ_PEDANTIC_OFF

	static void *comparator[] = {
		&&positive_step,
		&&non_positive_step
	};
	TValue *idx;
	lua_Number i, stop;

	/* NYI: checktimeout */
	/* NYI: assert_bad_for_arg_type */

	idx = vm_slot_ra(base, ins);

	i = numV(idx);
	stop = numV(idx + 1);

	i += numV(idx + 2);
	setnumV(idx, i);
	setnumV(idx + 3, i);

	goto *comparator[rawV(idx + 2) >> 63];

positive_step:
	if (stop >= i)
		pc += (int16_t)vm_raw_rd(ins) - BCBIAS_J;
	DISPATCH();

non_positive_step:
	if (stop < i)
		pc += (int16_t)vm_raw_rd(ins) - BCBIAS_J;
	DISPATCH();

UJ_PEDANTIC_OFF
}

static void *uj_BC_IFUNCF(HANDLER_SIGNATURE)
{
	TValue *top = vm_slot_ra(base, ins);

	/* Setup kbase: */
	vmf->kbase = pc2proto(((base - 1)->fr.func)->fn.l.pc)->k;

	/* NYI: set_vmstate_lfunc */

	if (top > vmf->L->maxstack)
		; /* NYI: grow stack */

	/* NYI: checktimeout */

	/* Clear missing parameters */
	for (ptrdiff_t i = vmf->nargs; i < pc2proto(pc - 1)->numparams; i++)
		setnilV(base + i);

	DISPATCH();
}

static LJ_AINLINE void vm_copy_slots(TValue *dst, const TValue *src, size_t n)
{
	while (n) {
		*dst++ = *src++;
		n--;
	}
}

static LJ_AINLINE void vm_nil_slots(TValue *dst, size_t n)
{
	while (n)
		setnilV(dst + --n);
}

static void *uj_BC_FUNCC(HANDLER_SIGNATURE)
{
	lua_State *L;
	uint8_t got_nres;
	uint8_t exp_nres;

	/* NYI: set_vmstate_lfunc */
	/* NYI: grow stack */

	L = vmf->L;
	L->base = base;
	L->top = base + vm_raw_rd(ins) - 1;
	got_nres = (uint8_t)(((base - 1)->fr.func)->fn.c.f(L));
	base = L->base;

	/* NYI: Return from C to C */

	pc = (base - 1)->fr.tp.pcr; /* restore_PC */
	exp_nres = *((uint8_t *)pc - 1); /* PC_RB, in fact nres + 1 */

	if (!exp_nres--) {
		vm_assert(0); /* NYI: MULTRES */
	} else {
		if (got_nres)
			vm_copy_slots(base - 1, L->top - got_nres, got_nres);
		if (exp_nres > got_nres)
			vm_nil_slots(base + got_nres, exp_nres - got_nres);
	}

	base -= (int8_t)((*((uint8_t *)pc - 3) >> 1) + 1); /* restore_base */
	vmf->kbase = pc2proto(((base - 1)->fr.func)->fn.l.pc)->k; /* setup_kbase */
	/* NYI: set_vmstate_lfunc */
	/* NYI: checktimeout */

	DISPATCH();
}

/* RET0 rbase lit */
static void *uj_BC_RET0(HANDLER_SIGNATURE)
{
	/* NYI: set_vmstate INTERP */

	pc = (base - 1)->fr.tp.pcr; /* restore_PC */

	if ((((uintptr_t)pc) & FRAME_TYPE)) {
		/* Not returning to a fixarg Lua func? */
		if ((((uintptr_t)pc - FRAME_VARG) & FRAME_TYPEP))
			return vm_return(HANDLER_ARGUMENTS);
		else
			vm_assert(0); /* NYI: Return from vararg function: relocate BASE down and RA up. */
	}

	/* Clear missing return values. */
	for (ptrdiff_t i = -1; i <= (ptrdiff_t)vm_raw_rb(*(pc - 1)) - 2; i++)
		setnilV(base + i);

	base -= (int8_t)((*((uint8_t *)pc - 3) >> 1) + 1); /* restore_base */
	vmf->kbase = pc2proto(((base - 1)->fr.func)->fn.l.pc)->k; /* setup_kbase */
	/* NYI: set_vmstate_lfunc */
	/* NYI: checktimeout */

	DISPATCH();
}
