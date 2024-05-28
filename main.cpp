/*Made by Youssef Tarek 20100326
  Made by Seif Ahmed Mansour 20102592
*/
#include <windows.h>
#include <mmsystem.h>
#include <GL/glut.h>
#include <cstdio>
#include <math.h>
#include <ctime>
#include <chrono>
#include <vector>
#include <array>

#if __has_include("songinfo.h")
# include "songinfo.h"
#else
#define SONG_BPM 87.5
#define SONG_OFFSET 620
#define SONG_DURATION 244800
#define SONG_PATH "audio lowered.wav"
#endif

#define PI 3.14159265359

const int playerCoordinates[][2] = {{0, 0}, {-2, -2}, {-3, 0}, {0, 3}, {3, 0}, {2, -2}};
int playerDirection = 0; // 0:UP 1:DOWN 2:LEFT 3:RIGHT
float playerSize = 1.0f;
float playerAngleShift = 0.0f;
float playerLastEatenArrowType = false;
float eatenIndicatorOpacity = 0.0f;

bool running = false; // game is running
bool ranBefore = false; // timer has started before
long long lastDisplayTime = 0; // time from last frame
long long nextBeatTime = 0;

int difficulty = 0; // 0:easy 1:medium 2:hard

int score = 0;
int combo = 0;
int logWidth = 100;
int logHeight = 100;
int phyHeight = 800;
int phyWidth = 800;
int centerX = logWidth / 2;
int centerY = logHeight / 2;
int timerInterval = 1000 / 60; // Interval for the timer in milliseconds

long long startTime;
int randomNumber;

int lastBgColor = 0;
float backgroundAngle = 0;

long long getCurrentTime()
{
	auto now = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
	long long milliseconds = duration.count();

	return milliseconds;
}

float getAngleFromDirection(int d)
{
	switch (d)
	{
	case 0:
		return 0; // UP
	case 1:
		return PI; // DOWN
	case 2:
		return PI / 2; // LEFT
	case 3:
		return -PI / 2; // RIGHT
	default:
		break;
	}
}

class Animation {
private:
	int percentageLength;
	long long startTime;
	float p[10][2];
	float* value;
	int duration;

	long long getEndTime() {
		return startTime + (long long) duration;
	}

public:
	bool running;

	Animation(float* value, int duration) {
		this->value = value;
		this->duration = duration;

		this->percentageLength = 0;
		this->running = false;
	}

	void addPercentage(float percentage, float newValue) {
		p[percentageLength][0] = percentage;
		p[percentageLength][1] = newValue;
		percentageLength++;
	}

	void update() {
		if (!running)
			return;

		long long currentTime = getCurrentTime();
		if (currentTime > getEndTime()) {
			this->running = false;
			*value = p[percentageLength - 1][1];
			return;
		}

		float percentage = (float) (currentTime - startTime) / (float) duration;

		int i;
		for (i = 0; i < percentageLength; i++)
			if (percentage < p[i + 1][0])
				break;
			else if (percentage == p[i + 1][0]) {
				*value = p[i + 1][1];
				return;
			}

		float newValue = p[i+1][1] + (((p[i][1] - p[i+1][1]) * (p[i+1][0] - percentage))
			/ (p[i+1][0] - p[i][0]));

		*value = newValue;
	}

	void start() {
		if (percentageLength < 2)
			return;
		this->running = true;
		this->startTime = getCurrentTime();
	}
};

// done
class Print
{
public:
	static void printSome(char *str, int x, int y)
	{
		glRasterPos2d(x, y);
		for (int i = 0; i < strlen(str); i++)
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
		glFlush();
	}
};
bool inStartMenu = true;

