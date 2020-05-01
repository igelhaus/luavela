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
-- 0003    KCDATA   1   1
-- 0004    KSHORT   2   0
-- 0005    KNUM     3   0      ; 3.1415926
-- 0006    KPRI     4   0
-- 0007    KPRI     5   1
-- 0008    KPRI     6   2
-- 0009    KNIL     7   9
-- 0010    RET0     0   1

local ktest = function()
	local kstr = 'imun'
	local kcdata = -1ULL
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

local cfunc = function ()
	print("Canary alive?")
end

local cinterpcall = ujit.debug.cinterpcall
assert(type(cinterpcall) == "function")

cinterpcall(loop01, 0)
cinterpcall(loop02, 0)
cinterpcall(loop03, 0)
cinterpcall(loop04, 0)
cinterpcall(loop05, 0)
cinterpcall(loop06, 0)
cinterpcall(loop07, 0)
cinterpcall(ktest, 0)
cinterpcall(misctest, 0)
cinterpcall(cfunc, 0)

print("Canary alive!")
