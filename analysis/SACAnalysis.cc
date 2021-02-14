// SACAnalysis.cc
// --------------------------------------------------------------
// run with "root -l -b SACAnalysis.cc(<BeamType>, <NEvents>)"
// --------------------------------------------------------------
// History:
//
// Created by Emily Tsai (emily.tsai11@gmail.com) 2020-7-20
// --------------------------------------------------------------

#include <map>

// -------------------- Constants -------------------- //

static const int NEnergies = 10;
static const string SEnergies[] = {
	"100MeV",
	"200MeV",
	"500MeV",
	"1GeV",
	"2GeV",
	"5GeV",
	"10GeV",
	"20GeV",
	"50GeV",
	"100GeV"
};
static const double DEnergies[] = { // in GeV
	0.1,
	0.2,
	0.5,
	1.0,
	2.0,
	5.0,
	10.0,
	20.0,
	50.0,
	100.0
};
static const double StdEnergies[] = { // in GeV
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0
};
static const double BEnergies[] = { // in GeV
	0.07,
	0.12,
	0.3,
	0.7,
	1.2,
	3.0,
	7.0,
	12.0,
	30.0,
	70.0,
	120.0
};

static const int NParticles = 14;
static const string SParticles[] = {
	"all",
	"e+",
	"e-",
	"gamma",
	"mu+",
	"mu-",
	"neutron",
	"opticalphoton",
	"other",
	"pi+",
	"pi-",
	"pi0",
	"proton",
	"untracked"
};
static const string SymbParticles[] = {
	"E_{total}",
	"e^{+}",
	"e^{-}",
	"#gamma",
	"#mu^{+}",
	"#mu^{-}",
	"n",
	"Cer. #gamma",
	"E_{other}",
	"#pi^{+}",
	"#pi^{-}",
	"#pi^{0}",
	"p",
	"E_{untracked}"
};

static const int SACNRows = 10;
static const int SACNCols = 10;
static const int SACNLayers = 4;

static const char* DrawOpt = "AP";
static const int Colors[] = {
	1,
	2,
	4,
	3,
	29,
	30,
	6,
	7,
	9,
	39,
	40,
	49,
	5,
	28
};
static const double MSize = 1.3;
static const int MStyle = 8;
static const int LWidth = 1;
static const int LStyle = 1;

static const int StackColors[] = {
	2,
	3,
	4,
	5
};

static string Beam;
static const string FOutName = "plots.pdf";

static bool firstPlot = true;
static bool lastPlot = false;

// -------------------- Data structures -------------------- //

static map<string, TFile*> MFiles;
static map<string, vector<double>> MData;

// -------------------- Helper methods -------------------- //

string GetBeamType(int bt)
{
	switch(bt)
	{
		case 1: return "kaon0L";
		case 2: return "kaon0L";
		case 3: return "neutron";
		default: return "gamma";
	}
}

void GetMeanAndStd(string h, vector<double> &m, vector<double> &std)
{
	for(int e = 0; e < NEnergies; e++)
	{
		TH1D* temp = (TH1D*) MFiles[SEnergies[e]]->Get(h.c_str());
		m[e] = temp->GetMean();
		std[e] = temp->GetStdDev();
	}
}

void ReadInGraphVals(string masterKey)
{
	vector<double> yValMean(NEnergies);
	vector<double> yValStd(NEnergies);
	string key;
	for(int p = 0; p < NParticles; p++)
	{
		key = masterKey + SParticles[p];
		GetMeanAndStd(key, yValMean, yValStd);
		MData[key + "_m"] = yValMean;
		MData[key + "_std"] = yValStd;
	}
}

void ReadInStackedBarVals(string masterKey)
{
	string key;

	// Read in values from files
	vector<double> yVal(SACNLayers);
	for(int p = 0; p < NParticles; p++)
	{
		key = masterKey + SParticles[p];
		for(int e = 0; e < NEnergies; e++)
		{
			TH1D* temp = (TH1D*) MFiles[SEnergies[e]]->Get(key.c_str());
			for(int l = 0; l < SACNLayers; l++)
				yVal[l] = temp->GetBinContent(l + 1);
			MData[key + SEnergies[e]] = yVal;
		}
	}

	// Rearrange values into what I need to plot them
	vector<double> yStackedVal(NEnergies);
	for(int p = 0; p < NParticles; p++)
	{
		key = masterKey + SParticles[p];
		for(int l = 0; l < SACNLayers; l++)
		{
			for(int e = 0; e < NEnergies; e++)
				yStackedVal[e] = MData[key + SEnergies[e]][l];
			MData[key + std::to_string(l)] = yStackedVal;
		}
	}
}