class StartMenu
{
public:
	static void draw()
	{
		glClearColor(0.969f, 0.0f, 0.388f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glPushMatrix();
		glTranslatef(-30 + centerX, centerY, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POLYGON);
		glVertex2f(-10, -5);
		glVertex2f(-10, 5);
		glVertex2f(10, 5);
		glVertex2f(10, -5);
		glEnd();

		glColor3f(0.0f, 0.0f, 0.0f);
		Print::printSome((char *)"Easy", -3, -1);

		glPopMatrix();

		glPushMatrix();
		glTranslatef(centerX, centerY, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POLYGON);
		glVertex2f(-10, -5);
		glVertex2f(-10, 5);
		glVertex2f(10, 5);
		glVertex2f(10, -5);
		glEnd();

		glColor3f(0.0f, 0.0f, 0.0f);
		Print::printSome((char *)"Normal", -3, -1);

		glPopMatrix();

		glPushMatrix();
		glTranslatef(30 + centerX, centerY, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POLYGON);
		glVertex2f(-10, -5);
		glVertex2f(-10, 5);
		glVertex2f(10, 5);
		glVertex2f(10, -5);
		glEnd();

		glColor3f(0.0f, 0.0f, 0.0f);
		Print::printSome((char *)"Hard", -3, -1);

		glPopMatrix();
	}
};
// done
class Lines
{
public:
	static void line_design()
	{
		glPushMatrix();
		glTranslatef(centerX, centerY, 0.0f);

		glColor3f(1.0f, 1.0f, 1.0f);

		glBegin(GL_LINES);
		glVertex2f(7, 7);
		glVertex2f(18, 18);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(-7, 7);
		glVertex2f(-18, 18);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(7, -7);
		glVertex2f(18, -18);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(-7, -7);
		glVertex2f(-18, -18);
		glEnd();

		glPopMatrix();
	}
};
// TRY TO WORK
class Enemy
{
public:
	int direction;
	int targetDirection;
	bool shifting;
	bool shifted;
	float radius;
	float angle;
	float initialAngle;
	float speed;
	long long spawnTime;

	Enemy() {};

	Enemy(int direction, bool shifting)
	{
		this->direction = direction;
		this->shifting = shifting;

		shifted = false;

		this->radius = 48;
		this->speed = 370 * 87.5 / 60000;

		if (!shifting)
			this->targetDirection = direction;
		else
		{
			switch (direction)
			{
			case 0:
				this->targetDirection = 1;
				break;
			case 1:
				this->targetDirection = 0;
				break;
			case 2:
				this->targetDirection = 3;
				break;
			case 3:
				this->targetDirection = 2;
				break;
			default:
				break;
			}
		}

		this->angle = getAngleFromDirection(direction);
		this->initialAngle = angle;
		this->spawnTime = getCurrentTime();
	}

	float getX()
	{
		return -radius * cos(angle - PI / 2);
	}

	float getY()
	{
		return -radius * sin(angle - PI / 2);
	}

	void update(float deltaTime)
	{
		radius -= speed * deltaTime / timerInterval;
		// radius -= speed;

		long long currentTime = getCurrentTime();
		if (shifting && !shifted && currentTime > spawnTime + 400) // only start rotation animation after 400ms from spawn time
		{
			float deltaAngle = PI - (angle - initialAngle);
			if (deltaAngle < 0.4)
				deltaAngle = 0.4;
			angle += 0.08 * deltaAngle;
			if (angle - initialAngle > PI)
			{
				shifted = true;
				angle = initialAngle + PI;
			}
		}
	}

