#ifndef RUNACTIONMESSENGER_HH
#define RUNACTIONMESSENGER_HH

#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithAString;
class G4UIcommand;
class RunAction;

class RunActionMessenger : public G4UImessenger {
public:
  explicit RunActionMessenger(RunAction* runAction);
  virtual ~RunActionMessenger();

  virtual void SetNewValue(G4UIcommand* command, G4String newValue);

private:
  RunAction* fRunAction;
  G4UIdirectory* fRunDirectory;
  G4UIcmdWithAString* fSetOutputFileCmd;
};

#endif
