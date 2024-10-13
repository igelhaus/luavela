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


-- 0001    HOTCNT
-- 0002    KPRI     0   0
-- 0003    ISTC     1   0
-- 0004    JMP      1 => 0006
-- 0005    KSHORT   1   0
-- 0006    ISFC     2   0
-- 0007    JMP      2 => 0009
-- 0008    KSHORT   2   0
-- 0009    IST          0
-- 0010    JMP      3 => 0012
-- 0011    KPRI     0   2
-- 0012    ISF          0
-- 0013    JMP      3 => 0015
-- 0014    KPRI     0   1
-- 0015    GGET     3   0      ; "print"
-- <print test result>
-- 0027    RET0     0   1

local uttest = function()
	local test
	local tcopy = test or  0          -- ISTC
	local fcopy = test and 0          -- ISFC
	if not test then test = true  end -- IST
	if     test then test = false end -- ISF
	print('Unary test and copy:', test or fcopy or tcopy and 'OK' or 'FAIL')
end


-- 0001    HOTCNT
-- 0002    KSHORT   0   0
-- 0003    KSHORT   1   9
-- 0004    UGET     2   0      ; huge
-- 0005    ISLT     2   1
-- 0006    JMP      2 => 0009
-- 0007    KSHORT   2   1
-- 0008    JMP      3 => 0010
-- 0009    KSHORT   2  -1
-- 0010    ADD      0   0   2
-- 0011    KSHORT   2   0
-- 0012    ISLT     2   1
-- 0013    JMP      2 => 0016
-- 0014    KSHORT   2   1
-- 0015    JMP      3 => 0017
-- 0016    KSHORT   2  -1
-- 0017    ADD      0   0   2
-- 0018    UGET     2   0      ; huge
-- 0019    ISGE     1   2
-- 0020    JMP      2 => 0023
-- 0021    KSHORT   2   1
-- 0022    JMP      3 => 0024
-- 0023    KSHORT   2  -1
-- 0024    ADD      0   0   2
-- 0025    KSHORT   2   0
-- 0026    ISGE     1   2
-- 0027    JMP      2 => 0030
-- 0028    KSHORT   2   1
-- 0029    JMP      3 => 0031
-- 0030    KSHORT   2  -1
-- 0031    ADD      0   0   2
-- 0032    KSHORT   2   0
-- 0033    ISLE     1   2
-- 0034    JMP      2 => 0037
-- 0035    KSHORT   2   1
-- 0036    JMP      3 => 0038
-- 0037    KSHORT   2  -1
-- 0038    ADD      0   0   2
-- 0039    UGET     2   0      ; huge
-- 0040    ISLE     1   2
-- 0041    JMP      2 => 0044
-- 0042    KSHORT   2   1
-- 0043    JMP      3 => 0045
-- 0044    KSHORT   2  -1
-- 0045    ADD      0   0   2
-- 0046    KSHORT   2   0
-- 0047    ISGT     2   1
-- 0048    JMP      2 => 0051
-- 0049    KSHORT   2   1
-- 0050    JMP      3 => 0052
-- 0051    KSHORT   2  -1
-- 0052    ADD      0   0   2
-- 0053    UGET     2   0      ; huge
-- 0054    ISGT     2   1
-- 0055    JMP      2 => 0058
-- 0056    KSHORT   2   1
-- 0057    JMP      3 => 0059
-- 0058    KSHORT   2  -1
-- 0059    ADD      0   0   2
-- 0060    KSHORT   2   0
-- 0061    KSHORT   3   0
-- 0062    DIV      2   3   2
-- 0063    KSHORT   3   0
-- 0064    ISLT     3   2
-- 0065    JMP      3 => 0068
-- 0066    KSHORT   3   0
-- 0067    JMP      4 => 0069
-- 0068    KSHORT   3  -1
-- 0069    ADD      0   0   3
-- 0070    KSHORT   3   0
-- 0071    ISGE     2   3
-- 0072    JMP      3 => 0075
-- 0073    KSHORT   3   0
-- 0074    JMP      4 => 0076
-- 0075    KSHORT   3  -1
-- 0076    ADD      0   0   3
-- 0077    KSHORT   3   0
-- 0078    ISLE     2   3
-- 0079    JMP      3 => 0082
-- 0080    KSHORT   3   0
-- 0081    JMP      4 => 0083
-- 0082    KSHORT   3  -1
-- 0083    ADD      0   0   3
-- 0084    KSHORT   3   0
-- 0085    ISGT     3   2
-- 0086    JMP      3 => 0089
-- 0087    KSHORT   3   0
-- 0088    JMP      4 => 0090
-- 0089    KSHORT   3  -1
-- 0090    ADD      0   0   3
-- 0091    KSTR     3   0      ; "QQ"
-- 0092    ISNES    3   0      ; "QQ"
-- 0093    JMP      4 => 0096
-- 0094    KSHORT   4   1
-- 0095    JMP      5 => 0097
-- 0096    KSHORT   4  -1
-- 0097    ADD      0   0   4
-- 0098    ISEQS    3   0      ; "QQ"
-- 0099    JMP      4 => 0102
-- 0100    KSHORT   4   1
-- 0101    JMP      5 => 0103
-- 0102    KSHORT   4  -1
-- 0103    ADD      0   0   4
-- 0104    ISEQS    3   1      ; "QKRQ"
-- 0105    JMP      4 => 0108
-- 0106    KSHORT   4   1
-- 0107    JMP      5 => 0109
-- 0108    KSHORT   4  -1
-- 0109    ADD      0   0   4
-- 0110    ISNES    3   1      ; "QKRQ"
-- 0111    JMP      4 => 0114
-- 0112    KSHORT   4   1
-- 0113    JMP      5 => 0115
-- 0114    KSHORT   4  -1
-- 0115    ADD      0   0   4
-- 0116    GGET     4   2      ; "newproxy"
-- 0117    KPRI     5   2
-- 0118    CALL     4   2   2
-- 0119    ISNES    4   0      ; "QQ"
-- 0120    JMP      5 => 0123
-- 0121    KSHORT   5   0
-- 0122    JMP      6 => 0124
-- 0123    KSHORT   5  -1
-- 0124    ADD      0   0   5
-- 0125    ISEQS    4   0      ; "QQ"
-- 0126    JMP      5 => 0129
-- 0127    KSHORT   5   0
-- 0128    JMP      6 => 0130
-- 0129    KSHORT   5   1
-- 0130    ADD      0   0   5
-- 0131    KSHORT   5   9
-- 0132    ISNEN    5   0      ; 9
-- 0133    JMP      6 => 0136
-- 0134    KSHORT   6   1
-- 0135    JMP      7 => 0137
-- 0136    KSHORT   6  -1
-- 0137    ADD      0   0   6
-- 0138    ISEQN    5   0      ; 9
-- 0139    JMP      6 => 0142
-- 0140    KSHORT   6   1
-- 0141    JMP      7 => 0143
-- 0142    KSHORT   6  -1
-- 0143    ADD      0   0   6
-- 0144    ISEQN    5   1      ; 0
-- 0145    JMP      6 => 0148
-- 0146    KSHORT   6   1
-- 0147    JMP      7 => 0149
-- 0148    KSHORT   6  -1
-- 0149    ADD      0   0   6
-- 0150    ISNEN    5   1      ; 0
-- 0151    JMP      6 => 0154
-- 0152    KSHORT   6   1
-- 0153    JMP      7 => 0155
-- 0154    KSHORT   6  -1
-- 0155    ADD      0   0   6
-- 0156    GGET     6   2      ; "newproxy"
-- 0157    KPRI     7   2
-- 0158    CALL     6   2   2
-- 0159    ISNEN    6   0      ; 9
-- 0160    JMP      7 => 0163
-- 0161    KSHORT   7   0
-- 0162    JMP      8 => 0164
-- 0163    KSHORT   7  -1
-- 0164    ADD      0   0   7
-- 0165    ISEQN    6   0      ; 9
-- 0166    JMP      7 => 0169
-- 0167    KSHORT   7   0
-- 0168    JMP      8 => 0170
-- 0169    KSHORT   7   1
-- 0170    ADD      0   0   7
-- 0171    GGET     7   3      ; "print"
-- <print test result>
-- 0179    RET0     0   1
local huge = math.huge
local cmptest = function()
	local test = 0

	local cmp = 9
	-- ISLT no JMP
	test = test + (not (cmp > huge) and 1 or -1)
	-- ISLT with JMP
	test = test + (not (cmp > 0) and 1 or -1)
	-- ISGE no JMP
	test = test + (cmp < huge and 1 or -1)
	-- ISGE with JMP
	test = test + (cmp < 0 and 1 or -1)
	-- ISLE no JMP
	test = test + (not (cmp <= 0) and 1 or -1)
	-- ISLE with JMP
	test = test + (not (cmp <= huge) and 1 or -1)
	-- ISGT no JMP
	test = test + (cmp >= 0 and 1 or -1)
	-- ISGT with JMP
	test = test + (cmp >= huge and 1 or -1)

	-- XXX: NaN can't be compared to any value, hence no jump
	-- occurs for any relation test bytecode.
	local nan = 0/0
	-- ISLT with NaN
	test = test + (not (nan > 0) and 0 or -1)
	-- ISGE with NaN
	test = test + (nan < 0 and 0 or -1)
	-- ISLE with NaN
	test = test + (not (nan <= 0) and 0 or -1)
	-- ISGT with NaN
	test = test + (nan >= 0 and 0 or -1)

	local str = 'QQ'
	-- ISNES no JMP
	test = test + (str == 'QQ' and 1 or -1)
	-- ISEQS with JMP
	test = test + (str ~= 'QQ' and 1 or -1)
	-- ISEQS no JMP
	test = test + (str ~= 'QKRQ' and 1 or -1)
	-- ISNES with JMP
	test = test + (str == 'QKRQ' and 1 or -1)

	local sud = newproxy(true)
	-- ISNES with JMP (non-string)
	test = test + (sud == 'QQ' and 0 or -1)
	-- ISEQS with JMP (non-string)
	test = test + (sud ~= 'QQ' and 0 or 1)

	local num = 9
	-- ISNEN no JMP
	test = test + (num == 9 and 1 or -1)
	-- ISEQN with JMP
	test = test + (num ~= 9 and 1 or -1)
	-- ISEQN no JMP
	test = test + (num ~= 0 and 1 or -1)
	-- ISNEN with JMP
	test = test + (num == 0 and 1 or -1)

	local nud = newproxy(true)
	-- ISNEN with JMP (non-numeric)
	test = test + (nud == 9 and 0 or -1)
	-- ISEQN with JMP (non-numeric)
	test = test + (nud ~= 9 and 0 or 1)

	print('Comparison ops:', test == 0 and 'OK' or 'FAIL')
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
cinterpcall(uttest)
cinterpcall(cmptest)

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
