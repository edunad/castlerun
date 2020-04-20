// CastleRun_thegame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <mmsystem.h>
#include <conio.h>

#pragma comment(lib, "winmm.lib")

/*
192 | 196 | 179 | 218 | 217 | 191 = WALLS
176 = Grass
42 | 15 = Flowers
24 | 25 | 26 | 27 = Arrows
6 | 5 = Trees
178 = Sand
30 = Mountains
2 = Human
*/

// WINDOWS API
HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
CONSOLE_SCREEN_BUFFER_INFO OriginalConsole;

// OTHER VARIABLES
#define MAX_NAME 9
#define MAX_DISC 1000
#define MAX_BUFFER 1000
#define TEXT_SPEED 15
#define MAX_DESC 59
#define MAX_ROOMS 47
#define MELTDOWN_ROUNDS 40

// AI VARIABLES
#define TOTAL_Knights 12
#define MAX_ATTEMPTS 100

// BATTLE VARIABLES
#define CRIT_DAMAGE 5

// ROLL VARIABLES
#define ROLL_POISON 6
#define ROLL_PERMA 7

// ROOM VARIABLES
#define POISON_DAMAGE 15
#define ARMOR_GAIN 5
#define ATTACK_GAIN 5
#define HEAL_GAIN 60
#define MAX_MAPSIZE 300

#define INTRO_ENABLED true

// PATHS
#define ROOM_PATH_TEMPLATE "Data/Rooms/room_"
#define ROOM_PATH_FORMAT ".mproom"
#define GUI_PATH "Data/GUI/"
#define GUI_PATH_FORMAT ".guifgt"
#define SAVE_PATH "Data/Saves/"
#define SAVE_PATH_FORMAT ".save"
#define MAP_PATH "Data/Map/"
#define MAP_PATH_FORMAT ".map"

// USER INPUT VARS
#define X_POS 0
#define Y_POS 10

// OTHER VARS
#define ReadTime 1300

// COLOR VARIABLES
#pragma region COLOR_VARS
#define FOREGROUND_WHITE		(FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW       	(FOREGROUND_RED | FOREGROUND_GREEN)
#define FOREGROUND_CYAN		        (FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_MAGENTA	        (FOREGROUND_RED | FOREGROUND_BLUE)
#define FOREGROUND_BLACK		0

#define FOREGROUND_INTENSE_RED		(FOREGROUND_RED | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_GREEN	(FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_BLUE		(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_WHITE	(FOREGROUND_WHITE | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_YELLOW	(FOREGROUND_YELLOW | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_CYAN		(FOREGROUND_CYAN | FOREGROUND_INTENSITY)
#define FOREGROUND_INTENSE_MAGENTA	(FOREGROUND_MAGENTA | FOREGROUND_INTENSITY)

#define BACKGROUND_WHITE		(BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN)
#define BACKGROUND_YELLOW	        (BACKGROUND_RED | BACKGROUND_GREEN)
#define BACKGROUND_CYAN		        (BACKGROUND_BLUE | BACKGROUND_GREEN)
#define BACKGROUND_MAGENTA	        (BACKGROUND_RED | BACKGROUND_BLUE)
#define BACKGROUND_GRAY	        (BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN | 0)
#define BACKGROUND_BLACK		0

#define BACKGROUND_INTENSE_RED		(BACKGROUND_RED | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_GREEN	(BACKGROUND_GREEN | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_BLUE		(BACKGROUND_BLUE | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_WHITE	(BACKGROUND_WHITE | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_YELLOW	(BACKGROUND_YELLOW | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_CYAN		(BACKGROUND_CYAN | BACKGROUND_INTENSITY)
#define BACKGROUND_INTENSE_MAGENTA	(BACKGROUND_MAGENTA | BACKGROUND_INTENSITY)
#pragma endregion

#pragma region Structs_Data

struct Desmoviption{
	char desc[MAX_DESC + 1];
};

struct RoomMap{
	char Map_Prt[100];
};

struct Player{
	char name[MAX_NAME];
	int HP;
	int room;
	int attack;
	int defense;

	bool hasTreasure;
	bool isFigthing;

	int dmgPoison;
	int roundPoison;

	int roundRemains;

	int initHP;
};

struct Dices{
	int Dice_1;
	int Dice_2;
	bool IsRolling;
};

struct Knight{
	char name[MAX_NAME];
	char deathMessage[200];
	char introMessage[200];
	char plyMessage[200];
	char image[20];

	int HP;
	int room;
	int defense;
	int attack;

	bool isFrozen;
	int init_HP;
};

struct Position{
	int forward, backward, left, right;
};

struct RoomType{
	bool Ispoison;
	bool Isshop;
	bool Isdeath;
	bool Istreasure;
};

struct Map{
	char mapDrw[10][MAX_MAPSIZE];
	int SizeY;
	int Sound;
};

struct Room
{
	Position pos;
	RoomType rtype;
	Map map;
	char disc[MAX_DISC];
	int guardID;
};

#pragma endregion

#pragma region GlobalVars

int ID_MUSIC;

Player ply;
Room Castle[MAX_ROOMS];
Knight AI[TOTAL_Knights];
Dices TheDices;

bool IsAdmin = false;
bool IsDestroying = false;
bool MiniMap = false;

char SCREEN_STATUS[30]; // Screen Manager

COORD Cursorpos = {0, 0};

#pragma endregion

#pragma region Functions

#pragma region Windows_API

void setCursor(int posX,int posY){
	Cursorpos.X = posX;
	Cursorpos.Y = posY;
	SetConsoleCursorPosition( hStdout, Cursorpos );
}

void moveCursor(int posX,int posY){
	Cursorpos.X += posX;
	Cursorpos.Y += posY;
	SetConsoleCursorPosition( hStdout, Cursorpos );
}

void cleanStripe(int posX,int posY,int amount){
	setCursor(posX,posY);

	for(int I = 0;I < amount;I++)
		printf(" ");
}

void centerConsole(){

	// Taken from : http://social.msdn.microsoft.com/Forums/en-US/d50184d2-313b-4944-8307-e0343e865879/cc-putting-the-window-in-center-of-screen?forum=vcgeneral
	// Modded.

	//Get the window console handle(isn't the right code but works for these sample
	HWND ConsoleWindow;
	ConsoleWindow=GetForegroundWindow();

	//Getting the desktop hadle and rectangule
	HWND   hwndScreen;
	RECT   rectScreen;		
	hwndScreen=GetDesktopWindow ();
	GetWindowRect(hwndScreen,&rectScreen); 		
	
	// Get the current width and height of the console
	RECT rConsole;
	GetWindowRect (ConsoleWindow,&rConsole); 		
	int Width = rConsole.left = rConsole.right;
	int Height = rConsole.bottom - rConsole.top;
	
	//caculate the window console to center of the screen	
	int ConsolePosX;
	int ConsolePosY;		
	ConsolePosX = ((rectScreen.right-rectScreen.left)/2-Width/2 );
	ConsolePosY = ((rectScreen.bottom-rectScreen.top)/2- Height/2 );	
	SetWindowPos(ConsoleWindow,NULL,ConsolePosX,ConsolePosY, Width, Height, SWP_SHOWWINDOW || SWP_NOSIZE);

}

#pragma endregion

#pragma region Util

bool convertBool(int tr){
	if(tr == 1)
		return true;
	else
		return false;
}

#pragma endregion

#pragma region ScreenEffect

void bloodEffect(){ // Like Pokemon

	system("COLOR CF");
	Sleep(40);
	system("COLOR 0F");

}

void poisonEffect(){ 

	system("COLOR 2F");
	Sleep(40);
	system("COLOR 0F");

}

void thunderEffect(){ 

	system("COLOR F0");
	Sleep(40);
	system("COLOR 0F");

}

void defenseEffect(){ 

	system("COLOR 1F");
	Sleep(40);
	system("COLOR 0F");

}

#pragma endregion

#pragma region Sound_Control

