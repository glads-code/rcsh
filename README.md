# rcsh

Shell rc des scripts du poste sous Windows : `rcsh` de [9pm](https://9p.io/sources/extra/) (Plan 9 pour Windows, 2005), corrigé et recompilé avec tcc. Sources reprises de `9pm051031.zip` (sha256 `9e9281ba379efde3e94666e4358d9959971af9956416d2c858905f9b3e308dd3`), licence LPL 1.02 (`LICENSE.txt`, `NOTICE`).

Le dépôt ne contient que rcsh et sa bibliothèque (`src/9pm`, compilée d'origine en `9pm2.dll`) ; les autres outils de 9pm (`sed`, `grep`, `9sort`…) restent ceux de l'archive.

## Corrections

Le `rcsh.exe` de 9pm rend les scripts dangereux : un échec peut passer inaperçu.

- **Statut des tubes** (`src/9pm/win32/proc.c`) : `proc()` fermait le handle du processus créé et `procwait()` le rouvrait par son numéro. Un enfant déjà terminé (le côté gauche d'un tube, presque toujours) n'existait plus : son échec devenait un succès (`test -f absent | cat` donnait `|`). Les handles sont gardés jusqu'à l'attente ; un processus introuvable compte comme un échec.
- **Fonctions dans les sous-shells** (`src/rcsh/var.c`, `main.c`) : un rcsh enfant (côté gauche d'un tube, capture `` `{…} ``) définissait les fonctions héritées de l'environnement (`fn#nom`) avant d'avoir démarré son interpréteur, et plantait. Dès qu'un script définissait une fonction (`tube` dans tous nos scripts), le côté gauche de ses tubes et ses captures ne s'exécutaient plus. Les fonctions sont maintenant définies juste après le démarrage, comme dans le rc de Plan 9.
- **`errstr` sur une chaîne littérale** (`proc.c`) : écrivait dans une constante, en mémoire en lecture seule avec tcc ; plantait un enfant lancé sans entrée standard (toute capture).

- **Commandes de Windows introuvables** (`src/9pm/win32/env.c`) : quand `9pm` était défini, la bibliothèque ne recopiait pas le `Path` de Windows dans `path`, où rcsh cherche les commandes : seules celles de `$9pm/bin` étaient trouvées (ni `certutil`, ni le `tar` de Windows). Sans `9pm` défini, elle déduit maintenant son dossier de l'exécutable s'il n'y a pas de `9pm2.dll`.
- **Scripts lancés par leur nom** (`proc.c`) : seule l'extension `.rcs` désignait un script rc. Un script `.rc` (`./scripts/contours.rc`) était lancé comme un programme Windows et échouait, un script sans extension (`tri-mails`) n'était pas trouvé. Les deux passent par rcsh, avec un `$0` écrit avec des `/` (`basename -d $0` donne son dossier).
- **Commande introuvable** (`simple.c`) : rcsh affichait l'erreur sans changer le statut ; `flag e` ne s'arrêtait pas et le script continuait comme si de rien n'était.
- **`$0` dans les sous-shells** (`main.c`, `lib/rcmain`) : une capture ou un côté de tube relisait `rcmain`, qui fixait `$0` sur lui-même ; `` `{basename -d $0} `` donnait le dossier de 9pm. Le sous-shell (`-S`) garde le `$0` du script.
- **Redirections** (`exec.c`) : captures, côtés de tubes et sous-shells `@{…}` héritent de l'entrée du script et des redirections en cours (`{ … } > fichier`), comme dans rc ; `` x=`{cat} `` lisait rien, et la sortie d'un `@{…}` échappait au fichier.

`flag e` et le code de sortie (`exit`, échec d'une commande) fonctionnaient déjà.

## Compiler

Sur deb : `./compiler.rc` produit `rcsh.exe` (32 bits, bibliothèque de 9pm liée dedans, sans `9pm2.dll`).

Il faut le tcc croisé i386-win32, compilé depuis les sources de tinycc (`https://repo.or.cz/tinycc.git`, dans `~/.cache/wak-src/tinycc` par défaut, autre dossier en argument) :

    ./configure --targetos=WIN32 --cpu=x86_64 --enable-cross
    make cross-i386-win32

Adaptations à tcc (le reste est la source d'origine) :
- `tcc/` : `lm.h` et `winnetwk.h` (parties utilisées, absentes des en-têtes de tcc), `win.c` (remplace la bibliothèque `win` de 9pm, qui choisissait entre fonctions Unicode et ANSI : Unicode seul), `advapi32.def` et `mpr.def` (tirés des DLL 32 bits de Windows 11 par `tcc -impdef`).
- `include/9pm/windows.h` : redéclarations des fonctions Windows réservées au compilateur de Microsoft.
- `thread.c`, `9pm.h` : variable par thread (`CT`) par `TlsAlloc`, tcc n'ayant pas `__declspec(thread)`.
- `tas.c` : `InterlockedExchange` au lieu d'assembleur MSVC ; `crt.c` : initialisation de la bibliothèque (`DllMain`) appelée au démarrage, puisqu'elle n'est plus une DLL.
- `src/rcsh/y.tab.c` et `y.tab.h` : générés depuis `syn.y` par le `yacc.exe` de 9pm.

## Vérifier et installer

`essai.rc` vérifie ce dont nos scripts dépendent (tubes, fonctions dans les sous-shells, captures, `flag e`, `exit`) ; il passe avec le rc de 9base et avec ce rcsh, et échoue sur la plupart des points avec celui de 9pm (dont les blocs `ifs=… {…}` de tri-mails, victimes du même plantage que les fonctions).

    rcsh essai.rc < NUL

Installation : remplacer `C:\9pm\bin\rcsh.exe` par `rcsh.exe` (les rcsh enfants sont lancés depuis `%9pm%\bin`) et `C:\9pm\lib\rcmain` par `lib/rcmain`.

## Support

Dépôt publié tel quel, pour nos propres besoins : pas de support, les issues et pull requests ne sont pas suivies.
