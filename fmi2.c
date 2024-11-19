//#include "headers/fmi2.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "headers/fmi2TypesPlatform.h"
#include "headers/fmi2FunctionTypes.h"
#include "headers/fmi2Functions.h"
#include "fmu/sources/config.h"
#include "fmu/sources/model.h"

#define CONCAT_(prefix, name) prefix##_##name
#define CONCAT(prefix, name) CONCAT_(prefix,name)
#define FUNCTION(name) CONCAT(MODEL_IDENTIFIER, name)


/**
 * @struct FMU
 * @brief Structure representing an FMU (Functional Mock-up Unit) with function pointers for various FMI (Functional Mock-up Interface) operations.
 * 
 * This structure contains function pointers for common functions, Co-Simulation functions, and Model Exchange functions as defined by the FMI 2.0 standard.
 * 
 * @var FMU::getTypesPlatform
 * Pointer to the function that returns the types platform.
 * 
 * @var FMU::getVersion
 * Pointer to the function that returns the version of the FMI standard.
 * 
 * @var FMU::setDebugLogging
 * Pointer to the function that enables or disables debug logging.
 * 
 * @var FMU::instantiate
 * Pointer to the function that instantiates the FMU.
 * 
 * @var FMU::freeInstance
 * Pointer to the function that frees the FMU instance.
 * 
 * @var FMU::setupExperiment
 * Pointer to the function that sets up the experiment.
 * 
 * @var FMU::enterInitializationMode
 * Pointer to the function that enters initialization mode.
 * 
 * @var FMU::exitInitializationMode
 * Pointer to the function that exits initialization mode.
 * 
 * @var FMU::terminate
 * Pointer to the function that terminates the FMU.
 * 
 * @var FMU::reset
 * Pointer to the function that resets the FMU.
 * 
 * @var FMU::getReal
 * Pointer to the function that gets real variables.
 * 
 * @var FMU::getInteger
 * Pointer to the function that gets integer variables.
 * 
 * @var FMU::getBoolean
 * Pointer to the function that gets boolean variables.
 * 
 * @var FMU::getString
 * Pointer to the function that gets string variables.
 * 
 * @var FMU::setReal
 * Pointer to the function that sets real variables.
 * 
 * @var FMU::setInteger
 * Pointer to the function that sets integer variables.
 * 
 * @var FMU::setBoolean
 * Pointer to the function that sets boolean variables.
 * 
 * @var FMU::setString
 * Pointer to the function that sets string variables.
 * 
 * @var FMU::getFMUstate
 * Pointer to the function that gets the FMU state.
 * 
 * @var FMU::setFMUstate
 * Pointer to the function that sets the FMU state.
 * 
 * @var FMU::freeFMUstate
 * Pointer to the function that frees the FMU state.
 * 
 * @var FMU::serializedFMUstateSize
 * Pointer to the function that gets the size of the serialized FMU state.
 * 
 * @var FMU::serializeFMUstate
 * Pointer to the function that serializes the FMU state.
 * 
 * @var FMU::deSerializeFMUstate
 * Pointer to the function that deserializes the FMU state.
 * 
 * @var FMU::getDirectionalDerivative
 * Pointer to the function that gets the directional derivative.
 * 
 * @var FMU::setRealInputDerivatives
 * Pointer to the function that sets real input derivatives (Co-Simulation).
 * 
 * @var FMU::getRealOutputDerivatives
 * Pointer to the function that gets real output derivatives (Co-Simulation).
 * 
 * @var FMU::doStep
 * Pointer to the function that performs a simulation step (Co-Simulation).
 * 
 * @var FMU::cancelStep
 * Pointer to the function that cancels a simulation step (Co-Simulation).
 * 
 * @var FMU::getStatus
 * Pointer to the function that gets the status (Co-Simulation).
 * 
 * @var FMU::getRealStatus
 * Pointer to the function that gets the real status (Co-Simulation).
 * 
 * @var FMU::getIntegerStatus
 * Pointer to the function that gets the integer status (Co-Simulation).
 * 
 * @var FMU::getBooleanStatus
 * Pointer to the function that gets the boolean status (Co-Simulation).
 * 
 * @var FMU::getStringStatus
 * Pointer to the function that gets the string status (Co-Simulation).
 * 
 * @var FMU::enterEventMode
 * Pointer to the function that enters event mode (Model Exchange).
 * 
 * @var FMU::newDiscreteStates
 * Pointer to the function that computes new discrete states (Model Exchange).
 * 
 * @var FMU::enterContinuousTimeMode
 * Pointer to the function that enters continuous time mode (Model Exchange).
 * 
 * @var FMU::completedIntegratorStep
 * Pointer to the function that completes an integrator step (Model Exchange).
 * 
 * @var FMU::setTime
 * Pointer to the function that sets the simulation time (Model Exchange).
 * 
 * @var FMU::setContinuousStates
 * Pointer to the function that sets continuous states (Model Exchange).
 * 
 * @var FMU::getDerivatives
 * Pointer to the function that gets derivatives (Model Exchange).
 * 
 * @var FMU::getEventIndicators
 * Pointer to the function that gets event indicators (Model Exchange).
 * 
 * @var FMU::getContinuousStates
 * Pointer to the function that gets continuous states (Model Exchange).
 * 
 * @var FMU::getNominalsOfContinuousStates
 * Pointer to the function that gets the nominal values of continuous states (Model Exchange).
 */
