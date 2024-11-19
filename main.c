#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// Headers from the FMI standard
#include "headers/fmi2TypesPlatform.h"
#include "headers/fmi2FunctionTypes.h"
#include "headers/fmi2Functions.h"

// Headers from the FMU file
#include "fmu/sources/config.h"
#include "fmu/sources/model.h"

// Other necessary files
#include "fmi2.c"
#include "output.c"


//TODO: Make those values read by the Makefile

#define FMI_VERSION 2

#ifndef fmuGUID
#define fmuGUID "{1AE5E10D-9521-4DE3-80B9-D0EAAA7D5AF1}"
#endif

#ifndef DEBUG
#define INFO(message, ...)
#else
#define INFO(message, ...) do { \
	char buffer[256]; \
	snprintf(buffer, sizeof(buffer), message, ##__VA_ARGS__); \
	printf("%s", buffer); \
} while (0)
#endif


#define min(a,b) ((a)>(b) ? (b) : (a))

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
    double tEnd;                     // end time
    fmi2EventInfo eventInfo;         // event info
    ScalarVariable *variables;       // model variables
    int nVariables;                  // number of variables
    double **output;                 // output array
    int nSteps;                      // current step count
    int nTimeEvents;                 // number of time events
    int nStateEvents;                // number of state events
    int nStepEvents;                 // number of step events
    fmi2Boolean loggingOn;          // logging flag
} SimulationState;


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
	char *copy;
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
 * Simulates the behavior of an FMU (Functional Mock-up Unit) over a specified time period.
 *
 * @param fmu A pointer to the FMU to be simulated.
 * @param tEnd The end time of the simulation.
 * @param h The fixed step size for the simulation.
 * @param loggingOn A boolean flag indicating whether logging is enabled.
 * @param nCategories The number of logging categories.
 * @param categories An array of strings representing the logging categories.
 * @return An integer indicating the success (1) or failure (0) of the simulation.
 *
 * This function performs the following steps:
 * 1. Declares and initializes necessary variables.
 * 2. Instantiates the FMU.
 * 3. Sets up debug logging if required.
 * 4. Allocates memory for state variables and event indicators.
 * 5. Sets up the experiment parameters.
 * 6. Initializes the FMU.
 * 7. Performs event iteration to update discrete states.
 * 8. Enters continuous-time mode and starts the simulation loop.
 * 9. In each simulation step, it advances time, updates states, checks for events, and handles them.
 * 10. Cleans up by terminating the FMU, freeing allocated memory, and printing a simulation summary.
 *
 * The function uses the forward Euler method to advance the state variables.
 * It also handles time events, state events, and step events during the simulation.
 */
