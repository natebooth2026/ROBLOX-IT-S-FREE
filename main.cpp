#include <GL/glut.h>
#include <GL/freeglut.h>
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <irrKlang/irrKlang.h>
using namespace irrklang;

//EXTERNAL SOURCE BATTLE MUSIC <NEED TO DOCUMENT IN README.txt>: https://www.youtube.com/watch?v=S-XBFN6LR0Y
//EXTERNAL SOURCE GUN SHOT SOUND EFFECT: https://www.youtube.com/watch?v=bdjxBg55mRk
//EXTERNAL SOURCE GUN HIT SOUND EFFECT: https://www.youtube.com/watch?v=n49bNiFBvgU

/// 
/// AUDIO/DANCE RELATED
/// 
bool playing = false; //indicates if music is playing or not
bool dancing = false; //indicates if we are dancing or not
bool flipped = false; //indicates if we hit y-boundary during dance
const double DANCE_ROTATE_ROC = 8.0; //holds ROC of dance rotation during dance
ISoundEngine* sfxEngine = createIrrKlangDevice();

//graphics related
bool solid = true;
bool wire = false;
bool axes = true;
double cubeSize = 0.2;
const int NUM_OF_ROBOTS = 20;
int WIN_W = 640; //width
int WIN_H = 480; //height
const float FRACTION = 0.2f; //for camera modifications
bool menuOpen = false;

//camera related
bool camera2 = true; //holds if whole-scene cam is on/off
bool cameraFlip = false; //holds if first-person cam and whole-scene cam have swapped viewports
float theta = 0.0; //holds the angle of camera rotation
float cameraX = 0.0;
const float cameraY = 0.35;
double cameraZ = 3.0;
float cameraLX = 0.0;
float cameraLZ = -1.0;

//arcball related
GLint left, right; //holds state of left and right mouse buttons
int mX = 0, mY = 0; //holds mouse position
float arcballTheta = 2.8, arcballPhi = 2.0, arcballRadius = 7.0; //holds arcball camera rotation
float arcballX, arcballY, arcballZ; //holds arcball camera position

///
/// COLOR-RELATED
/// 
const double headColors[3] = { 1.0, 1.0, 0.0 }; //this and 4 below hold character colors
const double leftArmColors[3] = { 1.0, 1.0, 0.0 };
const double bodyColors[3] = { 0.0, 0.0, 1.0 };
const double rightArmColors[3] = { 1.0, 1.0, 0.0 };
const double leftLegColors[3] = { 0.0, 1.0, 0.0 };
const double rightLegColors[3] = { 0.0, 1.0, 0.0 };

///
/// GAME-RELATED
/// 
int score = 0;
const int SCORE_MODIF = 10;
int kills = 0;
const int STARTING_TIMER = 30;
int timer = STARTING_TIMER;
const float FAST_SPEED = 1.5f;
const float SLOW_SPEED = FAST_SPEED / 4;
const float VERY_FAST_SPEED = FAST_SPEED * 3;
float currentSpeed = FAST_SPEED;
int menuID = 0;
const float GROUND_SIZE = 10.0;
const float BULLET_RADIUS = 0.025;
const float ROBOT_RADIUS = 0.095 + ((0.36 * (cubeSize / 0.5)));
bool showColliders = false;
bool canShoot = true;

//resetting
bool reset = false;

//BONUS TOGGLES
bool walkToggle = false;
bool extraSounds = false;

//robot instances
class robot {
public:
	robot() : rID_(-1), x_(0.0), y_(0.0), z_(0.0), cubeSize_(cubeSize),
		rX_(0.0), rY_(0.0), rZ_(0.0),
		xAdj_(0.0), yAdj_(0.0), zAdj_(0.0), 
		limbRX_(0.0), limbRY_(0.0), limbRZ_(0.0), walkX_(0.0), 
		walkFX_(((rand() % 2) - 1.0) / 10), walkFZ_(((rand() % 2) - 1.0) / 10),
		flipped_(false), limbFlipped_(false), walkFlipped_(false)
	{
		while (walkFX_ == 0) walkFX_ = ((rand() % 2) - 1.0) / 10;
		while (walkFZ_ == 0) walkFZ_ = ((rand() % 2) - 1.0) / 10;
	}

