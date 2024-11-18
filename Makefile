# Compilateur et options
CC = gcc
CFLAGS = -Iheaders -Isources -Wall -g -DFMI_VERSION=2 -DModelFMI_COSIMULATION=0 -DMODEL_IDENTIFIER=BouncingBall -DFMI2_OVERRIDE_FUNCTION_PREFIX="" -fno-common #-DDEBUG

# Dossiers de sources et d'en-têtes
SRCDIR = fmu/sources
#HEADERS = headers/fmi2Functions.h headers/fmi2FunctionTypes.h headers/fmi2TypesPlatform.h $(SRCDIR)/model.h $(SRCDIR)/config.h

# Fichiers sources à compiler
SOURCES = main.c $(SRCDIR)/all.c

OBJECTS = main.o all.o

# Fichier cible
TARGET = fmusim

# Règles pour compiler le projet
all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) -ldl

# Nettoyage des fichiers objets et de l'exécutable
clean:
	rm -f $(TARGET) *.o