static int simulate(FMU *fmu, double tEnd, double h, fmi2Boolean loggingOn, int nCategories, char **categories) {
	INFO("Entering simulate\n");

	int i;
	double dt, tPre;
	fmi2Boolean timeEvent, stateEvent, stepEvent, terminateSimulation;
	double time;
	int nx;                          // number of state variables
	int nz;                          // number of state event indicators
	double *x = NULL;                // continuous states
	double *xdot = NULL;             // the corresponding derivatives in same order
	double *z = NULL;                // state event indicators
	double *prez = NULL;             // previous values of state event indicators
	fmi2EventInfo eventInfo;         // updated by calls to initialize and eventUpdate
	fmi2CallbackFunctions callbacks = {fmuLogger, calloc, free, NULL, fmu}; // called by the model during simulation
	fmi2Component c;                 // instance of the fmu
	fmi2Status fmi2Flag;             // return code of the fmu functions
	fmi2Real tStart = 0;             // start time
	fmi2Boolean toleranceDefined = fmi2False; // true if model description define tolerance
	fmi2Real tolerance = 0;          // used in setting up the experiment
	fmi2Boolean visible = fmi2False; // no simulator user interface
	int nSteps = 0;
	int nTimeEvents = 0;
	int nStepEvents = 0;
	int nStateEvents = 0;

	ScalarVariable * variables;
	int nVariables;

	INFO("Variables declared\n");

	// instantiate the fmu
	c = fmu->instantiate("BouncingBall", fmi2ModelExchange, fmuGUID, NULL, &callbacks, visible, loggingOn); //TODO: Parse the name
	if (!c) return error("could not instantiate model");

	INFO("FMU instantiated\n");

	//Set debug logging -- ignored for now TODO: Implement
	// if (nCategories > 0) {
	// 	fmi2Flag = fmu->setDebugLogging(c, fmi2True, nCategories, categories);
	// 	if (fmi2Flag > fmi2Warning) {
	// 		return error("could not initialize model; failed FMI set debug logging");
	// 	}
	// }

	//INFO("Debug logging set\n");

	// allocate memory
	nx = getNumberOfContinuousStates(c); // Parseable from the model description, but with a counter on the variables
	nz = getNumberOfEventIndicators(c); // Parseable from the model description

	x = (double *) calloc(nx, sizeof(double));
	xdot = (double *) calloc(nx, sizeof(double));
	if (nz > 0) {
		z = (double *) calloc(nz, sizeof(double));
		prez = (double *) calloc(nz, sizeof(double));
	}
	if ((!x || !xdot) || (nz > 0 && (!z || !prez))) return error("out of memory");

	INFO("Memory allocated\n");

	//TODO: Initialiser la structure de sortie

	get_variable_list(&variables);
	nVariables = get_variable_count();
	// Initialize the output array
	double **output = (double **)calloc(nVariables + 1, sizeof(double *));
	for (i = 0; i < nVariables + 1; i++) {
		output[i] = (double *)calloc((int)tEnd/h+10, sizeof(double)); //Au cas où +10, normallement on devrait pas en avoir besoin TODO: Vérifier
	}

	

	// setup
	time = tStart;
	fmi2Flag = fmu->setupExperiment(c, toleranceDefined, tolerance, tStart, fmi2True, tEnd);
	if (fmi2Flag > fmi2Warning) {
		return error("could not initialize model; failed FMI setup experiment");
	}

	INFO("Experiment setup\n");

	// initialize
	fmi2Flag = fmu->enterInitializationMode(c);
	if (fmi2Flag > fmi2Warning) {
		return error("could not initialize model; failed FMI enter initialization mode");
	}
	fmi2Flag = fmu->exitInitializationMode(c);
	if (fmi2Flag > fmi2Warning) {
		return error("could not initialize model; failed FMI exit initialization mode");
	}

	INFO("Initialization mode done\n");

	// event iteration
	eventInfo.newDiscreteStatesNeeded = fmi2True;
	eventInfo.terminateSimulation = fmi2False;
	while (eventInfo.newDiscreteStatesNeeded && !eventInfo.terminateSimulation) {
		// update discrete states
		fmi2Flag = fmu->newDiscreteStates(c, &eventInfo);
		if (fmi2Flag > fmi2Warning) return error("could not set a new discrete state");
	}

	INFO("Event iteration done\n");

	if (eventInfo.terminateSimulation) {
		printf("model requested termination at t=%.16g\n", time);
	} else {
		// enter Continuous-Time Mode
		fmi2Flag = fmu->enterContinuousTimeMode(c);
		if (fmi2Flag > fmi2Warning) return error("could not initialize model; failed FMI enter continuous time mode");

		INFO("Continuous time mode entered\n");
		// TODO: Handle the ouput of a row of values
		// Initialize the output array with variable values
		INFO("Initializing output array for %d variables\n", nVariables);
		for (i = 0; i < nVariables-1; i++) {
			printf("Getting variable %s\n", variables[i].name);
			if (variables[i].type == REAL) {
				fmu->getReal(c, &variables[i].valueReference, 1, &output[i + 1][0]);
			} else if (variables[i].type == INTEGER) {
				fmi2Integer intValue;
				fmu->getInteger(c, &variables[i].valueReference, 1, &intValue);
				output[i + 1][0] = (double)intValue;
			}
		}
		//fmu->getReal(c, 0, 1, &output[1][nSteps]);


		// enter the simulation loop
		while (time < tEnd) {
			INFO("Entering simulation loop\n");
			// get the current state and derivatives
			fmi2Flag = fmu->getContinuousStates(c, x, nx);
			if (fmi2Flag > fmi2Warning) return error("could not retrieve states");

			fmi2Flag = fmu->getDerivatives(c, xdot, nx);
			if (fmi2Flag > fmi2Warning) return error("could not retrieve derivatives");

			INFO("States and derivatives retrieved\n");

			// advance time
			tPre = time;
			time = min(time + h, tEnd);
			timeEvent = eventInfo.nextEventTimeDefined && time >= eventInfo.nextEventTime;
			if (timeEvent) time = eventInfo.nextEventTime;
			dt = time - tPre;
			fmi2Flag = fmu->setTime(c, time);
			if (fmi2Flag > fmi2Warning) return error("could not set time");

			INFO("Time set\n");

			// perform one step
			for (i = 0; i < nx; i++) x[i] += dt * xdot[i]; // forward Euler method
			fmi2Flag = fmu->setContinuousStates(c, x, nx);
			if (fmi2Flag > fmi2Warning) return error("could not set continuous states");
			if (loggingOn) printf("time = %g\n", time);

			INFO("Step performed\n");

			// check for state event
			for (i = 0; i < nz; i++) prez[i] = z[i];
			fmi2Flag = fmu->getEventIndicators(c, z, nz);
			if (fmi2Flag > fmi2Warning) return error("could not retrieve event indicators");
			stateEvent = fmi2False;
			for (i = 0; i < nz; i++) stateEvent = stateEvent || (prez[i] * z[i] < 0);

			INFO("State event checked\n");

			// check for step event
			fmi2Flag = fmu->completedIntegratorStep(c, fmi2True, &stepEvent, &terminateSimulation);
			if (fmi2Flag > fmi2Warning) return error("could not complete integrator step");
			if (terminateSimulation) {
				printf("model requested termination at t=%.16g\n", time);
				break; // success
			}

			INFO("Step event checked\n");

			// handle events
			if (timeEvent || stateEvent || stepEvent) {
				INFO("Event detected\n");
				fmi2Flag = fmu->enterEventMode(c);
				if (fmi2Flag > fmi2Warning) return error("could not enter event mode");

				if (timeEvent) {
					INFO("Time event\n");
					nTimeEvents++;
					if (loggingOn) printf("time event at t=%.16g\n", time);
				}
				if (stateEvent) {
					INFO("State event\n");
					nStateEvents++;
					if (loggingOn) for (i = 0; i < nz; i++) printf("state event %s z[%d] at t=%.16g\n", (prez[i] > 0 && z[i] < 0) ? "-\\-" : "-/-", i, time);
				}
				if (stepEvent) {
					INFO("Step event\n");
					nStepEvents++;
					if (loggingOn) printf("step event at t=%.16g\n", time);
				}
				INFO("Event handled\n");

				// event iteration in one step, ignoring intermediate results
				eventInfo.newDiscreteStatesNeeded = fmi2True;
				eventInfo.terminateSimulation = fmi2False;
				while (eventInfo.newDiscreteStatesNeeded && !eventInfo.terminateSimulation) {
					// update discrete states
					fmi2Flag = fmu->newDiscreteStates(c, &eventInfo);
					if (fmi2Flag > fmi2Warning) return error("could not set a new discrete state");

					// check for change of value of states
					if (eventInfo.valuesOfContinuousStatesChanged && loggingOn) {
						printf("continuous state values changed at t=%.16g\n", time);
					}
					if (eventInfo.nominalsOfContinuousStatesChanged && loggingOn) {
						printf("nominals of continuous state changed  at t=%.16g\n", time);
					}
				}
				if (eventInfo.terminateSimulation) {
					printf("model requested termination at t=%.16g\n", time);
					break; // success
				}

				// enter Continuous-Time Mode
				fmi2Flag = fmu->enterContinuousTimeMode(c);
				if (fmi2Flag > fmi2Warning) return error("could not enter continuous time mode");
			} // if event
			for (i = 0; i < nVariables-1; i++) {
				if (variables[i].type == REAL) {
					fmu->getReal(c, &variables[i].valueReference, 1, &output[i + 1][nSteps]);
				} else if (variables[i].type == INTEGER) {
					fmi2Integer intValue;
					fmu->getInteger(c, &variables[i].valueReference, 1, &intValue);
					output[i + 1][nSteps] = (double)intValue;
				}
			}
			nSteps++;
		} // while
	}
	// cleanup
	fmi2Flag = fmu->terminate(c);
	if (fmi2Flag > fmi2Warning) return error("could not terminate model");
	
	fmu->freeInstance(c);
	if (x != NULL) free(x);
    if (xdot != NULL) free(xdot);
    if (z != NULL) free(z);
    if (prez != NULL) free(prez);

	//print simulation summary
    printf("Simulation from %g to %g terminated successful\n", tStart, tEnd);
    printf("  steps ............ %d\n", nSteps);
    printf("  fixed step size .. %g\n", h);
    printf("  time events ...... %d\n", nTimeEvents);
    printf("  state events ..... %d\n", nStateEvents);
    printf("  step events ...... %d\n", nStepEvents);

	//print the output
	for (i = 0; i < nVariables - 1; i++) {
		printf("%s: ", variables[i+1].name);
		for (int j = 0; j < nSteps; j++) {
			printf("%f ", output[i+1][j]);
		}
		printf("\n");
	}
	
	//free the output
	for (i = 0; i < nVariables + 1; i++) {
		free(output[i]);
	}
	free(output);

    return 1; // success
};

