#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Headers from the FMI standard
#include "headers/fmi2TypesPlatform.h"
#include "headers/fmi2FunctionTypes.h"
#include "headers/fmi2Functions.h"

// Headers from the FMU file
#include "fmu/sources/config.h"
#include "fmu/sources/model.h"

// Other necessary files
#include "fmi2.c"
#include "modelDescription.c"


// If DEBUG is defined, INFO will print messages to the console
#ifndef DEBUG
#define INFO(message, ...)
#else
#define INFO(message, ...) do { \
	char buffer[256]; \
	snprintf(buffer, sizeof(buffer), message, ##__VA_ARGS__); \
	printf("%s", buffer); \
} while (0)
#endif

// Minimum macro
#define min(a,b) ((a)>(b) ? (b) : (a))

// Structure to hold the simulation state
typedef struct {
    fmi2Component component;
    int nx;                          // number of state variables
    int nz;                          // number of state event indicators
    double *x;                       // continuous states
    double *xdot;                    // derivatives
    double *z;                       // state event indicators
    double *prez;                    // previous state event indicators
    double time;                     // current simulation time
    double h;                        // step size
    double tStart;                   // start time
    double tEnd;                     // end time
    fmi2EventInfo eventInfo;         // event info
    ScalarVariable *variables;       // model variables
    int nVariables;                  // number of variables
    double **output;                 // output array
    int nSteps;                      // current step count
    int nTimeEvents;                 // number of time events
    int nStateEvents;                // number of state events
    int nStepEvents;                 // number of step events
} SimulationState;

// Initialize the FMU structure, which contains function pointers for FMI operations
FMU fmu;

/**
 * @brief Converts an fmi2Status enum value to its corresponding string representation.
 *
 * This function takes an fmi2Status value and returns a string that represents the status.
 * The possible status values and their corresponding strings are:
 * - fmi2OK: "OK"
 * - fmi2Warning: "Warning"
 * - fmi2Discard: "Discard"
 * - fmi2Error: "Error"
 * - fmi2Fatal: "Fatal"
 * - fmi2Pending: "Pending"
 * - default: "?"
 *
 * @param status The fmi2Status value to be converted to a string.
 * @return A string representing the given fmi2Status value.
 */
char * fmi2StatusToString(fmi2Status status) {
	switch (status) {
		case fmi2OK: return "OK";
		case fmi2Warning: return "Warning";
		case fmi2Discard: return "Discard";
		case fmi2Error: return "Error";
		case fmi2Fatal: return "Fatal";
		case fmi2Pending: return "Pending";
		default: return "?";
	}
}


#define MAX_MSG_SIZE 1000
/**
 * @brief Logs messages from the FMU (Functional Mock-up Unit).
 *
 * This function is used to log messages from the FMU, providing information about the instance,
 * status, category, and the message itself. It formats the message using variable arguments.
 *
 * @param componentEnvironment A pointer to the component environment (unused in this function).
 * @param instanceName The name of the FMU instance. If NULL, it defaults to "?".
 * @param status The status of the FMU, represented as an fmi2Status enum.
 * @param category The category of the message. If NULL, it defaults to "?".
 * @param message The message to be logged, which can include format specifiers.
 * @param ... Additional arguments for the format specifiers in the message.
 */
void fmuLogger (void *componentEnvironment, fmi2String instanceName, fmi2Status status,
               fmi2String category, fmi2String message, ...) {
	char msg[MAX_MSG_SIZE];
	va_list argp;

	va_start(argp, message);
	vsprintf(msg, message, argp);
	va_end(argp);

	if (instanceName == NULL) instanceName = "?";
	if (category == NULL) category = "?";
	printf("%s %s (%s): %s\n", fmi2StatusToString(status), instanceName, category, msg);
}

/**
 * @brief Prints an error message to the standard output.
 *
 * This function takes a string message as input and prints it to the standard
 * output followed by a newline character. It then returns 0.
 *
 * @param message The error message to be printed.
 * @return Always returns 0.
 */
int error(const char *message) {
	printf("%s\n", message);
	return 0;
}

/**
 * @brief Frees all resources associated with the simulation state.
 *
 * @param fmu Pointer to the FMU structure
 * @param state Pointer to the simulation state to be freed
 */