	//controls walking
	void walk() {
		if (!walkStopped_ && walkToggle) {
			if (x_ + walkFX_ <= (-GROUND_SIZE / 2.0)) {
				x_ = -GROUND_SIZE / 2.0;
				walkFX_ = -((rand() % 2) - 1.0) / 10.0;
			}
			else if (x_ + walkFX_ >= (GROUND_SIZE / 2.0)) {
				x_ = GROUND_SIZE / 2.0;
				walkFX_ = ((rand() % 2) - 1.0) / 10.0;
			}
			else {
				x_ += walkFX_;
			}
			if (z_ + walkFZ_ <= (-GROUND_SIZE / 2.0)) {
				z_ = -GROUND_SIZE / 2.0;
				walkFZ_ = -((rand() % 2) - 1.0) / 10.0;
			}
			else if (z_ + walkFZ_ >= (GROUND_SIZE / 2.0)) {
				z_ = GROUND_SIZE / 2.0;
				walkFZ_ = (rand() % 2) - 1.0 / 10.0;
			}
			else {
				z_ += walkFZ_;
			}
		}
	}

	bool canDelete() const { return canDelete_; }

	//draws the character part-by-part
	void drawCharacter() {
		if (y_ < 3) {
			//walking animation
			if (!dancing && !walkStopped_ && walkToggle) {
				if (walkX_ < 20.0 && !walkFlipped_) {
					walkX_ += DANCE_ROTATE_ROC / 10;
					if (walkX_ > 20.0) walkFlipped_ = true;
				}
				else if (walkX_ <= 30.0 && walkFlipped_) {
					walkX_ -= DANCE_ROTATE_ROC / 10;
					if (walkX_ <= -20.0) walkFlipped_ = false;
				}
			}
			else if (!walkToggle) {
				walkX_ = 0.0;
			}

			glPushMatrix();

			//position control
			glTranslatef(x_, y_, z_);

			//collider sphere display
			if (showColliders) {
				glPushMatrix();
				glColor3f(1.0, 0.0, 0.0);
				glutWireSphere(colliderRadius_, 10, 10);
				glPopMatrix();
			}

			//rotation control for dancing
			glRotatef(rZ_, 0.0, 0.0, 1.0);
			glRotatef(rY_, 0.0, 1.0, 0.0);
			glRotatef(rX_, 1.0, 0.0, 0.0);

			//drawing the left leg
			glPushMatrix();
			glRotatef(walkX_, 1.0, 0.0, 0.0);
			glTranslatef(-0.12 * (cubeSize / 0.5), -0.495 * (cubeSize / 0.5), 0.0);
			glScalef(0.45, 1.0, 0.75);

			glColor3f(1.0, 1.0, 1.0);
			if (wire) glutWireCube(cubeSize);

			glColor3f(leftLegColors[0], leftLegColors[1], leftLegColors[2]);
			if (solid) glutSolidCube(cubeSize);
			glPopMatrix();

			//drawing the right leg
			glPushMatrix();
			glRotatef(-walkX_, 1.0, 0.0, 0.0);
			glTranslatef(0.12 * (cubeSize / 0.5), -0.495 * (cubeSize / 0.5), 0.0);
			glScalef(0.45, 1.0, 0.75);

			glColor3f(1.0, 1.0, 1.0);
			if (wire) glutWireCube(cubeSize);

			glColor3f(rightLegColors[0], rightLegColors[1], rightLegColors[2]);
			if (solid) glutSolidCube(cubeSize);
			glPopMatrix();

			//drawing the left arm
			glPushMatrix();
			glTranslatef(0.0, limbRX_ / 875, 0.0);
			glRotatef(limbRX_, 1.0, 0.0, 0.0);
			glTranslatef(-0.36 * (cubeSize / 0.5), 0.0, 0.0);
			glScalef(0.5, 1.0, 0.75);

			glColor3f(1.0, 1.0, 1.0);
			if (wire) glutWireCube(cubeSize);

			glColor3f(leftArmColors[0], leftArmColors[1], leftArmColors[2]);
			if (solid) glutSolidCube(cubeSize);
			glPopMatrix();

			//drawing the body
			glPushMatrix();
			glScalef(0.95, 1.0, 0.75);

			glColor3f(1.0, 1.0, 1.0);
			if (wire) glutWireCube(cubeSize);

			glColor3f(bodyColors[0], bodyColors[1], bodyColors[2]);
			if (solid) glutSolidCube(cubeSize);
			glPopMatrix();

			//drawing the right arm
			glPushMatrix();
			glTranslatef(0.0, limbRX_ / 875, 0.0);
			glRotatef(limbRX_, 1.0, 0.0, 0.0);
			glTranslatef(0.36 * (cubeSize / 0.5), 0.0, 0.0);
			glScalef(0.5, 1.0, 0.75);

			glColor3f(1.0, 1.0, 1.0);
			if (wire) glutWireCube(cubeSize);

			glColor3f(rightArmColors[0], rightArmColors[1], rightArmColors[2]);
			if (solid) glutSolidCube(cubeSize);
			glPopMatrix();

			// drawing the head
			glPushMatrix();
			glTranslatef(0.0, (0.66 * 0.6) * (cubeSize / 0.5), 0.0);
			glScalef(0.57, 0.6, 0.75);

			glColor3f(1.0, 1.0, 1.0);
			if (wire) glutWireCube(cubeSize);

			glColor3f(headColors[0], headColors[1], headColors[2]);
			if (solid) glutSolidCube(cubeSize);
			glPopMatrix();

			glPopMatrix();
		}	
	}

