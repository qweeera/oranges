;%define	_BOOT_DEBUG_  ;做Boot Sector时要注释掉  将此行打开是用来生成.com文件

%ifdef	_BOOT_DEBUG_
	org	0100h
%else
	org	07c00h		;Boot状态 bios将把boot sector加载到0:07c00处开始执行
%endif

;===========================================================================
%ifdef _BOOT_DEBUG_
BaseOfStack	equ	0100h;
%else
BaseOfStack	equ	07c00h;堆栈基地址 栈底  从这个位置向低地址生长
%endif

BaseOfLoader		equ	09000h	; LOADER.BIN 被加载到的位置 ----  段地址
OffsetOfLoader		equ	0100h	; LOADER.BIN 被加载到的位置 ---- 偏移地址


;===========================================================================
	jmp	short	LABEL_START
	nop

	%include "fat12hdr.inc"

LABEL_START:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, BaseOfStack   ;堆栈地址

	; 清屏
	mov	ax, 0600h		; AH = 6,  AL = 0h
	mov	bx, 0700h		; 黑底白字(BH = 07h)
	mov	cx, 0			; 左上角: (0, 0)
	mov	dx, 0184fh		; 右下角: (80, 50)
	int	10h			; int 10h
	; 打印booting 字符
	mov	dh, 0			; "Booting  "
	call	DispStr			; 显示字符串


	xor	ah, ah	; 
	xor	dl, dl	;  
	int	13h	; 复位软驱
	
; 下面在 A 盘的根目录寻找 LOADER.BIN
	mov	word [wSectorNo], SectorNoOfRootDirectory ;根目录第一个扇区号
LABEL_SEARCH_IN_ROOT_DIR_BEGIN:
	cmp	word [wRootDirSizeForLoop], 0	; 判断根目录区是不是已经读完
	jz	LABEL_NO_LOADERBIN		; 如果读完表示没有找到 LOADER.BIN
	dec	word [wRootDirSizeForLoop]	; 
	mov	ax, BaseOfLoader
	mov	es, ax			; es <- BaseOfLoader
	mov	bx, OffsetOfLoader	; bx <- OffsetOfLoader      es:bx -> 09100h
	mov	ax, [wSectorNo]		; ax <- Root Directory 中的某 Sector 号
	mov	cl, 1			;读一个扇区
	call	ReadSector

	mov	si, LoaderFileName	; ds:si -> "LOADER  BIN"
	mov	di, OffsetOfLoader	; es:di -> BaseOfLoader:0100
	cld
	mov	dx, 10h     ;每个条目占32字节  那么一个扇区只有16个条目
LABEL_SEARCH_FOR_LOADERBIN:
	cmp	dx, 0				   ;  循环次数控制,
	jz	LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR ;   如果已经读完了一个 Sector,
	dec	dx				   ;   就跳到下一个 Sector
	mov	cx, 11				   ;loader  bin总共有11个字符
LABEL_CMP_FILENAME:
	cmp	cx, 0
	jz	LABEL_FILENAME_FOUND	; 如果比较了 11 个字符都相等, 表示找到
	dec	cx
	lodsb				; ds:si -> al
	cmp	al, byte [es:di]
	jz	LABEL_GO_ON
	jmp	LABEL_DIFFERENT		; 只要发现不一样的字符就表明本 DirectoryEntry
					; 不是我们要找的 LOADER.BIN
LABEL_GO_ON:
	inc	di
	jmp	LABEL_CMP_FILENAME	; 继续循环

LABEL_DIFFERENT:
	and	di, 0FFE0h		;  di &= E0 为了让它指向本条目开头
	add	di, 20h			;  每个条目占32个字节  这是到达下一个条目
	mov	si, LoaderFileName	;  di += 20h  下一个目录条目
	jmp	LABEL_SEARCH_FOR_LOADERBIN; 

LABEL_GOTO_NEXT_SECTOR_IN_ROOT_DIR:
	add	word [wSectorNo], 1
	jmp	LABEL_SEARCH_IN_ROOT_DIR_BEGIN

LABEL_NO_LOADERBIN:
	mov	dh, 2			; "No LOADER."
	call	DispStr			; 显示字符串
