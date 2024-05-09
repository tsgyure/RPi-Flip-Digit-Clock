/********************************************************************
 * 																	*
 *	Raspberry Pi													*
 * 	BIG NOISY Flip Digit Clock										*
 * 	Todd S. Gyure	02/01/22										*
 * 																	*
 * 	Created from salvaged Sportissimo Sports Clock					*
 * 	Model				9051-4010									*
 * 	Serial Number		2											*
 * 	Manufactured Date	04/94										*
 *																	*
 * 	Set time zone by setting the local time on the RPi				*
 * 	using sudo raspi-config at the command prompt.					*
 * 																	*
 ********************************************************************/
// User Options
#define		TIME_24			TRUE	// Enable 24 hour display format
#define		NIGHT_QUIET		TRUE	// Disable noisy flip at night
#define		QUIET_START		2300	// Quiet time start	(24 hour clock)
#define		QUIET_END		700		// Quiet time end	(24 hour clock)

// Don't Change this
#define	PULSE_LENGTH		90000	// Minimum pulse for solenoids [us]

#include	<stdio.h>
#include	<unistd.h>
#include	<time.h>

#include	<stdlib.h>
#include	<signal.h>

#include	<wiringPi.h>

// RPi Pins - Digit enable
#define	ENABLE_DIGIT_0		24
#define	ENABLE_DIGIT_1		26
#define	ENABLE_DIGIT_2		32
#define	ENABLE_DIGIT_3		36

// RPi Pins - Segment SET
#define	SET_SEG_a	7
#define	SET_SEG_b	11
#define	SET_SEG_c	13
#define	SET_SEG_d	15
#define	SET_SEG_e	19
#define	SET_SEG_f	21
#define	SET_SEG_g	23

// RPi Pins - Segment SET
#define	RESET_SEG_a	29
#define	RESET_SEG_b	31
#define	RESET_SEG_c	33
#define	RESET_SEG_d	35
#define	RESET_SEG_e	8
#define	RESET_SEG_f	10
#define	RESET_SEG_g	12

//	Functions
void	ShutDown(void);					// For a tidy shutdown
void	SignalHandler(int SignalNumber);// For a tidy shutdown
int		initStuff(void);				// Initial setup
int		digitSet(int digit, int value);	// SET/RESET flippers for a number		
int		enableDigit(int digit);			// Enable a digit
void	ALL_OFF(void);					// Turn ALL outputs OFF
void	INOPdisplay(void);				// Display "----" to indicate
										//	that the clock is INOP.

int		main(int argc, char *argv[])
{
	struct		tm *t;
	time_t		now;
	int			bigTime;

	printf("Flip Clock started.\n\n");
	initStuff();

	// Initial display of current time
	now = time(NULL);			// Get the current time
	t = localtime(&now);		// Get the tm structure

	bigTime = (t->tm_hour) * 100 + (t->tm_min);

	if(TIME_24 == FALSE && t->tm_hour > 12)
	{
		bigTime = bigTime - 1200;
	}

	digitSet(3, bigTime / 1000);
	bigTime = bigTime - (bigTime / 1000) * 1000;
	
	digitSet(2, bigTime / 100);
	bigTime = bigTime - (bigTime / 100) * 100;

	digitSet(1, bigTime / 10);

	digitSet(0, bigTime - (bigTime / 10) * 10);

	while(1)	// Terminal loop START
	{
		do		//	Update time every minute at 0 seconds
		{
			now = time(NULL);			//	Get the current time
			t = localtime(&now);		//	Get the tm structure
			sleep(1);					//	Wait 1 second
		}while(t->tm_sec != 0);
		
		bigTime = (t->tm_hour) * 100 + (t->tm_min);

		// Shh!  Don't change flip segments at night?
		if(	bigTime >= 1200 && bigTime < QUIET_START ||
			bigTime < 1200 && bigTime > QUIET_END ||
			NIGHT_QUIET == FALSE)
		{
			// Modify, if 24 hour clock not 24 hour display format
			if(TIME_24 == FALSE && t->tm_hour > 12)
			{
				bigTime = bigTime - 1200;
			}

			digitSet(3, bigTime / 1000);
			bigTime = bigTime - (bigTime / 1000) * 1000;
			
			digitSet(2, bigTime / 100);
			bigTime = bigTime - (bigTime / 100) * 100;

			digitSet(1, bigTime / 10);

			digitSet(0, bigTime - (bigTime / 10) * 10);
		}
		
		// Display "----" if Quiet Mode ON
		if(NIGHT_QUIET == TRUE && bigTime == QUIET_START)
		{
			INOPdisplay();
		}
		
	}		// Terminal loop END

	return(0);
}

/********************************************************************
 * 																	*
 * 	Do the initial setup of the system.								*
 * 																	*
 ********************************************************************/