	//changes robot position
	void changePos(float x, float y, float z) { x_ = x; y_ = y; z_ = z; }

	float getX() const { return x_; }
	float getY() const { return y_; }
	float getZ() const { return z_; }
	void setID(int id) { rID_ = id; }

	//controls dance mechanics
	void dance() {
		walkX_ = 0.0;
		if (yAdj_ <= (0.50 * (rID_ + 1)) && !flipped_) { //going up
			yAdj_ += (0.05 * (rID_ + 1));
			y_ += (0.05 * (rID_ + 1));
			if (yAdj_ > (0.50 * (rID_ + 1))) flipped_ = true;
		}
		else if (yAdj_ <= (0.6 * (rID_ + 1)) && flipped_) { //going down
			yAdj_ -= (0.05 * (rID_ + 1));
			y_ -= (0.05 * (rID_ + 1));
			if (yAdj_ <= 0.0) flipped_ = false;
		}
		if (limbRX_ < 180.0 && !limbFlipped_) { //arms going up
			limbRX_ += DANCE_ROTATE_ROC;
			if (limbRX_ > 180.0) limbFlipped_ = true;
		}
		else if (limbRX_ <= 190.0 && limbFlipped_) { //arms going down
			limbRX_ -= DANCE_ROTATE_ROC;
			if (limbRX_ <= 0.0) limbFlipped_ = false;
		}
		rY_ += DANCE_ROTATE_ROC;
	}

	//resets all original attributes of robot after dancing ends
	void danceReset() {
		x_ -= xAdj_;
		y_ -= yAdj_;
		z_ -= zAdj_;
		rX_ = 0.0;
		rY_ = 0.0;
		rZ_ = 0.0;
		limbRX_ = 0.0;
		limbRY_ = 0.0;
		limbRZ_ = 0.0;
		xAdj_ = 0.0;
		yAdj_ = 0.0;
		zAdj_ = 0.0;
		flipped_ = false;
		limbFlipped_ = false;
	}

	//stops walking in event of death
	void stopWalk() {
		danceReset();
		walkStopped_ = true;
		walkX_ = 0.0;
		if (y_ < 3) {
			y_ += 0.05;
		}
		else {
			canDelete_ = true;
		}
	}

	//collision trackers
	bool collided_ = false;
	float colliderRadius_ = ROBOT_RADIUS;

	bool walkStopped_ = false; //stopping walk movement for death animation
private:
	int rID_; //ID num

	//position
	float x_;
	float y_;
	float z_;

	//size and collider
	float cubeSize_;

	//adjustment variables (position and rotation)
	float xAdj_;
	float yAdj_;
	float zAdj_;
	float rX_;
	float rY_;
	float rZ_;
	float limbRX_;
	float limbRY_;
	float limbRZ_;
	float walkX_;

	//walk forward vector
	float walkFX_, walkFZ_;

	//tracking
	bool flipped_; //flipping y-adjustment
	bool limbFlipped_; //flipping limb adjustment
	bool walkFlipped_; //flipping leg movement
	bool canDelete_ = false; //tracks if robot can be deleted after death (allows for animations)
};

//bullet instances
class bullet {
public:
	bullet(float x, float y, float z, float lX, float lZ, float v, float c = 0) 
		: x_(x), y_(y), z_(z), lX_(lX), lZ_(lZ), velocity_(v)
	{
		fX_ = (lX_ / 10) * (velocity_ / FAST_SPEED);
		fZ_ = (lZ_ / 10) * (velocity_ / FAST_SPEED);
	}

	float x_, y_, z_;
	const float velocity_;
	float lX_, lZ_; //lookAt vector values
	float fX_, fZ_; //foward vector
	const float colliderRadius_ = BULLET_RADIUS;
	bool bulletModif = true;
	bool draw = true;
};

std::vector<robot> robots(NUM_OF_ROBOTS); //holds all robots
std::vector<bullet*> bullets; //holds all bullets
std::vector<bullet*> modifyBullets; //holds bullets to be modified in handleBullets()

void arcballOrient(); //fowarded for init() call

//automatic timer functions
void timerDecrement(int) {
	--timer;
	glutPostRedisplay();
	if (timer > 0) glutTimerFunc(1000, timerDecrement, 0);
}

//controls robot walking
void triggerRobotWalk(int) {
	for (int i = 0; i < robots.size(); ++i) {
		robots[i].walk();
	}
	glutTimerFunc(100, triggerRobotWalk, 0);
}

