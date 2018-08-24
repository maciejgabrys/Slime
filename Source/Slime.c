#include "Lame2D.h"

enum sTile { sTile_Floor = 0, sTile_Wall = 1, sTile_Box = 2, sTile_Coin = 3, sTile_Match = 4, sTile_Bomb = 5, sTile_Explosion = 6, sTile_Dust = 7, sTile_Internal = 8 };

lmFont* sFont = NULL;
lmGraphic* sOrb = NULL;
lmGraphic* sTileset = NULL;
lmGraphic* sSprite[4] = { NULL, NULL, NULL, NULL };
lmMap* sMap = NULL;
lmGraphic* sRender = NULL;
lmSound* sSound_Intro = NULL;
lmSound* sSound_Coin = NULL;
lmSound* sSound_Match = NULL;
lmSound* sSound_Push = NULL;
lmSound* sSound_Bomb = NULL;
lmBool sFullscreen = lmFalse;
int sLevel_Current;
lmPoint sPlayer;
lmPoint sScroll;
int sDirection;
int sCoins;
int sMatches;

void sInit(void);
void sQuit(void);
void sMap_Load(void);
void sPlayer_Move(int sDelta_Left, int sDelta_Top);
lmBool sMap_HasDust(void);

int main(int argc, char* argv[])
{
	lmInit("Slime", (lmSize) { 320, 240 }, 0xFF180E0E, (lmSize) { 64, 56 }, 30);
	sInit();
	int sFade = 255;
	int sAngle = 0;
	int sCounter = 0;
	int sDistance = 64;
	int sZoom = 0;
	lmBool sMenu_Escaping = lmFalse;
	lmBool sLevel_Loading = lmFalse;
	lmBool sLevel_Escaping;
	enum sScene { sScene_Menu, sScene_Game, sScene_Pause };
	lmStack_Push(sScene_Menu);
	while (lmWindow_IsOpen())
	{
		if (lmKey_Read(lmKey_Return) == lmState_Pressed)
		{
			sFullscreen = !sFullscreen;
			lmWindow_SetMode(sFullscreen);
		}
		lmScreen_Clear(0xFF180E0E);
		switch (lmStack_Peek())
		{
		case sScene_Menu:
			if (sDistance == 6)
			{
				lmDraw_Text((lmRectangle) { 22, 2, 255, 0 }, sFont, "SLIME", 0.0, 0xFFFAFAFA);
				lmDraw_Text((lmRectangle) { 2, 8, 255, 0 }, sFont, "Lame2D tech demo", 0.0, 0xFFFAFAFA);
				lmDraw_Text((lmRectangle) { 128 - sAngle, 44, 255, 0 }, sFont, "Press SPACE to start, ENTER to toggle fullscreen or ESCAPE to exit.", 0.0, 0xFFFAFAFA);
				lmDraw_Text((lmRectangle) { 6, 50, 255, 0 }, sFont, "Kotziauke 2018", 0.0, 0xFFFAFAFA);
			}
			lmGraphic_SetBlending(sOrb, lmBlending_Additive, 0xFFFF0000);
			lmDraw_GraphicEx(sOrb, (lmRectangle) { 20 + (int)(sin(sAngle / 36.0 * M_PI) * sDistance) - sZoom, 16 - (int)(cos(sAngle / 36.0 * M_PI) * sDistance) - sZoom, 24 + sZoom * 2, 24 + sZoom * 2 }, (lmPoint) { 0, 0 }, 0.0, lmFlip_None);
			lmGraphic_SetBlending(sOrb, lmBlending_Additive, 0xFF00FF00);
			lmDraw_GraphicEx(sOrb, (lmRectangle) { 20 + (int)(sin((sAngle + 120) / 36.0 * M_PI) * sDistance) - sZoom, 16 - (int)(cos((sAngle + 120) / 36.0 * M_PI) * sDistance) - sZoom, 24 + sZoom * 2, 24 + sZoom * 2 }, (lmPoint) { 0, 0 }, 0.0, lmFlip_None);
			lmGraphic_SetBlending(sOrb, lmBlending_Additive, 0xFF0000FF);
			lmDraw_GraphicEx(sOrb, (lmRectangle) { 20 + (int)(sin((sAngle + 240) / 36.0 * M_PI) * sDistance) - sZoom, 16 - (int)(cos((sAngle + 240) / 36.0 * M_PI) * sDistance) - sZoom, 24 + sZoom * 2, 24 + sZoom * 2 }, (lmPoint) { 0, 0 }, 0.0, lmFlip_None);
			if (sMenu_Escaping)
			{
				sDistance += 2;
				if (sDistance == 64)
				{
					lmWindow_Close();
				}
			}
			else if (sLevel_Loading)
			{
				sZoom += 2;
				if (sZoom == 64)
				{
					sLevel_Current = 1;
					sMap_Load();
					sLevel_Loading = lmFalse;
					sLevel_Escaping = lmFalse;
					lmStack_Push(sScene_Game);
				}
			}
			else if (sDistance > 6)
			{
				sDistance--;
				if (sDistance == 6)
				{
					lmSound_Control(sSound_Intro, lmAction_Play, 0, lmFalse);
					lmStack_Pop();
					lmStack_Push(sScene_Menu);
				}
			}
			else if (sZoom > 0)
			{
				sZoom -= 2;
			}
			else if (lmKey_Read(lmKey_Space) == lmState_Pressed)
			{
				sLevel_Loading = lmTrue;
			}
			else if (lmKey_Read(lmKey_Escape) == lmState_Pressed)
			{
				sMenu_Escaping = lmTrue;
			}
			break;
		case sScene_Game:
			lmDestroy_Graphic(sRender);
			sRender = lmMap_Render(sMap, sTileset, (lmSize) { 8, 8 });
			lmDraw_Graphic(sRender, (lmPoint) { 0, 0 });
			lmDraw_Graphic(sSprite[sDirection], (lmPoint) { sPlayer.lmLeft * 8 + sScroll.lmLeft, sPlayer.lmTop * 8 + sScroll.lmTop });
			char sLevel_String[] = "Level ?";
			sLevel_String[6] = sLevel_Current + '0';
			lmDraw_Text((lmRectangle) { 2, 50, 64, 0 }, sFont, sLevel_String, 0.0, 0xFFFAFAFA);
			for (int i = 1; i <= 3; i++)
			{
				if (i <= sCoins)
				{
					lmDraw_Rectangle((lmRectangle) { 29 + i * 5, 50, 4, 4 }, 0xFFFACB1E);
				}
				else
				{
					lmDraw_Box((lmRectangle) { 29 + i * 5, 50, 4, 4 }, 0xFFF88F32);
				}
			}
			for (int i = 0; i < sMatches; i++)
			{
				lmDraw_Line((lmPoint) { 61 - i * 2, 51 }, (lmPoint) { 61 - i * 2, 53 }, 0xFFFCD89E);
				lmDraw_Point((lmPoint) { 61 - i * 2, 50 }, 0xFFDC5B41);
			}
			lmDraw_Rectangle((lmRectangle) { 0, 0, 64, 56 }, sFade << 24 | 0xFAFAFA);
			if (sScroll.lmLeft > 0)
			{
				sScroll.lmLeft--;
			}
			else if (sScroll.lmLeft < 0)
			{
				sScroll.lmLeft++;
			}
			else if (sScroll.lmTop > 0)
			{
				sScroll.lmTop--;
			}
			else if (sScroll.lmTop < 0)
			{
				sScroll.lmTop++;
			}
			else if (sCounter > 0)
			{
				sCounter--;
				if (sCounter == 0 && sMap_HasDust())
				{
					sCounter = 6;
				}
			}
			else if (sCoins == 3 || sLevel_Escaping)
			{
				sFade += 15;
				if (sFade == 255)
				{
					if (sLevel_Escaping || sLevel_Current == 4)
					{
						sAngle = 0;
						lmStack_Pop();
					}
					else
					{
						sLevel_Current++;
						sMap_Load();
					}
				}
			}
			else if (sFade > 0)
			{
				sFade -= 15;
			}
			else if (lmKey_Read(lmKey_Space) == lmState_Pressed && sMatches > 0)
			{
				int sDelta_Left = 0;
				int sDelta_Top = 0;
				switch (sDirection)
				{
				case 0:
					sDelta_Top = 1;
					break;
				case 1:
					sDelta_Top = -1;
					break;
				case 2:
					sDelta_Left = 1;
					break;
				case 3:
					sDelta_Left = -1;
					break;
				}
				if (lmMap_GetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left, sPlayer.lmTop + sDelta_Top }) == sTile_Bomb)
				{
					sMatches--;
					lmMap_SetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left, sPlayer.lmTop + sDelta_Top }, sTile_Explosion);
					lmSound_Control(sSound_Bomb, lmAction_Play, 0, lmFalse);
					sCounter = 6;
				}
			}
			else if (lmKey_Read(lmKey_Down) != lmState_Released)
			{
				sDirection = 0;
				sPlayer_Move(0, 1);
			}
			else if (lmKey_Read(lmKey_Up) != lmState_Released)
			{
				sDirection = 1;
				sPlayer_Move(0, -1);
			}
			else if (lmKey_Read(lmKey_Right) != lmState_Released)
			{
				sDirection = 2;
				sPlayer_Move(1, 0);
			}
			else if (lmKey_Read(lmKey_Left) != lmState_Released)
			{
				sDirection = 3;
				sPlayer_Move(-1, 0);
			}
			if (lmKey_Read(lmKey_Escape) == lmState_Pressed)
			{
				sLevel_Escaping = lmTrue;
			}
			break;
		case sScene_Pause:
			lmDraw_Text((lmRectangle) { 20, 26, 64, 0 }, sFont, "Paused", 0.0, 0xFFFAFAFA);
			break;
		case lmStatus_LostFocus:
			lmStack_Pop();
			lmStack_Push(sScene_Pause);
			break;
		case lmStatus_GainedFocus:
			lmStack_Pop();
			lmStack_Pop();
		}
		sAngle += 1;
		sAngle %= 360;
		lmScreen_Refresh();
	}
	sQuit();
	lmQuit();
	return EXIT_SUCCESS;
}

