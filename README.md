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

## Licence

Ce projet est sous licence [Nom de la Licence].
