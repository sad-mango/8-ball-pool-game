#ifndef CONFIG_H
#define CONFIG_H

// Window
#define WINDOW_TITLE "8 Ball Pool - Drag to Charge"
#define TARGET_FPS 60

// Table
#define TABLE_WIDTH 800
#define TABLE_HEIGHT 400
#define RAIL_WIDTH 40

// Balls
#define MAX_BALLS 16
#define BALL_RADIUS 15
#define POCKET_RADIUS 28

// Physics
#define FRICTION 0.985f
#define MIN_VELOCITY 0.06f
#define MAX_BALL_SPEED 26.0f

// Shot power
#define MAX_POWER_PIXELS 160.0f
#define MAX_SHOT_SPEED 22.0f

// Stick
#define STICK_LENGTH 120.0f
#define STICK_RECOIL_TIME 0.12f

#endif // CONFIG_H
