# Projet de Simulation FMU

Ce projet a pour objectif de compiler et exécuter une simulation à partir d'une archive FMU décompressée, nommée "fmu", dont les sources sous forme de fichiers c sont présentes.

## Structure du Projet

- `headers/`: Dossier contenant les fichiers d'en-tête nécessaires.
- `tests/`: Dossier contenant des scripts Python pour afficher les valeurs.
- `main.c`: Fichier source principal pour la simulation.
- `fmi2.c`: Fichier source liant les fonctions FMI 2.0 nécessaires à la simulation au reste du code c
- `Makefile`: Fichier pour automatiser la compilation et l'exécution.
- `parseFMU.sh`: Script pour analyser et extraire les informations nécessaires de l'archive FMU.

## Prérequis

- GCC (GNU Compiler Collection)
- Make

## Compilation

Pour compiler le projet, exécutez la commande suivante dans le terminal :

```sh
make
```

Cette commande utilise le `Makefile` pour extraire les fichiers sources de l'archive .fmu, crée un fichier C en parsant `modelDescription.xml` et compile les sources pour générer l'exécutable `fmusim`.

## Exécution

Une fois la compilation terminée, vous pouvez lancer la simulation avec l'exécutable généré :

```sh
./fmusim StartTime EndTime StepSize
```

Pour directement afficher un graphique :
```sh
./fmusim StartTime EndTime StepSize > ./tests/out.txt | python3 ./tests/plot.py
```

## Nettoyage

Pour nettoyer les fichiers générés, utilisez la commande :

```sh
make clean
```

## Configuration

Les configurations spécifiques au projet, telles que les options de débogage, peuvent être modifiées dans le `Makefile`.

## Auteurs

- Tom REYNAUD

## Base project
Basé sur le projet [fmuSDK](https://github.com/qtronic/fmusdk) qui utilise les fichiers dll pour récupérer les fonctions nécessaire à la simulation

## License

### FMU SDK License

The FMU SDK is provided by Synopsys under the BSD 2-Clause License.

#### FMU SDK License Text

Copyright (c) 2008-2018, QTronic GmbH. All rights reserved. The FMU SDK is licensed by the copyright holder under the 2-Clause BSD License:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

**THIS SOFTWARE IS PROVIDED BY QTRONIC GMBH "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL QTRONIC GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.**
