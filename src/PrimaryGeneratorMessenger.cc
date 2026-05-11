// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "PrimaryGeneratorMessenger.hh"

#include "PrimaryGeneratorAction.hh"

#include "G4ApplicationState.hh"
#include "G4Exception.hh"
#include "G4StateManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "G4UIdirectory.hh"
#include "G4UIparameter.hh"
#include "G4UIcommand.hh"

#include <sstream>

PrimaryGeneratorMessenger::PrimaryGeneratorMessenger(PrimaryGeneratorAction* generatorAction)
  : G4UImessenger(),
    fGeneratorAction(generatorAction),
    fGeneratorDirectory(0),
    fAddPdgRangeCmd(0),
    fClearPdgsCmd(0),
    fListPdgsCmd(0) {
  fGeneratorDirectory = new G4UIdirectory("/toyG4/generator/");
  fGeneratorDirectory->SetGuidance("Primary generator configuration.");

  fAddPdgRangeCmd = new G4UIcommand("/toyG4/generator/addPdgRange", this);
  fAddPdgRangeCmd->SetGuidance("Add a kinetic-energy range for one PDG code.");
  fAddPdgRangeCmd->SetGuidance("Usage: /toyG4/generator/addPdgRange <pdg> <emin> <emax> <unit> [linear|log]");
  fAddPdgRangeCmd->SetGuidance("Multiple entries for the same PDG are allowed.");
  fAddPdgRangeCmd->SetGuidance("Sampling mode is optional and defaults to 'linear'.");

  G4UIparameter* pdgParam = new G4UIparameter("pdg", 'i', false);
  pdgParam->SetGuidance("PDG code (e.g. 13, -13, 11, 2212).");
  fAddPdgRangeCmd->SetParameter(pdgParam);

  G4UIparameter* eminParam = new G4UIparameter("emin", 'd', false);
  eminParam->SetGuidance("Minimum kinetic energy.");
  fAddPdgRangeCmd->SetParameter(eminParam);

  G4UIparameter* emaxParam = new G4UIparameter("emax", 'd', false);
  emaxParam->SetGuidance("Maximum kinetic energy.");
  fAddPdgRangeCmd->SetParameter(emaxParam);

  G4UIparameter* unitParam = new G4UIparameter("unit", 's', false);
  unitParam->SetGuidance("Energy unit, e.g. MeV or GeV.");
  fAddPdgRangeCmd->SetParameter(unitParam);

  G4UIparameter* modeParam = new G4UIparameter("mode", 's', true);
  modeParam->SetGuidance("Sampling mode: 'linear' (default) or 'log'.");
  modeParam->SetDefaultValue("linear");
  fAddPdgRangeCmd->SetParameter(modeParam);

  fAddPdgRangeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fClearPdgsCmd = new G4UIcmdWithoutParameter("/toyG4/generator/clearPdgs", this);
  fClearPdgsCmd->SetGuidance("Clear all configured PDG ranges.");
  fClearPdgsCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fListPdgsCmd = new G4UIcmdWithoutParameter("/toyG4/generator/listPdgs", this);
  fListPdgsCmd->SetGuidance("Print all configured PDG ranges.");
  fListPdgsCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

PrimaryGeneratorMessenger::~PrimaryGeneratorMessenger() {
  delete fAddPdgRangeCmd;
  delete fClearPdgsCmd;
  delete fListPdgsCmd;
  delete fGeneratorDirectory;
}

void PrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
  if (command == fAddPdgRangeCmd) {
    std::istringstream input(newValue);
    G4int pdg = 0;
    G4double emin = 0.;
    G4double emax = 0.;
    G4String unit;
    G4String modeStr = "linear";
    input >> pdg >> emin >> emax >> unit;

    if (!input || unit.empty()) {
      G4Exception("PrimaryGeneratorMessenger::SetNewValue",
                  "ToyG4Gen001",
                  JustWarning,
                  "Invalid addPdgRange syntax. Use: <pdg> <emin> <emax> <unit> [linear|log]");
      return;
    }

    // mode is optional; ignore stream failure if absent
    input >> modeStr;

    const G4double unitScale = G4UIcommand::ValueOf(unit);
    if (unitScale <= 0.) {
      G4Exception("PrimaryGeneratorMessenger::SetNewValue",
                  "ToyG4Gen002",
                  JustWarning,
                  "Unknown unit in addPdgRange command.");
      return;
    }

    fGeneratorAction->SetEnergyRangeForPdg(pdg, emin * unitScale, emax * unitScale,
                                            modeStr == "log"
                                              ? PrimaryGeneratorAction::kLog
                                              : PrimaryGeneratorAction::kLinear);
    return;
  }

  if (command == fClearPdgsCmd) {
    fGeneratorAction->ClearConfiguredPdgs();
    return;
  }

  if (command == fListPdgsCmd) {
    fGeneratorAction->PrintConfiguration();
    return;
  }

  G4Exception("PrimaryGeneratorMessenger::SetNewValue",
              "ToyG4Gen003",
              JustWarning,
              "Unknown generator command.");
}
