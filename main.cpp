#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>

#define w_global 46	//better % 2
#define h_global 35
#define f_global 24

#define white "\033[38;5;255m"
#define gray "\033[38;5;245m"
#define red "\033[38;5;124m"

void console_config(short width, short height, short f_sizeX, short f_sizeY, short centerize, COLORREF bg_clr = RGB(0, 0, 0), int f_style = FW_NORMAL, const wchar_t* font = L"", const wchar_t* title = L"") {
	system(0);
	HWND hwnd = GetConsoleWindow();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	RECT old_rect;
	GetWindowRect(hwnd, &old_rect);
	int oldc_width = old_rect.right - old_rect.left;
	int oldc_height = old_rect.bottom - old_rect.top;

	CONSOLE_FONT_INFOEX cfi{};
	cfi.cbSize = sizeof(cfi);
	cfi.dwFontSize.X = f_sizeX;
	cfi.dwFontSize.Y = f_sizeY;
	cfi.FontWeight = f_style;
	wcscpy_s(cfi.FaceName, font);
	SetCurrentConsoleFontEx(handle, FALSE, &cfi);

	CONSOLE_SCREEN_BUFFER_INFOEX csbi;
	csbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(handle, &csbi);
	csbi.dwSize.X = width;
	csbi.dwSize.Y = height;
	csbi.srWindow = { 0, 0, width, height };
	csbi.dwMaximumWindowSize = { width, height };
	for (int i = 0; i < 16; i++)
		csbi.ColorTable[i] = bg_clr;
	SetConsoleScreenBufferInfoEx(handle, &csbi);

	DWORD mode;
	GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode);
	mode &= ~(ENABLE_QUICK_EDIT_MODE | ENABLE_INSERT_MODE);
	SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), mode);

	SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
	ShowScrollBar(hwnd, SB_BOTH, 0);
	if (title != L"")
		SetConsoleTitle(title);

	RECT rect;
	GetWindowRect(hwnd, &rect);
	int c_width = rect.right - rect.left;
	int c_height = rect.bottom - rect.top;
	if (centerize == 1 && (GetSystemMetrics(SM_CXSCREEN) / 2 - c_width / 2 != rect.left || GetSystemMetrics(SM_CYSCREEN) / 2 - c_height / 2 != rect.top))
		SetWindowPos(hwnd, 0, GetSystemMetrics(SM_CXSCREEN) / 2 - c_width / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - c_height / 2, c_width, c_height, 0);
	if (centerize == 2 && (c_width != oldc_width || c_height != oldc_height))
		SetWindowPos(hwnd, 0, rect.left + (oldc_width - c_width) / 2, rect.top + (oldc_height - c_height) / 2, c_width, c_height, 0);
	system(0);
}

bool GetKey(int vKey) {
	return ((GetAsyncKeyState(vKey) & 0x8000) && GetConsoleWindow() == GetForegroundWindow());
}

int centerX(std::string str = "") {
	return (w_global - str.length()) / 2;
}
int centerY() {
	return h_global / 2;
}

int cursorX() {
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(GetConsoleWindow(), &cursor);
	return cursor.x / f_global + 1;
}
int cursorY() {
	POINT cursor;
	GetCursorPos(&cursor);
	ScreenToClient(GetConsoleWindow(), &cursor);
	return cursor.y / f_global + 1;
}

#define xc cursorX()
#define yc cursorY()

