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

local cinterpcall = ujit.debug.cinterpcall
assert(type(cinterpcall) == "function")

cinterpcall(loop01, 0)
cinterpcall(loop02, 0)
cinterpcall(loop03, 0)
cinterpcall(loop04, 0)
cinterpcall(loop05, 0)
cinterpcall(loop06, 0)
cinterpcall(loop07, 0)

print("Canary alive")
