# Projet de Simulation FMU

Ce projet a pour objectif de compiler et exécuter une simulation à partir d'une archive FMU décompressée, nommée "fmu", dont les sources sous forme de fichiers c sont présentes.

## Structure du Projet

- `fmu/`: Dossier contenant les sources décompressées de l'archive FMU.
- `headers/`: Dossier contenant les fichiers d'en-tête nécessaires.
- `main.c`: Fichier source principal pour la simulation.
- `Makefile`: Fichier pour automatiser la compilation et l'exécution.

## Prérequis

- GCC (GNU Compiler Collection)
- Make

## Compilation

Pour compiler le projet, exécutez la commande suivante dans le terminal :

```sh
make
```

Cette commande utilise le `Makefile` pour compiler les sources et générer l'exécutable `fmusim`.

## Exécution

Une fois la compilation terminée, vous pouvez lancer la simulation avec l'exécutable généré :

```sh
./fmusim
```

## Nettoyage

Pour nettoyer les fichiers objets et l'exécutable généré, utilisez la commande :

```sh
make clean
```

## Configuration

Les configurations spécifiques au projet, telles que les GUID et les options de débogage, peuvent être modifiées dans le fichier `main.c` et le `Makefile`.

## Auteurs

- Tom REYNAUD

## Base project
Base on the [fmuSDK simulator](https://github.com/qtronic/fmusdk) that used the dll files in order to setup the simulation

## License

### FMU SDK License

The FMU SDK is provided by Synopsys under the BSD 2-Clause License.

The following additional tools are distributed with the FMU SDK under their respective licenses:

- **7z 4.57** by Igor Pavlov, used here to zip and unzip FMUs (7-Zip License for use and distribution).
- **eXpat 2.0.1** by James Clark, used here to parse the modelDescription.xml file of an FMU 1.0 (MIT License).

#### FMU SDK License Text

Copyright (c) 2008-2018, QTronic GmbH. All rights reserved. The FMU SDK is licensed by the copyright holder under the 2-Clause BSD License:

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

**THIS SOFTWARE IS PROVIDED BY QTRONIC GMBH "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL QTRONIC GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.**

- **libxml**, the XML C parser and toolkit of Gnome, used here to parse the modelDescription.xml file of an FMU 2.0 (MIT license).
- The contribution guide is adapted from **normalize.css** (MIT License) and **chris.beams.io**.