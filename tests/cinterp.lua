-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   1
-- 0004    KNUM     2   0      ; 10000000
-- 0005    KSHORT   3   1
-- 0006    FORI     1 => 0010
-- 0007    ADD      0   0   4
-- 0008    HOTCNT
-- 0009    FORL     1 => 0007
-- 0010    GGET     1   0      ; "print"
-- 0011    MOV      2   0
-- 0012    CALL     1   1   2
-- 0013    RET0     0   1

local loop01 = function()
	local s = 0
	for i = 1, 1e7 do
		s = s + i
	end
	-- print(s) -- FIXME: NYI
end


-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   1
-- 0004    KNUM     2   0      ; 10000000
-- 0005    KSHORT   3   1
-- 0006    FORI     1 => 0010
-- 0007    SUB      0   0   4
-- 0008    HOTCNT
-- 0009    FORL     1 => 0007
-- 0010    RET0     0   1

local loop02 = function()
	local s = 0
	for i = 1, 1e7 do
		s = s - i
	end
end


-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   1
-- 0004    KNUM     2   0      ; 10000000
-- 0005    KSHORT   3   1
-- 0006    FORI     1 => 0012
-- 0007    KSHORT   5   2
-- 0008    MUL      5   4   5
-- 0009    ADD      0   0   5
-- 0010    HOTCNT
-- 0011    FORL     1 => 0007
-- 0012    RET0     0   1

local loop03 = function()
	local s = 0
	for i = 1, 1e7 do
		s = s + i * 2
	end
end


-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   1
-- 0004    KNUM     2   0      ; 10000000
-- 0005    KSHORT   3   1
-- 0006    FORI     1 => 0012
-- 0007    KSHORT   5   2
-- 0008    DIV      5   4   5
-- 0009    ADD      0   0   5
-- 0010    HOTCNT
-- 0011    FORL     1 => 0007
-- 0012    RET0     0   1

local loop04 = function()
	local s = 0
	for i = 1, 1e7 do
		s = s + i / 2
	end
end


-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   1
-- 0004    KNUM     2   0      ; 10000000
-- 0005    KSHORT   3   1
-- 0006    FORI     1 => 0012
-- 0007    KSHORT   5   2
-- 0008    MOD      5   4   5
-- 0009    ADD      0   0   5
-- 0010    HOTCNT
-- 0011    FORL     1 => 0007
-- 0012    RET0     0   1

local loop05 = function()
	local s = 0
	for i = 1, 1e7 do
		s = s + i % 2
	end
end


-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   1
-- 0004    KNUM     2   0      ; 10000000
-- 0005    KSHORT   3   1
-- 0006    FORI     1 => 0012
-- 0007    KSHORT   5   2
-- 0008    POW      5   4   5
-- 0009    ADD      0   0   5
-- 0010    HOTCNT
-- 0011    FORL     1 => 0007
-- 0012    RET0     0   1

local loop06 = function()
	local s = 0
	for i = 1, 1e7 do
		s = s + i ^ 2
	end
end


-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   1
-- 0004    KNUM     2   0      ; 10000000
-- 0005    KSHORT   3   1
-- 0006    FORI     1 => 0010
-- 0007    UNM      0   4
-- 0008    HOTCNT
-- 0009    FORL     1 => 0007
-- 0010    RET0     0   1

local loop07 = function()
	local s = 0
	for i = 1, 1e7 do
		s = -i
	end
end


-- 0001    HOTCNT
-- 0002    KSTR     0   0      ; "imun"
-- 0003    KSHORT   1   0
-- 0004    KNUM     2   0      ; 3.1415926
-- 0005    KPRI     3   0
-- 0006    KPRI     4   1
-- 0007    KPRI     5   2
-- 0008    KNIL     6   8
-- 0009    RET0     0   1

local ktest = function()
	local kstr = 'imun'
	local kshort = 0
	local knum = 3.1415926
	local knil, ktrue, kfalse, knil1, knil2, knil3 = nil, false, true
end


-- 0001    HOTCNT
-- 0002    KPRI     0   0
-- 0003    NOT      1   0
-- 0004    NOT      2   1
-- 0005    KSTR     3   0      ; "imun"
-- 0006    LEN      3   3
-- 0007    GGET     4   1      ; "_G"
-- 0008    LEN      4   4
-- 0009    KSTR     5   2      ; "c"
-- 0010    KSTR     6   3      ; "a"
-- 0011    KSTR     7   4      ; "t"
-- 0012    CAT      5   5   7
-- 0013    RET0     0   1