void cleanupSimulation(FMU *fmu, SimulationState *state) {
    if (!state) return;

    // Terminate the FMU
    if (state->component) {
        fmu->terminate(state->component);
        fmu->freeInstance(state->component);
    }

    // Free state variables
    if (state->x) free(state->x);
    if (state->xdot) free(state->xdot);
    if (state->z) free(state->z);
    if (state->prez) free(state->prez);

    // Free output array
    if (state->output) {
        for (int i = 0; i < state->nVariables + 1; i++) {
            if (state->output[i]) free(state->output[i]);
        }
        free(state->output);
    }

    // Free the state structure itself
    free(state);
}


/**
 * @brief Initializes the FMU simulation and returns a simulation state structure.
 *
 * @param fmu Pointer to the FMU structure
 * @param tEnd End time for simulation
 * @param h Step size
 * @return SimulationState* Pointer to initialized simulation state, NULL if error
 */
SimulationState* initializeSimulation(FMU *fmu, double tStart, double tEnd, double h) {
    SimulationState *state = (SimulationState*)calloc(1, sizeof(SimulationState));
    if (!state) return NULL;

    state->time = 0;
    state->h = h;
    state->tStart = tStart;
    state->tEnd = tEnd;
    state->nSteps = 0;
    state->nTimeEvents = 0;
    state->nStateEvents = 0;
    state->nStepEvents = 0;

    // Setup callback functions
    fmi2CallbackFunctions callbacks = {fmuLogger, calloc, free, NULL, fmu};

    // Instantiate the FMU
    state->component = fmu->instantiate(model.modelName, fmi2ModelExchange, 
                                      model.guid, NULL, &callbacks, fmi2False, fmi2False);
    if (!state->component) {
        cleanupSimulation(fmu,state);
        return NULL;
    }

    state->nx = model.numberOfContinuousStates;
    state->nz = model.numberOfEventIndicators;

    // Allocate memory for states and indicators
    state->x = (double*)calloc(state->nx, sizeof(double));
    state->xdot = (double*)calloc(state->nx, sizeof(double));
    if (state->nz > 0) {
        state->z = (double*)calloc(state->nz, sizeof(double));
        state->prez = (double*)calloc(state->nz, sizeof(double));
    }

    if ((!state->x || !state->xdot) || 
        (state->nz > 0 && (!state->z || !state->prez))) {
        // Cleanup and return on allocation failure
        cleanupSimulation(fmu,state);
        return NULL;
    }

    // Setup experiment
    fmi2Boolean toleranceDefined = fmi2False;
    fmi2Real tolerance = 0;
    fmi2Status fmi2Flag = fmu->setupExperiment(state->component, toleranceDefined, 
                                              tolerance, state->tStart, fmi2True, state->tEnd);
    if (fmi2Flag > fmi2Warning) {
        // Cleanup and return on setup failure
        cleanupSimulation(fmu,state);
        return NULL;
    }

    // Initialize the FMU
    fmi2Flag = fmu->enterInitializationMode(state->component);
    if (fmi2Flag > fmi2Warning) {
        cleanupSimulation(fmu,state);
        return NULL;
    }

    fmi2Flag = fmu->exitInitializationMode(state->component);
    if (fmi2Flag > fmi2Warning) {
        cleanupSimulation(fmu,state);
        return NULL;
    }

    // Initial event iteration
    state->eventInfo.newDiscreteStatesNeeded = fmi2True;
    state->eventInfo.terminateSimulation = fmi2False;
    while (state->eventInfo.newDiscreteStatesNeeded && 
           !state->eventInfo.terminateSimulation) {
        fmi2Flag = fmu->newDiscreteStates(state->component, &state->eventInfo);
        if (fmi2Flag > fmi2Warning) {
            cleanupSimulation(fmu,state);
            return NULL;
        }
    }

    if (!state->eventInfo.terminateSimulation) {
        fmi2Flag = fmu->enterContinuousTimeMode(state->component);
        if (fmi2Flag > fmi2Warning) {
            cleanupSimulation(fmu,state);
            return NULL;
        }
    }

    // Initialize variables and output array
    get_variable_list(&state->variables);
    state->nVariables = get_variable_count();
    state->output = (double**)calloc(state->nVariables, sizeof(double*));
    for (int i = 0; i < state->nVariables; i++) {
        state->output[i] = (double*)calloc((size_t)(tEnd/h + 10), sizeof(double));
    }

    // Initialize first output values
    for (int i = 0; i < state->nVariables; i++) {
        if (state->variables[i].type == REAL) {
            fmu->getReal(state->component, &state->variables[i].valueReference, 
                        1, &state->output[i][0]);
        } else if (state->variables[i].type == INTEGER) {
            fmi2Integer intValue;
            INFO("Trying to retrieve variable %s\n as integer", state->variables[i].name);
            fmu->getInteger(state->component, &state->variables[i].valueReference, 
                           1, &intValue);
            state->output[i][0] = (double)intValue;
        }
    }
    return state;
}