int		initStuff(void)
{
	// Shut down in a pleasant manner...
	signal(SIGINT, SignalHandler);
	atexit(ShutDown);

	wiringPiSetupPhys();

	// Configure SEGMENT SET pins
	digitalWrite(SET_SEG_a, LOW);
	digitalWrite(SET_SEG_b, LOW);
	digitalWrite(SET_SEG_c, LOW);
	digitalWrite(SET_SEG_d, LOW);
	digitalWrite(SET_SEG_e, LOW);
	digitalWrite(SET_SEG_f, LOW);
	digitalWrite(SET_SEG_g, LOW);
	pinMode(SET_SEG_a, OUTPUT);
	pinMode(SET_SEG_b, OUTPUT);
	pinMode(SET_SEG_c, OUTPUT);
	pinMode(SET_SEG_d, OUTPUT);
	pinMode(SET_SEG_e, OUTPUT);
	pinMode(SET_SEG_f, OUTPUT);
	pinMode(SET_SEG_g, OUTPUT);

	// Configure SEGMENT RESET pins
	digitalWrite(RESET_SEG_a, LOW);
	digitalWrite(RESET_SEG_b, LOW);
	digitalWrite(RESET_SEG_c, LOW);
	digitalWrite(RESET_SEG_d, LOW);
	digitalWrite(RESET_SEG_e, LOW);
	digitalWrite(RESET_SEG_f, LOW);
	digitalWrite(RESET_SEG_g, LOW);
	pinMode(RESET_SEG_a, OUTPUT);
	pinMode(RESET_SEG_b, OUTPUT);
	pinMode(RESET_SEG_c, OUTPUT);
	pinMode(RESET_SEG_d, OUTPUT);
	pinMode(RESET_SEG_e, OUTPUT);
	pinMode(RESET_SEG_f, OUTPUT);
	pinMode(RESET_SEG_g, OUTPUT);

	// Configure DIGIT pins
	digitalWrite(ENABLE_DIGIT_0, LOW);
	digitalWrite(ENABLE_DIGIT_1, LOW);
	digitalWrite(ENABLE_DIGIT_2, LOW);
	digitalWrite(ENABLE_DIGIT_3, LOW);
	pinMode(ENABLE_DIGIT_0, OUTPUT);
	pinMode(ENABLE_DIGIT_1, OUTPUT);
	pinMode(ENABLE_DIGIT_2, OUTPUT);
	pinMode(ENABLE_DIGIT_3, OUTPUT);

	return(0);
}

/********************************************************************
 * 																	*
 * 	SET/RESET a digit [3-0] for a number [0-9].						*
 * 																	*
 ********************************************************************/
