# rcsh

Shell rc pour scripts portables sous Windows : `rcsh` de [9pm](https://9p.io/sources/extra/) (Plan 9 pour Windows, 2005), corrigé et recompilé avec tcc. Sources reprises de `9pm051031.zip` (sha256 `9e9281ba379efde3e94666e4358d9959971af9956416d2c858905f9b3e308dd3`), licence LPL 1.02 (`LICENSE.txt`, `NOTICE`).

Le dépôt ne contient que rcsh et sa bibliothèque (`src/9pm`, compilée d'origine en `9pm2.dll`) ; les autres outils de 9pm (`sed`, `grep`, `9sort`…) restent ceux de l'archive.

## Corrections

Le `rcsh.exe` de 9pm rend les scripts dangereux : un échec peut passer inaperçu.

- **Statut des tubes** (`src/9pm/win32/proc.c`) : `proc()` fermait le handle du processus créé et `procwait()` le rouvrait par son numéro. Un enfant déjà terminé (le côté gauche d'un tube, presque toujours) n'existait plus : son échec devenait un succès (`test -f absent | cat` donnait `|`). Les handles sont gardés jusqu'à l'attente ; un processus introuvable compte comme un échec.
- **Fonctions dans les sous-shells** (`src/rcsh/var.c`, `main.c`) : un rcsh enfant (côté gauche d'un tube, capture `` `{…} ``) définissait les fonctions héritées de l'environnement (`fn#nom`) avant d'avoir démarré son interpréteur, et plantait. Dès qu'un script définissait une fonction (courant dans les scripts rc), le côté gauche de ses tubes et ses captures ne s'exécutaient plus. Les fonctions sont maintenant définies juste après le démarrage, comme dans le rc de Plan 9.
- **`errstr` sur une chaîne littérale** (`proc.c`) : écrivait dans une constante, en mémoire en lecture seule avec tcc ; plantait un enfant lancé sans entrée standard (toute capture).

- **Commandes de Windows introuvables** (`src/9pm/win32/env.c`) : quand `9pm` était défini, la bibliothèque ne recopiait pas le `Path` de Windows dans `path`, où rcsh cherche les commandes : seules celles de `$9pm/bin` étaient trouvées (ni `certutil`, ni le `tar` de Windows). Sans `9pm` défini, elle déduit maintenant son dossier de l'exécutable s'il n'y a pas de `9pm2.dll`.
- **Scripts lancés par leur nom** (`proc.c`) : seule l'extension `.rcs` désignait un script rc. Un script `.rc` (`./scripts/essai.rc`) était lancé comme un programme Windows et échouait, un script sans extension n'était pas trouvé. Les deux passent par rcsh, avec un `$0` écrit avec des `/` (`basename -d $0` donne son dossier).
- **Commande introuvable** (`simple.c`) : rcsh affichait l'erreur sans changer le statut ; `flag e` ne s'arrêtait pas et le script continuait comme si de rien n'était.
- **`$0` dans les sous-shells** (`main.c`, `lib/rcmain`) : une capture ou un côté de tube relisait `rcmain`, qui fixait `$0` sur lui-même ; `` `{basename -d $0} `` donnait le dossier de 9pm. Le sous-shell (`-S`) garde le `$0` du script.
- **Redirections** (`exec.c`) : captures, côtés de tubes et sous-shells `@{…}` héritent de l'entrée du script et des redirections en cours (`{ … } > fichier`), comme dans rc ; `` x=`{cat} `` lisait rien, et la sortie d'un `@{…}` échappait au fichier.

`flag e` et le code de sortie (`exit`, échec d'une commande) fonctionnaient déjà.

## Installer sous Windows

rcsh remplace celui de 9pm, à installer d'abord. Dans PowerShell :

```powershell
cd $env:USERPROFILE
# https échoue depuis Windows sur 9p.io (TLS) : http, avec vérification de l'empreinte
curl.exe -LO http://9p.io/sources/extra/9pm051031.zip
(Get-FileHash 9pm051031.zip).Hash   # 9E9281BA379EFDE3E94666E4358D9959971AF9956416D2C858905F9B3E308DD3
tar -xf 9pm051031.zip -C C:\        # donne C:\9pm
$p = [Environment]::GetEnvironmentVariable('Path', 'User')
[Environment]::SetEnvironmentVariable('Path', "C:\9pm\bin;$p", 'User')
# conseillé : sinon tar et sort sont ceux de 9pm et masquent ceux de Windows
Rename-Item C:\9pm\bin\tar.exe 9tar.exe
Copy-Item C:\9pm\bin\9sort.exe C:\9pm\bin\sort.exe
```

Puis compiler `rcsh.exe` (32 bits, bibliothèque de 9pm liée dedans, sans `9pm2.dll`) avec tcc (voir ci-dessous pour l'installer) et l'installer :

```powershell
curl.exe -Lo rcsh.zip https://github.com/glads-code/rcsh/archive/refs/heads/main.zip
tar -xf rcsh.zip
cd rcsh-main
powershell -ExecutionPolicy Bypass -File compiler.ps1 C:\tcc-dev
Move-Item -Force rcsh.exe C:\9pm\bin\rcsh.exe    # les rcsh enfants sont lancés depuis C:\9pm\bin
Copy-Item -Force lib\rcmain C:\9pm\lib\rcmain
```

Vérifier, dans une nouvelle fenêtre (pour le `PATH`), dans le dossier du dépôt : `cmd /c "rcsh essai.rc < NUL"` affiche `essai réussi`. Le `rcsh.exe` doit avoir quitté le dossier du dépôt : sinon cmd lance celui-là, qui ne trouve pas `rcmain`.

### Installer tcc sous Windows

Le tcc officiel pour Windows (0.9.27, 2017) est trop ancien pour ces sources (il lui manque `unistd.h`, entre autres) : il sert seulement à compiler un tcc récent, depuis les sources de tinycc. Dans PowerShell :

```powershell
cd $env:USERPROFILE
# 1. tcc 0.9.27 officiel
curl.exe -LO https://download.savannah.gnu.org/releases/tinycc/tcc-0.9.27-win64-bin.zip
(Get-FileHash tcc-0.9.27-win64-bin.zip).Hash   # 34A721949A2583FDFF725312DA092FA0F5F1F284B702E6F811C6954714FAABB2
tar -xf tcc-0.9.27-win64-bin.zip -C C:\
# il ne déclare pas cette fonction, dont le tcc récent a besoin
Add-Content C:\tcc\lib\kernel32.def AddVectoredExceptionHandler
# 2. tcc récent (version vérifiée : commit fc1fc50), installé dans C:\tcc-dev
curl.exe -Lo tinycc.tar.gz https://repo.or.cz/tinycc.git/snapshot/fc1fc50f7a5b3151a8932e9c2adb784140306db4.tar.gz
tar -xzf tinycc.tar.gz
cd tinycc-fc1fc50\win32
cmd /c "build-tcc.bat -c C:\tcc\tcc.exe -i C:\tcc-dev"            # tcc.exe, 64 bits
cmd /c "build-tcc.bat -c C:\tcc\tcc.exe -i C:\tcc-dev -x i386"    # i386-win32-tcc.exe, 32 bits (pour rcsh)
C:\tcc-dev\tcc.exe -v   # tcc version 0.9.28rc (x86_64 Windows)
```

## Debian 13

Rien à installer : le rc de Plan 9 est dans le paquet `9base` (`sudo apt install 9base`). `essai.rc` passe aussi avec lui : `./essai.rc rc < /dev/null`.

## Compilation

`compiler.ps1` compile les sources avec `i386-win32-tcc.exe`. Adaptations à tcc (le reste est la source d'origine) :
- `tcc/` : `lm.h` et `winnetwk.h` (parties utilisées, absentes des en-têtes de tcc), `win.c` (remplace la bibliothèque `win` de 9pm, qui choisissait entre fonctions Unicode et ANSI : Unicode seul), `advapi32.def` et `mpr.def` (tirés des DLL 32 bits de Windows 11 par `tcc -impdef`).
- `include/9pm/windows.h` : redéclarations des fonctions Windows réservées au compilateur de Microsoft.
- `thread.c`, `9pm.h` : variable par thread (`CT`) par `TlsAlloc`, tcc n'ayant pas `__declspec(thread)`.
- `tas.c` : `InterlockedExchange` au lieu d'assembleur MSVC ; `crt.c` : initialisation de la bibliothèque (`DllMain`) appelée au démarrage, puisqu'elle n'est plus une DLL.
- `src/rcsh/y.tab.c` et `y.tab.h` : générés depuis `syn.y` par le `yacc.exe` de 9pm.

## Vérifier

`essai.rc` vérifie ce dont des scripts rc portables dépendent (tubes, fonctions dans les sous-shells, captures, `flag e`, `exit`) ; il passe avec le rc de 9base et avec ce rcsh, et échoue sur la plupart des points avec celui de 9pm (dont les blocs `ifs=… {…}`, victimes du même plantage que les fonctions).

    rcsh essai.rc < NUL

Limite connue : la redirection `>[2=1]` échoue (« null list in concatenation ») ; rediriger stderr vers un fichier (`>[2]fichier`).

## Support

Dépôt publié tel quel, pour nos propres besoins : pas de support, les issues et pull requests ne sont pas suivies.