typedef struct {
    /***************************************************
    Common Functions
    ****************************************************/
    fmi2GetTypesPlatformTYPE         *getTypesPlatform;
    fmi2GetVersionTYPE               *getVersion;
    fmi2SetDebugLoggingTYPE          *setDebugLogging;
    fmi2InstantiateTYPE              *instantiate;
    fmi2FreeInstanceTYPE             *freeInstance;
    fmi2SetupExperimentTYPE          *setupExperiment;
    fmi2EnterInitializationModeTYPE  *enterInitializationMode;
    fmi2ExitInitializationModeTYPE   *exitInitializationMode;
    fmi2TerminateTYPE                *terminate;
    fmi2ResetTYPE                    *reset;
    fmi2GetRealTYPE                  *getReal;
    fmi2GetIntegerTYPE               *getInteger;
    fmi2GetBooleanTYPE               *getBoolean;
    fmi2GetStringTYPE                *getString;
    fmi2SetRealTYPE                  *setReal;
    fmi2SetIntegerTYPE               *setInteger;
    fmi2SetBooleanTYPE               *setBoolean;
    fmi2SetStringTYPE                *setString;
    fmi2GetFMUstateTYPE              *getFMUstate;
    fmi2SetFMUstateTYPE              *setFMUstate;
    fmi2FreeFMUstateTYPE             *freeFMUstate;
    fmi2SerializedFMUstateSizeTYPE   *serializedFMUstateSize;
    fmi2SerializeFMUstateTYPE        *serializeFMUstate;
    fmi2DeSerializeFMUstateTYPE      *deSerializeFMUstate;
    fmi2GetDirectionalDerivativeTYPE *getDirectionalDerivative;
    /***************************************************
    Functions for FMI2 for Co-Simulation
    ****************************************************/
    fmi2SetRealInputDerivativesTYPE  *setRealInputDerivatives;
    fmi2GetRealOutputDerivativesTYPE *getRealOutputDerivatives;
    fmi2DoStepTYPE                   *doStep;
    fmi2CancelStepTYPE               *cancelStep;
    fmi2GetStatusTYPE                *getStatus;
    fmi2GetRealStatusTYPE            *getRealStatus;
    fmi2GetIntegerStatusTYPE         *getIntegerStatus;
    fmi2GetBooleanStatusTYPE         *getBooleanStatus;
    fmi2GetStringStatusTYPE          *getStringStatus;
    /***************************************************
    Functions for FMI2 for Model Exchange
    ****************************************************/
    fmi2EnterEventModeTYPE                *enterEventMode;
    fmi2NewDiscreteStatesTYPE             *newDiscreteStates;
    fmi2EnterContinuousTimeModeTYPE       *enterContinuousTimeMode;
    fmi2CompletedIntegratorStepTYPE       *completedIntegratorStep;
    fmi2SetTimeTYPE                       *setTime;
    fmi2SetContinuousStatesTYPE           *setContinuousStates;
    fmi2GetDerivativesTYPE                *getDerivatives;
    fmi2GetEventIndicatorsTYPE            *getEventIndicators;
    fmi2GetContinuousStatesTYPE           *getContinuousStates;
    fmi2GetNominalsOfContinuousStatesTYPE *getNominalsOfContinuousStates;
} FMU;


