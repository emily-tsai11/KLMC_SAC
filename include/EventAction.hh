// EventAction.hh
// --------------------------------------------------------------
// History:
//
// Created by Emily Tsai (emily.tsai11@gmail.com) 2020-7-8
// --------------------------------------------------------------

#ifndef EventAction_H
#define EventAction_H 1

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4UserEventAction.hh"
#include "globals.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4Event;
class G4Timer;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class EventAction : public G4UserEventAction
{
	public:

		EventAction();
		~EventAction();

		void BeginOfEventAction(const G4Event*);
		void EndOfEventAction(const G4Event*);

		void SetEventID(G4int id) { fEventID = id; }
		G4long GetEventID() { return fEventID; }

	private:

		G4Timer* fTimer;
		G4long fEventID;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
