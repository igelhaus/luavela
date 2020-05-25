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
#include "uj_func.h"
#include "uj_lib.h"
#include "uj_meta.h"
#include "uj_upval.h"

#define vm_assert(x) ((x)? (void)0 : abort())

/* FIXME: Align with x2 API from lj_bc.h */

#define vm_raw_ra(ins) (((ins) >> 8) & 0xff)
#define vm_raw_rb(ins) ((ins) >> 24)
#define vm_raw_rc(ins) (((ins) >> 16) & 0xff)
#define vm_raw_rd(ins) (((ins) >> 16))
/* XXX: Jump targets are not x2-encoded, use the raw bc register value */
#define vm_rj(ins) ((ptrdiff_t)vm_raw_rd(ins) - BCBIAS_J)

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
#define vm_base_upval(base, uvindex) (base2func(base).l.uvptr[uvindex])
#define pc2proto(pc) ((const GCproto *)(pc) - 1)

struct vm_frame {
	lua_State *L;
	uint64_t nres1;
	uint64_t multres1;
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
static void *uj_BC_UGET(HANDLER_SIGNATURE);
static void *uj_BC_USETV(HANDLER_SIGNATURE);
static void *uj_BC_USETS(HANDLER_SIGNATURE);
static void *uj_BC_USETN(HANDLER_SIGNATURE);
static void *uj_BC_USETP(HANDLER_SIGNATURE);
static void *uj_BC_UCLO(HANDLER_SIGNATURE);
static void *uj_BC_FNEW(HANDLER_SIGNATURE);
static void *uj_BC_GGET(HANDLER_SIGNATURE);
static void *uj_BC_CALL(HANDLER_SIGNATURE);
static void *uj_BC_CALLT(HANDLER_SIGNATURE);
static void *uj_BC_VARG(HANDLER_SIGNATURE);
static void *uj_BC_RET0(HANDLER_SIGNATURE);
static void *uj_BC_RET1(HANDLER_SIGNATURE);
static void *uj_BC_IFUNCF(HANDLER_SIGNATURE);
static void *uj_BC_IFUNCV(HANDLER_SIGNATURE);
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
	uj_BC_UGET, /* 0x21 UGET */
	uj_BC_USETV, /* 0x22 USETV */
	uj_BC_USETS, /* 0x23 USETS */
	uj_BC_USETN, /* 0x24 USETN */
	uj_BC_USETP, /* 0x25 USETP */
	uj_BC_UCLO, /* 0x26 UCLO */
	uj_BC_FNEW, /* 0x27 FNEW */
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
	uj_BC_CALLT, /* 0x36 CALLT */
	uj_BC_NYI, /* 0x37 ITERC */
	uj_BC_NYI, /* 0x38 ITERN */
	uj_BC_VARG, /* 0x39 VARG */
	uj_BC_NYI, /* 0x3a ISNEXT */
	uj_BC_NYI, /* 0x3b RETM */
	uj_BC_NYI, /* 0x3c RET */
	uj_BC_RET0, /* 0x3d RET0 */
	uj_BC_RET1, /* 0x3e RET1 */
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
	uj_BC_IFUNCV, /* 0x53 FUNCV */ /* FIXME */
	uj_BC_IFUNCV, /* 0x54 IFUNCV */
	uj_BC_NYI, /* 0x55 JFUNCV */
	uj_BC_FUNCC, /* 0x56 FUNCC */
	uj_BC_NYI, /* 0x57 FUNCCW */
};