//sets up robots and displays the controls
void init() {
	srand(time(nullptr));

	//randomly positions the robots
	for (int i = 0; i < NUM_OF_ROBOTS; ++i) {
		robots[i].setID(i % 5);

		//random position generation and configuration
		float randX = ((rand() % 1800 + (i % 5)) / 1000.0 + (i % 5)) - 1.0;
		float randZ = ((rand() % 1000) / 100.0) - 6.5;
		if (randZ < -5.0) randZ += 1.5;
		robots[i].changePos(randX, (0.495 * (cubeSize / 0.5)) + 0.1, randZ);
	}

	//prints controls
	std::cout << "CONTROLS:\n"
		<< "w: wireframe\n"
		<< "s: solid-only\n"
		<< "c: toggle collider display\n"
		<< "a: toggle axis display\n"
		<< "b: change bullet speed (FAST/VERY FAST/SLOW)\n"
		<< "m: toggle dance-like motion\n"
		<< "p: toggle walking\n"
		<< "r: toggle realistic sound effects (TW: gun sounds)\n"
		<< "F1: toggle full screen\n"
		<< "F2: swap FPV and ESV camera views\n"
		<< "Up/Down Arrows - move camera forward/backward\n"
		<< "Left/Right Arrows - rotate camera left/right\n"
		<< "Spacebar - attack\n"
		<< "Left Mouse Button Drag - move arcball camera (in big ESV mode only)\n"
		<< "Right Mouse Button Drag - zoom in/out arcball camera (in big ESV mode only\n"
		<< "Middle Mouse Button Click - open RESUME/EXIT menu"
		<< "ESC - exit\n\n";

	//sets up timer, robot walking function, and arcball camera orientation
	glutTimerFunc(1000, timerDecrement, 0);
	glutTimerFunc(100, triggerRobotWalk, 0);
	arcballOrient();
}

