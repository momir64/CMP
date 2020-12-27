#include <opencv2/opencv.hpp>
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <conio.h>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <io.h>
#include "buttons.hpp"
#include "pa.h"
#define IVICAH 83
#define IVICAW 20

using namespace cv;
using namespace std;
using namespace chrono;

class AREA {
public:
	int X1 = 0;
	int X2 = 0;
	int Y1 = 0;
	int Y2 = 0;
};

float FPS = 0;
int dokraja = 0;
int stiglodo = 0;
float volume = 0.5;
double dt = 0, delta;
bool audioset = false;
bool printed[6], hovered[6];
double pusteno = 0, trajanje;
float speed = 1, speedvideo = 1;
int velicina = 0, velicinavideo = 0;
int RSTEP = 51, GSTEP = 51, BSTEP = 51;
int width = 0, widthbox = 0, height = 0;
bool playing = true, playingvideo = true, playingprinted = true, mota = false, motatraka = false, motavideo = false, showvol = false,
malovelko = true, printmv = false, tisina = false, volprinted = false, muted = false, firstever = true, pustenol = true, pustenod = true,
pustenog = true, pustenodl = true, pustenos = true, pustenof = true, pustenof5 = true;
int color[] = { 40, 44, 42, 46, 41, 45, 43, 47, 100, 104, 102, 106, 101, 105, 103, 107 };
AREA VOL, BACK, PLAY, FRONT, HDSD, SLIDE;
string temp(MAX_PATH, NULL);
string absolutedir;
string NLINE;