int get_midi_status(int ID){
if(ID_MUSIC != -1){
	TCHAR tch[100];

	if(!ply.isFigthing){
		switch(ID){

			case 0: 
				mciSendString(TEXT("status incastle1 mode"), tch, 100, NULL);
			break;

			case 1: 
				mciSendString(TEXT("status incastle2 mode"), tch, 100, NULL);
			break;

			case 2: 
				mciSendString(TEXT("status incastle3 mode"), tch, 100, NULL);
			break;

		}
	}else{
		switch(ID){

			case 0: 
				mciSendString(TEXT("status battle1 mode"), tch, 100, NULL);
			break;

			case 1: 
				mciSendString(TEXT("status battle2 mode"), tch, 100, NULL);
			break;

			case 2: 
				mciSendString(TEXT("status battle3 mode"), tch, 100, NULL);
			break;

			case 3: 
				mciSendString(TEXT("status battle4 mode"), tch, 100, NULL);
			break;

			case 4: 
				mciSendString(TEXT("status battle5 mode"), tch, 100, NULL);
			break;

			case 5:
				mciSendString(TEXT("status boss_theme mode"), tch, 100, NULL);
			break;

			case 6:
				mciSendString(TEXT("status kingmusic mode"), tch, 100, NULL);
			break;
		}
	}

	if(_tcscmp(tch,_T("s")) != -1)
		return 1;
	else
		return 0;

}else
	return 1;
}

void play_battle(){

	if(ID_MUSIC != -1){

		int rn = rand()% 5;
		ID_MUSIC = rn;

		mciSendString(TEXT("stop all"),NULL,0,NULL);
		switch(rn){
			case 0:
				mciSendString(TEXT("play battle1 from 0"),NULL,0,NULL);
				break;
			case 1:
				mciSendString(TEXT("play battle2 from 0"),NULL,0,NULL);
				break;
			case 2: 
				mciSendString(TEXT("play battle3 from 0"),NULL,0,NULL);
				break;
			case 3: 
				mciSendString(TEXT("play battle4 from 0"),NULL,0,NULL);
				break;
			case 4: 
				mciSendString(TEXT("play battle5 from 0"),NULL,0,NULL);
				break;
		}
	}
};

void play_castle(){

if(ID_MUSIC != -1){

	int rn = rand() % 3;
	ID_MUSIC = rn;

	mciSendString(TEXT("stop all"),NULL,0,NULL);

	switch(rn){
		case 0:
			mciSendString(TEXT("play incastle1 from 0"),NULL,0,NULL);
			break;
		case 1:
			mciSendString(TEXT("play incastle2 from 0"),NULL,0,NULL);
			break;
		case 2: 
			mciSendString(TEXT("play incastle3 from 0"),NULL,0,NULL);
			break;
	}
}

};

void play_victory(){
	if(ID_MUSIC != -1){
		int rn = rand() % 2;
		ID_MUSIC = rn;
	
		mciSendString(TEXT("stop all"),NULL,0,NULL);
	
		switch(rn){
			case 0:
				mciSendString(TEXT("play victory1 from 0"),NULL,0,NULL);
				break;
			case 1:
				mciSendString(TEXT("play victory2 from 0"),NULL,0,NULL);
				break;
		}
	}
};

#pragma endregion

#pragma region String_Control

void shrinkDisc(char Desc[], Desmoviption Tmp[]){

	int C = 0;
	int O = 0;
	int I = 0;

	char tm[MAX_DESC + 1];

	while(I < strlen(Desc)){

		tm[O] = Desc[I];

		I++;
		O++;

		if(O >= MAX_DESC){

			tm[O++] = '\0';
			O = 0;

			strcpy(Tmp[C].desc,tm);
			strcpy(tm,"");

			C++;
			
		}

	}

	tm[O++] = '\0';
	strcpy(Tmp[C].desc,tm);
}

void repeated_print(char c, int x){
	for(int i = 0; i < x; i++)
	{
		printf("%c", c);
	}
}

void drawBar(int x, int y, int Current, int MAX,int SIZE){

	setCursor(x,y);

	int Total = SIZE;
	int pr = (Current * Total) / MAX;
	int per100 = (Current * 100) / MAX;


	SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_RED );

	for(int I = 0; I < Total; I++)
		if(I < pr)
			printf("%c",char(219));
		else
			printf(" ");

	SetConsoleTextAttribute( hStdout, BACKGROUND_INTENSE_RED | FOREGROUND_BLACK );
	setCursor(x,y);
	printf(" %d%%",per100);
	SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );

}

void slowmo_print(char c[], int d){
	for(int i = 0; i < strlen(c); i++)
	{
		printf("%c", c[i]);
		Sleep(d);
	}
}

#pragma endregion

#pragma region GUI_Control

void playerStats(int x, int y)
{
	setCursor(x,y);
	printf(" Name : %s", ply.name);
	moveCursor(0,1);
	printf(" Health : %d", ply.HP);
	moveCursor(0,1);
	printf(" Attack : %d", ply.attack);
	moveCursor(0,1);
	printf(" Defence : %d\n", ply.defense);
	moveCursor(0,1);
	printf(" Treasure : %d\n", ply.hasTreasure);
}

void drawFigure(char txt[], int x, int y){

	FILE *f;
	char line[MAX_BUFFER];
	char file[30];

	strcpy(file,GUI_PATH);
	strcat(file,txt);
	strcat(file,GUI_PATH_FORMAT);

	f = fopen( file, "r" );

	setCursor(x,y);

	while( fgets(line, MAX_BUFFER, f) != NULL)
	{
		fputs( line, stdout );
		moveCursor(0,1);
	}

	fclose( f );
}

void roomDiscription(int x, int y){

	Desmoviption de[5];
	for(int I = 0; I < 5; I++)
		strcpy(de[I].desc," ");

	shrinkDisc(Castle[ply.room].disc, de);

	setCursor(x,y);
	printf("%s", de[0].desc);
	moveCursor(0,1);
	printf("%s", de[1].desc);
	moveCursor(0,1);
	printf("%s", de[2].desc);
	moveCursor(0,1);
	printf("%s", de[3].desc);
	moveCursor(0,1);
	printf("%s", de[4].desc);

}

void drawVerticalLines(int times, int x, int y){
	
	for (int i = 0; i < times; i++){
		setCursor(x,y);
		printf("%c", char(186));
		y++;
	}
}

void drawStatLines(int x, int y){
	
	setCursor(x,y);
	printf("%c", char(201));
	repeated_print(char(205),16);
	y++;
	drawVerticalLines(3, x, y);
	moveCursor(0,1);
	printf("%c", char(204));
	repeated_print(char(205),16);
	moveCursor(0,1);
	printf("%c", char(186));
	moveCursor(0,1);
	printf("%c", char(200));
	repeated_print(char(205),16);

	y--;
	x += 17;
	setCursor(x,y);
	printf("%c", char(187));
	y++;
	drawVerticalLines(3, x, y);
	moveCursor(0,1);
	printf("%c", char(185));
	moveCursor(0,1);
	printf("%c", char(186));
	moveCursor(0,1);
	printf("%c", char(188));

}

void drawBattleStats(int x, int y){

		setCursor(x, y);
		printf(" Name : %s", ply.name);
		moveCursor(0, 1);
		printf(" Attack  : %d", ply.attack);
		moveCursor(0, 1);
		printf(" Defense %c %d",char(16), ply.defense);
		drawBar(x, y + 4, ply.HP, ply.initHP,16);
}

void drawBattleStats(int x, int y, Knight &ai){
		
		setCursor(x, y);
		printf(" Name : %s", ai.name);
		moveCursor(0, 1);
		printf(" Attack  : %d", ai.attack);
		moveCursor(0, 1);
		printf(" Defense %c %d",char(16),  ai.defense);
		drawBar(x, y + 4, ai.HP, ai.init_HP,16);
}

void drawBattleInfo(int x, int y){

	setCursor(x,y);
	moveCursor(20,0);
	printf("%c", char(203));
	moveCursor(0,1);
	printf("%c", char(179));
	moveCursor(0,1);
	printf("%c", char(192));
	moveCursor(1,0);
	repeated_print(char(196), 39);
	printf("%c", char(217));
	setCursor(x + 60,y);
	printf("%c", char(203));
	moveCursor(0,1);
	printf("%c", char(179));
	moveCursor(0,1);
}