//plays/stops all music
void MusicHandler(int) {
	//play sound if not playing already if dancing and music is on
	if (!playing && dancing) {
		PlaySound(TEXT("audio/dance1.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		playing = true;
		while (!playing) {} //to account for potential lag
	}

	//plays standard music
	if (!dancing && !playing) {
		PlaySound(TEXT("audio/battle.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP); //changes to standard music
		playing = true;
		while (!playing) {} //to account for potential lag
	}

	glutTimerFunc(10, MusicHandler, 0);
}

//handles all dance operations
void danceHandler(int) {
	if (!dancing) { //not dancing
		for (int i = 0; i < robots.size(); ++i) {
			robots[i].danceReset();
		}
	}
	else { //dancing
		for (int i = 0; i < robots.size(); ++i) {
			robots[i].dance();
		}
	}

	glutPostRedisplay();
	if (dancing) glutTimerFunc(50, danceHandler, 0);
}

//draws the axes one-by-one
void drawAxes() {
	glBegin(GL_LINES);

	//x
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(2.0, 0.0, 0.0);

	//y
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 2.0, 0.0);

	//z
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 2.0);
	glEnd();
}

//draws the plane and studs
void drawPlane() {
	//plane
	glPushMatrix();
	glColor3f(0.65f, 0.65f, 0.65f);
	glScalef(1.0, 0.05, 1.0);
	glTranslatef(0.0, -5.0, 0.0);
	if (solid) glutSolidCube(GROUND_SIZE);
	if (wire) glutWireCube(GROUND_SIZE);
	glPopMatrix();

	//BONUS: studs inspired by old ROBLOX baseplate
	for (float i = 0.1; i < GROUND_SIZE - 0.1; i += 0.25) { //columns
		for (float j = 0.1; j < GROUND_SIZE - 0.1; j += 0.2) { //rows
			glPushMatrix();
			glColor3f(0.8f, 0.8f, 0.8f);
			glTranslatef(j - 5.0, 0.0, i - 4.95);
			glScalef(1.0, 0.2, 1.0);
			if (solid) glutSolidCube(0.1);
			if (wire) glutWireCube(0.1);
			glPopMatrix();
		}
	}
}

//toggles bullet modification for all bullets that have already been modified
void bulletUpdate(int) {
	for (auto it = modifyBullets.begin(); it != modifyBullets.end();) {
		(*it)->bulletModif = true;
		it = modifyBullets.erase(it);
	}
}

//draws and handles all bullet operations
void handleBullets() {
	for (auto& b : bullets) {
		if (b->draw) { //hasn't collided with robot
			//draws bullet
			glPushMatrix();
			glTranslatef(b->x_, b->y_, b->z_);
			glColor3f(0.0, 0.0, 0.0);
			if (solid) glutSolidSphere(0.025, 100, 100);
			else glutWireSphere(0.025, 100, 100);
			glPopMatrix();

			//draws collider if showing
			if (showColliders) {
				glPushMatrix();
				glTranslatef(b->x_, b->y_, b->z_);
				glColor3f(1.0, 0.0, 0.0);
				glutWireSphere(b->colliderRadius_, 10, 10);
				glPopMatrix();
			}

			//modifies bullet position and handles collisions
			if (b->bulletModif) {
				//moves bullet
				b->x_ += b->fX_;
				b->z_ += b->fZ_;
				if (b->x_ < (-(GROUND_SIZE / 2) - 0.1) || b->x_ >((GROUND_SIZE / 2) + 0.1)
					|| b->z_ < (-(GROUND_SIZE / 2) - 0.1) || b->z_ >((GROUND_SIZE / 2) + 0.1)) { //"register as miss" boundary
					score -= SCORE_MODIF;
					b->draw = false;
				}
				else { //in hittable area
					bool hit = false; //holds if bullet has hit a robot
					for (auto& r : robots) {
						//calculates the distance between the bullet and robot
						float xDist = sqrtf((r.getX() - b->x_) * (r.getX() - b->x_));
						float zDist = sqrtf((r.getZ() - b->z_) * (r.getZ() - b->z_));
						float colliderSum = r.colliderRadius_ + b->colliderRadius_;
						
						//collision detection
						if (xDist < colliderSum && zDist < colliderSum && !r.walkStopped_) { //collision
							++kills;
							score += SCORE_MODIF;
							r.collided_ = true;
							r.stopWalk(); //starts death trigger
							b->draw = false; //prepares to destroy bullet
							hit = true; //indicates that bullet needs destroyed
							
							//BONUS: realistic gun hit sound
							if (extraSounds) sfxEngine->play2D("audio/gunhit.mp3");

							//death sound
							sfxEngine->play2D("audio/oof.wav");
							break;
						}
					}
					if (!hit) { //readys bullet for future update (if no collision)
						b->bulletModif = false;
						modifyBullets.push_back(b);
						glutTimerFunc(100, bulletUpdate, 0);
					}
				}
			}
		}
	}

	//deletes all bullets that have collided with robot
	for (auto it = bullets.begin(); it != bullets.end();) {
		if (!(*it)->draw) { //collided with robot
			it = bullets.erase(it);
		}
		else { //hasn't collided with robot
			++it;
		}
	}
}

//preforms all non-menu, non-UI, non-gun drawing
void drawAll(bool depthTest = true) {
	if (reset) { //resets all transformation variables, including color
		std::cout << "RESET TRIGGERED (oof)\n";

		PlaySound(TEXT("audio/oof.wav"), NULL, SND_FILENAME); //freezes screen, plays speech

		//resets rotations, zooming, and random position adjustment
		theta = 0.0;
		cameraX = 0.0;
		cameraZ = 3.0;
		cameraLX = 0.0;
		cameraLZ = -1.0;
		arcballX = 0.0;
		arcballY = 0.0;
		arcballZ = 0.0;
		arcballRadius = 7.0;
		arcballTheta = 2.8;
		arcballPhi = 2.0;
		arcballOrient();
		camera2 = true;
		cameraFlip = false;

		//resets robots
		//readds deleted robots
		for (int i = robots.size(); i < NUM_OF_ROBOTS; ++i) {
			robots.push_back(robot());
			robots[i].setID(i % 5);
		}
		//random position adjustment
		for (int i = 0; i < NUM_OF_ROBOTS; ++i) {
			float randX = ((rand() % 1800 + (i % 5)) / 1000.0 + (i % 5)) - 1.0;
			float randZ = ((rand() % 1000) / 100.0) - 6.5;
			if (randZ < -5.0) randZ += 1.5;
			robots[i].changePos(randX, (0.495 * (cubeSize / 0.5)) + 0.1, randZ);
		}

		//resets all viewing variables
		solid = true;
		wire = false;
		axes = true;

		//resets game info
		if (timer == 0) {
			timer = STARTING_TIMER;
			glutTimerFunc(1000, timerDecrement, 0);
		}
		else {
			timer = STARTING_TIMER + 1;
		}
		currentSpeed = FAST_SPEED;
		kills = 0;
		score = 0;
		playing = false; //to switch back to battle music
		bullets.clear();
		modifyBullets.clear();
		showColliders = false;
		canShoot = true;

		//resets menu toggle
		menuOpen = false;

		//ends the reset
		reset = false;
		std::cout << "RESET COMPLETE\n";
	}

	if (depthTest) glEnable(GL_DEPTH_TEST);
	//TAKING INSPIRATION FORM THE ICONIC ROBLOX BASEPLATE
	drawPlane();
	if (depthTest) glDisable(GL_DEPTH_TEST);

	//axes
	if (axes) drawAxes();

	//draws the robots
	//TAKING INSPIRATION FROM THE ICONIC ROBLOX "NOOB" CHARACTER
	if (depthTest) glEnable(GL_DEPTH_TEST);
	for (auto it = robots.begin(); it != robots.end();) {
		if (!(*it).canDelete()) { //in death animation or still alive
			if ((*it).collided_) (*it).stopWalk(); //continues death animation
			(*it).drawCharacter();
			++it;
		}
		else { //death animation complete
			it = robots.erase(it);
		}
	}
	handleBullets();
	if (depthTest) glDisable(GL_DEPTH_TEST);

	// end drawing
	glFlush();
}

//creates camera box
void drawCameraBox() {
	glPushMatrix();
	glColor3f(0.4, 0.4, 0.4);
	glTranslatef(cameraX, cameraY, cameraZ);
	glScalef(0.35, 0.2, 0.1);
	glRotatef(theta * 100, 0.0, -1.0, 0.0);
	glutSolidCube(1.0);
	glColor3f(1.0, 1.0, 1.0);
	glutWireCube(1.1);
	glPopMatrix();
}

//draws all text to display on UI viewport
void drawText(float x, float y, void* f, const char* s) {
	glRasterPos2f(x, y);
	for (char* c = (char*)s; *c != 0; ++c) {
		if (*c != '\n') { //non-new line
			glutBitmapCharacter(f, *c);
		}
		else { //new line
			y -= 1.0; //creates line spacing
			glRasterPos2f(x, y); //resets raster positioning
		}
	}
}

//draws the aim marker
void drawAim() {
	glPushMatrix();
	glTranslatef(cameraX + cameraLX, cameraY, cameraZ + cameraLZ);
	glScalef(0.05, 0.05, 0.05);
	glColor3f(1.0, 0.0, 0.0);
	if (solid) glutSolidSphere(0.5, 100, 100);
	else glutWireSphere(0.5, 100, 100);
	glPopMatrix();
}

//draws the 2D gun
void drawGun() {
	glPushMatrix();
	//creates 2D viewing plane
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);

	//creates box
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(0.5, 0.5, 0.0);
	glColor3f(0.2, 0.2, 0.2);
	if (solid) glBegin(GL_QUADS);
	else glBegin(GL_LINE_LOOP);
	glVertex2f(-0.2, -0.25);
	glVertex2f(0.2, -0.25);
	glVertex2f(0.2, -0.75);
	glVertex2f(-0.2, -0.75);
	glEnd();
	glPopMatrix(); //pops out of box creation

	glPopMatrix(); //pops out of 2D plane creation
}

//handles all FPV drawing
void renderFPV() {
	glClearColor(0.0, 0.3, 0.8, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.0f);

	//sets up viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)WIN_W / WIN_H, 0.1, 100.0);

	//sets up camera and draws
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraX, cameraY, cameraZ, cameraX + cameraLX, cameraY, cameraZ + cameraLZ, 0.0, 1.0, 0.0);
	drawAll();
	drawAim();
	drawGun();
}

