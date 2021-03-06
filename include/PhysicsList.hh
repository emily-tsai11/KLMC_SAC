// PhysicsList.hh
// --------------------------------------------------------------
// Based on GEANT 4 - PhysicsList class from Hadr01 example
// --------------------------------------------------------------
// History:
//
// Antonino Sergi (Antonino.Sergi@cern.ch)
// Sergey Podolsky (siarhei.padolski@cern.ch) 03-09-2012
// Adapted from PADME by Emily Tsai (emily.tsai11@gmail.com) 2020-7-13
// --------------------------------------------------------------

#ifndef PhysicsList_H
#define PhysicsList_H 1

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "G4VModularPhysicsList.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class G4Cerenkov;
class G4Scintillation;
class G4OpAbsorption;
class G4OpRayleigh;
class G4OpBoundaryProcess;
class G4OpWLS;
class G4VPhysicsConstructor;
class PhysicsListMessenger;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class PhysicsList : public G4VModularPhysicsList
{
	public:

		PhysicsList();
		virtual ~PhysicsList();

		static PhysicsList* GetInstance();

		void ConstructParticle();
		void ConstructProcess();
		void SetMessengerParam();
		void SetCuts();
		void SetCutForGamma(G4double);
		void SetCutForElectron(G4double);
		void SetCutForPositron(G4double);
		void SetCutForProton(G4double);

		void AddPhysicsList(const G4String& name);
		void OpticalPhysics();
		void List() {}
		void AddParameterisation();

		void SetBrPie2(G4double); // Set the branching ratio of the pi+- --> e+- nu decay
		G4double GetBrPie2() { return fBrPie2; }
		void SetMuonDecay(G4int); // Set muon decay mode
		G4String GetPhysicsListName() { return fPhysicsListName; }

	private:

		static PhysicsList* fInstance;

		void SetBuilderList1(G4bool flagHP = false);
		void SetBuilderList2();

		G4double fCutForGamma;
		G4double fCutForElectron;
		G4double fCutForPositron;
		G4double fCutForProton;

		// Optical processes
		G4Cerenkov* fCerenkovProcess;
		G4Scintillation* fScintillationProcess;
		G4OpAbsorption* fAbsorptionProcess;
		G4OpRayleigh* fRayleighScatteringProcess;
		G4OpBoundaryProcess* fBoundaryProcess;
		G4OpWLS* fWLSProcess;

		G4VPhysicsConstructor* fEmPhysicsList;
		G4VPhysicsConstructor* fParticleList;

		std::vector<G4VPhysicsConstructor*> fHadronPhys;

		PhysicsListMessenger* fMessenger;

		G4double fBrPie2; // Branching ratio of the pi+- --> e+- nu decay
		G4double fMDS; // Needed for HNL mode from D mesons
		G4String fPhysicsListName;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
