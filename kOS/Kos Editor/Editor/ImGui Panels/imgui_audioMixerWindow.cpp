/******************************************************************/
/*!
\file      imgui_audioMixerWindow.cpp
\author    Chiu Jun Jie
\par       junjie.c@digipen.edu
\date      Oct 03,2025
\brief     This file contains the implementation of the ImGui Audio Mixer Window
           - Allow importing and exporting of wav files
           - Provides Audio Visualisation
           - Allow Audio Transport Controls (Play, Pause, Stop, Loop)




Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "implot_internal.h"
#include "Editor.h"
#include <FMOD/fmod.hpp>
#include "Scene/SceneManager.h"
#include "AssetManager/AssetManager.h"
#include "imgui_internal.h"

#include <Editor/WindowFile.h>

#include <algorithm>
#include <cmath>

#if defined(_WIN32)
#include <commdlg.h> // GetOpenFileNameA
static std::filesystem::path OpenWavDialogWin32(const char* startDir) {
	OPENFILENAMEA ofn{};
	CHAR fileBuf[MAX_PATH] = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = fileBuf;
	ofn.nMaxFile = sizeof(fileBuf);
	ofn.lpstrInitialDir = startDir;
	// Double-NULL-terminated filter string
	static const char kFilter[] = "WAV files\0*.wav\0All files\0*.*\0\0";
	ofn.lpstrFilter = kFilter;
	ofn.nFilterIndex = 1;      // default to WAV filter
	ofn.lpstrDefExt = "wav";  // default extension if user types a filename
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if (GetOpenFileNameA(&ofn) == TRUE)
		return std::filesystem::path(ofn.lpstrFile);
	return {};
}
#endif


// FMOD Globals
static bool           gInitDone = false;
static FMOD::System* gSystem = nullptr;
static FMOD::Sound* gSound = nullptr;
static FMOD::Channel* gChannel = nullptr;
static FMOD::DSP* gFFT = nullptr;

static bool           gLoop = false;
static float          gVol01 = 1.0f;      // 0..1
static unsigned int   gLenMS = 0;
static bool           gPaused = false;
static char           gPath[512] = "";
// dB <-> linear helpers
static inline float DbToGain(float db) { return powf(10.0f, db / 20.0f); }
static inline float GainToDb(float gain) { return (gain > 0.f) ? 20.0f * log10f(gain) : -80.0f; }


// Visualizer Buffer Globl
static constexpr int  kFftBands = 256;              // spectrum bins to display (<= FFT length)
static std::array<float, kFftBands> gSpectrum{};    // averaged across channels
static float          gRms = 0.f;

static void AttachFFT()
{
	if (!gSystem || gFFT) return;

	// Create FFT DSP
	FMOD_RESULT r = gSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &gFFT);
	if (r != FMOD_OK || !gFFT)
	{
		gFFT = nullptr; return;
	}

	const int windowSize = 1024;
	gFFT->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, windowSize);
	gFFT->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_HANNING);

	// Attach to master channel
	FMOD::ChannelGroup* master = nullptr;
	gSystem->getMasterChannelGroup(&master);
	if (master)
	{
		master->addDSP(0, gFFT);
		gFFT->setBypass(false);
		gFFT->setActive(true);
	}
}


static void FmodUpdatePerFrame() {
	if (!gSystem) return;

	gSystem->update();

	if (gChannel) {
		bool playing = false;
		if (gChannel->isPlaying(&playing) == FMOD_OK && !playing) {
			gChannel = nullptr; gPaused = false;
		}
	}

	AttachFFT();

	//Read FFT Data
	gSpectrum.fill(0.0f);
	gRms = 0.0f;

	if (gFFT) {
		void* pdata = nullptr;
		unsigned int length = 0;
		// Parameter index for spectrum data in FFT DSP
		// (enum order: 0 WINDOWSIZE, 1 WINDOWTYPE, 2 SPECTRUMDATA)
		FMOD_RESULT r = gFFT->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, &pdata, &length, nullptr, 0);
		if (r == FMOD_OK && pdata) {
			auto* fft = reinterpret_cast<FMOD_DSP_PARAMETER_FFT*>(pdata);
			if (fft && fft->length > 0 && fft->numchannels > 0 && fft->spectrum) {
				const int bins = std::min<int>(fft->length, kFftBands);

				for (int b = 0; b < bins; ++b) {
					float acc = 0.f;
					for (int c = 0; c < fft->numchannels; ++c) {
						acc += fft->spectrum[c][b];
					}
					gSpectrum[b] = acc / float(fft->numchannels);
				}

				double acc2 = 0.0;
				for (int b = 0; b < bins; ++b) {
					double v = (double)gSpectrum[b];
					acc2 += v * v;
				}
				gRms = (bins > 0) ? float(std::sqrt(acc2 / double(bins))) : 0.0f;
				gRms = std::clamp(gRms, 0.0f, 1.0f);
			}
		}
	}

}

static void FmodEnsureInit() {
	if (gInitDone) return;
	if (FMOD::System_Create(&gSystem) != FMOD_OK)
	{
		gSystem = nullptr; return;
	}

	if (gSystem->init(512, FMOD_INIT_NORMAL, nullptr) != FMOD_OK) {
		gSystem->release();
		gSystem = nullptr;
		return;
	}
	gInitDone = true;
}

static void StopPlayback() {
	if (gChannel) {
		gChannel->stop();
		gChannel = nullptr;
	}
	gPaused = false;
}

static void UnloadCurrent() {
	StopPlayback();
	if (gSound) {
		gSound->release();
		gSound = nullptr;
	}
	gLenMS = 0;
}

static void LoadFromPath(const char* path) {

	//Check if its strictly wav file
	if (!path || !*path) return;
	std::filesystem::path fsPath(path);
	std::string ext = fsPath.extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext != ".wav") {
		return;
	}

	FmodEnsureInit();
	UnloadCurrent();

	FMOD_MODE mode = FMOD_DEFAULT | FMOD_CREATESTREAM | (gLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	if (gSystem && gSystem->createSound(path, mode, nullptr, &gSound) == FMOD_OK && gSound) {
		FMOD_SOUND_TYPE type = FMOD_SOUND_TYPE_UNKNOWN;
		FMOD_SOUND_FORMAT fmt;
		int channels = 0, bits = 0;
		if (gSound->getFormat(&type, &fmt, &channels, &bits) == FMOD_OK) {
			//Extra check if not wave file then unload
			if (type != FMOD_SOUND_TYPE_WAV) {
				UnloadCurrent();
				return;
			}
		}
		gSound->getLength(&gLenMS, FMOD_TIMEUNIT_MS);
	}
}

static void PlayLoaded() {
	if (!gSystem || !gSound) return;
	if (gChannel) { gChannel->setPaused(false); gPaused = false; return; }
	gSystem->playSound(gSound, nullptr, false, &gChannel);
	if (gChannel) { gChannel->setVolume(gVol01); gChannel->setPaused(false); gPaused = false; }
}

static void SetPaused(bool paused) {
	if (!gChannel) return;
	gChannel->setPaused(paused);
	gPaused = paused;
}

static std::string fmt_time(unsigned int ms) {
	unsigned t = ms / 1000, m = t / 60, s = t % 60;
	char buf[16]; snprintf(buf, sizeof(buf), "%u:%02u", m, s);
	return buf;
}

void loadAudio(AssetManager& am)
{

	am.GetAssetWatcher()->Pause();

	// Open file explorer
	//std::filesystem::path path = filewindow::m_OpenfileDialog(
	//	AssetManager::GetInstance()->GetAssetManagerDirectory().c_str()
	//);

	std::filesystem::path path;
#if defined(_WIN32)
	// Use WAV-only dialog on Windows
	path = OpenWavDialogWin32(am.GetAssetManagerDirectory().c_str());
#else
	path = filewindow::m_OpenfileDialog(assetManager->GetAssetManagerDirectory().c_str());
#endif

	if (!path.empty() && (path.filename().extension().string() == ".wav")) {
		strncpy(gPath, path.string().c_str(), sizeof(gPath));
		gPath[sizeof(gPath) - 1] = '\0';
		LoadFromPath(gPath);             // Load FMOD
	}
	else {
		if (path.empty()) {
			LOGGING_POPUP("No file selected");
		}
		else {
			LOGGING_ERROR("Wrong File Type, only .wav");
		}
	}

	am.GetAssetWatcher()->Resume();
}

static bool WavExport(const char* inPath, const char* outPath, float linearGain)
{
	if (!inPath || !*inPath || !outPath || !*outPath) return false;

	FMOD::System* sys = nullptr;
	FMOD_RESULT r = FMOD::System_Create(&sys);
	if (r != FMOD_OK || !sys) {
		return false;
	}

	// Write to WAV
	sys->setOutput(FMOD_OUTPUTTYPE_WAVWRITER_NRT);

	const unsigned int initFlags = FMOD_INIT_NORMAL | FMOD_INIT_STREAM_FROM_UPDATE;

	r = sys->init(512, initFlags, (void*)outPath);
	if (r != FMOD_OK) {
		sys->release();
		return false;
	}

	// Load source
	FMOD::Sound* snd = nullptr;
	r = sys->createSound(inPath, FMOD_DEFAULT | FMOD_LOOP_OFF, nullptr, &snd);
	if (r != FMOD_OK || !snd) {
		sys->close();
		sys->release();
		return false;
	}

	float srcFreq = 0.0f; int srcPrio = 0;
	snd->getDefaults(&srcFreq, nullptr); // freq from file header



	// Length
	unsigned int lenPCM = 0;
	snd->getLength(&lenPCM, FMOD_TIMEUNIT_PCM);

	// Play paused, apply gain, then unpause
	FMOD::Channel* ch = nullptr;
	r = sys->playSound(snd, nullptr, true, &ch);

	if (r != FMOD_OK || !ch) {
		snd->release();
		sys->close();
		sys->release();
		return false;
	}

	ch->setVolume(linearGain);
	ch->setPaused(false);

	// Render loop
	const int kMaxIters = 10 * 1000 * 1000;
	int iters = 0;
	while (iters++ < kMaxIters) {
		sys->update();

		unsigned int posPCM = 0;

		if (ch->getPosition(&posPCM, FMOD_TIMEUNIT_PCM) == FMOD_OK) {
			if (posPCM >= lenPCM) break;
		}

		bool playing = false;
		if (ch->isPlaying(&playing) == FMOD_OK && !playing) break;
	}

	// Ensure channel is stopped (finalize writer)
	ch->stop();

	// Cleanup
	snd->release();
	sys->close();
	sys->release();

	// If we hit the iteration cap, treat it as failure
	if (iters >= kMaxIters) {
		LOGGING_ERROR("FMOD export: exceeded max iterations (possible decode stall).");
		return false;
	}
	return true;
}

static float GetMasterGain() {
	if (!gSystem) {
		return 1.0f;
	}

	FMOD::ChannelGroup* master = nullptr;
	gSystem->getMasterChannelGroup(&master);
	float v = 1.0f;
	if (master) {
		master->getVolume(&v);
	}
	return v;
}

static std::string MakeOutPath(const char* inPath) {
	std::filesystem::path p(inPath);
	return (p.parent_path() / (p.stem().string() + "_exported.wav")).string();
}


void gui::ImGuiHandler::DrawAudioMixerWindow() {
	FmodEnsureInit();
	FmodUpdatePerFrame();

	ImGui::Begin("Audio Mixer");
	{
		// File Path Load
		ImGui::TextUnformatted("Path:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(380.0f);
		ImGui::InputText("##audio_path", gPath, IM_ARRAYSIZE(gPath));
		ImGui::SameLine();
		if (ImGui::Button("Browse WAV")) {
			loadAudio(m_assetManager); // opens file dialog here
		}

		ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical); ImGui::SameLine(0, 8);

		// Transport
		bool canPlay = (gSound != nullptr);
		bool canCtl = (gChannel != nullptr);

		if (!canPlay) {
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Play")) {
			PlayLoaded();
		}

		if (!canPlay) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();
		if (!canCtl) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button(gPaused ? "Resume" : "Pause")) {
			SetPaused(!gPaused);
		}

		ImGui::SameLine();

		if (ImGui::Button("Stop")) {
			StopPlayback();
		}

		if (!canCtl) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical); ImGui::SameLine(0, 8);

		// Loop + Master Volume
		if (ImGui::Checkbox("Loop", &gLoop)) {
			if (gSound) {
				gSound->setMode(FMOD_DEFAULT | FMOD_CREATESTREAM | (gLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF));
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(180.0f);

		if (ImGui::SliderFloat("Master", &gVol01, 0.0f, 1.0f, "%.2f")) {
			if (gChannel) {
				gChannel->setVolume(gVol01);
			}
		}

		ImGui::SameLine();
		ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
		ImGui::SameLine(0, 8);

		//Waev Exporter button
		if (gPath[0] != '\0') {
			if (ImGui::Button("Export WAV (FMOD)")) {

				float linear = GetMasterGain();
				std::string outPath = MakeOutPath(gPath);

				if (!WavExport(gPath, outPath.c_str(), linear)) {
					LOGGING_ERROR("FMOD export failed");
				}
				else {
					LOGGING_POPUP("Exported: %s", outPath.c_str());
				}
			}
		}
		else {
			ImGui::BeginDisabled();
			ImGui::Button("Export WAV (FMOD)");
			ImGui::EndDisabled();
		}

	}

	// Position / Scrub
	if (gSound) {
		unsigned int posMs = 0;
		if (gChannel) gChannel->getPosition(&posMs, FMOD_TIMEUNIT_MS);
		int posI = static_cast<int>(posMs);
		int lenI = static_cast<int>(gLenMS);
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
		if (ImGui::SliderInt("##pos", &posI, 0, lenI,
			(fmt_time((unsigned)posI) + " / " + fmt_time(gLenMS)).c_str())) {
			if (gChannel) gChannel->setPosition((unsigned)posI, FMOD_TIMEUNIT_MS);
		}
	}
	else {
		ImGui::TextDisabled("No audio loaded.");
	}

	// Audio Visualizer
	if (gSound) {
		if (ImGui::CollapsingHeader("Visualizer", ImGuiTreeNodeFlags_DefaultOpen)) {
			float maxMag = 0.f;
			for (float v : gSpectrum) maxMag = std::max(maxMag, v);
			float scale = (maxMag > 0.001f) ? (1.0f / maxMag) : 1.0f;

			static float scaled[kFftBands];
			for (int i = 0; i < kFftBands; ++i) scaled[i] = std::min(gSpectrum[i] * scale, 1.0f);

			ImGui::TextUnformatted("Spectrum");
			ImGui::PlotLines("##spectrum", scaled, kFftBands, 0, nullptr, 0.0f, 1.0f, ImVec2(ImGui::GetContentRegionAvail().x, 100));

			float db = (gRms > 0.000001f) ? 20.0f * std::log10(gRms) : -96.0f;
			float vu = std::clamp(gRms, 0.0f, 1.0f);
			ImGui::Text("Level: %.1f dBFS", db);
			ImGui::ProgressBar(vu, ImVec2(ImGui::GetContentRegionAvail().x, 12));
		}
	}

	//Audio mixer (UNITYYYY STYLEE)
	ImGui::Separator();

	ImVec2 availAll = ImGui::GetContentRegionAvail();
	float bottomHeight = 190.0f;
	float topHeight = std::max(120.0f, availAll.y - bottomHeight);

	if (ImGui::BeginChild("##MixerCanvas", ImVec2(availAll.x, topHeight), ImGuiChildFlags_Border)) {

		// Master Strip
		ImGui::BeginGroup();
		ImGui::TextDisabled("AudioMixer1");
		ImGui::Separator();

		ImGui::BeginGroup();
		ImGui::TextUnformatted("Master");

		// Strip area
		const float stripWidth = 120.0f;
		ImGui::BeginChild("##MasterStrip", ImVec2(stripWidth, topHeight - 48.0f), ImGuiChildFlags_Border);

		// Left: meter with dB grid; Right: fader
		ImGui::BeginGroup();
		// dB scale
		const float meterH = 160.0f;
		const float meterW = 18.0f;
		ImVec2 meterSize(meterW, meterH);

		// Compute meter fill
		float dbNow = (gRms > 1e-6f) ? 20.0f * std::log10(gRms) : -80.0f; // 0 dBFS .. -80 dBFS
		dbNow = std::clamp(dbNow, -80.0f, 0.0f);

		// Draw meter as bar (0 dB at top, -80 at bottom)
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 p0 = ImGui::GetCursorScreenPos();
		ImVec2 p1 = ImVec2(p0.x + meterSize.x, p0.y + meterSize.y);
		dl->AddRectFilled(p0, p1, IM_COL32(40, 40, 40, 255), 3.0f);

		// grid lines 0, -20, -40, -60, -80
		for (int g = 0; g <= 4; ++g) {
			float dbTick = -20.0f * g;
			float t = (dbTick - (-80.0f)) / (0.0f - (-80.0f)); // 0..1
			float y = p0.y + (1.0f - t) * meterSize.y;
			dl->AddLine(ImVec2(p0.x, y), ImVec2(p1.x, y), IM_COL32(90, 90, 90, 255));
		}

		// fill
		float tFill = (dbNow - (-80.0f)) / (0.0f - (-80.0f));
		float yFill = p0.y + (1.0f - tFill) * meterSize.y;
		dl->AddRectFilled(ImVec2(p0.x + 2, yFill), ImVec2(p1.x - 2, p1.y - 2), IM_COL32(90, 200, 90, 255), 2.0f);

		ImGui::SetCursorScreenPos(ImVec2(p1.x + 6, p0.y - 2));
		ImGui::Text("0");

		// move cursor to right of meter
		ImGui::SetCursorScreenPos(ImVec2(p1.x + 36, p0.y));
		ImGui::EndGroup();

		ImGui::SameLine();

		// Fader (in dB)
		static float faderDb = -80.0f;

		// Read back master gain to sync initial display once
		static bool faderInit = false;
		if (!faderInit && gSystem) {
			FMOD::ChannelGroup* master = nullptr; gSystem->getMasterChannelGroup(&master);
			if (master) {
				float gain = 1.0f; master->getVolume(&gain);
				faderDb = std::clamp(GainToDb(gain), -80.0f, 20.0f);
				faderInit = true;
			}
		}

		// Vertical slider
		ImGui::BeginGroup();
		ImGui::Text("");
		if (ImGui::VSliderFloat("##faderDb", ImVec2(24, meterH), &faderDb, -80.0f, 20.0f, "")) {

			// apply to FMOD master
			if (gSystem) {
				FMOD::ChannelGroup* master = nullptr;
				gSystem->getMasterChannelGroup(&master);

				if (master) {
					master->setVolume(DbToGain(faderDb));
				}
			}
		}
		ImGui::Text("%.1f dB", faderDb);
		ImGui::EndGroup();

		ImGui::Spacing();
		ImGui::Separator();
		static bool solo = false, mute = false, bypass = false;

		// master mute
		if (ImGui::Button("S", ImVec2(28, 22))) {
			solo = !solo;
		}

		ImGui::SameLine();

		if (ImGui::Button("M", ImVec2(28, 22))) {
			mute = !mute;

			if (gSystem) {
				FMOD::ChannelGroup* master = nullptr;
				gSystem->getMasterChannelGroup(&master);

				if (master) {
					master->setMute(mute);
				}
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("B", ImVec2(28, 22))) {
			bypass = !bypass;
			if (gFFT) {
				gFFT->setBypass(bypass);
			}
		}

		////// ALACEHOLDER FOR ATTENUATION
		//ImGui::Spacing();
		//ImGui::TextDisabled("Attenuation");

		ImGui::EndChild(); // ##MasterStrip
		ImGui::EndGroup(); // master card
		ImGui::EndGroup(); // section

		//// PLACEHOLDER FOR ADD MIXER
		//ImGui::SameLine();
		//ImGui::BeginChild("##EmptyMixerArea", ImVec2(ImGui::GetContentRegionAvail().x, topHeight - 48.0f), ImGuiChildFlags_Border);
		//ImGui::TextDisabled("Add..");
		//ImGui::EndChild();

	}
	ImGui::EndChild();

	//// Mixer / Snapshot / group / view 
	//if (ImGui::BeginChild("##MixerDock", ImVec2(availAll.x, bottomHeight), ImGuiChildFlags_Border)) {
	//	if (ImGui::BeginTable("##Dock", 4, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV, ImGui::GetContentRegionAvail())) {
	//		ImGui::TableSetupColumn("Mixers");
	//		ImGui::TableSetupColumn("Snapshots");
	//		ImGui::TableSetupColumn("Groups");
	//		ImGui::TableSetupColumn("Views");
	//		ImGui::TableNextRow();

	//		// Mixers
	//		ImGui::TableSetColumnIndex(0);
	//		ImGui::BeginChild("##MixersPane", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border);
	//		ImGui::Selectable("AudioMixer1 (Audio Listener)", true);
	//		ImGui::EndChild();

	//		// Snapshots
	//		ImGui::TableSetColumnIndex(1);
	//		ImGui::BeginChild("##SnapshotsPane", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border);
	//		ImGui::Selectable("Snapshot", false);
	//		ImGui::EndChild();

	//		// Groups
	//		ImGui::TableSetColumnIndex(2);
	//		ImGui::BeginChild("##GroupsPane", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border);
	//		ImGui::Selectable("Master", true);
	//		ImGui::EndChild();

	//		// Views
	//		ImGui::TableSetColumnIndex(3);
	//		ImGui::BeginChild("##ViewsPane", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Border);
	//		ImGui::Selectable("View", true);
	//		ImGui::EndChild();

	//		ImGui::EndTable();
	//	}
	//}
	//ImGui::EndChild();

	//ImGui::Separator();
	// **ADD CHANNEL*

	// **ADD STATUS OF CHANNELS***
	//{
	//    bool playing = false;
	//    if (gChannel) gChannel->isPlaying(&playing);
	//    ImGui::Separator();

	//}

	ImGui::End();
}