static LJ_AINLINE uint8_t vm_min_u8(uint8_t x, uint8_t y)
{
	return y ^ ((x ^ y) & -(x < y));
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

int uj_vm_call(lua_State *L)
{
	const GCfunc *fn;
	struct vm_frame vmf = {0};
	uint8_t nargs1 = (uint8_t)(L->top - L->base);

	fn = uj_lib_checkfunc(L, 1);
	vm_assert(isluafunc(fn));

	vmf.L = L;

	L->base->fr.tp.ftsz = (int64_t)(FRAME_C | sizeof(*(L->base)));

	vm_next_call(proto_bc(funcproto(fn)), L->base + 1, &vmf, nargs1 << 16);

	return (int)(L->top - L->base);
}

static void *vm_return(HANDLER_SIGNATURE)
{
	uint8_t nres1 = vm_raw_rd(ins);
	lua_State *L = vmf->L;

	vm_copy_slots(base - 1, vm_slot_ra(base, ins), nres1 - 1);

	if (vmf->nres1 == 0) {
		/* 0 == LUA_MULTRET + 1 */

		/* Set stack top: */
		L->top = base + nres1 - 2;
	} else if (vmf->nres1 > nres1) {
		/* NYI: More results wanted. Check stack size and fill up results with nil. */
		vm_assert(0);
	}

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

static void *uj_BC_UGET(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	GCupval *uv = vm_base_upval(base, vm_raw_rd(ins));

	*dst = *uvval(uv);

	DISPATCH();
}

static void *uj_BC_USETV(HANDLER_SIGNATURE)
{
	GCupval *uv = vm_base_upval(base, vm_raw_ra(ins));
	TValue *src = vm_slot_rd(base, ins);

	*uvval(uv) = *src;
	if (LJ_UNLIKELY(isblack(obj2gco(uv)) && !uv->closed &&
			tvisgcv(src) && !iswhite(gcval(src))))
	{
		/*
		 * XXX: no L->base and L->top sync is necessary
		 * since lj_gc_barrieruv doesn't affect Lua stack.
		 */
		lj_gc_barrieruv(G(vmf->L), uvval(uv));
	}

	DISPATCH();
}

static void *uj_BC_USETS(HANDLER_SIGNATURE)
{
	GCupval *uv = vm_base_upval(base, vm_raw_ra(ins));
	GCstr *str = (GCstr *)vm_kbase_gco(vmf->kbase, vm_raw_rd(ins));

	setstrV(vmf->L, uvval(uv), str);
	if (LJ_UNLIKELY(isblack(obj2gco(uv)) && !uv->closed &&
			!iswhite(obj2gco(str))))
	{
		/*
		 * XXX: no L->base and L->top sync is necessary
		 * since lj_gc_barrieruv doesn't affect Lua stack.
		 */
		lj_gc_barrieruv(G(vmf->L), uvval(uv));
	}

	DISPATCH();
}

static void *uj_BC_USETN(HANDLER_SIGNATURE)
{
	GCupval *uv = vm_base_upval(base, vm_raw_ra(ins));
	TValue *src = vm_slot_rd(vmf->kbase, ins);

	setnumV(uvval(uv), numV(src));

	DISPATCH();
}

static void *uj_BC_USETP(HANDLER_SIGNATURE)
{
	GCupval *uv = vm_base_upval(base, vm_raw_ra(ins));
	uint32_t tag = ~vm_raw_rd(ins);

	settag(uvval(uv), tag);

	DISPATCH();
}

static void *uj_BC_UCLO(HANDLER_SIGNATURE)
{
	TValue *level = vm_slot_ra(base, ins);

	pc += vm_rj(ins);

	if (LJ_LIKELY(vmf->L->openupval))
	{
		/*
		 * XXX: no L->base and L->top sync is necessary
		 * since uj_upval_close doesn't affect Lua stack.
		 */
		uj_upval_close(vmf->L, level);
	}

	DISPATCH();
}

static void *uj_BC_FNEW(HANDLER_SIGNATURE)
{
	TValue *dst = vm_slot_ra(base, ins);
	GCproto* pt = (GCproto *)vm_kbase_gco(vmf->kbase, vm_raw_rd(ins));
	GCfuncL *parent = &base2func(base).l;

	/*
	 * XXX: L->base sync is necessary since FNEW uses it for
	 * upvalues initialization. L->top sync is made underneath
	 * via lj_gc_check_fixtop routine.
	 */
	vmf->L->base = base;

	setfuncV(vmf->L, dst, uj_func_newL_gc(vmf->L, pt, parent));
	/*
	 * XXX: New Lua function creation doesn't trigger guest
	 * stack reallocation, so base pointer stays valid.
	 */

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

static void *uj_BC_CALLT(HANDLER_SIGNATURE)
{
	TValue *slot_ra = vm_slot_ra(base, ins);
	GCfunc *fn = (GCfunc *)slot_ra->gcr;
	uint8_t nargs1 = vm_raw_rd(ins);

	/* NYI: set_vmstate INTERP */

	if (LJ_UNLIKELY(!tvisfunc(slot_ra)))
		vm_assert(0); /* NYI: metacall */

	pc = (base - 1)->fr.tp.pcr; /* restore_PC */

	if ((((uintptr_t)pc) & FRAME_TYPE)) {
		vm_assert(0); /* NYI: Tailcall from vararg function */
	}

	vm_assert(!isffunc(fn)); /* NYI: Tailcall into a fast function */

	vm_copy_slots(base, slot_ra + 1, nargs1 - 1);
	(base - 1)->gcr = (GCobj *)fn;
	pc = fn->l.pc;

	DISPATCH_CALL();
}

static void *uj_BC_VARG(HANDLER_SIGNATURE)
{
	TValue *slot_ra = vm_slot_ra(base, ins);
	TValue *base_varg = (TValue *)((char *)base + FRAME_VARG - (char *)((base - 1)->fr.tp.ftsz)) + vm_raw_rc(ins);
	uint8_t nres1 = vm_raw_rb(ins);
	uint8_t nvargs1 = base - base_varg;

	if (nres1 > 0) {
		uint8_t n = vm_min_u8(nvargs1, nres1) - 1;

		vm_copy_slots(slot_ra, base_varg, n);
		vm_nil_slots(slot_ra + n, nres1 - n - 1);
	} else {
		/* NYI: uj_state_stack_grow */

		vmf->multres1 = nvargs1;
		vm_copy_slots(slot_ra, base_varg, nvargs1 - 1);
	}

	DISPATCH();
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
	for (ptrdiff_t i = vm_raw_rd(ins) - 1; i < pc2proto(pc - 1)->numparams; i++)
		setnilV(base + i);

	DISPATCH();
}

static void *uj_BC_IFUNCV(HANDLER_SIGNATURE)
{
	uint8_t nargs1 = vm_raw_rd(ins);
	TValue *base_new = base + nargs1;
	uint8_t exp_nargs = pc2proto(pc - 1)->numparams;
	uint8_t n;

	(base_new - 1)->u64 = (base - 1)->u64;
	(base_new - 1)->u64_hi = nargs1 * sizeof(TValue) + FRAME_VARG;

	/* NYI: set_vmstate_lfunc */
	/* NYI: grow stack */

	n = vm_min_u8(nargs1 - 1, exp_nargs);
	vm_copy_slots(base_new, base, n);
	vm_nil_slots(base_new + n, exp_nargs - n);
	vm_nil_slots(base, n);

	base = base_new;

	/* Setup kbase: */
	vmf->kbase = pc2proto(((base - 1)->fr.func)->fn.l.pc)->k;

	DISPATCH();
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

restore_PC:
	pc = (base - 1)->fr.tp.pcr; /* restore_PC */
	vmf->multres1 = vm_raw_rd(ins);

	if ((((uintptr_t)pc) & FRAME_TYPE)) {
		/* Not returning to a fixarg Lua func? */
		if ((((uintptr_t)pc - FRAME_VARG) & FRAME_TYPEP))
			return vm_return(HANDLER_ARGUMENTS);

		base = (TValue *)((char *)base - ((uintptr_t)pc - FRAME_VARG));
		goto restore_PC;
	}

	/* Clear missing return values. */
	vm_nil_slots(base - 1, vm_raw_rb(*(pc - 1)) - 1);

	base -= (int8_t)((*((uint8_t *)pc - 3) >> 1) + 1); /* restore_base */
	vmf->kbase = pc2proto(((base - 1)->fr.func)->fn.l.pc)->k; /* setup_kbase */
	/* NYI: set_vmstate_lfunc */
	/* NYI: checktimeout */

	DISPATCH();
}

/* RET1 rbase lit */
static void *uj_BC_RET1(HANDLER_SIGNATURE)
{
	/* NYI: set_vmstate INTERP */
	TValue *slot_ra = vm_slot_ra(base, ins);

restore_PC:
	pc = (base - 1)->fr.tp.pcr; /* restore_PC */
	vmf->multres1 = vm_raw_rd(ins);

	if ((((uintptr_t)pc) & FRAME_TYPE)) {
		/* Not returning to a fixarg Lua func? */
		if ((((uintptr_t)pc - FRAME_VARG) & FRAME_TYPEP))
			return vm_return(HANDLER_ARGUMENTS);

		base = (TValue *)((char *)base - ((uintptr_t)pc - FRAME_VARG));
		slot_ra = (TValue *)((char *)slot_ra + ((uintptr_t)pc - FRAME_VARG));
		goto restore_PC;
	}

	*(base - 1) = *slot_ra;

	/* Clear missing return values. */
	vm_nil_slots(base, vm_raw_rb(*(pc - 1)) - 2);

	base -= (int8_t)((*((uint8_t *)pc - 3) >> 1) + 1); /* restore_base */
	vmf->kbase = pc2proto(((base - 1)->fr.func)->fn.l.pc)->k; /* setup_kbase */
	/* NYI: set_vmstate_lfunc */
	/* NYI: checktimeout */

	DISPATCH();
}