void sInit(void)
{
	sFont = lmFont_Load("Data\\Fonts\\Mr4u.ttf", lmStyle_Normal, 5, 0, lmHinting_Normal, lmFalse);
	sOrb = lmGraphic_Load("Data\\Graphics\\Orb.png");
	sTileset = lmGraphic_Load("Data\\Graphics\\Tileset.png");
	for (int i = 0; i < 4; i++)
	{
		sSprite[i] = lmGraphic_Copy(sTileset, (lmRectangle) { i * 8, 16, 8, 8 });
	}
	sSound_Intro = lmSound_Load("Data\\Sounds\\Intro.wav");
	sSound_Coin = lmSound_Load("Data\\Sounds\\Coin.wav");
	sSound_Match = lmSound_Load("Data\\Sounds\\Match.wav");
	sSound_Push = lmSound_Load("Data\\Sounds\\Push.wav");
	sSound_Bomb = lmSound_Load("Data\\Sounds\\Bomb.wav");
}

void sQuit(void)
{
	lmDestroy_Font(sFont);
	lmDestroy_Graphic(sOrb);
	lmDestroy_Graphic(sTileset);
	for (int i = 0; i < 4; i++)
	{
		lmDestroy_Graphic(sSprite[i]);
	}
	lmDestroy_Map(sMap);
	lmDestroy_Graphic(sRender);
	lmDestroy_Sound(sSound_Intro);
	lmDestroy_Sound(sSound_Coin);
	lmDestroy_Sound(sSound_Match);
	lmDestroy_Sound(sSound_Push);
	lmDestroy_Sound(sSound_Bomb);
}

