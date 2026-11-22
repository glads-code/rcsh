# Compile rcsh.exe (32 bits) sous Windows avec le tcc i386-win32 (voir
# README) : rcsh et la bibliothèque de 9pm liés en un seul exécutable,
# sans 9pm2.dll. Même liste de fichiers que compiler.rc.
# Usage : powershell -ExecutionPolicy Bypass -File compiler.ps1 [dossier de tcc]
param([string]$tcc = 'C:\tcc')
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$cc = Join-Path $tcc 'i386-win32-tcc.exe'
if (-not (Test-Path $cc)) { throw "tcc i386-win32 absent : $cc (voir README)" }
$o = 'obj'
if (Test-Path $o) { Remove-Item -Recurse $o }
New-Item -ItemType Directory $o | Out-Null
function cc($src, $obj) {
	& $cc "-B$tcc" "-I$tcc\include" '-D__int64=long long' -Iinclude -Itcc -w -c $src -o $obj
	if ($LASTEXITCODE) { throw "échec : $src" }
}
$w32 = 'thread lock qlock queue main fd env time proc sec crt dll frexp tas' -split ' '
$port = ('access doprint print malloc event qsort atexit convM2D convD2M ' +
	'werrstr dirfstat dirwstat dirstat dirread perror dirmodeconv mktemp ' +
	'readn abs asin atan atof atol charstod cleanname ctime ctype exp fabs ' +
	'floor fmod getfields log memccpy memchr memcmp memcpy nan pow pow10 ' +
	'rand ref rune sin sinh sqrt strcat strncat strchr strcmp strcpy strdup ' +
	'strlen strncmp strecpy strncpy strpbrk strrchr strtod strtol strtoul ' +
	'tan tanh tokenize toupper utflen utfrrune utfrune utftowstr utfutf ' +
	'wstrcat wstrchr wstrcmp wstrcpy wstrdup wstrecpy wstrlen wstrncat ' +
	'wstrncmp wstrncpy wstrrchr wstrtoutf wstrutflen memmove memset strstr') -split ' '
foreach ($f in $w32) { cc "src\9pm\win32\$f.c" "$o\$f.o" }
foreach ($f in $port) { cc "src\9pm\port\$f.c" "$o\$f.o" }
cc 'tcc\win.c' "$o\win.o"
foreach ($f in Get-ChildItem src\rcsh\*.c) { cc $f.FullName "$o\r-$($f.BaseName).o" }
& $cc "-B$tcc" -nostdlib '-Wl,-entry=mainCRTStartup' -Ltcc -o rcsh.exe (Get-ChildItem "$o\*.o").FullName -lkernel32 -luser32 -ladvapi32 -lgdi32 -lmpr "$tcc\lib\i386-win32-libtcc1.a"
if ($LASTEXITCODE) { throw 'échec de l''édition de liens' }
Remove-Item -Recurse $o
'compilé : rcsh.exe'