//handles all ESV drawing
void renderESV() {
	glClearColor(0.0, 0.3, 0.8, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.0f);

	//sets up viewing volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (float)WIN_W / WIN_H, 0.1, 1000.0);

	//sets up camera and draws
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(arcballX, arcballY, arcballZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	drawAll(false);
	drawCameraBox();
}

//performs all drawing functions
void MyDisplay() {
	//FPV (flipped: ESV)
	glPushMatrix();
	glViewport(0, 0, WIN_W, 7 * (WIN_H / 8) + 4);
	glEnable(GL_SCISSOR_TEST); 
	glScissor(0, 0, WIN_W, 7 * (WIN_H / 8) + 4);
	if (!cameraFlip) { //standard
		renderFPV();
	}
	else { //swapped with camera 2
		renderESV();
	}
	glDisable(GL_SCISSOR_TEST);
	glPopMatrix();

	//ESV (flipped: FPV)
	if (camera2) {
		glPushMatrix();
		glViewport(WIN_W * 0.7, 3 * (WIN_H / 4) - (WIN_H * 0.175) + 4, WIN_W * 0.3, WIN_H * 0.3);
		glEnable(GL_SCISSOR_TEST);
		glScissor(WIN_W * 0.7, 3 * (WIN_H / 4) - (WIN_H * 0.175) + 4, WIN_W * 0.3, WIN_H * 0.3);
		if (!cameraFlip) { //standard
			renderESV();
		}
		else { //swapped with camera 1
			renderFPV();
		}
		glDisable(GL_SCISSOR_TEST);
		glPopMatrix();
	}

	//UI
	glPushMatrix();
	glViewport(0, 7 * (WIN_H / 8) + 4, WIN_W, WIN_H / 8);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-2.0, 2.0, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glPushMatrix();
	glColor3f(1.0, 1.0, 1.0);
	glScalef(5.0, 3.0, 0.0);
	glRectf(-1.0, 2.0, 1.0, -2.0);
	glPopMatrix();

	//TEXT DRAWING
	glColor3f(0.0, 0.0, 0.0);
	drawText((WIN_W / 2000.0) - 1.1, 0.45, GLUT_BITMAP_TIMES_ROMAN_24, (char*)"ROBLOX: IT'S FREE!");
	char displayText[100]; char speedDisplay[100]; char scoringDisplay[100];
	if (timer > 0 && kills != NUM_OF_ROBOTS) {
		snprintf(displayText, sizeof(displayText),
			"Score: %d              Robots Killed: %d/%d              Timer: %d",
			score, kills, NUM_OF_ROBOTS, timer);
	}
	else {
		if (kills == NUM_OF_ROBOTS) {
			snprintf(displayText, sizeof(displayText),
				"Mission completed in %d seconds!",
				STARTING_TIMER - timer);
		}
		else {
			snprintf(displayText, sizeof(displayText),
				"Mission failed!");
		}
			
	}
	snprintf(scoringDisplay, sizeof(scoringDisplay),
			"SCORING:\nHit: +%d\nMiss: -%d",
			SCORE_MODIF, SCORE_MODIF);
	snprintf(speedDisplay, sizeof(speedDisplay),
			"Bullet Speed: %f%%",
			(currentSpeed / FAST_SPEED) * 100.0);
	drawText((WIN_W / 2000.0) - 1.25, -1.0, GLUT_BITMAP_HELVETICA_10, displayText);
	drawText(-1.5, 0.9, GLUT_BITMAP_HELVETICA_10, scoringDisplay);
	drawText(1.15, 0.9, GLUT_BITMAP_HELVETICA_10, speedDisplay);
	glPopMatrix();

	glutSwapBuffers();
}

//shooting cooldown function
void cooldownEnd(int) {
	canShoot = true;
}

//handles all keyboard functionality
//ALL INCASED IN "if (!reset)" TO PREVENT RESET ERRORS
//ALL INCASED WITH "if (timer > 0)" OR SIMILAR (except ESC) DUE TO REQUIREMENT OF DISABLING KEYBOARD AT END OF TIMER
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w': //wireframe
		if (!reset && timer > 0) {
			solid = false;
			wire = true;
			std::cout << "Wireframe display!\n";
			glutPostRedisplay();
		}
		break;
	case 's': //solid
		if (!reset && timer > 0) {
			solid = true;
			wire = false;
			std::cout << "Solid display!\n";
			glutPostRedisplay();
		}
		break;
	case 'c': //display colider
		if (timer > 0) {
			showColliders = !showColliders;
			glutPostRedisplay();
		}
		break;
	case 'a': //show/hide axes
		if (!reset && timer > 0) {
			axes = !axes;
			if (axes) std::cout << "Axes on!\n";
			else std::cout << "Axes off!\n";
			glutPostRedisplay();
		}
		break;
	case 'b': //change bullet speed
		if (timer > 0) {
			if (currentSpeed == FAST_SPEED) {
				currentSpeed = VERY_FAST_SPEED;
			}
			else if (currentSpeed == VERY_FAST_SPEED) {
				currentSpeed = SLOW_SPEED;
			}
			else {
				currentSpeed = FAST_SPEED;
			}
		}
		break;
	case 'm': //start/stop motion
		if (!reset && timer > 0) {
			dancing = !dancing;
			if (dancing) std::cout << "Now we are moving!\n";
			else std::cout << "Aww! We stopped moving! We were having fun! :'(\n";
			glutTimerFunc(0, danceHandler, 0);
			playing = false; //to switch back to battle music
			glutPostRedisplay();
		}
		break;
	case ' ': //shooting
		if (timer > 0 && canShoot) {
			bullet* temp = new bullet(cameraX, cameraY, cameraZ, cameraLX, cameraLZ, currentSpeed);
			bullets.push_back(temp);

			//BONUS: realistic gun shot sound
			if (extraSounds) sfxEngine->play2D("audio/gunshot.mp3");

			canShoot = false;
			glutTimerFunc(500, cooldownEnd, 0);
		}
		break;
	case 'p': //toggling walking
		if (timer > 0) walkToggle = !walkToggle;
		break;
	case 'r': //toggle realistic sounds
		if (timer > 0) extraSounds = !extraSounds;
		break;
	case 27:
		exit(0);
		break;
	}
}