/**
 * @brief Performs one simulation step and updates the simulation state.
 *
 * @param fmu Pointer to the FMU structure
 * @param state Pointer to the simulation state
 * @return fmi2Status Status of the simulation step
 */
fmi2Status simulationDoStep(FMU *fmu, SimulationState *state) {
    INFO("Entering simulation loop\n");
	if (state->time >= state->tEnd || state->eventInfo.terminateSimulation) {
        INFO("Simulation already terminated\n");
		return fmi2Discard;
    }

    fmi2Status fmi2Flag;
    double tPre = state->time;
    double dt;
    fmi2Boolean timeEvent, stateEvent, stepEvent, terminateSimulation;

    // Get current state and derivatives
    fmi2Flag = fmu->getContinuousStates(state->component, state->x, state->nx);
    if (fmi2Flag > fmi2Warning) return fmi2Flag;

    fmi2Flag = fmu->getDerivatives(state->component, state->xdot, state->nx);
    if (fmi2Flag > fmi2Warning) return fmi2Flag;

	INFO("States and derivatives retrieved\n");

    // Advance time
    state->time = min(state->time + state->h, state->tEnd);
    timeEvent = state->eventInfo.nextEventTimeDefined && 
                state->time >= state->eventInfo.nextEventTime;
    
    if (timeEvent) state->time = state->eventInfo.nextEventTime;
    dt = state->time - tPre;
    
    fmi2Flag = fmu->setTime(state->component, state->time);
    if (fmi2Flag > fmi2Warning) return fmi2Flag;

	INFO("Time set\n");

    // Perform one step (forward Euler)
    for (int i = 0; i < state->nx; i++) {
        state->x[i] += dt * state->xdot[i];
    }
    
    fmi2Flag = fmu->setContinuousStates(state->component, state->x, state->nx);
    if (fmi2Flag > fmi2Warning) return fmi2Flag;

	INFO("Step performed\n");

    // Check for state event
    for (int i = 0; i < state->nz; i++) {
        state->prez[i] = state->z[i];
    }
    
    fmi2Flag = fmu->getEventIndicators(state->component, state->z, state->nz);
    if (fmi2Flag > fmi2Warning) return fmi2Flag;

    stateEvent = fmi2False;
    for (int i = 0; i < state->nz; i++) {
        stateEvent = stateEvent || (state->prez[i] * state->z[i] < 0);
    }

	INFO("State event checked\n");

    // Check for step event
    fmi2Flag = fmu->completedIntegratorStep(state->component, fmi2True, 
                                           &stepEvent, &terminateSimulation);
    if (fmi2Flag > fmi2Warning) return fmi2Flag;

    if (terminateSimulation) {
        state->eventInfo.terminateSimulation = fmi2True;
        return fmi2OK;
    }

	INFO("Step event checked\n");

    // Handle events
    if (timeEvent || stateEvent || stepEvent) {
        fmi2Flag = fmu->enterEventMode(state->component);
        if (fmi2Flag > fmi2Warning) return fmi2Flag;

        if (timeEvent) state->nTimeEvents++;
        if (stateEvent) state->nStateEvents++;
        if (stepEvent) state->nStepEvents++;
		INFO("Event handled\n");

        // Event iteration
        state->eventInfo.newDiscreteStatesNeeded = fmi2True;
        state->eventInfo.terminateSimulation = fmi2False;
        
        while (state->eventInfo.newDiscreteStatesNeeded && 
               !state->eventInfo.terminateSimulation) {
            fmi2Flag = fmu->newDiscreteStates(state->component, &state->eventInfo);
            if (fmi2Flag > fmi2Warning) return fmi2Flag;
        }

        if (state->eventInfo.terminateSimulation) {
            return fmi2OK;
        }

        // Re-enter continuous-time mode
        fmi2Flag = fmu->enterContinuousTimeMode(state->component);
        if (fmi2Flag > fmi2Warning) return fmi2Flag;
    }

    // Update outputs
    for (int i = 0; i < state->nVariables; i++) {
        if (state->variables[i].type == REAL) {
            fmu->getReal(state->component, &state->variables[i].valueReference, 
                        1, &state->output[i][state->nSteps]);
			INFO("DEBUG:   %s (ref %d): %f\n", state->variables[i].name, 
                   state->variables[i].valueReference, state->output[i][state->nSteps]);
        } else if (state->variables[i].type == INTEGER) {
            fmi2Integer intValue;
            fmu->getInteger(state->component, &state->variables[i].valueReference, 
                           1, &intValue);
            state->output[i][state->nSteps] = (double)intValue;
        }
    }

    state->nSteps++;
    return fmi2OK;
}