void GetDir() {
	char ExePath[MAX_PATH];
	GetModuleFileNameA(NULL, ExePath, MAX_PATH);
	absolutedir = ExePath;
	while (absolutedir.back() != '\\')
		absolutedir.pop_back();
}
string cf(int c, int x) {
	if (!x)	return "\x1b[40m ";
	return "\x1b[" + to_string(color[3 * x + c - 2]) + "m ";
}
void SetUpConsole() {
	HANDLE OUT_HANDLE = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE INPUT_HANDLE = GetStdHandle(STD_INPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFOEX info;
	CONSOLE_FONT_INFOEX cfi;
	CONSOLE_CURSOR_INFO cci;
	DWORD tmp, mode;

	info.cbSize = sizeof(info);
	cfi.cbSize = sizeof(cfi);

	GetConsoleMode(INPUT_HANDLE, &mode);
	GetConsoleScreenBufferInfoEx(OUT_HANDLE, &info);
	GetCurrentConsoleFontEx(OUT_HANDLE, false, &cfi);

	info.ColorTable[0] = RGB(0, 0, 0);

	info.ColorTable[1] = RGB(255 - RSTEP * 4, 0, 0);
	info.ColorTable[4] = RGB(255 - RSTEP * 3, 0, 0);
	info.ColorTable[7] = RGB(255 - RSTEP * 2, 0, 0);
	info.ColorTable[10] = RGB(255 - RSTEP * 1, 0, 0);
	info.ColorTable[13] = RGB(255, 0, 0);

	info.ColorTable[2] = RGB(0, 255 - GSTEP * 4, 0);
	info.ColorTable[5] = RGB(0, 255 - GSTEP * 3, 0);
	info.ColorTable[8] = RGB(0, 255 - GSTEP * 2, 0);
	info.ColorTable[11] = RGB(0, 255 - GSTEP * 1, 0);
	info.ColorTable[14] = RGB(0, 255, 0);

	info.ColorTable[3] = RGB(0, 0, 255 - BSTEP * 4);
	info.ColorTable[6] = RGB(0, 0, 255 - BSTEP * 3);
	info.ColorTable[9] = RGB(0, 0, 255 - BSTEP * 2);
	info.ColorTable[12] = RGB(0, 0, 255 - BSTEP * 1);
	info.ColorTable[15] = RGB(0, 0, 255);

	wcscpy_s(cfi.FaceName, L"Consolas");
	if (!velicina) {
		cfi.dwFontSize.X = 6;
		cfi.dwFontSize.Y = 2;
	}
	else if (velicina == -1) {
		height = -21;
		widthbox = 80;
		cfi.dwFontSize.X = 15;
		cfi.dwFontSize.Y = 30;
		info.ColorTable[7] = RGB(204, 204, 204);
	}
	else {
		cfi.dwFontSize.X = 9;
		cfi.dwFontSize.Y = 3;
	}
	cfi.nFont = 0;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;

	cci.dwSize = 100;
	cci.bVisible = FALSE;

	SetCurrentConsoleFontEx(OUT_HANDLE, FALSE, &cfi);
	if (firstever) {
		if (velicina != -1)
			SetWindowLongA(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
		SetConsoleMode(OUT_HANDLE, ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);
		SetConsoleScreenBufferInfoEx(OUT_HANDLE, &info);
		SetConsoleCursorInfo(OUT_HANDLE, &cci);
		system(("MODE " + to_string(widthbox + IVICAW) + "," + to_string(3 * height + IVICAH)).c_str());
		FillConsoleOutputAttribute(OUT_HANDLE, NULL, (widthbox + IVICAW) * (3 * height + IVICAH), { 0,0 }, &tmp);
		SetConsoleMode(INPUT_HANDLE, ENABLE_EXTENDED_FLAGS | (mode & ~ENABLE_QUICK_EDIT_MODE) | ENABLE_PROCESSED_INPUT | ENABLE_MOUSE_INPUT);
		firstever = false;
	}
	if (velicina == -1) {
		cout << cf(0, 0) << "\rRezolucija videa je prevelika!" << endl << "Pritisinite bilo koje dugme za nastavak...";
		_getch();
		exit(-1);
	}
}

void HideCursor() {
	CONSOLE_CURSOR_INFO cinf;
	cinf.dwSize = 100;
	cinf.bVisible = false;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cinf);
}
bool Foreground() {
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return false;
	HWND curhwnd = GetConsoleWindow();
	if (curhwnd == NULL)
		return false;

	DWORD pid;
	if (GetWindowThreadProcessId(hwnd, &pid) == 0)
		return false;
	DWORD curpid;
	if (GetWindowThreadProcessId(curhwnd, &curpid) == 0)
		return false;

	return (pid == curpid);
}
bool operator==(COORD a, COORD b) {
	return a.X == b.X && a.Y == b.Y;
}
std::size_t DirNum(std::filesystem::path path) {
	return (std::size_t)std::distance(std::filesystem::directory_iterator{ path }, std::filesystem::directory_iterator{});
}
int simple(int x) {
	if (x > 229) return 5;
	if (x > 178) return 4;
	if (x > 127) return 3;
	if (x > 74)	return 2;
	if (x > 25)	return 1;
	return 0;
}
string pos(int y, int x) {
	return "\x1b[" + to_string(y) + ";" + to_string(x) + "H";
}
string nline(int x) {
	return "\n\x1b[" + to_string(x) + "G";
}

vector<short> zvuk;
float scale(float i) {
	if (abs(round(i) - i) <= 0.00001f)
		return zvuk[round(i)];
	int i1 = floor(i), i2 = ceil(i);
	float p = i - i1;
	return zvuk[i1] * p + zvuk[i2] * (1.0f - p);
}

uintmax_t audiosize = 0;
uintmax_t audiopusteno = 0;
void paFunc(const float *in, float *out, long frames, void *data) {
	if (audioset) {
		audiopusteno = pusteno * 44100;
		audioset = false;
	}
	if (!playingvideo || motavideo || tisina)
		for (long i = 0; i < frames; i++)
			*out++ = 0;
	else if (audiopusteno + frames < audiosize) {
		float volumetmp = muted ? 0 : volume * volume;
		if (speed == 1) {
			for (long i = 0; i < frames; i++)
				*out++ = (float)zvuk[audiopusteno + i] / MAXSHORT * volumetmp;
			audiopusteno += frames;
		}
		else {
			for (float i = 0; i < frames * speed; i += speed)
				*out++ = scale(audiopusteno + i) / MAXSHORT * volumetmp;
			audiopusteno += frames * speed;
		}
	}
}

bool InArea(COORD krd, AREA a) {
	return krd.X >= a.X1 && krd.X <= a.X2 && krd.Y >= a.Y1 && krd.Y <= a.Y2;
}

void mouse() {
	COORD krd;
	DWORD br = 0;
	INPUT_RECORD input;
	HANDLE INPUT_HANDLE = GetStdHandle(STD_INPUT_HANDLE);

	while (true) {
		ReadConsoleInputW(INPUT_HANDLE, &input, 1, &br);
		krd = input.Event.MouseEvent.dwMousePosition;

		bool tmp = hovered[0], voltr = false, muted1 = false;
		for (int i = 0; i < 6; i++)
			hovered[i] = false;

		if (InArea(krd, VOL)) {
			hovered[0] = true;
			if (!tmp) VOL.X2 += 27;
		}
		else if (InArea(krd, BACK)) hovered[1] = true;
		else if (InArea(krd, PLAY)) hovered[2] = true;
		else if (InArea(krd, FRONT)) hovered[3] = true;
		else if (InArea(krd, HDSD)) hovered[4] = true;
		else if (InArea(krd, SLIDE)) hovered[5] = true;

		if (!hovered[0] && tmp)
			VOL.X2 -= 27;

		if (input.Event.MouseEvent.dwButtonState & 1) {
			while (true) {
				if (hovered[5]) {
					mota = true;
					motavideo = true;
					motatraka = true;
					dt = trajanje * (min(max(krd.X, SLIDE.X1), SLIDE.X2 - 2) - SLIDE.X1) / (SLIDE.X2 - SLIDE.X1 - 2);
				}
				if (input.Event.MouseEvent.dwEventFlags & 1 && (voltr || (hovered[0] && krd.Y >= VOL.Y1 + 7 && krd.Y <= VOL.Y1 + 13 && krd.X >= VOL.X1 + 12 && krd.X <= VOL.X1 + 32)))
					volume = max(0, min((krd.X - VOL.X1 - 12), 20)) / 20.0,
					voltr = true;
				else if (hovered[0] && krd.X < 12)
					muted1 = true;

				ReadConsoleInputW(INPUT_HANDLE, &input, 1, &br);
				krd = input.Event.MouseEvent.dwMousePosition;
				if (!(input.Event.MouseEvent.dwButtonState & 1)) {
					if (hovered[2] && InArea(krd, PLAY))
						playing = !playing;
					else if (hovered[5]) {
						dt = trajanje * (min(max(krd.X, SLIDE.X1), SLIDE.X2 - 2) - SLIDE.X1) / (SLIDE.X2 - SLIDE.X1 - 2);
						mota = false;
					}
					else if (voltr)
						volume = max(0, min((krd.X - VOL.X1 - 12), 20)) / 20.0;
					else if (muted1 && InArea(krd, VOL) && krd.X < 12)
						if (volume)
							muted = !muted;
						else {
							volume = 0.05;
							if (muted) muted = !muted;
						}
					else if (hovered[1] && InArea(krd, BACK))
						speed = max(0.25, speed - 0.25);
					else if (hovered[3] && InArea(krd, FRONT))
						speed = min(2, speed + 0.25);
					else if (hovered[4] && InArea(krd, HDSD) && height < 87 && width < 195) {
						velicina = velicina ? 0 : 1;
						malovelko = !malovelko;
					}

					break;
				}
			}
		}
	}
}

string volIco(int x) {
	if (volume < 0.05 || muted) return mute[x];
	else if (volume >= 0.75) return vol75[x];
	else if (volume >= 0.50) return vol50[x];
	else if (volume >= 0.25) return vol25[x];
	return vol0[x];
}

string buttonpos(int x) {
	if (x == 0) return pos(VOL.Y1 + 1, VOL.X1 + 1);
	if (x == 1) return pos(BACK.Y1 + 1, BACK.X1 + 1);
	if (x == 2) return pos(PLAY.Y1 + 1, PLAY.X1 + 1);
	if (x == 3) return pos(FRONT.Y1 + 1, FRONT.X1 + 1);
	if (x == 4) return pos(HDSD.Y1 + 1, HDSD.X1 + 1);
	if (x == 5) return pos(SLIDE.Y1 + 1, SLIDE.X1 + 1 + stiglodo);
}

string button(int x, bool b = false) {
	int c = b ? 5 : 3;
	if (x == 0)	return buttonpos(x) + volIco(c);
	if (x == 1)	return buttonpos(x) + back[c];
	if (x == 2) return buttonpos(x) + (playing ? pause[c] : play[c]);
	if (x == 3)	return buttonpos(x) + front[c];
	if (x == 4)	return buttonpos(x) + (malovelko ? velko[c] : malo[c]);
	if (x == 5)	return buttonpos(x) + (b ? slide[5] : slide[4]);
}

void TimeTitle() {
	int sat2 = dt / 3600, minut2 = ((uintmax_t)dt % 3600) / 60, sekund2 = (uintmax_t)dt % 60;
	int sat1 = trajanje / 3600, minut1 = ((uintmax_t)trajanje % 3600) / 60, sekund1 = (uintmax_t)trajanje % 60;
	stringstream title;
	if (sat1)
		title << "  " << setfill('0') << setw(2) << sat2 << ":" << setw(2) << minut2 << ":" << setw(2) << sekund2 << " / " << setw(2) << sat1 << ":" << setw(2) << minut1 << ":" << setw(2) << sekund1;
	else
		title << "  " << setfill('0') << setw(2) << minut2 << ":" << setw(2) << sekund2 << " / " << setw(2) << minut1 << ":" << setw(2) << sekund1;
	if (speed != 1)
		title << "  x" << speed;
	SetConsoleTitleA(title.str().c_str());
}

string buttons() {
	string buf;
	bool tmpmv = printmv;
	for (int i = 0; i < 6; i++)
		if (i == 2 && playing != playingprinted)
			buf += button(i, hovered[i]),
			playingprinted = !playingprinted;
		else if (printed[i] != hovered[i])
			buf += button(i, hovered[i]),
			printed[i] = !printed[i];
		else if ((i == 4 && tmpmv) || (i == 0 && showvol) || (i == 0 && printed[0]))
			buf += button(i, hovered[i]);
	printmv = tmpmv ? false : printmv;

	if (showvol || printed[0]) {
		int idido = volume * 20;
		string voltr = pos(VOL.Y1 + 10, VOL.X1 + 12);
		for (int b = 0; b < 3; b++) {
			for (int i = 0; i < idido; i++)
				voltr += i == 0 ? cf(b, 3) : " ";
			voltr += cf(b, 5) + " ";
			for (int i = idido; i < 20; i++)
				voltr += i == idido ? cf(b, 1) : " ";
			if (b != 2)
				voltr += nline(VOL.X1 + 12);
		}
		buf += voltr;
		volprinted = true;
	}
	else if (volprinted) {
		int idido = volume * 20;
		string voltr = pos(VOL.Y1 + 10, VOL.X1 + 12);
		for (int b = 0; b < 3; b++) {
			voltr += cf(0, 0);
			for (int i = 0; i <= 20; i++)
				voltr += " ";
			if (b != 2)
				voltr += nline(VOL.X1 + 12);
		}
		buf += voltr;
		volprinted = false;
	}

	int idido = dt / trajanje * (SLIDE.X2 - SLIDE.X1 - 2);
	stiglodo = min(idido, stiglodo);
	string traka = pos(SLIDE.Y1 + 1, SLIDE.X1 + 1 + stiglodo);
	for (int h = 0; h < 2; h++) {
		for (int b = 0; b < 3; b++) {
			for (int i = stiglodo; i < idido; i++)
				traka += i == stiglodo ? cf(b, 3) : " ";
			traka += cf(b, printed[5] ? 5 : 4) + "  ";
			for (int i = idido + 2; (dokraja || motatraka) && i < SLIDE.X2 - SLIDE.X1; i++)
				traka += i == idido + 2 ? cf(b, 1) : " ";
			if (h != 1 || b != 2)
				traka += nline(5 + stiglodo);
		}
	}
	stiglodo = idido;
	buf += traka;

	if (mota != motatraka)
		motatraka = !motatraka;
	dokraja = max(0, dokraja - 1);

	TimeTitle();

	return buf;
}

void varset() {
	VOL.Y1 = 3 * height + IVICAH - 35; VOL.X1 = 4;
	VOL.Y2 = 3 * height + IVICAH - 14;  VOL.X2 = 11;
	BACK.Y1 = 3 * height + IVICAH - 35; BACK.X1 = (widthbox + IVICAW) / 2 - 19;
	BACK.Y2 = 3 * height + IVICAH - 14;  BACK.X2 = (widthbox + IVICAW) / 2 - 12;
	PLAY.Y1 = 3 * height + IVICAH - 41; PLAY.X1 = (widthbox + IVICAW) / 2 - 5;
	PLAY.Y2 = 3 * height + IVICAH - 8;  PLAY.X2 = (widthbox + IVICAW) / 2 + 5;
	FRONT.Y1 = 3 * height + IVICAH - 35; FRONT.X1 = (widthbox + IVICAW) / 2 + 12;
	FRONT.Y2 = 3 * height + IVICAH - 14;  FRONT.X2 = (widthbox + IVICAW) / 2 + 19;
	HDSD.Y1 = 3 * height + IVICAH - 35; HDSD.X1 = widthbox + IVICAW - 11;
	HDSD.Y2 = 3 * height + IVICAH - 14;  HDSD.X2 = widthbox + IVICAW - 5;
	SLIDE.Y1 = 3 * height + IVICAH - 57; SLIDE.X1 = 4;
	SLIDE.Y2 = 3 * height + IVICAH - 51;  SLIDE.X2 = widthbox + IVICAW - 5;
}

void layout() {
	for (int i = 0; i < 5; i++)
		cout << button(i);

	string traka = pos(SLIDE.Y1 + 1, SLIDE.X1 + 1);
	int idido = dt / trajanje * (SLIDE.X2 - SLIDE.X1 - 2);
	for (int h = 0; h < 2; h++) {
		for (int b = 0; b < 3; b++) {
			for (int i = 0; i < idido; i++)
				traka += i == 0 ? cf(b, 3) : " ";
			traka += cf(b, printed[5] ? 5 : 4) + "  ";
			for (int i = idido + 2; i < SLIDE.X2 - SLIDE.X1; i++)
				traka += i == idido + 2 ? cf(b, 1) : " ";
			if (h != 1 || b != 2)
				traka += nline(5);
		}
	}

	cout << traka;
}

void main(int argc, char **argv) {
	GetDir();
	HideCursor();
	ios_base::sync_with_stdio(false);
	GetTempPathA(MAX_PATH, temp.data());
	SetCurrentDirectoryA(absolutedir.c_str());
	string videoname = argc > 1 ? argv[1] : "demo.mp4";

	system(("ffmpeg.exe -y -i \"" + videoname + "\" -acodec pcm_s16le -f s16le -ac 1 -ar 44100 \"" + temp.c_str() + "ConsolePlayerAudio.pcm\" 2>NUL").c_str());
	audiosize = std::filesystem::file_size(temp.c_str() + string("ConsolePlayerAudio.pcm"));
	ifstream fajl(temp.c_str() + string("ConsolePlayerAudio.pcm"), ios::binary);
	zvuk.resize(audiosize / 2);
	fajl.read((char *)zvuk.data(), audiosize);
	Pa pa(paFunc, 0, 1, 44100, 0, NULL);
	audiosize /= 2;
	fajl.close();
	DeleteFileA((temp.c_str() + string("ConsolePlayerAudio.pcm")).c_str());

	high_resolution_clock::time_point pt, ptl = high_resolution_clock::now(), ptd = high_resolution_clock::now(),
		ptg = high_resolution_clock::now(), ptdl = high_resolution_clock::now(), hidevol = high_resolution_clock::now();
	bool setup = true, first = true, next = false;
	VideoCapture video(videoname);
	uintmax_t frame = 0;
	Mat img, imgold;
	string buf;

	while (true) {
		if (!pustenof5 && !GetAsyncKeyState(VK_F5))
			pustenof5 = true;
		if (pustenof5 && GetAsyncKeyState(VK_F5) && Foreground()) {
			velicina = 0;
			malovelko = true;
			firstever = true;
			SetUpConsole();
			layout();
			pustenof5 = false;
			first = true;
		}
		if (!pustenof && !GetAsyncKeyState('F'))
			pustenof = true;
		if (pustenof && GetAsyncKeyState('F') && Foreground() && height < 87 && width < 195) {
			velicina = velicina ? 0 : 1;
			malovelko = !malovelko;
			pustenof = false;
			printmv = true;
		}
		if (!pustenos && !GetAsyncKeyState(VK_SPACE))
			pustenos = true;
		if (pustenos && GetAsyncKeyState(VK_SPACE) && Foreground()) {
			pustenos = false;
			playing = !playing;
		}
		if (!pustenog && (!GetAsyncKeyState(VK_UP) || (duration_cast<duration<double>>(high_resolution_clock::now() - ptg)).count() > 0.1))
			pustenog = true;
		if (!pustenodl && (!GetAsyncKeyState(VK_DOWN) || (duration_cast<duration<double>>(high_resolution_clock::now() - ptdl)).count() > 0.1))
			pustenodl = true;
		if (!pustenod && (!GetAsyncKeyState(VK_RIGHT) || (duration_cast<duration<double>>(high_resolution_clock::now() - ptd)).count() > 0.1))
			pustenod = true;
		if (!pustenol && (!GetAsyncKeyState(VK_LEFT) || (duration_cast<duration<double>>(high_resolution_clock::now() - ptl)).count() > 0.1))
			pustenol = true;
		if (pustenog && GetAsyncKeyState(VK_UP) && Foreground()) {
			ptg = high_resolution_clock::now();
			volume = min(volume + 0.05, 1);
			pustenog = false;
			hidevol = high_resolution_clock::now();
			showvol = true;
		}
		if (pustenodl && GetAsyncKeyState(VK_DOWN) && Foreground()) {
			ptdl = high_resolution_clock::now();
			volume = max(volume - 0.05, 0);
			pustenodl = false;
			hidevol = high_resolution_clock::now();
			showvol = true;
		}
		if ((duration_cast<duration<double>>(high_resolution_clock::now() - hidevol)).count() > 0.3)
			showvol = false;
		if (pustenod && GetAsyncKeyState(VK_RIGHT) && Foreground()) {
			ptd = high_resolution_clock::now();
			pt = high_resolution_clock::now();
			pusteno = min(dt + 5, trajanje);
			frame = pusteno * FPS;
			video.set(CAP_PROP_POS_FRAMES, frame);
			pustenod = false;
			audioset = true;
			first = true;
		}
		if (pustenol && GetAsyncKeyState(VK_LEFT) && Foreground()) {
			ptl = high_resolution_clock::now();
			pt = high_resolution_clock::now();
			pusteno = max(dt - 5, 0);
			frame = pusteno * FPS;
			video.set(CAP_PROP_POS_FRAMES, frame);
			pustenod = false;
			audioset = true;
			first = true;
			dokraja++;
		}
		if (velicina != velicinavideo) {
			velicinavideo = velicina;
			SetUpConsole();
		}
		if (speedvideo != speed) {
			pusteno = dt;
			audioset = true;
			speedvideo = speed;
			pt = high_resolution_clock::now();
		}
		if (motavideo) {
			frame = min(dt, trajanje - 0.01) * FPS;
			video.set(CAP_PROP_POS_FRAMES, frame);
			first = true;
		}
		if (mota != motavideo) {
			pt = high_resolution_clock::now();
			pusteno = dt;
			frame = dt * FPS;
			motavideo = !motavideo;
			audioset = true;
		}
		if (playingvideo != playing) {
			if (!playingvideo) {
				pt = high_resolution_clock::now();
				playingvideo = true;
				audioset = true;
				first = true;
			}
			else {
				pusteno = dt;
				playingvideo = false;
			}
		}
		if (!playingvideo && !motavideo) {
			Sleep(1);
			cout << buttons();
			continue;
		}
		video.read(img);
		if (img.empty()) {
			dt = trajanje;
			tisina = true;
			Sleep(1);
			cout << buttons();
			continue;
		}
		tisina = false;
		if (setup) {
			widthbox = max(120, img.cols);
			width = img.cols;
			height = img.rows;
			NLINE = nline((IVICAW + widthbox - width) / 2 + 1);
			FPS = video.get(CAP_PROP_FPS);
			trajanje = video.get(CAP_PROP_FRAME_COUNT) / FPS;
			varset();
			if (height > 142 || width > 300)
				velicina = -1;
			SetUpConsole();
			thread Mover(mouse);
			Mover.detach();
			layout();
			pa.start();
			pt = high_resolution_clock::now();
		}
		while (!motavideo) {
			dt = pusteno + (duration_cast<duration<double>>(high_resolution_clock::now() - pt)).count() * speedvideo;
			delta = frame - dt * FPS;
			if (delta <= -1) {
				frame++;
				next = true;
			}
			else if (delta >= 1) {
				Sleep(1);
				continue;
			}
			break;
		}
		if (next) {
			next = false;
			continue;
		}

		buf = pos(12, (IVICAW + widthbox - width) / 2 + 1);
		int c[3], cx[3];

		for (int j = 0; j < height; j++) {
			bool f[]{ 1, 1, 1 };
			for (int cbr = 0; cbr < 3; cbr++) {
				for (int i = 0; i < width; i++) {
					int isto = 0;
					while (!first && i < width && simple(img.at<Vec3b>(j, i)[2 - cbr]) == simple(imgold.at<Vec3b>(j, i)[2 - cbr])) isto++, i++;
					if (i == width)
						break;
					if (isto)
						buf += "\x1b[" + to_string(isto) + "C";

					c[cbr] = simple(img.at<Vec3b>(j, i)[2 - cbr]);

					if (f[cbr] || c[cbr] != cx[cbr])
						buf += cf(cbr, c[cbr]);
					else
						buf += " ";
					f[cbr] = false;
					cx[cbr] = c[cbr];
				}
				if (j != height - 1 || cbr != 2)
					buf += NLINE;
			}
		}

		cout << buttons() << buf;
		frame++;

		imgold = img.clone();
		setup = false;
		first = false;
	}
}