%ifdef	_BOOT_DEBUG_
	mov	ax, 4c00h		; 
	int	21h			;   没有找到 LOADER.BIN, 回到 DOS
%else
	jmp	$			; 没有找到 LOADER.BIN, 死循环在这里
%endif

LABEL_FILENAME_FOUND:			; 找到 LOADER.BIN 后便来到这里继续
	mov	ax, RootDirSectors	; 根目录占用的空间
	and	di, 0FFE0h		; di 指向当前目录的首地址
	add	di, 01Ah		; +26个字节指向条目中的开始扇区
	mov	cx, word [es:di]	; 获得开始扇区的值
	push	cx			;保存该值 （FAT中的序号）
	add	cx, ax			;
	add	cx, DeltaSectorNo	;该簇号 + 根目录占用扇区数+19-2
					; 减2是因为是从第二个簇开始的
	mov	ax, BaseOfLoader
	mov	es, ax			; es <- BaseOfLoader
	mov	bx, OffsetOfLoader	; bx <- OffsetOfLoader
	mov	ax, cx			; ax <- 扇区号

LABEL_GOON_LOADING_FILE:
	push	ax			;
	push	bx			;
	mov	ah, 0Eh			;
	mov	al, '.'			;
	mov	bl, 0Fh			;
	int	10h
	pop	bx
	pop	ax			;运行之后会显示一个点 int10的作用？？

	mov	cl, 1
	call	ReadSector		;读一个扇区 通过ax中的扇区号读取

	pop 	ax			;取出此sector在fat中的序号（簇号）
	call	GetFATEntry		;通过簇号查找到下一个扇区
	cmp	ax, 0FFFh		;到达文件尾
	jz	LABEL_FILE_LOADED	;
	
	push	ax			;保存sector在FAT中的序号
	mov	dx, RootDirSectors	;根目录占扇区数
	add	ax, dx
	add	ax, DeltaSectorNo	; 序号+根目录占扇区数+19-2=真正的扇区号
	add	bx, [BPB_BytsPerSec]    ;每个扇区所占字节数  bx是内存偏移量
	jmp	LABEL_GOON_LOADING_FILE

LABEL_FILE_LOADED:
	mov	dh, 1
	call	DispStr			;显示字符串  ready
	jmp	BaseOfLoader:OffsetOfLoader  ;跳转到loader.bin处开始运行



wRootDirSizeForLoop	dw	RootDirSectors	; Root Directory 占用的扇区数，
						; 在循环中会递减至零.
wSectorNo		dw	0		; 要读取的扇区号
bOdd			db	0		; 奇数还是偶数

;字符串
LoaderFileName		db	"LOADER  BIN", 0 ; LOADER.BIN 之文件名
; 为简化代码, 下面每个字符串的长度均为 MessageLength
MessageLength		equ	9
BootMessage:		db	"Booting  " ; 9字节, 不够则用空格补齐. 序号 0
Message1		db	"Ready.   " ; 9字节, 不够则用空格补齐. 序号 1
Message2		db	"No LOADER" ; 9字节, 不够则用空格补齐. 序号 2
;============================================================================