/**
 * @brief Initializes the FMU simulation and returns a simulation state structure.
 *
 * @param fmu Pointer to the FMU structure
 * @param tEnd End time for simulation
 * @param h Step size
 * @param loggingOn Flag for logging
 * @param nCategories Number of logging categories
 * @param categories Array of logging category strings
 * @return SimulationState* Pointer to initialized simulation state, NULL if error
 */
SimulationState* initializeSimulation(FMU *fmu, double tEnd, double h, 
                                    fmi2Boolean loggingOn, int nCategories, 
                                    char **categories) {
    SimulationState *state = (SimulationState*)calloc(1, sizeof(SimulationState));
    if (!state) return NULL;

    state->time = 0;
    state->h = h;
    state->tEnd = tEnd;
    state->nSteps = 0;
    state->nTimeEvents = 0;
    state->nStateEvents = 0;
    state->nStepEvents = 0;
    state->loggingOn = loggingOn;

    // Setup callback functions
    fmi2CallbackFunctions callbacks = {fmuLogger, calloc, free, NULL, fmu};

    // Instantiate the FMU
    state->component = fmu->instantiate("BouncingBall", fmi2ModelExchange, 
                                      fmuGUID, NULL, &callbacks, fmi2False, loggingOn);
    if (!state->component) {
        free(state);
        return NULL;
    }

    // Get state dimensions
    state->nx = getNumberOfContinuousStates(state->component);
    state->nz = getNumberOfEventIndicators(state->component);

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
        if (state->x) free(state->x);
        if (state->xdot) free(state->xdot);
        if (state->z) free(state->z);
        if (state->prez) free(state->prez);
        fmu->freeInstance(state->component);
        free(state);
        return NULL;
    }

    // Setup experiment
    fmi2Boolean toleranceDefined = fmi2False;
    fmi2Real tolerance = 0;
    fmi2Status fmi2Flag = fmu->setupExperiment(state->component, toleranceDefined, 
                                              tolerance, state->time, fmi2True, tEnd);
    if (fmi2Flag > fmi2Warning) {
        // Cleanup and return on setup failure
        fmu->freeInstance(state->component);
        free(state);
        return NULL;
    }

    // Initialize the FMU
    fmi2Flag = fmu->enterInitializationMode(state->component);
    if (fmi2Flag > fmi2Warning) {
        fmu->freeInstance(state->component);
        free(state);
        return NULL;
    }

    fmi2Flag = fmu->exitInitializationMode(state->component);
    if (fmi2Flag > fmi2Warning) {
        fmu->freeInstance(state->component);
        free(state);
        return NULL;
    }

    // Initial event iteration
    state->eventInfo.newDiscreteStatesNeeded = fmi2True;
    state->eventInfo.terminateSimulation = fmi2False;
    while (state->eventInfo.newDiscreteStatesNeeded && 
           !state->eventInfo.terminateSimulation) {
        fmi2Flag = fmu->newDiscreteStates(state->component, &state->eventInfo);
        if (fmi2Flag > fmi2Warning) {
            fmu->freeInstance(state->component);
            free(state);
            return NULL;
        }
    }

    if (!state->eventInfo.terminateSimulation) {
        fmi2Flag = fmu->enterContinuousTimeMode(state->component);
        if (fmi2Flag > fmi2Warning) {
            fmu->freeInstance(state->component);
            free(state);
            return NULL;
        }
    }

    // Initialize variables and output array
    get_variable_list(&state->variables);
    state->nVariables = get_variable_count();
    state->output = (double**)calloc(state->nVariables + 1, sizeof(double*));
    for (int i = 0; i < state->nVariables + 1; i++) {
        state->output[i] = (double*)calloc((int)tEnd/h + 10, sizeof(double));
    }

    // Initialize first output values
    for (int i = 0; i < state->nVariables - 1; i++) {
        if (state->variables[i].type == REAL) {
            fmu->getReal(state->component, &state->variables[i].valueReference, 
                        1, &state->output[i + 1][0]);
        } else if (state->variables[i].type == INTEGER) {
            fmi2Integer intValue;
            fmu->getInteger(state->component, &state->variables[i].valueReference, 
                           1, &intValue);
            state->output[i + 1][0] = (double)intValue;
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
    for (int i = 0; i < state->nVariables - 1; i++) {
        if (state->variables[i].type == REAL) {
            fmu->getReal(state->component, &state->variables[i].valueReference, 
                        1, &state->output[i + 1][state->nSteps]);
			INFO("DEBUG:   %s (ref %d): %f\n", state->variables[i].name, 
                   state->variables[i].valueReference, state->output[i + 1][state->nSteps]);
        } else if (state->variables[i].type == INTEGER) {
            fmi2Integer intValue;
            fmu->getInteger(state->component, &state->variables[i].valueReference, 
                           1, &intValue);
            state->output[i + 1][state->nSteps] = (double)intValue;
        }
    }

    state->nSteps++;
    return fmi2OK;
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

    // Print simulation summary
    printf("Simulation from %g to %g terminated successfully\n", 0.0, state->tEnd);
    printf("  steps ............ %d\n", state->nSteps);
    printf("  fixed step size .. %g\n", state->h);
    printf("  time events ...... %d\n", state->nTimeEvents);
    printf("  state events ..... %d\n", state->nStateEvents);
    printf("  step events ...... %d\n", state->nStepEvents);

    // Print the output
    for (int i = 0; i < state->nVariables-1; i++) {
        printf("%s: ", state->variables[i+1].name);
        for (int j = 0; j < state->nSteps; j++) {
            printf("%f ", state->output[i+1][j]);
        }
        printf("\n");
    }

    // Free the state structure itself
    free(state);
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
// int main(int argc, char *argv[]) {
// 	double tEnd = 3;
// 	double h = 0.01;
// 	int loggingOn;
// 	int nCategories=4+1; //TODO: This should come from a struct that is created by the makefile and read from the xml file
// 	char **logCategories;

// 	loadFunctions(&fmu);

// 	simulate(&fmu, tEnd, h, loggingOn, nCategories, logCategories);

// 	return 0;
// }
int main(int argc, char *argv[]) {
	// Liste des paramètres à récupérer
	// tStart, tEnd, h, True/False

	if (argc < 5) {
		printf("Usage: %s tStart tEnd h Step/Whole\n", argv[0]);
		return -1;
	}


    // Simulation parameters
    double tEnd = atof(argv[2]);
    double h = atof(argv[3]);
    fmi2Boolean loggingOn = fmi2False;
    int nCategories = 2; // TODO: This should come from a struct created by the makefile and read from the xml file
    char **logCategories = NULL;

	if (strcmp(argv[4], "Step") == 0) {
    // Load FMU functions
		loadFunctions(&fmu);

		// Initialize the simulation
		SimulationState *state = initializeSimulation(&fmu, tEnd, h, loggingOn, nCategories, logCategories);
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

		// Cleanup and free resources
		cleanupSimulation(&fmu, state);
	} else if (strcmp(argv[4], "Whole") == 0) {
		loadFunctions(&fmu);
		simulate(&fmu, tEnd, h, loggingOn, nCategories, logCategories);
	} else {
		printf("Invalid simulation mode: %s\nCorrect mode are Step/Whole", argv[4]);
		return -1;
	}

    return 0;
}