void DrawTGraphErrors(TGraphErrors* g, int p, string n, string t, string x,
	string y)
{
	g->SetName(n.c_str());
	g->SetTitle((t + ";" + x + ";" + y).c_str());

	g->SetMarkerColor(Colors[p]);
	g->SetMarkerSize(MSize);
	g->SetMarkerStyle(MStyle);
	g->SetLineWidth(LWidth);
	g->SetLineStyle(LStyle);

	TCanvas* c = new TCanvas(n.c_str(), n.c_str(), 800, 600);
	gPad->SetLogx(1);
	g->Draw(DrawOpt);

	string fname = Beam + "/" + Beam + FOutName;
	if(firstPlot) fname += "(";
	else if(lastPlot) fname += ")";

	c->Print(fname.c_str());
}

void DrawTMultiGraph(TMultiGraph* mg, TLegend* l, string n, string t, string x,
	string y)
{
	mg->SetName(n.c_str());
	mg->SetTitle((t + ";" + x + ";" + y).c_str());

	l->SetFillStyle(0);
	l->SetBorderSize(0);

	TCanvas* c = new TCanvas(n.c_str(), n.c_str(), 800, 600);
	gPad->SetLogx(1);
	mg->Draw(DrawOpt);
	l->Draw();

	string fname = Beam + "/" + Beam + FOutName;
	if(firstPlot) fname += "(";
	else if(lastPlot) fname += ")";

	c->Print(fname.c_str());
}

void DrawGraphs(string masterKey, string t, string x, string y)
{
	TMultiGraph* mg = new TMultiGraph();
	TLegend* legend = new TLegend(0.905, 0.2, 0.995, 0.8);

	string key;
	string title;
	for(int p = 0; p < NParticles; p++)
	{
		key = masterKey + SParticles[p];

		TGraphErrors* g = new TGraphErrors(NEnergies, DEnergies,
			MData[key + "_m"].data(), StdEnergies, MData[key + "_std"].data());
		DrawTGraphErrors(g, p, key, (SymbParticles[p] + " " + t).c_str(), x, y);
		firstPlot = false;

		mg->Add(g);
		legend->AddEntry(g, SymbParticles[p].c_str());
	}

	DrawTMultiGraph(mg, legend, masterKey, t, x, y);
}

void DrawStackedBar(string masterKey)
{
	string key;
	for(int p = 0; p < NParticles; p++)
	{
		key = masterKey + SParticles[p];
		THStack* hs = new THStack(key.c_str(),
			(SymbParticles[p] + " % Energy Deposition Per SAC Layer").c_str());
		for(int l = 0; l < SACNLayers; l++)
		{
			TH1D* temp = new TH1D((key + "_" + std::to_string(l)).c_str(),
				("z = " + std::to_string(l)).c_str(), NEnergies, BEnergies);
			for(int e = 0; e < NEnergies; e++)
				temp->AddBinContent(e + 1, MData[key + std::to_string(l)][e]);
			temp->SetFillColor(StackColors[l]);
			hs->Add(temp);
		}

		TCanvas* c = new TCanvas(key.c_str(), key.c_str(), 800, 600);
		gPad->SetLogx(1);
		hs->Draw();
		gPad->BuildLegend(0.905, 0.2, 0.995, 0.8);

		if(p == NParticles - 1) lastPlot = true;

		string fname = Beam + "/" + Beam + FOutName;
		if(firstPlot) fname += "(";
		else if(lastPlot) fname += ")";

		c->Print(fname.c_str());
	}
}

// -------------------- SAC Analysis -------------------- //

void SACAnalysis(int BeamType, int NEvents)
{
	// Read in input files
	Beam = GetBeamType(BeamType);
	string FName;
	for(int e = 0; e < NEnergies; e++)
	{
		FName = Beam + "/SAC_" + std::to_string(BeamType) + "_"
			+ SEnergies[e] + "_n" + std::to_string(NEvents) + ".root";
		MFiles[SEnergies[e]] = new TFile(FName.c_str());
	}

	// Read in y values
	ReadInGraphVals("h1EDep_PerEvent_");
	ReadInGraphVals("h1Mult_PerEvent_");
	ReadInStackedBarVals("h1EDep_PerLayer_");

	// Close input files
	for(int e = 0; e < NEnergies; e++) MFiles[SEnergies[e]]->Close();

	// Draw graphs
	DrawGraphs("h1EDep_PerEvent_", "% of Energy Deposition Per Event",
		"Incident Energy [MeV]", "[%]");
	DrawGraphs("h1Mult_PerEvent_", "Multiplicity Per Event",
		"Incident Energy [MeV]", "[1 / GeV]");
	DrawStackedBar("h1EDep_PerLayer_");
}

int main(int argc, char** argv)
{
	SACAnalysis(std::stoi(argv[0]), std::stoi(argv[1]));
	return 0;
}