void sleep(int ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void draw_tower(int floors, int x, int curr_floor = 0) {
	for (int flr = 0; flr < floors; flr++) {
		if (floors - curr_floor == flr)
			printf(white"\033[%d;%dH\333\333\333\033[%d;%dH\333\033[38;5;86m\376%s\333\033[%d;%dH\333\333\333\033[0m", (h_global - floors * 2) / 2 + flr * 2, x, (h_global - floors * 2) / 2 + flr * 2 + 1, x, white, (h_global - floors * 2) / 2 + flr * 2 + 2, x);
		else {
			printf(white"\033[%d;%dH\333\333\333\033[%d;%dH\333%s\260%s\333\033[%d;%dH\333\333\333\033[0m", (h_global - floors * 2) / 2 + flr * 2, x, (h_global - floors * 2) / 2 + flr * 2 + 1, x, gray, white, (h_global - floors * 2) / 2 + flr * 2 + 2, x);
		}
		if (flr == floors - 1)
			printf(white"\033[%d;%dH\033[4m \333\333\333 \033[0m", (h_global - floors * 2) / 2 + flr * 2 + 2, x - 1);
	}
}

void redefine(int& floors, int& curr_floor, std::vector<int>& ch_floor, int& near_floor, int& timer, bool& ch_seq, int& ch_amount, int& el_delay, int& elevator_speed, int& req_delay, std::vector<int>& req, std::vector<int>& req_perc, int& perc_delay, int& score, int& wait_time) {
	floors = 6;
	curr_floor = 1;
	ch_floor.clear();
	ch_floor.push_back(0);
	near_floor = 0;
	timer = 0;
	ch_seq = true;
	ch_amount = 4;
	el_delay = 0;
	elevator_speed = 10;
	req_delay = 0;
	req.clear();
	req.push_back(0);
	req_perc.clear();
	req_perc.push_back(0);
	perc_delay = 0;
	score = 0;
	wait_time = 90;
}

void menu(bool pause, int& high_score) {
	system("cls");
	std::string title = "elevator_simulator";
	std::string button1 = "start_elevating.exe";
	std::string button2 = "go_on_foot.exe";
	std::string hs = "high_score: " + std::to_string(high_score);
	if (pause) {
		title = "paused";
		button1 = "continue_elevating.exe";
	}

	int y_anim = centerY();

	if (!pause) {
		for (int word_anim = 0; word_anim <= title.length(); word_anim++) {
			if (y_anim > centerY() / 2) {
				y_anim--;
				system("cls");
			}
			printf(white"\033[4m\033[%d;%dH %s \033[0m", y_anim, centerX(title.substr(0, word_anim)) - 1, title.substr(0, word_anim).c_str());
			sleep(50);
		}
	}
	else
		printf(white"\033[4m\033[%d;%dH %s \033[0m", centerY() / 2, centerX(title) - 1, title.c_str());

	while (true) {
		printf(white"\033[%d;%dH%s\033[%d;%dH%s\033[0m", centerY() - 2, centerX(button1), button1.c_str(), centerY() + 1, centerX(button2), button2.c_str());

		if (high_score > 0) {
			printf(gray"\033[%d;%dH%s\033[0m", h_global - h_global / 7, centerX(hs), hs.c_str());

			if (yc == h_global - h_global / 7 && xc >= centerX(hs) && xc < centerX(hs) + hs.length()) {
				printf(red"\033[%d;1H\033[0K\033[%d;%dHclear_data?\033[0m", h_global - h_global / 7, h_global - h_global / 7, centerX("delete_data?"));

				if (GetKey(VK_LBUTTON)) {	//click
					high_score = 0;

					std::ofstream file;
					char* appdata_path;
					_dupenv_s(&appdata_path, 0, "APPDATA");
					file.open(std::string(appdata_path) + "\\elevator_data");
					file << high_score;

					while (GetKey(VK_ESCAPE))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
					printf("\033[%d;1H\033[0K", h_global - h_global / 7);
				}
			}
		}

		if (yc == centerY() - 2 && xc >= centerX(button1) && xc < centerX(button1) + button1.length()) {	//button1
			printf(white"\033[4m\033[%d;%dH%s\033[0m", centerY() - 2, centerX(button1), button1.c_str());
			if (GetKey(VK_LBUTTON))	//click
				break;
		}
		if (yc == centerY() + 1 && xc >= centerX(button2) && xc < centerX(button2) + button2.length()) {	//button2
			printf(white"\033[4m\033[%d;%dH%s\033[0m", centerY() + 1, centerX(button2), button2.c_str());
			if (GetKey(VK_LBUTTON))	//click
				exit(0);
		}
		if (GetKey(VK_ESCAPE) && pause) {	//esc
			while (GetKey(VK_ESCAPE))
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			break;
		}
		sleep(10);
	}
	system("cls");
}

int end_screen(int score, int high_score = 0) {
	std::string title = "you_left_them_angry";
	std::string button1 = "restart.exe";
	std::string button2 = "go_to_menu.exe";
	std::string s = "score: " + std::to_string(score);
	std::string hs = "high_score: " + std::to_string(high_score);

	printf(red"\033[%d;%dH\033[4m %s \033[0m", centerY() / 2, centerX(title) - 1, title.c_str());
	sleep(350);

	while (true) {
		printf(red"\033[%d;%dH%s\033[%d;%dH%s\033[0m", centerY() - 2, centerX(button1), button1.c_str(), centerY() + 1, centerX(button2), button2.c_str());
		printf(red"\033[%d;%dH%s\033[0m", h_global - h_global / 7 - 3, centerX(s), s.c_str());
		if (high_score > 0)
			printf(red"\033[%d;%dH%s\033[0m", h_global - h_global / 7, centerX(hs), hs.c_str());

		if (yc == centerY() - 2 && xc >= centerX(button1) && xc < centerX(button1) + button1.length()) {	//button1
			printf(red"\033[4m\033[%d;%dH%s\033[0m", centerY() - 2, centerX(button1), button1.c_str());
			if (GetKey(VK_LBUTTON))	//click
				return 1;
		}
		if (yc == centerY() + 1 && xc >= centerX(button2) && xc < centerX(button2) + button2.length()) {	//button2
			printf(red"\033[4m\033[%d;%dH%s\033[0m", centerY() + 1, centerX(button2), button2.c_str());
			if (GetKey(VK_LBUTTON))	//click
				return 2;
		}

		sleep(10);
	}
	return 0;
}

int main() {
	srand(time(0));
	system("cls");
	printf("\033[?25l");
	console_config(w_global, h_global, f_global, f_global, 1, RGB(0, 0, 0), FW_NORMAL, L"Terminal", L"elevator_simulator.exe");

	int floors{ 6 };
	int max_floors{ 15 };
	int curr_floor{ 1 };
	std::vector<int> ch_floor(1);
	int near_floor{};

	int score{};
	int high_score{};

	std::fstream file;
	char* appdata_path;
	_dupenv_s(&appdata_path, 0, "APPDATA");
	file.open(std::string(appdata_path) + "\\elevator_data");
	file >> high_score;
	file.close();

	menu(false, high_score);
	for (int floors_anim = 0; floors_anim <= floors; floors_anim++) {
		system("cls");
		draw_tower(floors_anim, w_global / 5, curr_floor);
		sleep(50);
	}

	int timer{};
	bool ch_seq{ true };

	int ch_amount{ 4 };

	int el_delay{};
	int elevator_speed{ 10 };

	int wait_time{ 90 };

	int req_delay{};
	std::vector<int> req(1);
	std::vector<int> req_perc(1);
	int perc_delay{};

	redefine(floors, curr_floor, ch_floor, near_floor, timer, ch_seq, ch_amount, el_delay, elevator_speed, req_delay, req, req_perc, perc_delay, score, wait_time);
	while (true) {
		int perc_speed{ (max_floors - floors) / 2 };

		int tmp_flr = floors;
		floors = (60 + score / 2) / 10;
		if (floors > max_floors)
			floors = max_floors;
		if (tmp_flr != floors)
			system("cls");

		if (floors >= 10) {
			ch_amount = 5;
			elevator_speed = 6;
			wait_time = 75;
		}

		int req_speed = (max_floors - (floors / 2 + rand() % floors / 2)) * 5;
		if (curr_floor > floors)	//limit check
			curr_floor = floors;
		for (int check = 0; check < ch_floor.size(); check++) {
			if (ch_floor[check] > floors)
				ch_floor.erase(ch_floor.begin() + check);
		}

		if (ch_seq && ch_floor.size() > 1) {
			el_delay = 0;
			timer++;
		}
		else
			el_delay++;
		req_delay++;
		perc_delay++;

		draw_tower(floors, w_global / 5, curr_floor);

		std::string choice_str = "choices_left: " + std::to_string(ch_amount - ch_floor.size() + 1);
		printf(gray"\033[%d;1H\033[0K\033[%d;%dH%s\033[0m", h_global - 1, h_global - 1, int(w_global - choice_str.length()), choice_str.c_str());

		printf(gray"\033[%d;2Hscore: %d", h_global - 1, score);

		if (req_delay >= req_speed) {
			int rand_floor = 1 + rand() % floors;
			req_delay = 0;
			bool check = true;
			for (int ch = 1; ch < req.size(); ch++) {
				if (req[ch] == rand_floor)
					check = false;
			}
			if (check && rand_floor != curr_floor) {
				req.push_back(rand_floor);
				req_perc.push_back(0);
			}
		}
		if (perc_delay >= perc_speed) {
			perc_delay = 0;
			for (int perc_inc = 1; perc_inc < req_perc.size(); perc_inc++) {
				if (req_perc[perc_inc] < 100)
					req_perc[perc_inc]++;
			}
		}

		int angryness{};
		for (int ang = 1; ang < req_perc.size(); ang++) {
			angryness += req_perc[ang];
		}
		if (req_perc.size() > 1)
			angryness /= (req_perc.size() - 1);
		else
			angryness = 0;
		printf(gray"\033[%d;2Hangryness: \033[38;2;%d;%d;%dm%d%%\033[0m  ", h_global - 3, 155 + angryness, 155 - angryness, 155 - angryness, angryness);

		if (angryness >= 100 && ch_seq) {	//end screen
			if (score > high_score) {
				high_score = score;

				file.open(std::string(appdata_path) + "\\elevator_data");
				file << high_score;
				file.close();
			}

			sleep(500);
			for (int cent_anim = w_global / 5; cent_anim <= centerX() - 1; cent_anim++) {
				system("cls");
				draw_tower(floors, cent_anim);
				sleep(100);
			}
			sleep(300);
			for (int floors_anim = floors; floors_anim >= 0; floors_anim--) {
				draw_tower(floors_anim, centerX() - 1);
				sleep(100);
				system("cls");
			}
			sleep(500);
			int ret = end_screen(score, high_score);
			if (ret == 1)
				redefine(floors, curr_floor, ch_floor, near_floor, timer, ch_seq, ch_amount, el_delay, elevator_speed, req_delay, req, req_perc, perc_delay, score, wait_time);
			if (ret == 2) {
				menu(false, high_score);
				redefine(floors, curr_floor, ch_floor, near_floor, timer, ch_seq, ch_amount, el_delay, elevator_speed, req_delay, req, req_perc, perc_delay, score, wait_time);
			}

			for (int floors_anim = 0; floors_anim <= floors; floors_anim++) {
				system("cls");
				draw_tower(floors_anim, w_global / 5, curr_floor);
				sleep(50);
			}
		}

		for (int button = 0; button < floors; button++) {	//drawing and checking the buttons
			printf(gray"\033[%d;%dH %d \033[0m", (h_global - floors * 2) / 2 + button * 2 + 1, w_global - w_global / 5 - 1, floors - button);

			if (ch_seq && ((GetKey(std::to_string(floors - button)[0]) && floors - button <= 9) || (yc == (h_global - floors * 2) / 2 + button * 2 + 1 && xc >= w_global - w_global / 5 - 1 && xc < w_global - w_global / 5 + std::to_string(floors - button).length() + 1))) {
				printf(white"\033[4m\033[%d;%dH %d \033[0m", (h_global - floors * 2) / 2 + button * 2 + 1, w_global - w_global / 5 - 1, floors - button);
				if (GetKey(VK_LBUTTON) || (GetKey(std::to_string(floors - button)[0]) && floors - button <= 9)) {	//click
					bool check{ true };
					for (int ch = 1; ch < ch_floor.size(); ch++) {	//checking
						if (ch_floor[ch] == floors - button) {
							ch_floor.erase(ch_floor.begin() + ch);
							check = false;
							timer = 0;
						}
					}
					if (check && ch_floor.size() - 1 < ch_amount && floors - button != curr_floor) {
						ch_floor.push_back(floors - button);
						timer = 0;
					}

					while (GetKey(VK_LBUTTON) || (GetKey(std::to_string(floors - button)[0]) && floors - button <= 9))
						FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
				}
			}
			printf("\033[%d;%dH ", (h_global - floors * 2) / 2 + button * 2 + 1, w_global / 5 + 5);	//erasing the marks
			printf("\033[%d;%dH ", (h_global - floors * 2) / 2 + button * 2 + 1, w_global / 5 - 3);	//erasing the requests
			for (int mark = 1; mark < ch_floor.size(); mark++)	//drawing the marks
				printf(gray"\033[%d;%dH<\033[0m", (h_global - floors * 2) / 2 + (floors - ch_floor[mark]) * 2 + 1, w_global / 5 + 5);
			for (int mark_r = 1; mark_r < req.size(); mark_r++)	//drawing the requests
				printf("\033[38;2;%d;%d;%dm\033[%d;%dH\1\033[0m", 155 + req_perc[mark_r], 155 - req_perc[mark_r], 155 - req_perc[mark_r], (h_global - floors * 2) / 2 + (floors - req[mark_r]) * 2 + 1, w_global / 5 - 3);
		}

		if (timer >= wait_time) {	//elevating sequence
			ch_seq = false;
			printf(gray"\033[%d;%dHelevating...\033[0m", h_global / 10, centerX("elevating"));

			if (near_floor == 0) {	//find the nearest floor
				near_floor = ch_floor[1];
				for (int find = 1; find < ch_floor.size(); find++) {
					if (abs(curr_floor - ch_floor[find]) < abs(curr_floor - near_floor))
						near_floor = ch_floor[find];
				}
			}
			else {	//reach the nearest floor
				if (el_delay >= elevator_speed) {
					if (curr_floor < near_floor)
						curr_floor++;
					if (curr_floor > near_floor)
						curr_floor--;
					el_delay = 0;
				}
			}
			draw_tower(floors, w_global / 5, curr_floor);	//just for the update
			if (curr_floor == near_floor) {	//delete the reached floor
				if (ch_floor.size() > 1) {
					for (int del_find = 1; del_find < ch_floor.size(); del_find++) {
						if (ch_floor[del_find] == curr_floor)
							ch_floor.erase(ch_floor.begin() + del_find);
					}
					for (int del_find = 1; del_find < req.size(); del_find++) {
						if (req[del_find] == curr_floor) {
							req.erase(req.begin() + del_find);
							req_perc.erase(req_perc.begin() + del_find);
							score++;
						}
					}
				}
				else {
					ch_seq = true;
					timer = 0;
				}
				near_floor = 0;
			}
		}
		else
			printf("\033[%d;%dH            ", h_global / 10, centerX("elevating"));
		sleep(10);

		if (GetKey(VK_ESCAPE)) {	//pause
			while (GetKey(VK_ESCAPE))
				FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
			menu(true, high_score);
		}
	}
	return 0;
}