	void draw()
	{
		glPushMatrix();
		glTranslatef(getX() + centerX, getY() + centerX, 0.0f);
		glRotatef(angle * 180 / PI, 0.0f, 0.0f, 1.0f);

		if (shifting)
			glColor3f(0.01f, 0.70f, 0.79f);
		else
			glColor3f(0.97f, 0.85f, 0.29f);

		glBegin(GL_POLYGON);

		glVertex2f(0, 0);
		glVertex2f(-1, 1);
		glVertex2f(-2, 0);
		glVertex2f(0, -2);
		glVertex2f(2, 0);
		glVertex2f(1, 1);

		glEnd();

		glPopMatrix();
	}
};

std::vector<Enemy> enemies;
std::vector<Animation> animations;
std::vector<std::array<int, 2>> backgrounds; // {[color_code, size], ...}

class beat
{
public:
	static void start_music()
	{
		sndPlaySound(SONG_PATH, SND_ASYNC);
	}
	static void stop_music()
	{
		PlaySound(NULL, NULL, 0);
	}
};

class Player_Design
{
public:
	static void layout()
	{
		// layout rhombus
		glPushMatrix();
		glTranslatef(centerX, centerY, 0.0f);
		float layoutvertices[4][2] = {
			{0, 48},
			{48, 0},
			{0, -48},
			{-48, 0}};
		// creating layout
		glColor3f(0.133f, 0.129f, 0.2f);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 4; i++)
		{
			glVertex2f(layoutvertices[i][0], layoutvertices[i][1]);
		}
		glEnd();
		glPopMatrix();
	}
	static void interact_outer()
	{
		// outer rhombus
		glPushMatrix();
		glTranslatef(centerX, centerY, 0.0f);
		float vertices1[4][2] = {
			{0, 10},
			{10, 0},
			{0, -10},
			{-10, 0}};
		// creating outer rhombus
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 4; i++)
		{
			glVertex2f(vertices1[i][0], vertices1[i][1]);
		}
		glEnd();
		glPopMatrix();
	}
	static void design()
	{
		glPushMatrix();
		glTranslatef(centerX, centerY, 0.0f);
		// inner rhombus
		float vertices2[4][2] = {
			{0, 6},
			{6, 0},
			{0, -6},
			{-6, 0}};
		// creating inner rhombus
		glColor3f(0.133f, 0.129f, 0.2f);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 4; i++)
		{
			glVertex2f(vertices2[i][0], vertices2[i][1]);
		}
		glEnd();
		glPopMatrix();
	}
};

void returnToStartMenu();
class Control_Player
{
public:
	static void keyboard(unsigned char key, int x, int y)
	{
		// closing game key
		switch (key)
		{
		case 27: // escape
			returnToStartMenu();
			// exit(0);
			break;
		case 'w':
			playerDirection = 0;
			break;
		case 's':
			playerDirection = 1;
			break;
		case 'a':
			playerDirection = 2;
			break;
		case 'd':
			playerDirection = 3;
			break;
		default:
			break;
		}
		// if(key == 13){ //enter
		//	Time::stopwatch();
		//	glutSpecialFunc(specialKeyboard);
		// }
	}
	// control player
	static void specialKeyboard(int key, int x, int y)
	{
		switch (key) {
			case GLUT_KEY_UP:
				playerDirection = 0;
				break;
			case GLUT_KEY_DOWN:
				playerDirection = 1;
				break;
			case GLUT_KEY_LEFT:
				playerDirection = 2;
				break;
			case GLUT_KEY_RIGHT:
				playerDirection = 3;
				break;
			default:
				break;
		}

		glutPostRedisplay();
	}
	static void start_control()
	{
		glutKeyboardFunc(keyboard);
		glutSpecialFunc(specialKeyboard);
	}
	// player animation
	static void player_arrow()
	{
		glPushMatrix();
		glTranslatef(centerX, centerY, 0.0f);
		glScalef(playerSize, playerSize, 1.0f);

		glRotatef(
			(getAngleFromDirection(playerDirection) + playerAngleShift) * 180 / PI,
			0.0f, 0.0f, 1.0f
		);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 6; i++)
			glVertex2f(playerCoordinates[i][0], playerCoordinates[i][1]);
		glEnd();

		if (eatenIndicatorOpacity != 0)
		{
			if (playerLastEatenArrowType)
				glColor4f(0.01f, 0.70f, 0.79f, eatenIndicatorOpacity);
			else
				glColor4f(0.97f, 0.85f, 0.29f, eatenIndicatorOpacity);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBegin(GL_LINE_STRIP);
			glVertex2f(9, 1);
			glVertex2f(0, 10);
			glVertex2f(-9, 1);
			glEnd();
		}

		glPopMatrix();
	}
};

void onbeat()
{
	if (!running)
		return;

	int randomDirection = rand() % 4;

	int randomNumber = rand() % 100;

	Enemy newEnemy;

	switch (difficulty) {
		case 0:
			newEnemy = Enemy (randomDirection, false); // yellow only
			enemies.push_back(newEnemy);
			break;
		case 1:
			newEnemy = Enemy (randomDirection, randomNumber < 20); // 20% blue
			enemies.push_back(newEnemy);
			break;
		case 2:
			newEnemy = Enemy (randomDirection, true); // blue only
			enemies.push_back(newEnemy);
			break;
		default:
			break;
	}

	backgrounds.push_back({lastBgColor++ % 3, 70});
}