;----------------------------------------------------------------------------
; 函数名: DispStr
;----------------------------------------------------------------------------
; 作用:
;	显示一个字符串, 函数开始时 dh 中应该是字符串序号(0-based)
DispStr:
	mov	ax, MessageLength
	mul	dh
	add	ax, BootMessage
	mov	bp, ax			; `.
	mov	ax, ds			;  | ES:BP = 串地址
	mov	es, ax			; /
	mov	cx, MessageLength	; CX = 串长度
	mov	ax, 01301h		; AH = 13,  AL = 01h
	mov	bx, 0007h		; 页号为0(BH = 0) 黑底白字(BL = 07h)
	mov	dl, 0
	int	10h			; int 10h
	ret

;----------------------------------------------------------------------------
; 函数名: ReadSector
;----------------------------------------------------------------------------
; 作用:
;	从第 ax 个 Sector 开始, 将 cl 个 Sector 读入 es:bx 中
ReadSector:
	; ----------------------------------------------------------------------
	; 怎样由扇区号求扇区在磁盘中的位置 (扇区号 -> 柱面号, 起始扇区, 磁头号)
	; ----------------------------------------------------------------------
	; 设扇区号为 x
	;                          ┌ 柱面号 = y >> 1
	;       x           ┌ 商 y ┤
	; -------------- => ┤      └ 磁头号 = y & 1
	;  每磁道扇区数     │
	;                   └ 余 z => 起始扇区号 = z + 1
	push	bp     ;将原来的bp保存
	mov	bp, sp   
	sub	esp, 2 ; 辟出两个字节的堆栈区域保存要读的扇区数: byte [bp-2]

	mov	byte [bp-2], cl   ;要读的扇区有多少个 cl
	push	bx			; 保存 bx
	mov	bl, [BPB_SecPerTrk]	; bl: 除数  每磁道扇区数   被除数是ax  
	div	bl			; y 在 al 中, z 在 ah 中
	inc	ah			; z ++
	mov	cl, ah			; cl <- 起始扇区号
	mov	dh, al			; dh <- y
	shr	al, 1			; y >> 1 (y/BPB_NumHeads)
	mov	ch, al			; ch <- 柱面号
	and	dh, 1			; dh & 1 = 磁头号
	pop	bx			; 恢复 bx
	; 至此, "柱面号, 起始扇区, 磁头号" 全部得到
	mov	dl, [BS_DrvNum]		; 驱动器号 (0 表示 A 盘)
.GoOnReading:
	mov	ah, 2			; 读
	mov	al, byte [bp-2]		; 读 al 个扇区
	int	13h
	jc	.GoOnReading		; 如果读取错误 CF 会被置为 1, 
					; 这时就不停地读, 直到正确为止
	add	esp, 2
	pop	bp

	ret


;----------------------------------------------------------------------------
; 函数名: GetFATEntry
;----------------------------------------------------------------------------
; 作用:
;	找到序号为 ax 的 Sector 在 FAT 中的条目, 结果放在 ax 中
;	需要注意的是, 中间需要读 FAT 的扇区到 es:bx 处, 所以函数一开始保存了 es 和 bx
GetFATEntry:
	push	es
	push	bx
	push	ax
	mov	ax, BaseOfLoader; `.
	sub	ax, 0100h	;  | 在 BaseOfLoader 后面留出 4K 空间用于存放 FAT
	mov	es, ax		; /
	pop	ax
	mov	byte [bOdd], 0
	mov	bx, 3
	mul	bx			; dx:ax = ax * 3
	mov	bx, 2
	div	bx			; dx:ax / 2  ==>  ax <- 商, dx <- 余数
	cmp	dx, 0
	jz	LABEL_EVEN
	mov	byte [bOdd], 1
LABEL_EVEN:;偶数
	; 现在 ax 中是 FATEntry 在 FAT 中的偏移量,下面来
	; 计算 FATEntry 在哪个扇区中(FAT占用不止一个扇区)
	xor	dx, dx			
	mov	bx, [BPB_BytsPerSec]
	div	bx ; dx:ax / BPB_BytsPerSec
		   ;  ax <- 商 (FATEntry 所在的扇区相对于 FAT 的扇区号)
		   ;  dx <- 余数 (FATEntry 在扇区内的偏移)。
	push	dx
	mov	bx, 0 ; bx <- 0 于是, es:bx = (BaseOfLoader - 100):00
	add	ax, SectorNoOfFAT1 ; 此句之后的 ax 就是 FATEntry 所在的扇区号
	mov	cl, 2
	call	ReadSector ; 读取 FATEntry 所在的扇区, 一次读两个, 避免在边界
			   ; 发生错误, 因为一个 FATEntry 可能跨越两个扇区
	pop	dx
	add	bx, dx
	mov	ax, [es:bx]
	cmp	byte [bOdd], 1
	jnz	LABEL_EVEN_2
	shr	ax, 4
LABEL_EVEN_2:
	and	ax, 0FFFh

LABEL_GET_FAT_ENRY_OK:

	pop	bx
	pop	es
	ret
;--------------------------------------------------------------------------




times 	510-($-$$)	db	0	; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55				; 结束标志
