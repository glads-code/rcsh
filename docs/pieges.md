# Pièges de rc, de rcsh et des outils Plan 9

Ce qu'il faut savoir pour écrire des scripts rc qui tournent à l'identique sous Linux (rc de [9base](https://tools.suckless.org/9base/)) et sous Windows (rcsh de ce dépôt avec les outils de 9pm).

## Le langage

- `flag e` arrête le script sur tout statut d'échec, y compris celui d'une **boucle `for` dont la dernière commande est un `if` faux** et d'une **fonction qui finit par un `if` faux**. Une affectation ne remet pas le statut à zéro : `x=1` après un `if` faux garde l'échec. Finir les corps de boucle et les fonctions par une commande qui réussit (`test -f $f || echo $f` plutôt que `if(! test -f $f) echo $f`, ou `status=''`).
- `flag e` ne regarde que la **dernière** commande d'un tube : `awk … | sort > f` continue si awk échoue (statut `1|`). Une fonction qui teste `$status` après le tube comble le manque :

  ```
  fn tube { s=$status; if(~ $s *[0-9a-zA-Z]*){ echo tube en échec : $s >[1=2]; exit tube }; status='' }
  ```

  Une condition `if(cmd | cmd)`, elle, voit tout le tube. Les tâches lancées avec `&` ne remontent pas leur échec : écrire leur résultat à côté puis le renommer, et vérifier après `wait` que tous les résultats sont là.
- Dans une **fonction**, une commande en échec arrête le script même quand la fonction sert de condition (`if(connu …)`) : une fonction qui répond oui ou non pose elle-même son statut (`if(cmd) status=''` puis `if not status=non`). Une commande dont l'échec est attendu (par exemple `curl --max-time` qui garde la connexion jusqu'au délai) se met elle aussi en condition.
- rc **exporte toutes ses variables** : une longue liste (des dizaines de milliers de mots) fait échouer le lancement de chaque commande, sans message clair. Garder les longues listes dans une fonction ou un fichier, pas dans une variable. Corollaire : ne jamais rediriger directement vers un fichier de données précieux (`cmd > $f`) ; écrire à côté puis renommer.
- `.` rend le statut de la dernière commande du fichier lu : le finir par `status=''`.
- Pas de `return`, `break` ni `continue` dans le rc de 9base : `if` imbriqués.
- Une capture collée à une chaîne se découpe en mots et se distribue (`'a '^`{echo b c}` donne `a b` et `a c`) : capturer d'abord dans une variable, avec `ifs` réglé sur le saut de ligne, puis `$"var`.
- Rien ne se colle tout seul après un indice : `$x(1).tmp` donne deux mots (`$x(1)` et `.tmp`) ; écrire `$x(1)^.tmp`.
- Un script relatif s'appelle avec `./` (`./scripts/x.rc`) : sinon rc le cherche dans `$path`.
- `=` et `|` doivent être protégés dans les arguments : `-v X'='$v`, `-v OFS'=|'`.
- `for(...){...} | cmd` envoie chaque tour séparément dans le tube : entourer la boucle d'accolades.
- `$"var` joint une liste en une chaîne ; `` `{cmd} `` capture une sortie ; `>[1=2]` écrit sur la sortie d'erreur.

## Les outils (9base et 9pm)

- Absents de 9base ou de 9pm : `cut`, `paste`, `xargs`, `awk` (celui de 9base compte en octets : faux sur l'UTF-8), `find`, `stat`, `mktemp`, `head` (`sed 1q` le remplace), `wget`. `sha1sum` n'existe que dans 9base.
- `seq 0` affiche une ligne dans 9base, aucune dans 9pm (comme GNU) : traiter le cas vide à part.
- `ls -p` pour les noms seuls (`ls` affiche les chemins) ; pas de `ls -l`.
- `grep` sans `-x` ni `-o` ; `sed` sans `-E`, mais regex étendues par défaut, sans `{n}`.
- `sort` : mêmes clés que GNU (`-k1,1n`, `-k12,12gr`) et même départage des égalités sur la ligne entière, mais pas de `-s` (tri stable : numéro de ligne en second critère) ni de `-S` ; environ 4 fois plus lent. Dans 9pm, il s'appelle `9sort`.
- `date` n'a pas de format (« Wed Nov 18 19:43:06 CET 2026 », même sortie sous 9pm) : le lire avec awk, par exemple `date | awk '{ printf "%s-%02d-%02d\n", $6, (index("JanFebMarAprMayJunJulAugSepOctNovDec", $2) + 2) / 3, $3 }'`. `date -n` donne les secondes depuis 1970 et `date <secondes>` une autre date.
- `du -s` compte en blocs de 1 Kio.
- Aucun outil ne dépend de la locale (9base lit l'UTF-8 sans locale) : pas besoin de `LC_ALL`.
- Nombres au-delà de 2 Gio : calculer dans awk (`printf "%.0f"`), pas avec `test` (32 bits sous Windows).
- `for (k in t)` d'awk : ordre non défini, différent d'un awk à l'autre ; un résultat ne doit jamais en dépendre.
- Le `chmod` de 9pm réécrit les droits NTFS et, hors élévation, rend le fichier illisible : ne pas l'utiliser, Windows n'a pas besoin du bit x.

## rcsh sous Windows

- Se lance par `rcsh script.rc < NUL` : sans `< NUL`, rcsh attend l'entrée. Pour lire son code de sortie depuis `cmd`, `cmd /v:on` et `!errorlevel!` (`%errorlevel%` est lu avant la commande).
- `$path` est une seule chaîne (le `PATH` de Windows tel quel), pas une liste. Lancé depuis Git Bash, les outils de MSYS (`basename`, `sed`…) passent devant ceux de 9pm : mettre `C:/9pm/bin` en tête (`path=(C:/9pm/bin $path)`).
- `$home` reprend `HOME`, ou `%USERPROFILE%` quand `HOME` manque (session Windows ordinaire).
- Statut d'échec de la forme `error code 1` ; `false` n'existe pas.
- Here-documents : rcsh écrit leur texte dans `C:\tmp`, qui doit exister. Un here-document à gauche d'un tube (`cmd <<FIN | autre`) échoue encore.
- rcsh est un programme 32 bits : Windows lui redirige `C:\Windows\System32` vers SysWOW64, où `OpenSSH` n'existe pas (`ssh: file not found`). Mettre `C:\Windows\Sysnative\OpenSSH` dans le `PATH` des scripts.
- Lancé depuis une session ssh de Windows (sans console), `ssh.exe` se fige dès que son entrée est un tube ou un here-document (`tar | ssh`, `ssh … <<FIN`, `git clone` par ssh) ; depuis une session de bureau, tout passe.
- Accents : les scripts s'écrivent en UTF-8 sans BOM, LF ou CRLF. PowerShell 5 les abîme : `Set-Content` sans encodage écrit en ANSI, `-Encoding UTF8` ajoute un BOM qui colle au premier mot (`echo: file not found` sur la ligne 1).
- Noms de fichiers : 9pm (`ls`) et rcsh (motifs `*`) coupent les noms à **63 octets** (en UTF-8, un accent compte deux), sans erreur : un fichier au nom plus long est invisible pour un motif. Chemin complet limité à 260 caractères (`MAX_PATH`). Les noms accentués ou avec espaces se créent, se lisent, se copient et se trient (ordre des octets).
- Tâches planifiées : rcsh lit sa ligne de commande à la manière de Windows, les apostrophes n'y protègent rien (`-c "a || b"`, jamais `-c '…'`) ; pas d'accent dans les arguments d'une tâche, ils arrivent abîmés.
