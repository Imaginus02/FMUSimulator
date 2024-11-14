#!/bin/bash

output_file="output.c"

# Start the C file with includes and struct definition
cat <<EOT > "$output_file"
#include <stdio.h>
#include <stdlib.h>

typedef enum { INTEGER, REAL } VarType;

typedef struct {
    char *name;
    int valueReference;
    char *description;
    VarType type;
    union {
        int intValue;
        double realValue;
    } start;
    union {
        int intMin;
        double realMin;
    } min;
    union {
        int intMax;
        double realMax;
    } max;
} ScalarVariable;

void initialize(ScalarVariable *var, char *name, int valueReference, char *description,
                VarType type, void *start, void *min, void *max) {
    var->name = name;
    var->valueReference = valueReference;
    var->description = description;
    var->type = type;
    if (type == INTEGER) {
        var->start.intValue = start ? *(int*)start : 0;
        var->min.intMin = min ? *(int*)min : 0;
        var->max.intMax = max ? *(int*)max : 0;
    } else {
        var->start.realValue = start ? *(double*)start : 0.0;
        var->min.realMin = min ? *(double*)min : 0.0;
        var->max.realMax = max ? *(double*)max : 0.0;
    }
}

int main() {
EOT

# Read input and populate variables
while IFS= read -r line; do
    # Parse each attribute from the line
    name=$(echo "$line" | grep -oP 'name="\K[^"]*')
    valueReference=$(echo "$line" | grep -oP 'valueReference="\K[^"]*')
    description=$(echo "$line" | grep -oP 'description="\K[^"]*')
    type=$(echo "$line" | grep -oP 'type="\K[^"]*')
    start=$(echo "$line" | grep -oP 'start="\K[^"]*')
    min=$(echo "$line" | grep -oP 'min="\K[^"]*')
    max=$(echo "$line" | grep -oP 'max="\K[^"]*')

    # Set default values if fields are missing
    valueReference=${valueReference:-0}
    description=${description:-NULL}
    type_enum="INTEGER"
    start_val=0
    min_val=0
    max_val=0

    # Adjust types and defaults based on type
    if [ "$type" = "Real" ]; then
        type_enum="REAL"
        start_val=${start:-0.0}
        min_val=${min:-0.0}
        max_val=${max:-0.0}
    else
        start_val=${start:-0}
        min_val=${min:-0}
        max_val=${max:-0}
    fi

    # Generate C code to initialize each variable
    cat <<EOT >> "$output_file"
    {
        ScalarVariable var;
        char *name = "$name";
        char *description = "$description";
        int valueReference = $valueReference;
        VarType type = $type_enum;
        ${type_enum} start = ${start_val};
        ${type_enum} min = ${min_val};
        ${type_enum} max = ${max_val};
        initialize(&var, name, valueReference, description, type, &start, &min, &max);
        // Add var to an array or perform additional operations here
    }
EOT

done

# End of the C file
echo "    return 0;" >> "$output_file"
echo "}" >> "$output_file"

# Notify user
echo "Generated $output_file"