//camera controls
void specKey(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_LEFT: //rotate left
		if (timer > 0) {
			theta -= 0.05;
			cameraLX = sin(theta);
			cameraLZ = -cos(theta);
		}
		break;
	case GLUT_KEY_RIGHT: //rotate right
		if (timer > 0) {
			theta += 0.05;
			cameraLX = sin(theta);
			cameraLZ = -cos(theta);
		}
		break;
	case GLUT_KEY_UP: //move forward
		if (timer > 0) {
			cameraX += (cameraLX * FRACTION);
			cameraZ += (cameraLZ * FRACTION);

			//camera bound checking
			if (cameraX < -(GROUND_SIZE / 2)) cameraX = -(GROUND_SIZE / 2);
			else if (cameraX > (GROUND_SIZE / 2)) cameraX = (GROUND_SIZE / 2);
			if (cameraZ < -(GROUND_SIZE / 2)) cameraZ = -(GROUND_SIZE / 2);
			else if (cameraZ > (GROUND_SIZE / 2)) cameraZ = (GROUND_SIZE / 2);
		}
		break;
	case GLUT_KEY_DOWN: //move backward
		if (timer > 0) {
			cameraX -= (cameraLX * FRACTION);
			cameraZ -= (cameraLZ * FRACTION);

			//camera bound checking
			if (cameraX < -(GROUND_SIZE / 2)) cameraX = -(GROUND_SIZE / 2);
			else if (cameraX > (GROUND_SIZE / 2)) cameraX = (GROUND_SIZE / 2);
			if (cameraZ < -(GROUND_SIZE / 2)) cameraZ = -(GROUND_SIZE / 2);
			else if (cameraZ > (GROUND_SIZE / 2)) cameraZ = (GROUND_SIZE / 2);
		}
		break;
	case GLUT_KEY_F1: //toggle full-screen
		if (timer > 0) glutFullScreenToggle();
		break;
	case GLUT_KEY_F2: //swap FSV and ESV
		if (timer > 0) cameraFlip = !cameraFlip;
		break;
	}

	glutPostRedisplay();
}