/**
 * @brief Loads the function pointers for the FMU (Functional Mock-up Unit) structure.
 *
 * This function assigns the appropriate function pointers to the FMU structure
 * based on the FMI (Functional Mock-up Interface) standard. The function pointers
 * are cast to the appropriate types and assigned to the corresponding members
 * of the FMU structure.
 *
 * @param fmu A pointer to the FMU structure where the function pointers will be loaded.
 * @return Returns 0 on success.
 *
 * The function pointers loaded include:
 * - fmi2GetTypesPlatform
 * - fmi2GetVersion
 * - fmi2SetDebugLogging
 * - fmi2Instantiate
 * - fmi2FreeInstance
 * - fmi2SetupExperiment
 * - fmi2EnterInitializationMode
 * - fmi2ExitInitializationMode
 * - fmi2Terminate
 * - fmi2Reset
 * - fmi2GetReal
 * - fmi2GetInteger
 * - fmi2GetBoolean
 * - fmi2GetString
 * - fmi2SetReal
 * - fmi2SetInteger
 * - fmi2SetBoolean
 * - fmi2SetString
 * - fmi2GetFMUstate
 * - fmi2SetFMUstate
 * - fmi2FreeFMUstate
 * - fmi2SerializedFMUstateSize
 * - fmi2SerializeFMUstate
 * - fmi2DeSerializeFMUstate
 * - fmi2GetDirectionalDerivative
 *
 * If FMI_COSIMULATION is defined, additional function pointers for co-simulation are loaded:
 * - fmi2SetRealInputDerivatives
 * - fmi2GetRealOutputDerivatives
 * - fmi2DoStep
 * - fmi2CancelStep
 * - fmi2GetStatus
 * - fmi2GetRealStatus
 * - fmi2GetIntegerStatus
 * - fmi2GetBooleanStatus
 * - fmi2GetStringStatus
 *
 * If FMI_COSIMULATION is not defined, additional function pointers for model exchange are loaded:
 * - fmi2EnterEventMode
 * - fmi2NewDiscreteStates
 * - fmi2EnterContinuousTimeMode
 * - fmi2CompletedIntegratorStep
 * - fmi2SetTime
 * - fmi2SetContinuousStates
 * - fmi2GetDerivatives
 * - fmi2GetEventIndicators
 * - fmi2GetContinuousStates
 * - fmi2GetNominalsOfContinuousStates
 */