void sMap_Load(void)
{
	char sLevel_Path[] = "Data\\Maps\\Level?.csv";
	sLevel_Path[15] = sLevel_Current + '0';
	lmDestroy_Map(sMap);
	sMap = lmMap_Load(sLevel_Path, (lmSize) { 8, 6 });
	sPlayer.lmLeft = 0;
	sPlayer.lmTop = 0;
	sScroll.lmLeft = 0;
	sScroll.lmTop = 0;
	sDirection = 0;
	sCoins = 0;
	sMatches = 0;
}

void sPlayer_Move(int sDelta_Left, int sDelta_Top)
{
	lmBool sPlayer_Move = lmFalse;
	switch (lmMap_GetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left, sPlayer.lmTop + sDelta_Top }))
	{
	case sTile_Floor:
		sPlayer_Move = lmTrue;
		break;
	case sTile_Box:
	case sTile_Bomb:
		if (lmMap_GetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left * 2, sPlayer.lmTop + sDelta_Top * 2 }) == sTile_Floor)
		{
			lmMap_SetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left * 2, sPlayer.lmTop + sDelta_Top * 2 }, lmMap_GetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left, sPlayer.lmTop + sDelta_Top }));
			lmMap_SetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left, sPlayer.lmTop + sDelta_Top }, sTile_Floor);
			lmSound_Control(sSound_Push, lmAction_Play, 0, lmFalse);
			sPlayer_Move = lmTrue;
		}
		break;
	case sTile_Coin:
		lmMap_SetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left, sPlayer.lmTop + sDelta_Top }, sTile_Floor);
		sCoins++;
		lmSound_Control(sSound_Coin, lmAction_Play, 0, lmFalse);
		sPlayer_Move = lmTrue;
		break;
	case sTile_Match:
		lmMap_SetTile(sMap, (lmPoint) { sPlayer.lmLeft + sDelta_Left, sPlayer.lmTop + sDelta_Top }, sTile_Floor);
		sMatches++;
		lmSound_Control(sSound_Match, lmAction_Play, 0, lmFalse);
		sPlayer_Move = lmTrue;
		break;
	}
	if (sPlayer_Move)
	{
		sPlayer.lmLeft += sDelta_Left;
		sPlayer.lmTop += sDelta_Top;
		sScroll.lmLeft = sDelta_Left * -7;
		sScroll.lmTop = sDelta_Top * -7;
	}
}