void update()
{
	if (!running)
		return;

	// check if song has ended -> set running to false && return
	long long currentTime = getCurrentTime();
	if (currentTime - startTime > SONG_DURATION) {
		returnToStartMenu();
		return;
	}
	float deltaTime = static_cast<float>(currentTime - lastDisplayTime);
	backgroundAngle += 0.3;

	if (currentTime >= nextBeatTime)
	{
		onbeat();
		nextBeatTime = currentTime + (long long)(60000 / SONG_BPM);
	}

	auto bg = backgrounds.begin();
	while (bg != backgrounds.end())
	{
		(*bg)[1] += 1; // increase size of background
		if ((*bg)[1] > 400) // delete background if size is greater than 400
			bg = backgrounds.erase(bg);
		else
			bg++;
	}

	auto animation = animations.begin();
	while (animation != animations.end())
	{
		if (animation->running)
		{
			animation->update();
			animation++;
		} else
			animation = animations.erase(animation);
	}

	auto enemy = enemies.begin();
	while (enemy != enemies.end())
	{
		enemy->update(deltaTime);
		if (enemy->radius <= 10.0f)
		{
			if (playerDirection == enemy->targetDirection)
			{ // successful arrow hit
				score++;
				combo++;
				playerLastEatenArrowType = enemy->shifting;
				enemy = enemies.erase(enemy);

				Animation enlargePlayerSizeAnimation(&playerSize, 200);
				enlargePlayerSizeAnimation.addPercentage(0, playerSize);
				enlargePlayerSizeAnimation.addPercentage(0.5, playerSize + 0.2);
				enlargePlayerSizeAnimation.addPercentage(1, playerSize);

				Animation showEatenIndicatorAnimation(&eatenIndicatorOpacity, 200);
				showEatenIndicatorAnimation.addPercentage(0, 0);
				showEatenIndicatorAnimation.addPercentage(0.33, 1);
				showEatenIndicatorAnimation.addPercentage(0.66, 1);
				showEatenIndicatorAnimation.addPercentage(1, 0);

				enlargePlayerSizeAnimation.start();
				animations.push_back(enlargePlayerSizeAnimation);
				showEatenIndicatorAnimation.start();
				animations.push_back(showEatenIndicatorAnimation);
			}
			else
			{ // unsuccessful arrow hit
				combo = 0;
				enemy = enemies.erase(enemy);

				Animation shiftPlayerAngleAnimation(&playerAngleShift, 200);
				shiftPlayerAngleAnimation.addPercentage(0, playerAngleShift);
				shiftPlayerAngleAnimation.addPercentage(0.33, playerAngleShift + 0.2);
				shiftPlayerAngleAnimation.addPercentage(0.66, playerAngleShift - 0.2);
				shiftPlayerAngleAnimation.addPercentage(1, playerAngleShift);

				shiftPlayerAngleAnimation.start();
				animations.push_back(shiftPlayerAngleAnimation);
			}
		}
		else
			enemy++;
	}

	lastDisplayTime = currentTime;
}

void drawAnimatedBackrounds(std::array<int, 2> background)
{
	glPushMatrix();
	glTranslatef(centerX, centerY, 0.0f);
	glRotatef(backgroundAngle, 0.0f, 0.0f, 1.0f);

	glScalef(background[1] * 0.05, background[1] * 0.05, 1.0f);

	switch (background[0])
	{
	case 0:
		glColor3f(0.0f, 0.706f, 0.588f);
		break;
	case 1:
		glColor3f(0.906f, 0.816f, 0.039f);
		break;
	case 2:
		glColor3f(0.0f, 1.0f, 1.0f);
		break;
	}

	glBegin(GL_POLYGON);
	glVertex2f(0, 10);
	glVertex2f(10, 0);
	glVertex2f(0, -10);
	glVertex2f(-10, 0);
	glEnd();

	glPopMatrix();
}

