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

rcsh remplace celui de 9pm, à installer d'abord :

1. Télécharger [9pm051031.zip](https://9p.io/sources/extra/9pm051031.zip) et vérifier son empreinte (PowerShell) : `(Get-FileHash 9pm051031.zip).Hash` doit donner `9E9281BA379EFDE3E94666E4358D9959971AF9956416D2C858905F9B3E308DD3`.
2. Extraire l'archive dans `C:\9pm` et ajouter `C:\9pm\bin` au `PATH` de l'utilisateur.
3. Conseillé : renommer `C:\9pm\bin\tar.exe` en `9tar.exe` (il masque le `tar.exe` de Windows) et copier `9sort.exe` en `sort.exe` (sinon `sort` est celui de Windows).

Compiler `rcsh.exe` sous Debian 13 (32 bits, bibliothèque de 9pm liée dedans, sans `9pm2.dll`). Le tcc croisé se construit une fois depuis les sources de tinycc (dossier attendu par défaut par les scripts : `~/.cache/wak-src/tinycc`, autre dossier en argument) :

    sudo apt install git gcc make 9base
    git clone https://repo.or.cz/tinycc.git ~/.cache/wak-src/tinycc
    cd ~/.cache/wak-src/tinycc
    ./configure --targetos=WIN32 --cpu=x86_64 --enable-cross
    make cross-x86_64-win32 cross-i386-win32

Puis :

    git clone https://github.com/glads-code/rcsh.git
    cd rcsh
    ./compiler.rc

Sous Windows, remplacer `C:\9pm\bin\rcsh.exe` par ce `rcsh.exe` (les rcsh enfants sont lancés depuis le `bin` de 9pm) et `C:\9pm\lib\rcmain` par `lib/rcmain` du dépôt. Vérifier dans le dépôt : `rcsh essai.rc < NUL`.

## Debian 13

Rien à installer : le rc de Plan 9 est dans le paquet `9base` (`sudo apt install 9base`). `essai.rc` passe aussi avec lui : `./essai.rc rc < /dev/null`.

## Compilation

Adaptations à tcc (le reste est la source d'origine) :
- `tcc/` : `lm.h` et `winnetwk.h` (parties utilisées, absentes des en-têtes de tcc), `win.c` (remplace la bibliothèque `win` de 9pm, qui choisissait entre fonctions Unicode et ANSI : Unicode seul), `advapi32.def` et `mpr.def` (tirés des DLL 32 bits de Windows 11 par `tcc -impdef`).
- `include/9pm/windows.h` : redéclarations des fonctions Windows réservées au compilateur de Microsoft.
- `thread.c`, `9pm.h` : variable par thread (`CT`) par `TlsAlloc`, tcc n'ayant pas `__declspec(thread)`.
- `tas.c` : `InterlockedExchange` au lieu d'assembleur MSVC ; `crt.c` : initialisation de la bibliothèque (`DllMain`) appelée au démarrage, puisqu'elle n'est plus une DLL.
- `src/rcsh/y.tab.c` et `y.tab.h` : générés depuis `syn.y` par le `yacc.exe` de 9pm.

## Vérifier

`essai.rc` vérifie ce dont des scripts rc portables dépendent (tubes, fonctions dans les sous-shells, captures, `flag e`, `exit`) ; il passe avec le rc de 9base et avec ce rcsh, et échoue sur la plupart des points avec celui de 9pm (dont les blocs `ifs=… {…}`, victimes du même plantage que les fonctions).

    rcsh essai.rc < NUL

## Support

Dépôt publié tel quel, pour nos propres besoins : pas de support, les issues et pull requests ne sont pas suivies.
