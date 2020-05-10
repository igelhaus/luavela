/*
 * uJIT C interpreter.
 * Copyright (C) 2015-2019 IPONWEB Ltd. See Copyright Notice in COPYRIGHT
 */

#ifndef _UJ_VM_H
#define _UJ_VM_H

struct lua_State;

int uj_vm_call(lua_State *L);

#endif /* !_UJ_VM_H */