local misctest = function()
	local notobj = nil
	local notfalse = not notobj
	local nottrue = not notfalse
	local strlen = #'imun'
	-- FIXME: we can't create a table within cinterpcall and can't load
	-- an upvalue for now. We even can't spoil _G a bit to change its
	-- length for test! OK, bet everything on zero.
	local tablen = #_G
	local cat = 'c' .. 'a' ..'t'
end

-- 0001    HOTCNT
-- 0002    GGET     0   0      ; "print"
-- 0003    KSTR     1   1      ; "Canary alive?"
-- 0004    CALL     0   1   2
-- 0005    RET0     0   1

local cfunc = function ()
	print("Canary alive?")
end

-- 0001    HOTCNT
-- 0002    GGET     2   0      ; "print"
-- 0003    KSTR     3   1      ; "x = "
-- 0004    MOV      4   0
-- 0005    KSTR     5   2      ; "y = "
-- 0006    MOV      6   1
-- 0007    CALL     2   1   5
-- 0008    RET0     0   1

function globalfn01(x, y) -- Please keep global
	print("x = ", x, "y = ", y)
end

-- 0001    HOTCNT
-- 0002    GGET     1   0      ; "globalfn01"
-- 0003    MOV      2   0
-- 0004    KSTR     3   1      ; "bar"
-- 0005    CALLT    1   3

function globalfn02(x) -- Please keep global
	return globalfn01(x, "bar")
end

-- 0001    HOTCNT
-- 0002    GGET     0   0      ; "globalfn02"
-- 0003    KSTR     1   1      ; "foo"
-- 0004    CALL     0   1   2
-- 0005    RET0     0   1

local taillcall01 = function ()
	globalfn02("foo")
end

-- 0001    HOTCNT
-- 0002    RET1     0   2

function globalfn03(x) -- Please keep global
	return x
end

-- 0001    HOTCNT
-- 0002    GGET     3   0      ; "globalfn03"
-- 0003    MOV      4   1
-- 0004    CALL     3   4   2
-- 0005    GGET     6   1      ; "print"
-- 0006    MOV      7   3
-- 0007    MOV      8   4
-- 0008    MOV      9   5
-- 0009    CALL     6   1   4
-- 0010    RET0     0   1

local ret1 = function(x, y, z)
	local a, b, c = globalfn03(y)
	print(a, b, c)
end


-- 0001    HOTCNT
-- 0002    KPRI     0   0
-- 0003    FNEW     1   0      ; tests/cinterp.lua:276
-- 0004    MOV      2   1
-- 0005    KSHORT   3   0
-- 0006    CALL     2   1   2
-- 0007    UCLO     0 => 0008
-- 0008    RET0     0   1

local utest = function()
	local upval = nil

	-- 0001    HOTCNT
	-- 0002    UGET     1   0      ; upval
	-- 0003    USETV    0   0      ; upval
	-- 0004    USETS    0   0      ; upval ; "imun"
	-- 0005    USETN    0   0      ; upval ; 9
	-- 0006    USETP    0   0      ; upval
	-- 0007    RET0     0   1
	local fnew = function(arg)
		local lcval = upval
		upval = arg
		upval = 'imun'
		upval = 9
		upval = nil
	end
	fnew(0)
end

local cinterpcall = ujit.debug.cinterpcall
assert(type(cinterpcall) == "function")

cinterpcall(loop01)
cinterpcall(loop02)
cinterpcall(loop03)
cinterpcall(loop04)
cinterpcall(loop05)
cinterpcall(loop06)
cinterpcall(loop07)
cinterpcall(ktest)
cinterpcall(misctest)
cinterpcall(cfunc)
cinterpcall(taillcall01)
cinterpcall(ret1, "FAIL1", "OK", "FAIL2")
cinterpcall(utest)

local rv
rv = cinterpcall(function(x, y) return y .. " world!" end,
		 "Goodbye, cruel", "Hello,")
assert(rv == "Hello, world!")

local t = {}
rv = cinterpcall(function(x, ...) -- <-- IFUNCV
	local a, b, c = ...       -- <-- VARG
	print(x, a, b, c)
	return b
end, "Hello from", "var", t, "arg")
assert(rv == t)

print("Canary alive!")

if (pcall(require, 'ffi')) then
	loadstring([[
		-- 0001    HOTCNT
		-- 0002    KCDATA   0   0
		-- 0003    RET0     0   1
		local kcdata_test = function()
			local kcdata = -1ULL
		end

		ujit.debug.cinterpcall(kcdata_test)

		print("Canary alive! (FFI)")
	]])()
end