int		digitSet(int digit, int value)
{
	enableDigit(digit);
	
	switch(value)
	{
		case	0:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(SET_SEG_d, HIGH);
			digitalWrite(SET_SEG_e, HIGH);
			digitalWrite(SET_SEG_f, HIGH);
			digitalWrite(RESET_SEG_g, HIGH);
			break;

		case	1:
			digitalWrite(RESET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(RESET_SEG_d, HIGH);
			digitalWrite(RESET_SEG_e, HIGH);
			digitalWrite(RESET_SEG_f, HIGH);
			digitalWrite(RESET_SEG_g, HIGH);
			break;

		case	2:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(RESET_SEG_c, HIGH);
			digitalWrite(SET_SEG_d, HIGH);
			digitalWrite(SET_SEG_e, HIGH);
			digitalWrite(RESET_SEG_f, HIGH);
			digitalWrite(SET_SEG_g, HIGH);
			break;

		case	3:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(SET_SEG_d, HIGH);
			digitalWrite(RESET_SEG_e, HIGH);
			digitalWrite(RESET_SEG_f, HIGH);
			digitalWrite(SET_SEG_g, HIGH);
			break;

		case	4:
			digitalWrite(RESET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(RESET_SEG_d, HIGH);
			digitalWrite(RESET_SEG_e, HIGH);
			digitalWrite(SET_SEG_f, HIGH);
			digitalWrite(SET_SEG_g, HIGH);
			break;

		case	5:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(RESET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(SET_SEG_d, HIGH);
			digitalWrite(RESET_SEG_e, HIGH);
			digitalWrite(SET_SEG_f, HIGH);
			digitalWrite(SET_SEG_g, HIGH);
			break;

		case	6:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(RESET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(SET_SEG_d, HIGH);
			digitalWrite(SET_SEG_e, HIGH);
			digitalWrite(SET_SEG_f, HIGH);
			digitalWrite(SET_SEG_g, HIGH);
			break;
		case	7:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(RESET_SEG_d, HIGH);
			digitalWrite(RESET_SEG_e, HIGH);
			digitalWrite(RESET_SEG_f, HIGH);
			digitalWrite(RESET_SEG_g, HIGH);
			break;

		case	8:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(SET_SEG_d, HIGH);
			digitalWrite(SET_SEG_e, HIGH);
			digitalWrite(SET_SEG_f, HIGH);
			digitalWrite(SET_SEG_g, HIGH);
			break;

		case	9:
			digitalWrite(SET_SEG_a, HIGH);
			digitalWrite(SET_SEG_b, HIGH);
			digitalWrite(SET_SEG_c, HIGH);
			digitalWrite(SET_SEG_d, HIGH);
			digitalWrite(RESET_SEG_e, HIGH);
			digitalWrite(SET_SEG_f, HIGH);
			digitalWrite(SET_SEG_g, HIGH);
			break;
			
		default:
			printf("Invalid digit VALUE!\nDigit:\t%d\tValue: %d\n", digit, value);
			return(-1);
	}
	
	ALL_OFF();
	return(0);
}

/********************************************************************
 * 																	*
 * 	Enable one of the digits to be modified.						*
 * 	Select MSD [3] through LSD [0]									*
 * 																	*
 ********************************************************************/
int		enableDigit(int digit)
{
	switch(digit)
	{
		case	0:
			digitalWrite(ENABLE_DIGIT_1, LOW);
			digitalWrite(ENABLE_DIGIT_2, LOW);
			digitalWrite(ENABLE_DIGIT_3, LOW);
			digitalWrite(ENABLE_DIGIT_0, HIGH);
			break;
			
		case	1:
			digitalWrite(ENABLE_DIGIT_0, LOW);
			digitalWrite(ENABLE_DIGIT_2, LOW);
			digitalWrite(ENABLE_DIGIT_3, LOW);
			digitalWrite(ENABLE_DIGIT_1, HIGH);
			break;
			
		case	2:
			digitalWrite(ENABLE_DIGIT_0, LOW);
			digitalWrite(ENABLE_DIGIT_1, LOW);
			digitalWrite(ENABLE_DIGIT_3, LOW);
			digitalWrite(ENABLE_DIGIT_2, HIGH);
			break;
			
		case	3:
			digitalWrite(ENABLE_DIGIT_0, LOW);
			digitalWrite(ENABLE_DIGIT_1, LOW);
			digitalWrite(ENABLE_DIGIT_2, LOW);
			digitalWrite(ENABLE_DIGIT_3, HIGH);
			break;
			
		default:
			printf("Invalid digit!\n");
			digitalWrite(ENABLE_DIGIT_0, LOW);
			digitalWrite(ENABLE_DIGIT_1, LOW);
			digitalWrite(ENABLE_DIGIT_2, LOW);
			digitalWrite(ENABLE_DIGIT_3, LOW);
			return(-1);
	}
}

// Turn ALL outputs OFF
void	ALL_OFF(void)
{
	usleep(PULSE_LENGTH);	// Pulse width for prior actuations
	
	digitalWrite(SET_SEG_a, LOW);
	digitalWrite(SET_SEG_b, LOW);
	digitalWrite(SET_SEG_c, LOW);
	digitalWrite(SET_SEG_d, LOW);
	digitalWrite(SET_SEG_e, LOW);
	digitalWrite(SET_SEG_f, LOW);
	digitalWrite(SET_SEG_g, LOW);

	digitalWrite(RESET_SEG_a, LOW);
	digitalWrite(RESET_SEG_b, LOW);
	digitalWrite(RESET_SEG_c, LOW);
	digitalWrite(RESET_SEG_d, LOW);
	digitalWrite(RESET_SEG_e, LOW);
	digitalWrite(RESET_SEG_f, LOW);
	digitalWrite(RESET_SEG_g, LOW);

	digitalWrite(ENABLE_DIGIT_0, LOW);
	digitalWrite(ENABLE_DIGIT_1, LOW);
	digitalWrite(ENABLE_DIGIT_2, LOW);
	digitalWrite(ENABLE_DIGIT_3, LOW);

	usleep(PULSE_LENGTH);	// Pulse width for following actuations
	return;
}

// Display "----" to indicate that the clock is INOP.
void	INOPdisplay(void)
{
		digitalWrite(RESET_SEG_a, HIGH);
		digitalWrite(RESET_SEG_b, HIGH);
		digitalWrite(RESET_SEG_c, HIGH);
		digitalWrite(RESET_SEG_d, HIGH);
		digitalWrite(RESET_SEG_e, HIGH);
		digitalWrite(RESET_SEG_f, HIGH);
		digitalWrite(SET_SEG_g, HIGH);
		
		enableDigit(3);
		usleep(PULSE_LENGTH);	// Pulse width for actuation
		enableDigit(2);
		usleep(PULSE_LENGTH);	// Pulse width for actuation
		enableDigit(1);
		usleep(PULSE_LENGTH);	// Pulse width for actuation
		enableDigit(0);
		usleep(PULSE_LENGTH);	// Pulse width for actuation
		ALL_OFF();

		return;
}

void	SignalHandler(int SignalNumber)
{
   printf("\nCaught signal %d, coming out...\n", SignalNumber);
   exit(1);
}

void	ShutDown(void)
{
	printf("\nShutting down Flip Clock...\n");

	INOPdisplay();
	
	printf("\t\t... Flip Clock shutdown done.\n\n");
}