void drawBattleFight(int x,int y, Knight &ai){
	drawBattleStats(x + 2, y + 18);
	drawBattleStats(x + 62, y + 18, ai);
}

void drawBattleHUD(int x, int y,Knight &ai){

	drawBattleInfo(0,0);

	drawFigure("playerFigure", x + 1, y + 4);
	drawFigure("knightFigure", x + 50, y + 4);
	drawFigure("vs", x + 28, y + 8);
	
	drawStatLines(x + 1, y + 17);
	drawStatLines(x + 61, y + 17);

	drawBattleFight(x,y,ai);

}

#pragma endregion

#pragma region Game_Control

void GameOver(){

	system("cls");

	PlaySound(NULL,NULL,SND_ASYNC);

	drawFigure("dead",0,4);

	mciSendString(TEXT("stop all"),NULL,0,NULL);
	mciSendString(TEXT("play gameover"),NULL,0,NULL);

	setCursor(31,22);
	slowmo_print("GAME OVER", 600);

	Sleep(1000);
	exit(0);

}

#pragma endregion

#pragma region Map_Control

void destroyMAP(){

	if(IsDestroying){

		int destroy = ply.roundRemains - 1;

		if( destroy > 0 )
			ply.roundRemains-=1;
		else
			GameOver();
	}

}

void convertToBinary(){

	char map[1000] = MAP_PATH;
	strcat(map,"mapconv");
	strcat(map,MAP_PATH_FORMAT);

	FILE *f = fopen(map, "wb");
	fwrite(&Castle, sizeof(Room), MAX_ROOMS, f);

	fclose(f);

}

void readBinary(){

	char map[1000] = MAP_PATH;
	strcat(map,"mapconv");
	strcat(map,MAP_PATH_FORMAT);

	FILE *handleRead=fopen(map,"rb");
    fread(Castle,sizeof(Room), MAX_ROOMS ,handleRead);
	
	for(int I = 0;I < MAX_ROOMS;I++)
		Castle[I].guardID = -1;
}

void saveFile(char sv[]){

	char map[1000] = SAVE_PATH;
	strcat(map,sv);
	strcat(map,SAVE_PATH_FORMAT);

	FILE *f = fopen(map, "wb");
	fwrite(&ply, sizeof(Player), 1, f);
	fwrite(&AI, sizeof(Knight), TOTAL_Knights, f);

	fclose(f);

}

bool FileLoaded = false;

void loadSaveFile(char sv[]){

	char map[1000] = SAVE_PATH;
	strcat(map,sv);
	strcat(map,SAVE_PATH_FORMAT);

	FILE *handleRead= fopen(map,"rb");

	if(handleRead != NULL){

		fread(&ply,sizeof(Player), 1 ,handleRead);
		fread(AI,sizeof(Knight), TOTAL_Knights ,handleRead);

		FileLoaded = true;

		mciSendString(TEXT("stop all"),NULL,0,NULL);
		mciSendString(TEXT("play save from 0"),NULL,0,NULL);

		if(strcmp(ply.name,"popcorn") == 0)
			IsAdmin = true;

		slowmo_print("Save Loaded!",80);
		Sleep(1000);

	}else{

		mciSendString(TEXT("stop all"),NULL,0,NULL);
		mciSendString(TEXT("play notfound from 0"),NULL,0,NULL);
		slowmo_print("\nFile Not FOUND!",80);
		Sleep(1000);
	}
}

void setup_castle()
{
	FILE *f;
	char line[MAX_BUFFER];
	char word[MAX_DISC];

	char map[1000] = MAP_PATH;
	strcat(map,"map");
	strcat(map,MAP_PATH_FORMAT);

	f = fopen( map, "r" );
	int ttlt = 0;

	while( fgets(line, MAX_BUFFER, f) != NULL)
	{
		int c = 0;
		int i = 0;
		int s = 0;
		int r = 0;
		strcpy(word, "");

		while(line[i] != '\0'){

			if(line[i] == ';'){

				word[s] = '\0';

				switch (c){

				case 0:
					r = atoi(word);
				case 1:
					Castle[r].pos.forward = atoi(word);
					break;
				case 2:
					Castle[r].pos.backward = atoi(word);
					break;
				case 3:
					Castle[r].pos.right = atoi(word);
					break;
				case 4:
					Castle[r].pos.left = atoi(word);
					break;
				case 5:
					strcpy(Castle[r].disc, word);
					break;
				case 6:
					Castle[r].rtype.Istreasure = convertBool(atoi(word));
					break;
				case 7:
					Castle[r].rtype.Ispoison = convertBool(atoi(word));
					break;
				case 8:
					Castle[r].rtype.Isshop = convertBool(atoi(word));
					break;
				case 9:
					Castle[r].rtype.Isdeath = convertBool(atoi(word));
					break;
				default:
					break;
				}

				Castle[r].guardID = -1;

				strcpy(word, "");
				s = 0;
				c++;
			}
			else{
				word[s] = line[i];
				s++;
			}

			i++;
		}

		Sleep(10);
		drawBar(20,13,ttlt,47,39);

		ttlt++;
	}

	fclose( f );

	convertToBinary();
}

void Apply_MapColorCore(int cl){

	if(cl == 0){
		SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );
	}else{
		if(cl == 192 || cl == 196 || cl == 179 || cl == 218 || cl == 217 || cl == 191)
			SetConsoleTextAttribute( hStdout, BACKGROUND_GRAY );

		if(cl == 176)
			SetConsoleTextAttribute( hStdout, 2 );

		if(cl == 42 || cl == 15){
			int rnd = rand() % 2;

			if(rnd == 1)
				SetConsoleTextAttribute( hStdout,  FOREGROUND_YELLOW );
			else
				SetConsoleTextAttribute( hStdout,  FOREGROUND_RED );
		}

		if(cl == 126 || cl == 174 || cl == 175)
			SetConsoleTextAttribute( hStdout,  BACKGROUND_BLUE | FOREGROUND_INTENSE_BLUE);

		if(cl >= 24 && cl <= 27)
			SetConsoleTextAttribute( hStdout,  FOREGROUND_INTENSE_YELLOW);

		if(cl == 6 || cl == 5)
			SetConsoleTextAttribute( hStdout,  FOREGROUND_INTENSE_GREEN);

		if(cl == 178)
			SetConsoleTextAttribute( hStdout,  BACKGROUND_INTENSE_YELLOW);

		if(cl == 30)
			SetConsoleTextAttribute( hStdout,  FOREGROUND_WHITE | BACKGROUND_BLACK);
	}

}

void LoadRoomMap(int ID, Room &mp){

	FILE *f;

	char line[MAX_MAPSIZE];
	char text[30];
	char num[4];

	strcpy(text,ROOM_PATH_TEMPLATE);
	sprintf(num, "%d", ID);
	strcat(text,num);
	strcat(text,ROOM_PATH_FORMAT);

	f = fopen( text, "r" );

	int c = 0;
	
	cleanStripe(0,21,70);
	setCursor(1,21);
	printf("[Loading-Room] %s",text);
	Sleep(10);

	while( fgets(line, MAX_MAPSIZE, f) != NULL){
		
		strcpy(mp.map.mapDrw[c], line);
		c++;

	}

	mp.map.SizeY = c;
	fclose(f);

}

void BuildMap(Room &mp, int posX, int posY){

	setCursor(posX,posY);

	for(int I = 0; I < mp.map.SizeY;I++){

		char TMP[10];
		int Count = 0;

		for(int O = 0; O < strlen(mp.map.mapDrw[I]); O++){

			TMP[Count] = mp.map.mapDrw[I][O];

			if( TMP[Count] == ';' ){

				TMP[Count++] = '\0';
				int Ascii = atoi(TMP);

				Apply_MapColorCore(Ascii);
				printf("%c",Ascii);
				Apply_MapColorCore(0);

				strcpy(TMP,"");
				Count = 0;

			}else
				Count++;

		}

		moveCursor(0,1);

	}

}