//handles window size changes
void resizeWindow(GLsizei w, GLsizei h) {
	WIN_W = w;
	WIN_H = h;
}

//handles pop-up menu functionality
void resumeExitMenu(int v) {
	if (v == 0) { //RESUME (reset trigger)
		reset = true;
		dancing = false;
		glutPostRedisplay();
		glFlush();
	}
	else if (v == 1) {
		exit(0);
	}
	menuOpen = false;
}

//handles recomputation of arcball camera rotations
void arcballOrient() {
	arcballX = arcballRadius * sinf(arcballTheta) * sinf(arcballPhi);
	arcballZ = arcballRadius * cosf(arcballTheta) * sinf(arcballPhi);
	arcballY = arcballRadius * sinf(arcballPhi);
	glutPostRedisplay();
}

//handles mouse clicks
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && cameraFlip) {
		left = state;
		menuOpen = false;
	}
	if (button == GLUT_RIGHT_BUTTON && cameraFlip) {
		right = state;
		menuOpen = false;
	}
	if (button == GLUT_MIDDLE_BUTTON) {
		menuOpen = true;
	}
	mX = x;
	mY = y;
}

//handles mouse motion computations
//based off of example code from Lectures 10 & 11 slides
void motion(int x, int y) {
	if (left == GLUT_DOWN && cameraFlip && !menuOpen) { //moving camera
		arcballTheta += (mX - x) * 0.005;
		arcballPhi += (mY - y) * 0.005;
		if (arcballPhi <= 0) {
			arcballPhi = 0.2;
		}
		else if (arcballPhi >= acosf(-1.0)) {
			arcballPhi = acosf(-1.0) - 0.01;
		}
		arcballOrient();
	}
	else if (right == GLUT_DOWN && cameraFlip && !menuOpen) { //zooming control
		double totalChange = (x - mX) + (y - mY);

		arcballRadius += totalChange * 0.01;

		if (arcballRadius < 2.0) {
			arcballRadius = 2.0;
		}
		else if (arcballRadius > 10.0) {
			arcballRadius = 10.0;
		}
		arcballOrient();
	}
	mX = x;
	mY = y;
}

int main(int argc, char** argv) {
	//initalization and display configuration
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); // RGB mode
	glutInitWindowSize(WIN_W, WIN_H); // window size
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Assignment 3 | Nate Booth (nbooth5 // 811168294)");
	glClearColor(0.0, 0.0, 0.0, 1.0); // clear the window screen
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 4.0);

	init(); //controls printing

	//menu setup
	menuID = glutCreateMenu(resumeExitMenu);
	glutAddMenuEntry("RESUME", 0);
	glutAddMenuEntry("EXIT", 1);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	//callback configurations
	glutDisplayFunc(MyDisplay); // call the drawing function
	glutReshapeFunc(resizeWindow);
	glutIdleFunc(MyDisplay);
	glutTimerFunc(0, MusicHandler, 0); //call the sound function
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specKey);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutMainLoop();
	return 0;
}