lmBool sMap_HasDust(void)
{
	lmBool sFlag = lmFalse;
	for (int sTop = 0; sTop < 6; sTop++)
	{
		for (int sLeft = 0; sLeft < 8; sLeft++)
		{
			if (lmMap_GetTile(sMap, (lmPoint) { sLeft, sTop }) == sTile_Dust)
			{
				lmMap_SetTile(sMap, (lmPoint) { sLeft, sTop }, sTile_Floor);
			}
		}
	}
	for (int sTop = 0; sTop < 6; sTop++)
	{
		for (int sLeft = 0; sLeft < 8; sLeft++)
		{
			if (lmMap_GetTile(sMap, (lmPoint) { sLeft, sTop }) == sTile_Explosion)
			{
				lmMap_SetTile(sMap, (lmPoint) { sLeft, sTop }, sTile_Dust);
				for (int sRange_Top = sTop - 1; sRange_Top <= sTop + 1; sRange_Top++)
				{
					for (int sRange_Left = sLeft - 1; sRange_Left <= sLeft + 1; sRange_Left++)
					{
						switch (lmMap_GetTile(sMap, (lmPoint) { sRange_Left, sRange_Top }))
						{
						case sTile_Floor:
						case sTile_Box:
						case sTile_Coin:
						case sTile_Match:
							lmMap_SetTile(sMap, (lmPoint) { sRange_Left, sRange_Top }, sTile_Dust);
							break;
						case sTile_Bomb:
							lmMap_SetTile(sMap, (lmPoint) { sRange_Left, sRange_Top }, sTile_Internal);
							lmSound_Control(sSound_Bomb, lmAction_Play, 0, lmFalse);
							break;
						}
					}
				}
				sFlag = lmTrue;
				break;
			}
		}
	}
	for (int sTop = 0; sTop < 6; sTop++)
	{
		for (int sLeft = 0; sLeft < 8; sLeft++)
		{
			if (lmMap_GetTile(sMap, (lmPoint) { sLeft, sTop }) == sTile_Internal)
			{
				lmMap_SetTile(sMap, (lmPoint) { sLeft, sTop }, sTile_Explosion);
			}
		}
	}
	return sFlag;
}