void Apply_RoomSound(Room &mp){

}

#pragma endregion

void drawHUD(int x, int y){

	setCursor(x, y);
	repeated_print(char(205),18);
	printf("%c",char(203));
	repeated_print(char(205),61);

	drawVerticalLines(5, x + 18, y + 1);

	setCursor(x, y + 6);
	repeated_print(char(205),18);
	printf("%c",char(202));
	repeated_print(char(205),61);

	playerStats(x, y + 1);

	roomDiscription(x + 20, y + 1);
	
	if(MiniMap)
		BuildMap(Castle[ply.room],68,7);

	setCursor(x + 1,y + 21);
	printf("W = Forward");
	setCursor(x + 1,y + 22);
	printf("S = Backwards");
	setCursor(x + 1,y + 23);
	printf("A = Left");
	setCursor(x + 1,y + 24);
	printf("D = Right");

	setCursor(x , y + 20);
	repeated_print(char(205),15);
	printf("%c",char(187));

	// ROUND REMAINING
	if(IsDestroying){
		drawVerticalLines(2, x + 15, y + 21);
		moveCursor(0,1);
		printf("%c",char(204));
		drawVerticalLines(1, x + 15, y + 21);
		drawVerticalLines(1, x + 15, y + 24);
		setCursor(x + 16 , y + 23);
		repeated_print(char(205),24);
		printf("%c",char(187));
		setCursor(x + 17 , y + 24);
		printf("ROUNDS REMAINING : %d",ply.roundRemains);
		drawVerticalLines(1, x + 40, y + 24);
	}else
		drawVerticalLines(4, x + 15, y + 21);
	
}

#pragma region AI_Control

bool roomHasGuard(int room){

	if(Castle[room].guardID == -1){

		return false;

	}else{

		if(IsAdmin){
			setCursor(X_POS,Y_POS + 3);
			printf("[Debug] Room %d is Occupied By : %s",room,AI[Castle[room].guardID].name);
			Sleep(10);
		}

		return true;
	}
}

void knightMove(Knight &guard, int Pos, int ID)
{
	int c = rand() % 4;
	bool HasMoved = false;
	int Attempts = 0;

	Castle[guard.room].guardID = -1;

	cleanStripe(X_POS,Y_POS + 2,60);
	cleanStripe(X_POS,Y_POS + 3,60);

	while(HasMoved == false && Attempts < MAX_ATTEMPTS){

		if(c == 0 && Castle[guard.room].pos.forward != -1){

			if(roomHasGuard(Castle[guard.room].pos.forward) == false){
				guard.room = Castle[guard.room].pos.forward;
				HasMoved = true;

				if(IsAdmin){
					setCursor(X_POS,Y_POS + 2);
					printf("[Debug] Knight %s Moved Forward (ROOM : %d)\n",guard.name,guard.room);
					Sleep(300);
				}

			}

		}else if(c == 1 && Castle[guard.room].pos.backward != -1){

			if(roomHasGuard(Castle[guard.room].pos.backward) == false){
				guard.room = Castle[guard.room].pos.backward;
				HasMoved = true;

				if(IsAdmin){
					setCursor(X_POS,Y_POS + 2);
					printf("[Debug] Knight %s Moved Backwards (ROOM : %d)\n",guard.name,guard.room);
					Sleep(300);
				}
			}

		}else if(c == 2 && Castle[guard.room].pos.left != -1){
			if(roomHasGuard(Castle[guard.room].pos.left) == false){
				guard.room = Castle[guard.room].pos.left;
				HasMoved = true;

				if(IsAdmin){
					setCursor(X_POS,Y_POS + 2);
					printf("[Debug] Knight %s Moved Left (ROOM : %d)\n",guard.name,guard.room);
					Sleep(300);
				}

			}
		}else if(c == 3 && Castle[guard.room].pos.right != -1){

			if(roomHasGuard(Castle[guard.room].pos.right) == false){
				guard.room = Castle[guard.room].pos.right;
				HasMoved = true;

				if(IsAdmin){
					setCursor(X_POS,Y_POS + 2);
					printf("[Debug] Knight %s Moved Right (ROOM : %d)\n",guard.name,guard.room);
					Sleep(300);
				}
			}

		}

		Attempts++;
	}

	if(HasMoved){
		if(guard.room == Pos){
			guard.room = ply.room;

			if(IsAdmin){
				setCursor(X_POS,Y_POS + 3);
				printf("[Debug] Knight %s Found Player on Corridor!\n",guard.name);
				Sleep(300);
			}

		}
	}else{

		Castle[guard.room].guardID = ID;

			if(IsAdmin){
				setCursor(X_POS,Y_POS + 3);
				printf("[Debug] Knight %s Failed to Move! (Blocked) (ROOM : %d)\n",guard.name,guard.room);
				Sleep(300);
			}
	}
	
}

void getGuardExpression(){
	int rnd = rand() % 4;

	setCursor(X_POS,Y_POS+1);

	switch(rnd){
	case 0:
		printf("You Hear Someone Snoring Near you..");
		break;
	case 1:
		printf("You Hear some Footsteps Near you..");
		break;
	case 2:
		printf("You Hear someone Eating Near you..");
		break;
	case 3:
		printf("You Hear someone Sneazing Near you..");
		break;
	}

	Sleep(ReadTime);
	system("cls");
	drawHUD(0,0);
	setCursor(X_POS,Y_POS);

}

void nearGuardEmotes(){

	bool was_heard = false;

	int forward = Castle[ply.room].pos.forward;
	int backward = Castle[ply.room].pos.backward;
	int left = Castle[ply.room].pos.left;
	int right = Castle[ply.room].pos.right;

	if(forward != -1){
		
		if(Castle[forward].guardID != -1){
			was_heard = true;
		}

	}else if(backward != -1){
		
		if(Castle[backward].guardID != -1){
			was_heard = true;
		}

	}else if(left != -1){
		
		if(Castle[left].guardID != -1){
			was_heard = true;
		}

	}else if(right != -1){
		
		if(Castle[right].guardID != -1){
			was_heard = true;
		}

	}

	if(was_heard)
		getGuardExpression();

}