static int loadFunctions(FMU *fmu) {
    fmu->getTypesPlatform          = (fmi2GetTypesPlatformTYPE *)      fmi2GetTypesPlatform;
    fmu->getVersion                = (fmi2GetVersionTYPE *)            fmi2GetVersion;
    fmu->setDebugLogging           = (fmi2SetDebugLoggingTYPE *)       fmi2SetDebugLogging;
    fmu->instantiate               = (fmi2InstantiateTYPE *)           fmi2Instantiate;
    fmu->freeInstance              = (fmi2FreeInstanceTYPE *)          fmi2FreeInstance;
    fmu->setupExperiment           = (fmi2SetupExperimentTYPE *)       fmi2SetupExperiment;
    fmu->enterInitializationMode   = (fmi2EnterInitializationModeTYPE *) fmi2EnterInitializationMode;
    fmu->exitInitializationMode    = (fmi2ExitInitializationModeTYPE *) fmi2ExitInitializationMode;
    fmu->terminate                 = (fmi2TerminateTYPE *)             fmi2Terminate;
    fmu->reset                     = (fmi2ResetTYPE *)                 fmi2Reset;
    fmu->getReal                   = (fmi2GetRealTYPE *)               fmi2GetReal;
    fmu->getInteger                = (fmi2GetIntegerTYPE *)            fmi2GetInteger;
    fmu->getBoolean                = (fmi2GetBooleanTYPE *)            fmi2GetBoolean;
    fmu->getString                 = (fmi2GetStringTYPE *)             fmi2GetString;
    fmu->setReal                   = (fmi2SetRealTYPE *)               fmi2SetReal;
    fmu->setInteger                = (fmi2SetIntegerTYPE *)            fmi2SetInteger;
    fmu->setBoolean                = (fmi2SetBooleanTYPE *)            fmi2SetBoolean;
    fmu->setString                 = (fmi2SetStringTYPE *)             fmi2SetString;
    fmu->getFMUstate               = (fmi2GetFMUstateTYPE *)           fmi2GetFMUstate;
    fmu->setFMUstate               = (fmi2SetFMUstateTYPE *)           fmi2SetFMUstate;
    fmu->freeFMUstate              = (fmi2FreeFMUstateTYPE *)          fmi2FreeFMUstate;
    fmu->serializedFMUstateSize    = (fmi2SerializedFMUstateSizeTYPE *) fmi2SerializedFMUstateSize;
    fmu->serializeFMUstate         = (fmi2SerializeFMUstateTYPE *)     fmi2SerializeFMUstate;
    fmu->deSerializeFMUstate       = (fmi2DeSerializeFMUstateTYPE *)   fmi2DeSerializeFMUstate;
    fmu->getDirectionalDerivative  = (fmi2GetDirectionalDerivativeTYPE *) fmi2GetDirectionalDerivative;
#ifdef FMI_COSIMULATION
    fmu->setRealInputDerivatives   = (fmi2SetRealInputDerivativesTYPE *) fmi2SetRealInputDerivatives;
    fmu->getRealOutputDerivatives  = (fmi2GetRealOutputDerivativesTYPE *) fmi2GetRealOutputDerivatives;
    fmu->doStep                    = (fmi2DoStepTYPE *)                fmi2DoStep;
    fmu->cancelStep                = (fmi2CancelStepTYPE *)            fmi2CancelStep;
    fmu->getStatus                 = (fmi2GetStatusTYPE *)             fmi2GetStatus;
    fmu->getRealStatus             = (fmi2GetRealStatusTYPE *)         fmi2GetRealStatus;
    fmu->getIntegerStatus          = (fmi2GetIntegerStatusTYPE *)      fmi2GetIntegerStatus;
    fmu->getBooleanStatus          = (fmi2GetBooleanStatusTYPE *)      fmi2GetBooleanStatus;
    fmu->getStringStatus           = (fmi2GetStringStatusTYPE *)       fmi2GetStringStatus;
#else // FMI2 for Model Exchange
    fmu->enterEventMode            = (fmi2EnterEventModeTYPE *)        fmi2EnterEventMode;
    fmu->newDiscreteStates         = (fmi2NewDiscreteStatesTYPE *)     fmi2NewDiscreteStates;
    fmu->enterContinuousTimeMode   = (fmi2EnterContinuousTimeModeTYPE *) fmi2EnterContinuousTimeMode;
    fmu->completedIntegratorStep   = (fmi2CompletedIntegratorStepTYPE *) fmi2CompletedIntegratorStep;
    fmu->setTime                   = (fmi2SetTimeTYPE *)               fmi2SetTime;
    fmu->setContinuousStates       = (fmi2SetContinuousStatesTYPE *)   fmi2SetContinuousStates;
    fmu->getDerivatives            = (fmi2GetDerivativesTYPE *)        fmi2GetDerivatives;
    fmu->getEventIndicators        = (fmi2GetEventIndicatorsTYPE *)    fmi2GetEventIndicators;
    fmu->getContinuousStates       = (fmi2GetContinuousStatesTYPE *)   fmi2GetContinuousStates;
    fmu->getNominalsOfContinuousStates = (fmi2GetNominalsOfContinuousStatesTYPE *) fmi2GetNominalsOfContinuousStates;
#endif
    return 0;
}
