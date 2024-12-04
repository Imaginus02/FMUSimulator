# Compilateur et options
CC = gcc
CFLAGS = -Iheaders -Isources -Wall -g -DFMI_VERSION=2 -DModelFMI_COSIMULATION=0  -DFMI2_OVERRIDE_FUNCTION_PREFIX="" -fno-common #-DDEBUG #-DMODEL_IDENTIFIER=BouncingBall

# Dossiers de sources et d'en-têtes
SRCDIR = fmu/sources
#HEADERS = headers/fmi2Functions.h headers/fmi2FunctionTypes.h headers/fmi2TypesPlatform.h $(SRCDIR)/model.h $(SRCDIR)/config.h

# Fichiers sources à compiler
SOURCES = main.c $(SRCDIR)/all.c

OBJECTS = main.o all.o

# Fichier cible
TARGET = fmusim

# Règles pour compiler le projet
all: prepare $(TARGET)

prepare:
	@fmu_files=$$(find . -maxdepth 1 -name '*.fmu'); \
	if [ $$(echo $$fmu_files | wc -w) -ne 1 ]; then \
		echo "Error: There should be exactly one .fmu file in the directory."; \
		exit 1; \
	fi; \
	unzip -o $$fmu_files -d fmu/
	./parseFMU.sh

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) -ldl

# Nettoyage des fichiers objets, de l'exécutable, du répertoire fmu/ et du fichier modelDescription.c
clean:
	rm -f $(TARGET) *.o
	rm -rf fmu/
	rm -f modelDescription.c