void knightLoad(){

	strcpy(AI[0].name,"Bugsalot");
	strcpy(AI[0].deathMessage,"No! How Can This Be!?.....");
	strcpy(AI[0].introMessage,"Who are you!? INTRUDER!");
	strcpy(AI[0].plyMessage,"Im here to kill the king!");
	strcpy(AI[0].image,"guard");

	AI[0].defense = 5;
	AI[0].attack = 5;
	AI[0].HP = 20;
	AI[0].isFrozen = false;
	AI[0].room = 1;

	strcpy(AI[1].name,"Seiga");
	strcpy(AI[1].deathMessage,"I shall have my revenge!");
	strcpy(AI[1].introMessage,"Kill Intruder!");
	strcpy(AI[1].plyMessage,"Ha! This will be fun..");
	strcpy(AI[1].image,"guard");
	AI[1].defense = 5;
	AI[1].attack = 5;
	AI[1].HP = 20;
	AI[1].isFrozen = false;
	AI[1].room = 4;

	strcpy(AI[2].name,"Lazana");
	strcpy(AI[2].deathMessage,"No! No!!! Must protect the king!");
	strcpy(AI[2].introMessage,"Protect the King!");
	strcpy(AI[2].plyMessage,"You Won't stop me!");
	strcpy(AI[2].image,"guard");
	AI[2].defense = 5;
	AI[2].attack = 5;
	AI[2].HP = 20;
	AI[2].isFrozen = false;
	AI[2].room = 12;

	strcpy(AI[3].name,"Username");
	strcpy(AI[3].deathMessage,"Garen....avenge me....");
	strcpy(AI[3].introMessage,"You Cannot defeat me!");
	strcpy(AI[3].plyMessage,"We will see about that!");
	strcpy(AI[3].image,"guard");
	AI[3].defense = 5;
	AI[3].attack = 5;
	AI[3].HP = 20;
	AI[3].isFrozen = false;
	AI[3].room = 16;

	strcpy(AI[4].name,"Bacon");
	strcpy(AI[4].deathMessage,"NomNomNomNOmNOMNOMNOONM");
	strcpy(AI[4].introMessage,"Oink Oink!");
	strcpy(AI[4].plyMessage,"What the...just DIE");
	strcpy(AI[4].image,"guard");
	AI[4].defense = 7;
	AI[4].attack = 8;
	AI[4].HP = 30;
	AI[4].isFrozen = false;
	AI[4].room = 22;

	strcpy(AI[5].name,"Lunatic");
	strcpy(AI[5].deathMessage,"My.. king....i failed you...");
	strcpy(AI[5].introMessage,"What are you doing here!?");
	strcpy(AI[5].plyMessage,"Nothing! I sware!");
	strcpy(AI[5].image,"guard");
	AI[5].defense = 7;
	AI[5].attack = 8;
	AI[5].HP = 30;
	AI[5].isFrozen = false;
	AI[5].room = 25;

	strcpy(AI[6].name,"Mariah");
	strcpy(AI[6].deathMessage,"Hes too powerful...");
	strcpy(AI[6].introMessage,"Intruder! Intruder! Terminate!");
	strcpy(AI[6].plyMessage,"Terminate me? HA!");
	strcpy(AI[6].image,"guard");
	AI[6].defense = 7;
	AI[6].attack = 8;
	AI[6].HP = 30;
	AI[6].isFrozen = false;
	AI[6].room = 27;

	strcpy(AI[7].name,"Bunnie");
	strcpy(AI[7].deathMessage,"Someone... protect the king!");
	strcpy(AI[7].introMessage,"Hi Im bunnie! And im here to kill you! :D");
	strcpy(AI[7].plyMessage,"Uhm..ok...(weirdo)");
	strcpy(AI[7].image,"guard");
	AI[7].defense = 7;
	AI[7].attack = 8;
	AI[7].HP = 30;
	AI[7].isFrozen = false;
	AI[7].room = 37;

	strcpy(AI[8].name,"Chewy");
	strcpy(AI[8].deathMessage,"ARWRWARWARAWRAWRAWRAWRAWA");
	strcpy(AI[8].introMessage,"GEAGRGAWGRAGWRGAGWRA!");
	strcpy(AI[8].plyMessage,"ARWARAWRAWRAWRAWRAW to you too!");
	strcpy(AI[8].image,"guard");
	AI[8].defense = 10;
	AI[8].attack = 9;
	AI[8].HP = 50;
	AI[8].isFrozen = false;
	AI[8].room = 40;

	strcpy(AI[9].name,"Garen");
	strcpy(AI[9].deathMessage,"How can this be...my spin2win never fails...");
	strcpy(AI[9].introMessage,"FOR DEMACIAAAAAA!");
	strcpy(AI[9].plyMessage,"Were you in that bush this whole time!?");
	strcpy(AI[9].image,"guard");
	AI[9].defense = 10;
	AI[9].attack = 9;
	AI[9].HP = 50;
	AI[9].isFrozen = false;
	AI[9].room = 43;

	strcpy(AI[10].name,"Merlien");
	strcpy(AI[10].deathMessage,"You think im tough..? The king will destroy you!");
	strcpy(AI[10].introMessage,"You Think you can defeat me!? HAHAHA! Go my Minion! Defend the king!");
	strcpy(AI[10].plyMessage,"We will see about that wizard!");
	strcpy(AI[10].image,"wizard");
	AI[10].defense = 5;
	AI[10].attack = 20;
	AI[10].HP = 75;
	AI[10].isFrozen = true;
	AI[10].room = 19;

	strcpy(AI[11].name,"Aretur");
	strcpy(AI[11].deathMessage,"No...NO! My Kingdom!! Nooooooo!!!");
	strcpy(AI[11].introMessage,"Why are you doing this! I only wanted peace!");
	strcpy(AI[11].plyMessage,"I Hated the dam Flowers!");
	strcpy(AI[11].image,"guard");
	AI[11].defense = 10;
	AI[11].attack = 1;
	AI[11].HP = 1;
	AI[11].isFrozen = true;
	AI[11].room = 20;

	// Apply the Guards to the original rooms.
	for(int I = 0;I < TOTAL_Knights;I++)
		Castle[AI[I].room].guardID = I;
}

#pragma endregion

#pragma region Dice_Control

void dice_top(int n){
	if(n == 1){
		printf("%c       %c", 179, 179);
	}else if(n == 2 || n == 3){
		printf("%c     0 %c", 179, 179);
	}else if(n >= 4){
		printf("%c 0   0 %c", 179, 179);
	}
}

void dice_mid(int n){

	if(n == 1 || n == 3 || n == 5){
		printf("%c   0   %c", 179, 179);
	}else if(n == 6){
		printf("%c 0   0 %c", 179, 179);
	}else{
		printf("%c       %c", 179, 179);
	}

}

void dice_bot(int n){

	if(n == 2 || n == 3 ){
		printf("%c 0     %c", 179, 179);
	}else if(n != 1){
		printf("%c 0   0 %c", 179, 179);
	}else{
		printf("%c       %c", 179, 179);
	}

}

void drawDice(int dice, int x, int y){

	setCursor(x,y);

	printf("%c", char(218));
	repeated_print(char(196), 7);
	printf("%c", char(191));

	moveCursor(0,1);
	dice_top(dice);
	moveCursor(0,1);
	dice_mid(dice);
	moveCursor(0,1);
	dice_bot(dice);
	moveCursor(0,1);

	printf("%c",char(192));
	repeated_print(char(196), 7);
	printf("%c", char(217));

}

int rollDices(Dices &dice)
{
	int c = (rand()%10) + 10;
	int i = 0;

	while(i <= c)
	{
		dice.IsRolling = true;

		dice.Dice_1 = (rand()%6) + 1;
		dice.Dice_2 = (rand()%6) + 1;
		
		drawDice(dice.Dice_1, 29, 17);
		drawDice(dice.Dice_2, 38, 17);

		Sleep(20);
		i++;

	}

	dice.IsRolling = false;

	return (dice.Dice_1 + dice.Dice_2);
}

#pragma endregion

#pragma region Room_Control

void applyPoison(){
	if(ply.roundPoison > 0){
		ply.roundPoison -= 1;

		// Only poison if he has the HP for it, Don't kill the player.
		int Calc = ply.HP - ply.dmgPoison;
		if(Calc > 0){
			ply.HP -= ply.dmgPoison;
			poisonEffect();
		}

	}
}