void printOutput(SimulationState *state) {
    // Print simulation summary
    INFO("Simulation from %g to %g terminated successfully\n", state->tStart, state->tEnd);
    INFO("  steps ............ %d\n", state->nSteps);
    INFO("  fixed step size .. %g\n", state->h);
    INFO("  time events ...... %d\n", state->nTimeEvents);
    INFO("  state events ..... %d\n", state->nStateEvents);
    INFO("  step events ...... %d\n", state->nStepEvents);

    // Print the output
    for (int j = 0; j < state->nSteps; j++) {
        printf("Step %d: ", j);
        for (int i = 0; i < state->nVariables; i++) {
            printf("%s=%f ", state->variables[i].name, state->output[i][j]);
        }
        printf("\n");
    }
}

void printCsv(SimulationState *state, char sep) {
    // Print headers
    printf("step%c", sep);
    for (int i = 0; i < state->nVariables; i++) {
        printf("%s", state->variables[i].name);
        if (i < state->nVariables - 1) {
            printf("%c", sep);
        }
    }
    printf("\n");
    
    // Print the output
    for (int j = 0; j < state->nSteps; j++) {
        printf("%d%c", j, sep);
        for (int i = 0; i < state->nVariables; i++) {
            printf("%f", state->output[i][j]);
            if (i < state->nVariables - 1) {
                printf("%c", sep);
            }
        }
        printf("\n");
    }
}

/**
 * @brief Main function to initialize and run the simulation.
 *
 * This function sets up the simulation parameters, loads necessary functions,
 * and starts the simulation.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 *
 * @return Returns 0 upon successful completion.
 */
int main(int argc, char *argv[]) {

    double tStart;
    double tEnd;
    double h;
    int csv = 0;
    char sep = ',';

	// Liste des paramètres à récupérer
	// tStart, tEnd, h, --csv, sep

	// Validate minimum number of arguments
    if (argc < 4) {
        printf("Usage: %s tStart tEnd h [--csv separator]\n", argv[0]);
        return -1;
    }

    // Handle optional CSV arguments
    if (argc >= 5) {
        if (strcmp(argv[4], "--csv") == 0) {
            csv = 1;
            // Check if a separator is provided
            if (argc == 6) {
                sep = argv[5][0];  // Use the first character of the separator
            }
        } else {
            // Invalid argument after the required three
            printf("Usage: %s tStart tEnd h [--csv separator]\n", argv[0]);
            return -1;
        }
    }

    INFO("tStart: %s, tEnd: %s, h: %s\n", argv[1], argv[2], argv[3]);
    if (csv) {
        INFO("CSV Mode enabled with separator: '%c'\n", sep);
    }
    

    // Simulation parameters
    tStart = atof(argv[1]);
    tEnd = atof(argv[2]);
    h = atof(argv[3]);

	loadFunctions(&fmu);

	// Initialize the simulation
	SimulationState *state = initializeSimulation(&fmu, tStart, tEnd, h);
	if (!state) {
		printf("Failed to initialize simulation\n");
		return -1;
	}

	// Run the simulation step by step
	while (state->time < state->tEnd && !state->eventInfo.terminateSimulation) {
		fmi2Status status = simulationDoStep(&fmu, state);
		if (status > fmi2Warning) {
			printf("Simulation step failed at time %g\n", state->time);
			break;
		}
	}

    // Print the output
    if (csv) {
        printCsv(state, sep);
    } else {
        printOutput(state);
    }

	// Cleanup and free resources
	cleanupSimulation(&fmu, state);

    return 0;
}