class Time
{
public:
	static void timerCallback(int value)
	{
		update();

		glutPostRedisplay();
		glutTimerFunc(timerInterval, timerCallback, 0);
	}

	void startTimer()
	{
		if (ranBefore)
			return;
		ranBefore = true;
		glutTimerFunc(timerInterval, timerCallback, 0);
	}
	static void stopwatch()
	{
		char timeString[50];
		char scoreString[50];
		char comboString[50];
		// interaction
		int duration = getCurrentTime() - startTime;
		int milliseconds = duration % 1000;
		int seconds = duration / 1000;

		// no interaction
		// int milliseconds=0;
		// int seconds=0;
		sprintf(timeString, "%02d.%03d", seconds, milliseconds);
		glColor3f(1.0f, 1.0f, 1.0f);
		Print::printSome(timeString, centerX - 3.5, centerY + 25);
		// Convert score & combo to string
		sprintf(scoreString, "Score:   %d", score);
		Print::printSome(scoreString, centerX - 3.5, centerY - 25);
		sprintf(comboString, "Combo: %d", combo);
		Print::printSome(comboString, centerX - 3.5, centerY - 28);
		glutPostRedisplay();
	}
};

class Show
{
public:
	void init2D()
	{
		glClearColor(1.0f, 0.0f, 0.392f, 1.0f);
		glMatrixMode(GL_PROJECTION);
		gluOrtho2D(0.0, logWidth, 0.0, logHeight);
	}
	void start_display()
	{
		glutDisplayFunc(display);
	}
	static void display()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		if (inStartMenu)
		{
			StartMenu::draw();
		}
		else
		{
			// glMatrixMode(GL_PROJECTION);
			// glLoadIdentity();
			// glOrtho(-50.0f, 50.0f, -50.0f, 50.0f, 0.0f, 1.0f);
			// glMatrixMode(GL_MODELVIEW);
			// glLoadIdentity();

			// pattern
			// Background::pattern();
			auto bg = backgrounds.begin();
			while (bg != backgrounds.end())
			{
				drawAnimatedBackrounds(*bg);
				bg++;
			}

			Player_Design::layout(); // black rhombus

			auto enemy = enemies.begin();
			while (enemy != enemies.end())
			{
				enemy->draw();
				enemy++;
			}

			Player_Design::interact_outer(); // outerplayer
			Player_Design::design();		 // inner player
			// creating 4 lines
			Lines::line_design();
			// control
			Control_Player::start_control();

			Control_Player::player_arrow();
			Time::stopwatch();
		}
		glutSwapBuffers();
	}
};

void returnToStartMenu() {
	beat::stop_music();
	inStartMenu = true;
	running = false;
	backgrounds.clear();
	enemies.clear();
}

void startGame(int diff = 0) {
	difficulty = diff;
	inStartMenu = false;
	// Start the game
	Time timer;
	nextBeatTime = getCurrentTime() + (long long)SONG_OFFSET;
	timer.startTimer();
	score = 0;
	combo = 0;
	running = true;
	startTime = getCurrentTime();
	// Start the music
	beat::start_music();
}

// Function to handle mouse click in the start menu
void handleStartMenuClick(int button, int state, int x, int y)
{
	if (!inStartMenu)
		return;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		// Check if the click occurred within the "Start" button area
		if (x >= 10 * 8 && x <= 30 * 8 && y >= 45 * 8 && y <= 55 * 8)
		{
			startGame(0);
		}
		else if (x >= 40 * 8 && x <= 60 * 8 && y >= 45 * 8 && y <= 55 * 8)
		{
			startGame(1);
		}
		else if (x >= 70 * 8 && x <= 90 * 8 && y >= 45 * 8 && y <= 55 * 8)
		{
			startGame(2);
		}
	}
}

//=======================================================================
int main(int argc, char **argv)
{
	enemies.reserve(20);
	backgrounds.reserve(10);
	Show show;
	glutInit(&argc, argv);
	glutInitWindowSize(phyWidth, phyHeight);
	glutInitWindowPosition(10, 10);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Project Rhombus");
	glutMouseFunc(handleStartMenuClick);
	// glutFullScreen();
	show.init2D();
	show.start_display();
	glutMainLoop();
}