void checkTypeOfRoom(){

	if(Castle[ply.room].rtype.Ispoison){

		cleanStripe(1,Y_POS+5,70);
		setCursor(X_POS,Y_POS+5);
		printf("Its a poison room! Roll more that %d so you won't be poisoned!",ROLL_POISON);
		Sleep(ReadTime);

		int rolled = rollDices(TheDices);

		if(rolled < ROLL_POISON){

			ply.roundPoison = rand() % 4 + 2;
			ply.dmgPoison = POISON_DAMAGE / ply.roundPoison;

			cleanStripe(1,Y_POS+5,70);
			setCursor(X_POS,Y_POS+5);
			printf("You got Poisoned for %d Rounds!",ply.roundPoison);
			Sleep(ReadTime);
		}else{
			cleanStripe(1,Y_POS+5,70);
			setCursor(X_POS,Y_POS+5);
			slowmo_print("You Managed to Escape the Poison! Just make sure you never come back!",50);
			Sleep(ReadTime);
		}

		system("cls");
		drawHUD(0,0);

	}else if(Castle[ply.room].rtype.Isshop){
		// TODO : SHOP HUD AND STUFF

		mciSendString(TEXT("stop all"),NULL,0,NULL);

		int rnd = rand() % 2;

		if(rnd == 1)
			mciSendString(TEXT("play shop_1 from 0"),NULL,0,NULL);
		else
			mciSendString(TEXT("play shop_2 from 0"),NULL,0,NULL);

		system("cls");

		drawFigure("minion",18,3);

		cleanStripe(1,Y_POS + 5,70);
		setCursor(X_POS,Y_POS + 5);
		slowmo_print("Minion : Welcome! Welcome!!, Let me heal you up and give you some items!\nMinion : There you go! All Ready!\nMinion : FOR THE HORDE!",90);
		Sleep(ReadTime);

		ply.attack += 5;
		ply.defense += 2;
		ply.HP += 70;

		slowmo_print("\n\nThe minion vanished.",40);
		Sleep(ReadTime);
		Castle[ply.room].rtype.Isshop = false;

		system("cls");
		drawHUD(0,0);

	}else if(Castle[ply.room].rtype.Isdeath){

		cleanStripe(1,Y_POS+5,70);
		setCursor(X_POS,Y_POS+5);
		printf("Its a trap! The walls are slowly closing in! Roll more than %d to survive!",ROLL_PERMA);
		Sleep(ReadTime);

		int rolled = rollDices(TheDices);

		if(rolled < ROLL_PERMA){

			cleanStripe(1,Y_POS+5,70);
			setCursor(X_POS,Y_POS+5);
			slowmo_print("You were crushed into small pieces!",50);
			Sleep(ReadTime);

			GameOver();
		}else{
			cleanStripe(1,Y_POS+5,70);
			setCursor(X_POS,Y_POS+5);
			slowmo_print("You Managed to Escape the Trap! Just make sure you don't come back to it",50);
			Sleep(ReadTime);
		}

		system("cls");
		drawHUD(0,0);

	}
}

void playerMove(){

	fflush(stdin);
	fflush(stdin);
	Sleep(300);

	while(true){

		if( (GetAsyncKeyState(0x57) & 0x8000) && Castle[ply.room].pos.forward != -1 ) // W
		{
			ply.room = Castle[ply.room].pos.forward;
			break;
		}
		else if( (GetAsyncKeyState(0x53) & 0x8000) && Castle[ply.room].pos.backward != -1) // S
		{
			ply.room = Castle[ply.room].pos.backward;
			break;
		}
		else if ( (GetAsyncKeyState(0x41) & 0x8000) && Castle[ply.room].pos.left != -1) // A
		{
			ply.room = Castle[ply.room].pos.left;
			break;
		}
		else if ( (GetAsyncKeyState(0x44) & 0x8000) && Castle[ply.room].pos.right != -1) // D
		{
			ply.room = Castle[ply.room].pos.right;
			break;
		}
		else if(GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{ 
			if(!IsDestroying){

				slowmo_print("Do you wish to save? (Y/N)\n>",70);

				char sv[10];
				scanf("%s",&sv);

				if(strcmp(sv,"Y") == 0 || strcmp(sv,"y") == 0){

					slowmo_print("\nSave Name\n>",70);
					scanf("%s",&sv);

					saveFile(sv);
					Sleep(1000);
					mciSendString(TEXT("stop all"),NULL,0,NULL);
					mciSendString(TEXT("play save from 0"),NULL,0,NULL);

					slowmo_print("File Saved!",80);
					exit(0);
				}else
					exit(0);
			}
		}else if(GetAsyncKeyState(0x4D) & 0x8000){ // OPEN / Close Minimap
			MiniMap = !MiniMap;	

			if(MiniMap)
				slowmo_print("You Open the Minimap",80);
			else
				slowmo_print("You Close the Minimap",80);

			Sleep(500);

			system("cls");
			drawHUD(0,0);
			setCursor(X_POS,Y_POS);

		}
		
	}

	destroyMAP();

}

#pragma endregion

#pragma region Load_Control

int Progress = 0;

void ProgressMove(){

	Progress++;
	drawFigure("drg_loagin",6,4);
	drawBar(20,13,Progress,20,39);
	setCursor(33,12);
	printf("LOADING FILES");

	cleanStripe(0,21,80);
	setCursor(1,21);
}

void LoadSounds(){

	drawFigure("drg_loagin",6,4);
	cleanStripe(0,21,70);
	setCursor(1,21);
	printf("[Loading-Midi] Data\\Sounds\\inside_castle_0.mid");

	mciSendString(TEXT("open Data\\Sounds\\inside_castle_0.mid type sequencer alias incastle1"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\inside_castle_1.mid");
	mciSendString(TEXT("open Data\\Sounds\\inside_castle_1.mid type sequencer alias incastle2"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\inside_castle_2.mid");
	mciSendString(TEXT("open Data\\Sounds\\inside_castle_2.mid type sequencer alias incastle3"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\victory_0.mid");
	mciSendString(TEXT("open Data\\Sounds\\victory_0.mid type sequencer alias victory1"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\victory_1.mid");
	mciSendString(TEXT("open Data\\Sounds\\victory_1.mid type sequencer alias victory2"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\battle_0.mid");
	mciSendString(TEXT("open Data\\Sounds\\battle_0.mid type sequencer alias battle1"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\battle_1.mid");
	mciSendString(TEXT("open Data\\Sounds\\battle_1.mid type sequencer alias battle2"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\battle_2.mid");
	mciSendString(TEXT("open Data\\Sounds\\battle_2.mid type sequencer alias battle3"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\battle_3.mid");
	mciSendString(TEXT("open Data\\Sounds\\battle_3.mid type sequencer alias battle4"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\battle_4.mid");
	mciSendString(TEXT("open Data\\Sounds\\battle_4.mid type sequencer alias battle5"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\gameover.mid");
	mciSendString(TEXT("open Data\\Sounds\\gameover.mid type sequencer alias gameover"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\introhappy.mid");
	mciSendString(TEXT("open Data\\Sounds\\introhappy.mid type sequencer alias intro_happy"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\welcometohellmynameiscake.mid");
	mciSendString(TEXT("open Data\\Sounds\\welcometohellmynameiscake.mid type sequencer alias intro_sad"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\boss_1.mid");
	mciSendString(TEXT("open Data\\Sounds\\boss_1.mid type sequencer alias boss_theme"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\gamestart.mid");
	mciSendString(TEXT("open Data\\Sounds\\gamestart.mid type sequencer alias gamestart"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\miniontheme_1.mid");
	mciSendString(TEXT("open Data\\Sounds\\miniontheme_1.mid type sequencer alias shop_1"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\miniontheme_2.mid");
	mciSendString(TEXT("open Data\\Sounds\\miniontheme_2.mid type sequencer alias shop_2"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\kingbattle.mid");
	mciSendString(TEXT("open Data\\Sounds\\kingbattle.mid type sequencer alias kingmusic"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\savefile.mid");
	mciSendString(TEXT("open Data\\Sounds\\savefile.mid type sequencer alias save"),NULL,0,NULL);
	ProgressMove();
	printf("[Loading-Midi] Data\\Sounds\\savenotfound.mid");
	mciSendString(TEXT("open Data\\Sounds\\savenotfound.mid type sequencer alias notfound"),NULL,0,NULL);
	ProgressMove();

}
	
void LoadGame(){

	strcpy(SCREEN_STATUS,"LOADING");

	LoadSounds();

	setCursor(0,20);

	//setup_castle(); // Old File Not Binary
	readBinary(); // Read the new Binary file.
	knightLoad();

	for(int I = 0; I < MAX_ROOMS;I++){
		LoadRoomMap(I,Castle[I]);
		drawBar(20,13,I,47,39);
	}

	strcpy(SCREEN_STATUS,"READY");

}

#pragma endregion

#pragma endregion

#pragma region INTRO

void CreatePlayer(){

	scanf("%s",ply.name);

	if(strcmp(ply.name,"popcorn") == 0)
		IsAdmin = true;

	if(IsAdmin){
		ply.attack = 50;
		ply.defense = 12;
		ply.HP = 9999;
	}else{
		ply.attack = 5;
		ply.defense = 5;
		ply.HP = 100;
	}

	ply.room = 0;
	ply.roundRemains = MELTDOWN_ROUNDS;

	ply.isFigthing = false;

	mciSendString(TEXT("stop all"),NULL,0,NULL);
	mciSendString(TEXT("play gamestart from 13"),NULL,0,NULL);

	system("cls");

	drawFigure("castlebegin",14,3);

	setCursor(5,7);
	slowmo_print("   Let the GAME BEGIN!",100);
	Sleep(3000);
}

void StartIntro()
{
	if(INTRO_ENABLED){

		system("cls");
		mciSendString(TEXT("stop all"),NULL,0,NULL);
		mciSendString(TEXT("play intro_happy from 25"),NULL,0,NULL);
		Sleep(2000);
		drawFigure("scene_1",5,10);
		Sleep(500);
		setCursor(26,4);
		slowmo_print("In a far, far away land...",60);
		Sleep(2000);
		system("cls");
		drawFigure("scene_2",3,13);
		setCursor(22,4);
		slowmo_print("Where people lived happily ever after..",60);
		Sleep(500);
		setCursor(23,5);
		slowmo_print("Ruled by the greatest king of all...",60);
		Sleep(2000);
		system("cls");

		setCursor(23,12);

		SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_RED);
		slowmo_print("It's time to end this fairy tail.",60);
		SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );

		Sleep(1000);
		system("cls");
		mciSendString(TEXT("stop all"),NULL,0,NULL);
		mciSendString(TEXT("play intro_sad from 20"),NULL,0,NULL);
		drawFigure("scene_under",3,1);
		Sleep(1000);
		setCursor(5,23);
		slowmo_print("Forged inside the deepest mountain of the underworld, YOU were born.",60);
		Sleep(1500);

		system("cls");
		drawFigure("scene_demon",3,1);
		slowmo_print("      Being such a fearsome creature you were assigned with a task..\n",60);
		slowmo_print("                       The task of killing the king.",60);
		Sleep(1500);
	
	}

	system("cls");
	setCursor(28,4);
	slowmo_print("What should we call you ??",60);
	setCursor(31,5);
	printf("> ");

	CreatePlayer();
}

void StartEnding(){

	system("cls");
	Sleep(1000);

	PlaySound(TEXT("Data\\Sounds\\beatking_startdisaster_killdramien.wav"),NULL,SND_ASYNC | SND_FILENAME | SND_LOOP );
	thunderEffect();
	drawFigure("king_head",4,0);

	Sleep(1000);

	SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_RED );
	setCursor(1,22);
	slowmo_print("Its All Over....",200);
	setCursor(1,23);
	slowmo_print("The King is finaly dead.",100);
	Sleep(900);
	SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );

	thunderEffect();
	
	system("cls");
	drawFigure("minion",18,3);

	cleanStripe(1,Y_POS + 5,70);
	setCursor(X_POS + 1,Y_POS + 5);
	slowmo_print("Minion : Good Work! Looks like hes not going to bother us anymore..",120);
	Sleep(300);
	thunderEffect();

	system("cls");
	drawFigure("blood_knife",3,3);
	setCursor(X_POS + 1,Y_POS + 5);
	slowmo_print("Minion : Hurry! The castle is falling apart! We need to get out of here!",100);
	Sleep(700);

	thunderEffect();
	system("cls");

	drawFigure("minion",18,3);
	setCursor(X_POS + 1,Y_POS + 5);
	slowmo_print("Minion : You Must head back to the entrance! Or you will die here!",110);
	Sleep(700);
	thunderEffect();
	system("cls");

	ply.hasTreasure = true;
	IsDestroying = true;
	ply.isFigthing = false;

	Castle[ply.room].guardID = -1;
	AI[Castle[ply.room].guardID].HP = -1; // KILL THE KING!

	drawHUD(0,0);
}

void Ending(){

	system("cls");
	Sleep(1000);
	thunderEffect();

	PlaySound(TEXT("Data\\Sounds\\kingdeath.wav"),NULL,SND_FILENAME | SND_LOOP | SND_ASYNC);

	setCursor(32,12);
	slowmo_print("You did it..",120);
	Sleep(2000);

	system("cls");
	drawFigure("kingdeath",12,3);

	setCursor(27,20);
	slowmo_print("The King is finaly Dead..",130);
	Sleep(1000);

	exit(0);
}

#pragma endregion

#pragma region Menu

void drawMenuScroll(int x,int y){

	drawFigure("menuscrl",x, y);

	setCursor(x,y);

	moveCursor(7,3);

		repeated_print(char(10013),3);
	printf("    CASTLE RUN    ");
		repeated_print(char(10013),3);

	moveCursor(1,3);
		printf("1 %c Enter if you dare!",16);
	moveCursor(0,1);
		printf("2 %c  Load Saved Game",16);
	moveCursor(0,2);
		printf("0 %c  Run Away! (Exit)",16);
}

void drawMainMenu(int x, int y){

	drawFigure("castleFigure", x + 2, y + 3);
	drawMenuScroll(40,1);

}

int timeWasted = 0;

void draw_mainmenu()
{
	system("cls");

	fflush(stdin);
	drawMainMenu(0,0);

	char whatToDo[100];

	setCursor(45,16);
		slowmo_print("Are you up to the challenge?", TEXT_SPEED);
	setCursor(45,17);
		printf("> ");
	scanf("%s",&whatToDo);

	if(strcmp(whatToDo,"1") == 0){
		// Do Nothing. Just exit the mainmenu!
	}else if(strcmp(whatToDo,"2") == 0){
		
		system("cls");
		setCursor(28,4);
		slowmo_print("Name of the Save File",60);
		setCursor(31,5);
		printf("> ");

		char saveFile[100];
		scanf("%s",&saveFile);

		loadSaveFile(saveFile);
		Sleep(1500);

		if(!FileLoaded)
			draw_mainmenu();
	
	}else if(strcmp(whatToDo,"0") == 0){
		setCursor(2,23);
		slowmo_print("Come back when you're tough enough...", TEXT_SPEED);
		system ("pause");
		exit(0);
	}else{

		if(strcmp(whatToDo,"getguards") == 0){

			system("cls");

			SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_BLUE );
				printf("\t\t\t================== DEBUG MODE =================\n");
			SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );

			for(int I = 0; I < TOTAL_Knights;I++){
				printf("Guard %s Located at Room %d\n",AI[I].name,AI[I].room);
				printf("\t Attack : %d\n\t Defense : %d\n\t HP : %d\n\t CanMove : %d\n",AI[I].attack,AI[I].defense,AI[I].HP,AI[I].isFrozen);
			}
			printf("\n");
			system("pause");

		}else if(strcmp(whatToDo,"getrooms") == 0){

			system("cls");

			SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_BLUE );
				printf("\t\t\t================== DEBUG MODE =================\n");
			SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );

			for(int I = 0;I < MAX_ROOMS;I++){
				printf("Room %d Info : \n",I);
				printf("\t Desc : %s\n\t Guard : %d \n",Castle[I].disc,Castle[I].guardID);
			}

			printf("\n");
			system("pause");

		}else{

			if (timeWasted == 0)
			{
				setCursor(2,23);
				slowmo_print("Wait what!? I just said (1)/(2)/(0). Try again!", TEXT_SPEED);
				timeWasted++;
			}else if(timeWasted == 1)
			{
				setCursor(2,23);
				slowmo_print("Are you blind or just trying to waste my time? Try again...", TEXT_SPEED);
				timeWasted++;
			}else
			{
				setCursor(2,23);
				slowmo_print("I don't have time for this, I'm a busy program. Bye.", TEXT_SPEED);
				Sleep(1000);
				exit(0);
			}

		}
		Sleep(1000);
		draw_mainmenu();
	}

}

#pragma endregion

#pragma region LOGIC

bool GuardAttacking = false;

void HasPlayerWon(){
	if(ply.hasTreasure && ply.room == 0)
		Ending();
}

void BattleThink(){

	int Rolled = 0;
	bool WasCrit = false;

	GuardAttacking = false;

	cleanStripe(24,1,35);
	setCursor(24,1);
	printf("You are Attacking!");
	Sleep(600);

    Rolled = rollDices(TheDices);

	if(Rolled >= AI[Castle[ply.room].guardID].defense){

		int Attack = ply.attack;
		
		if(TheDices.Dice_1 == TheDices.Dice_2){
			WasCrit = true;
			Attack += CRIT_DAMAGE;
		}

		int HP = AI[Castle[ply.room].guardID].HP - Attack;

		if(HP > 0){

			AI[Castle[ply.room].guardID].HP -= Attack;
			cleanStripe(24,1,35);
			setCursor(24,1);
			printf("You hit the Guard!");

			bloodEffect();

			if(WasCrit)
				printf(" Critical HIT!");

			drawBattleFight(0,0,AI[Castle[ply.room].guardID]);
			Sleep(600);

		}else{

			setCursor(1,25);

			char MSG[500];

			play_victory();

			strcpy(MSG,AI[Castle[ply.room].guardID].name);
			strcat(MSG,": ");
			strcat(MSG,AI[Castle[ply.room].guardID].deathMessage);

			slowmo_print(MSG,60);

			if(strcmp(AI[Castle[ply.room].guardID].name,"Aretur") == 0){

				Sleep(1000);
				system("cls");
				ID_MUSIC = -1;
				mciSendString(TEXT("stop all"),NULL,0,NULL);
				StartEnding();

			}else{

				Sleep(3000);
				play_castle();

				AI[Castle[ply.room].guardID].HP = 0;
				ply.isFigthing = false;
				Castle[ply.room].guardID = -1;	
				system("cls");
				drawHUD(0,0);

				checkTypeOfRoom(); // Check the room.
			}
		}

	}else{
		
		defenseEffect();
		drawBattleFight(0,0,AI[Castle[ply.room].guardID]);

		cleanStripe(24,1,35);
		setCursor(24,1);
		printf("The Guard Dodged the Attack!");
		Sleep(600);
		
	}

	// GUARD ATTACK
	if(ply.isFigthing && AI[Castle[ply.room].guardID].HP > 0){

		cleanStripe(24,1,35);
		setCursor(24,1);
		printf("The Guard is Attacking You!");
		Sleep(600);

		GuardAttacking = true;
		WasCrit = false;

		Rolled = rollDices(TheDices);
		int Attack = AI[Castle[ply.room].guardID].attack;

		if(TheDices.Dice_1 == TheDices.Dice_2){
			WasCrit = true;
			Attack += CRIT_DAMAGE;
		}

		int HP = ply.HP - Attack;

		if(Rolled >= ply.defense){

			if(HP > 0){

				ply.HP -= AI[Castle[ply.room].guardID].attack;

				cleanStripe(24,1,35);
				setCursor(24,1);
				printf("The Guard Hit You!");

				if(WasCrit)
					printf(" Critical HIT!");

				bloodEffect();

				drawBattleFight(0,0,AI[Castle[ply.room].guardID]);

				Sleep(600);

			}else{
				ply.HP = 0;
				GameOver();
			}

		}else{

			defenseEffect();

			drawBattleFight(0,0,AI[Castle[ply.room].guardID]);

			cleanStripe(24,1,35);
			setCursor(24,1);
			printf("You Dodged the Attack!");
			Sleep(600);

		}
	}
}

void Think(){
	while(true){

		#pragma region MIDI_LOOP
		
			if( get_midi_status(ID_MUSIC) == 1){
				if(!ply.isFigthing)
					play_castle();
				else
					play_battle();
			}
		
		#pragma endregion

			if(!ply.isFigthing){ // If the Player is not Fighting
				// Move Everything.

				int OldPos = ply.room;

				setCursor(X_POS,Y_POS);

				playerMove();

				setCursor(X_POS,Y_POS + 1);
				printf("Game is Now Thinking...");

				for(int I = 0; I < TOTAL_Knights;I++)
					if(!AI[I].isFrozen)
						if(AI[I].HP > 0)
							knightMove(AI[I],OldPos,I);
						
				cleanStripe(X_POS,Y_POS,50);

				if(Castle[ply.room].guardID != -1){

					if(AI[Castle[ply.room].guardID].HP > 0){

						setCursor(X_POS,Y_POS);

						if(strcmp(AI[Castle[ply.room].guardID].name,"Merlien") == 0){
							ID_MUSIC = 5;
							mciSendString(TEXT("stop all"),NULL,0,NULL);
							mciSendString(TEXT("play boss_theme from 5"),NULL,0,NULL);
						}else if(strcmp(AI[Castle[ply.room].guardID].name,"Aretur") == 0){
							ID_MUSIC = 6;
							mciSendString(TEXT("stop all"),NULL,0,NULL);
							mciSendString(TEXT("play kingmusic from 10"),NULL,0,NULL);
						}else{
							play_battle();
						}

						Sleep(1000);
						system("cls");
						
						if(strcmp(AI[Castle[ply.room].guardID].name,"Merlien") == 0)
							drawFigure(AI[Castle[ply.room].guardID].image,20,0);
						else
							drawFigure(AI[Castle[ply.room].guardID].image,14,4);

						// INTRO MESSAGE
						SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_RED );

						setCursor(1,23);

						char MSG[500];
						strcpy(MSG,AI[Castle[ply.room].guardID].name);
						strcat(MSG,": ");
						strcat(MSG,AI[Castle[ply.room].guardID].introMessage);

						slowmo_print(MSG,50);
						Sleep(ReadTime);

						SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );

						// PLAYER INTRO SEQUENCE

						system("cls");
						drawFigure("plyintro",16,1);

						SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_GREEN );

						setCursor(1,23);

						char MSG2[500];
						strcpy(MSG2,ply.name);
						strcat(MSG2,": ");
						strcat(MSG2,AI[Castle[ply.room].guardID].plyMessage);

						slowmo_print(MSG2,50);
						Sleep(ReadTime);

						SetConsoleTextAttribute( hStdout, OriginalConsole.wAttributes );

						//

						ply.isFigthing = true;
						ply.initHP = ply.HP;

						AI[Castle[ply.room].guardID].init_HP = AI[Castle[ply.room].guardID].HP;

						Sleep(ReadTime);
						system("cls");

						drawBattleHUD(0,0,AI[Castle[ply.room].guardID]); // Draw Once
						Sleep(ReadTime);

					}else
						Castle[ply.room].guardID = -1; // ANTI-BUG. Guard was dead how can he be there?
					
				}
				
				
				if(!ply.isFigthing){

					applyPoison();

					system("cls");
					drawHUD(0,0);

					checkTypeOfRoom();
					HasPlayerWon();
					nearGuardEmotes(); // Hear the guards move/eat/snore/etc after they move. Meh, disabled for now..
				}

			}else{
				BattleThink();
			}
	}
}

#pragma endregion

int main(){
	// Console SETUP
	system("TITLE Castle Run : The Game - The Movie");
	system("COLOR 0F");
	system("cls");
	srand(time(NULL));
	GetConsoleScreenBufferInfo( hStdout, &OriginalConsole );
	centerConsole();

	// #############
	strcpy(SCREEN_STATUS,"IDLE");

	// LOADING STUFF
	LoadGame();
	// #############

	strcpy(SCREEN_STATUS,"MAINMENU");
	PlaySound(TEXT("Data\\Sounds\\mainmenu.wav"),NULL,SND_ASYNC | SND_FILENAME | SND_LOOP );
	draw_mainmenu();
	PlaySound(NULL,NULL,SND_ASYNC);

	if(FileLoaded == false){
		strcpy(SCREEN_STATUS,"INTRO");
		StartIntro();
	}

	strcpy(SCREEN_STATUS,"PLAY_NORM");
	system("cls");

	drawHUD(0,0);
	Think(); // START GAME LOGIC

	// CASE SOMETHING WRONG HAPPENS ON THE THINK
	system("cls");
	SetConsoleTextAttribute( hStdout, FOREGROUND_INTENSE_RED);
	printf("\n\n\n\n\n\t\a    Something Wrong Happened! You aren't Supose to be Here!");
	Sleep(10000);
	// ############

	return 0;
}