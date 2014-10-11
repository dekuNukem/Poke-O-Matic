#include <LiquidCrystal.h>
#include <stdio.h>
#include <stdlib.h>
#include <my_button.h>
#include <SD.h>
#include <stdint.h>
#define UNDEFINED -1
#define LCD_RS 26
#define LCD_EN 27
#define LCD_D4 28
#define LCD_D5 0
#define LCD_D6 1
#define LCD_D7 2

#define A_BUTTON_PIN 4
#define B_BUTTON_PIN 5
#define X_BUTTON_PIN 6
#define Y_BUTTON_PIN 7
#define RIGHT_SHOULDER_BUTTON_PIN UNDEFINED
#define LEFT_SHOULDER_BUTTON_PIN UNDEFINED
#define UP_BUTTON_PIN 8
#define DOWN_BUTTON_PIN 9
#define LEFT_BUTTON_PIN 14
#define RIGHT_BUTTON_PIN 15
#define DS_START_BUTTON_PIN UNDEFINED
#define HOME_BUTTON_PIN UNDEFINED
#define DS_SELECT_BUTTON_PIN UNDEFINED
#define POWER_BUTTON_PIN 3

#define TOUCH_SCREEN_X_PIN 23
#define TOUCH_SCREEN_Y_PIN A14
#define TOUCH_SCREEN_Y_SENSE_PIN 19
#define C_PAD_X_PIN 21
#define C_PAD_Y_PIN 22
#define LIGHT_SENSOR_PIN A10
#define AUDIO_IN_PIN UNDEFINED
#define BUZZER_PIN 32
#define BLUE_LED_PIN 31
#define BUTTON_PRESSED LOW
#define WALKING_MODE 0
#define FISHING_MODE 1
#define HORDE_MODE 2
#define CLONE_MODE 3
#define SR_MODE 4
#define EGG_MODE 5
#define RELEASE_MODE 6
#define FISHING_TIMEOUT 8000
#define WALKING_BUTTON_PINPRESS_DURATION 300
#define QUICKBALL_PAGE 0
#define QUICKBALL_POS 0
#define BACKUPBALL_PAGE 0
#define BACKUPBALL_POS 1
#define BATTLE_BUTTON_PINPRESS_DELAY 900
#define CATCH_TIMEOUT 50000
#define NO_TIMEOUT -1
#define DARK 0
#define BRIGHT 1
#define RETRY_ATTEMPT 3
#define SCREEN_DARK 0
#define SCREEN_BRIGHT 1
#define CHECK_SCRREN_DELAY 40
#define CURSOR_MODE 0
#define CURSOR_DARK_THRESHOLD 1
#define CURSOR_NICKNAME 2
#define CURSOR_BLACKOUT_MODE 3
#define CURSOR_RUN 4
#define ABILITY_NOT_STAT_MODIFYING 0
#define ABILITY_STAT_MODIFYING 1
#define ABILITY_DROUGHT 2
#define ALWAYS_CATCH 3
#define NEVER_CATCH 4
#define SET_TO_DEBUG() lcd.setCursor(9, 1)
#define SET_TO_LAST() lcd.setCursor(15, 1)
#define SET_TO_BEGINNING() lcd.setCursor(0, 0)
#define SET_TO_BEGINNING_ROW2() lcd.setCursor(0, 1)
#define CLEAR_LCD() lcd.clear()
#define LED_ON(a) digitalWrite(a, HIGH)
#define LED_OFF(a) digitalWrite(a, LOW)
#define MODE_NUM 7
#define READ_LIGHT_SENSOR() analogRead(LIGHT_SENSOR_PIN)
#define EGG_DARK_THRESHOLD 150
#define NAME_BUF_SIZE 13
#define CLEAR_NAME_BUF() memset(name_buf, 0, NAME_BUF_SIZE)
#define CYCLE_PERIOD 7000
//5170
#define C_PAD_DEFAULT_POTENTIAL 132
#define ERR_CHECK_SCREEN_PREMATURE_RETURN 0
#define ERR_WUE_PREMATURE_RETURN 1
#define ERR_FLEE_FAILED 2
#define ERR_SWEET_SCENT_SEL 3
#define ERR_FISHING_FALSE_BITE 4


LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
char current_mode = EGG_MODE;
unsigned char dark_threshold = 20;
char nickname_enabled = 0;
char blackout_mode = ABILITY_NOT_STAT_MODIFYING;
char check_screen_delay_enabled = 1;
unsigned int encounter_count = 0;
unsigned char catch_retry_count = 0;
unsigned char shiny_caught_count = 0;
char error_occurred = 0;
long last_blackout = 0;
char egg_in_pocket = 0;    
char hatched_in_pocket = 0;
char free_slot = 5;
char egg_request_interval = 25;
int total_hatched = 0;
int total_obtained = 0;
char pkmn_in_box = 0;
char name_buf[NAME_BUF_SIZE];
char walk_timer = 10;
char timer_updated = 0;
char egg_ui_set = 0;
int release_request = 30;
char is_just_exited_daycare = 0;
my_button sf_up_button(33, 0);
my_button sf_down_button(25, 0);
my_button sf_left_button(UNDEFINED, 0);
my_button sf_right_button(24, 0);

void setup()
{
	lcd.begin(16, 2);
	analogReadResolution(10);
	analogWriteResolution(9);
	pinMode(TOUCH_SCREEN_X_PIN, INPUT);
	pinMode(TOUCH_SCREEN_Y_PIN, INPUT);
	pinMode(TOUCH_SCREEN_Y_SENSE_PIN, INPUT);
	pinMode(BUZZER_PIN, OUTPUT);
	pinMode(BLUE_LED_PIN, OUTPUT);
	pinMode(C_PAD_X_PIN, OUTPUT);
	pinMode(C_PAD_Y_PIN, OUTPUT);
	c_pad_reset();
	do_UI();
	CLEAR_LCD();
}

void loop()
{
	do_mode(current_mode);
}

int is_shiny(long blackout_duration)
{
	last_blackout = blackout_duration;
	long shiny_threshold;
	switch(blackout_mode)
	{
		case ABILITY_NOT_STAT_MODIFYING: shiny_threshold = 14200; break;
		case ABILITY_STAT_MODIFYING: shiny_threshold = 15400; break;
		case ABILITY_DROUGHT: shiny_threshold = 17000; break;
		case ALWAYS_CATCH: return 1;
		case NEVER_CATCH: return 0;
	}

	if(is_between(blackout_duration, 12100, 13200) || (blackout_duration > shiny_threshold))
	{
		LED_ON(BLUE_LED_PIN);
		shiny_caught_count++;
		return 1;
	}
	else
		return 0;
}

void do_egg_UI()
{
	CLEAR_LCD();
	char cursor_pos = 3;
	while(1)
	{
		lcd.setCursor(1, 0);
		lcd.print("EiP");
		lcd.setCursor(5, 0);
		lcd.print("PiB");
		lcd.setCursor(9, 0);
		lcd.print("ERI");
		lcd.setCursor(13, 0);
		lcd.print("Go");

		lcd.setCursor(1, 1);
		lcd.print((int)egg_in_pocket);
		lcd.setCursor(5, 1);
		lcd.print((int)pkmn_in_box);
		lcd.print(" ");
		lcd.setCursor(9, 1);
		lcd.print((int)egg_request_interval);
		lcd.print(" ");

		if(sf_right_button.uniquePress())
			cursor_pos = (cursor_pos + 1) % 4;
		if(sf_left_button.uniquePress())
		{
			cursor_pos - 1 < 0 ? cursor_pos = 4 : cursor_pos;
			cursor_pos = (cursor_pos - 1) % 4;
		}

		switch(cursor_pos)
		{
			case 0:
			lcd.setCursor(0, 0);
			lcd.print(">");
			lcd.setCursor(4, 0);
			lcd.print(" ");
			lcd.setCursor(8, 0);
			lcd.print(" ");
			lcd.setCursor(12, 0);
			lcd.print(" ");
			if(sf_up_button.uniquePress())
				egg_in_pocket++;
			if(sf_down_button.uniquePress())
				egg_in_pocket--;
			break;
			case 1:
			lcd.setCursor(0, 0);
			lcd.print(" ");
			lcd.setCursor(4, 0);
			lcd.print(">");
			lcd.setCursor(8, 0);
			lcd.print(" ");
			lcd.setCursor(12, 0);
			lcd.print(" ");
			if(sf_up_button.uniquePress())
				pkmn_in_box++;
			if(sf_down_button.uniquePress())
				pkmn_in_box--;
			break;
			case 2:
			lcd.setCursor(0, 0);
			lcd.print(" ");
			lcd.setCursor(4, 0);
			lcd.print(" ");
			lcd.setCursor(8, 0);
			lcd.print(">");
			lcd.setCursor(12, 0);
			lcd.print(" ");
			if(sf_up_button.uniquePress())
				egg_request_interval += 5;
			if(sf_down_button.uniquePress())
				egg_request_interval -= 5;
			break;
			case 3:
			lcd.setCursor(0, 0);
			lcd.print(" ");
			lcd.setCursor(4, 0);
			lcd.print(" ");
			lcd.setCursor(8, 0);
			lcd.print(" ");
			lcd.setCursor(12, 0);
			lcd.print(">");
			if(sf_up_button.uniquePress() || sf_down_button.uniquePress())
			{
				egg_ui_set = 1;
				CLEAR_LCD();
				return;
			}
			break;
		}

		egg_in_pocket > 5 ? egg_in_pocket = 5 : egg_in_pocket;
		pkmn_in_box > 30 ? pkmn_in_box = 30 : pkmn_in_box;
		egg_request_interval > 120 ? egg_request_interval = 120 : egg_request_interval;

		egg_in_pocket < 0 ? egg_in_pocket = 0 : egg_in_pocket;
		pkmn_in_box < 0 ? pkmn_in_box = 0 : pkmn_in_box;
		egg_request_interval < 15 ? egg_request_interval = 15 : egg_request_interval;

		free_slot = 5 - egg_in_pocket - hatched_in_pocket;
	}
}

void do_release()
{
	CLEAR_LCD();
	while(1)
	{
		SET_TO_BEGINNING();
		lcd.print("Release how");
		SET_TO_BEGINNING_ROW2();
		lcd.print("many?");
		lcd.setCursor(8, 1);
		lcd.print(release_request);
		lcd.print("   ");
		if(sf_up_button.uniquePress())
			release_request++;
		if(sf_down_button.uniquePress())
			release_request--;
		if(sf_right_button.uniquePress())
			release_request+=30;
		release_request >= 900 ? release_request = 900 : release_request;
		release_request < 1 ? release_request = 1 : release_request;
		if(sf_left_button.uniquePress())
			break;
	}	
	CLEAR_LCD();
	SET_TO_BEGINNING();
	lcd.print("0 Pokemon");
	SET_TO_BEGINNING_ROW2();
	lcd.print("released");
	for(int i = 1; i <= release_request; i++)
	{
		press_once(A_BUTTON_PIN);
		press_ntimes(UP_BUTTON_PIN, 2);
		press_once(A_BUTTON_PIN);
		delay(200);
		press_once(UP_BUTTON_PIN);
		press_once(A_BUTTON_PIN);
		SET_TO_BEGINNING();
		lcd.print(i);
		lcd.print(" Pokemon");
		SET_TO_BEGINNING_ROW2();
		lcd.print("released");
		delay(500);
		press_ntimes(A_BUTTON_PIN, 2);
		press_once(RIGHT_BUTTON_PIN);
		if(i % 6 == 0)
		{
			for(int i = 0; i < 6; i++)
			{
				press(LEFT_BUTTON_PIN);
				delay(100);
				release(LEFT_BUTTON_PIN);
				delay(100);
			}
			press_once(DOWN_BUTTON_PIN);
		}
		if(i % 30 == 0)
		{
			press_once(DOWN_BUTTON_PIN);
			press_once(RIGHT_BUTTON_PIN);
			press_once(DOWN_BUTTON_PIN);
		}
	}
}

void walk_n_step(char dir, int n)
{
	release(dir);
	for(int i = 0; i < n; i++)
	{
		press(dir);
		delay(200);
		release(dir);
		delay(100);
	}
}

int walk_n_step_hatching_protected(char dir, int n)
{
	for(int i = 0; i < n; i++)
	{
		walk_n_step(dir, 1);
		press_once_fast(B_BUTTON_PIN);
		if(READ_LIGHT_SENSOR() < dark_threshold)
		{
			hatch_handler();
			//log hatching on foot here
			return 1;
		}
	}
	return 0;
}

void cycle_for_egg()
{
	if(!egg_ui_set)
		do_egg_UI();
	long now = millis();
	int ls_value = READ_LIGHT_SENSOR();

	if(is_between((now % (2*CYCLE_PERIOD + 400)), 0, CYCLE_PERIOD))
		press(RIGHT_BUTTON_PIN);
	if(is_between((now % (2*CYCLE_PERIOD + 400)), CYCLE_PERIOD, CYCLE_PERIOD+200))
		release(RIGHT_BUTTON_PIN);
	if(is_between((now % (2*CYCLE_PERIOD + 400)), CYCLE_PERIOD + 200, 2*CYCLE_PERIOD+200))
		press(LEFT_BUTTON_PIN);
	if(is_between((now % (2*CYCLE_PERIOD + 400)), 2*CYCLE_PERIOD+200, 2*CYCLE_PERIOD+400))
		release(LEFT_BUTTON_PIN);

	if(now % 500 < 100)
		press(B_BUTTON_PIN);
	else
		release(B_BUTTON_PIN);

	if(now % 1000 < 500)
	{
		if(timer_updated == 0)
		{
			(walk_timer+1) > egg_request_interval ? walk_timer : walk_timer++;
			timer_updated = 1;
		}
	}
	else
		timer_updated = 0;

	if(ls_value < dark_threshold)
		hatch_handler();
		
	if(free_slot <= 0 && hatched_in_pocket >= 5)
		store_hatched();

	if((walk_timer >= egg_request_interval) && (free_slot > 0))
	{
		walk_timer = 0;
		ask_for_egg();
	}
 
	SET_TO_BEGINNING();
	lcd.print((int)walk_timer);
	lcd.print(' ');

	lcd.setCursor(3, 0);
	lcd.print("Egg:");
	lcd.print((int)egg_in_pocket);

	lcd.setCursor(9, 0);
	lcd.print("PKMN:");
	lcd.print((int)hatched_in_pocket);

	lcd.setCursor(13, 1);
	lcd.print(ls_value);
	lcd.print("  ");

	SET_TO_BEGINNING_ROW2();
	lcd.print("Free:");
	lcd.print((int)free_slot);

	lcd.setCursor(7, 1);
	lcd.print("T:");
	lcd.print(total_hatched);
}

int check_hatch(int duration, int button)
{
	long now = millis();
	press(button);
	while(millis() - now <= duration)
	{
		if(millis() % 500 < 100)
			press(B_BUTTON_PIN);
		else
			release(B_BUTTON_PIN);

		if(READ_LIGHT_SENSOR() < dark_threshold)
		{
			hatch_handler();
			press(button);
			// log hatching while biking asking for egg here
			return 1;
		}
	}
	return 0;
}

void store_hatched()
{
	CLEAR_LCD();
	SET_TO_BEGINNING();
	lcd.print("storing ");
	lcd.print((int)hatched_in_pocket);
	lcd.print(" PKMN");
	release_all();
	delay(200);
	while(check_hatch(CYCLE_PERIOD+1, RIGHT_BUTTON_PIN));
	release_all();
	press_once(Y_BUTTON_PIN);
	press_once(LEFT_BUTTON_PIN);
	delay(1000);
	walk_n_step(LEFT_BUTTON_PIN, 10);
	walk_n_step(UP_BUTTON_PIN, 3);
	delay(3200);
	walk_n_step(UP_BUTTON_PIN, 6);
	walk_n_step(RIGHT_BUTTON_PIN, 2);
	walk_n_step(UP_BUTTON_PIN, 1);
	press_once(A_BUTTON_PIN);
	delay(2000);
	press_once(A_BUTTON_PIN);
	delay(350);
	press_once(A_BUTTON_PIN);
	delay(350);
	press_once(A_BUTTON_PIN);
	delay(2500);
	for(int i = 0; i < hatched_in_pocket; i++)
	{
		press_once(RIGHT_BUTTON_PIN);
		press_once(A_BUTTON_PIN);
		press_once(A_BUTTON_PIN);
		delay(350);
		if(pkmn_in_box >= 30)
		{
			press_once(RIGHT_BUTTON_PIN);
			pkmn_in_box = 0;
		}
		press_once(A_BUTTON_PIN);
		pkmn_in_box++;
		SET_TO_BEGINNING_ROW2();
		lcd.print("PKMN in box:");
		lcd.print((int)pkmn_in_box);
		lcd.print(" ");
	}
	press_once(B_BUTTON_PIN);
	delay(600);
	press_once(B_BUTTON_PIN);
	delay(600);
	press_once(B_BUTTON_PIN);
	delay(600);
	press_once(B_BUTTON_PIN);
	delay(4000);
	walk_n_step(LEFT_BUTTON_PIN, 2);
	walk_n_step(DOWN_BUTTON_PIN, 7);
	delay(3500);
	walk_n_step(DOWN_BUTTON_PIN, 1);
	free_slot += hatched_in_pocket;
	hatched_in_pocket = 0;
	walk_timer = egg_request_interval;
	is_just_exited_daycare = 1;
	CLEAR_LCD();
}

void hatch_handler()
{
	CLEAR_LCD();
	SET_TO_BEGINNING();
	lcd.print("egg is hatching!");
	release_all();
	delay(18200);
	if(nickname_enabled)
	{
		CLEAR_NAME_BUF();
		sprintf(name_buf, "5 IV!");
		give_nickname();
	}
	else
	{
		skip_nickname();
		delay(4000);
	}
	CLEAR_LCD();
	egg_in_pocket--;
	hatched_in_pocket++;
	total_hatched++;
}

void ask_for_egg()
{
	long now = 0;
	CLEAR_LCD();
	SET_TO_BEGINNING();
	lcd.print("asking for egg..");
	release_all();
	delay(200);	
	if(is_just_exited_daycare)
	{
		walk_n_step(LEFT_BUTTON_PIN, 5);
		is_just_exited_daycare = 0;
	}
	else
	{
		while(check_hatch(CYCLE_PERIOD+1, RIGHT_BUTTON_PIN));
		release_all();
		press_once(Y_BUTTON_PIN);
		press_once(LEFT_BUTTON_PIN);
		delay(1000);
		if(walk_n_step_hatching_protected(LEFT_BUTTON_PIN, 15))
		{
			press_once(Y_BUTTON_PIN);
			press_once(LEFT_BUTTON_PIN);
			delay(1000);
			return;
		}
	}
	walk_n_step(UP_BUTTON_PIN, 1);
	press_once(A_BUTTON_PIN);
	delay(500);
	SET_TO_BEGINNING_ROW2();
	if(READ_LIGHT_SENSOR() < EGG_DARK_THRESHOLD)
	{
		
		lcd.print("egg available");
		total_obtained++;
		free_slot--;
		egg_in_pocket++;
		// log egg available here
		now = millis();
		while(millis() - now < 4500)
			press_once(A_BUTTON_PIN);
		now = millis();
		while(millis() - now < 3250)
			press_once(B_BUTTON_PIN);
	}
	else
	{
		
		lcd.print("no egg");
		// log egg unavailable here
		now = millis();
		while(millis() - now < 2800)
			press_once(B_BUTTON_PIN);
	}
	walk_n_step(DOWN_BUTTON_PIN, 1);
	press_once(Y_BUTTON_PIN);
	press_once(LEFT_BUTTON_PIN);
	delay(1000);
	CLEAR_LCD();
}

void log_error(char err_no)
{
	;
}

void do_UI()
{
	char cursor_pos = 4;
	SET_TO_BEGINNING();
	lcd.print(" mode DS N Tm Go");
	print_mode(current_mode);
	print_DS();
	print_nickname_en();
	print_blackout_mode();
	while(1)
	{
		if(sf_right_button.uniquePress())
			cursor_pos = (cursor_pos + 1) % 5;
		if(sf_left_button.uniquePress())
		{
			cursor_pos - 1 < 0 ? cursor_pos = 5 : cursor_pos;
			cursor_pos = (cursor_pos - 1) % 5;
		}
		print_cursor(cursor_pos);

		switch(cursor_pos)
		{
			case CURSOR_MODE:
			if(sf_up_button.uniquePress())
				current_mode = (current_mode + 1) % MODE_NUM;
			if(sf_down_button.uniquePress())
			{
				current_mode - 1 < 0 ? current_mode = MODE_NUM : current_mode;
				current_mode = (current_mode - 1) % MODE_NUM;
			}
			print_mode(current_mode);
			break;

			case CURSOR_DARK_THRESHOLD:
			if(sf_up_button.uniquePress())
				dark_threshold >= 60 ? dark_threshold : dark_threshold += 5;
			if(sf_down_button.uniquePress())
				dark_threshold <= 10 ? dark_threshold : dark_threshold -= 5;
			print_DS();
			break;

			case CURSOR_NICKNAME:
			if(sf_up_button.uniquePress() || sf_down_button.uniquePress())
				nickname_enabled = (nickname_enabled + 1) % 2;
			print_nickname_en();
			break;

			case CURSOR_BLACKOUT_MODE:
			if(sf_up_button.uniquePress())
				blackout_mode = (blackout_mode + 1) % 5;
			if(sf_down_button.uniquePress())
			{
				blackout_mode - 1 < 0 ? blackout_mode = 5 : blackout_mode;
				blackout_mode = (blackout_mode - 1) % 5;
			}
			print_blackout_mode();
			break;

			case CURSOR_RUN:
			if(sf_up_button.uniquePress() || sf_down_button.uniquePress())
				return;

			current_mode == CLONE_MODE ? check_screen_delay_enabled = 0 : check_screen_delay_enabled = 1;
		}
	}
}

void do_clone()
{
	int count = 0;
	SET_TO_BEGINNING();
	lcd.print("clone   press");
	lcd.setCursor(0, 1);
	lcd.print("down to start");
	while(!sf_down_button.uniquePress());
	CLEAR_LCD();
	SET_TO_BEGINNING();
	lcd.print("Armed");
	check_screen(NO_TIMEOUT);
	CLEAR_LCD();
	delay(3920);
	press(POWER_BUTTON_PIN);
	delay(300);
	release(POWER_BUTTON_PIN);
	SET_TO_BEGINNING();
	lcd.print("restarting..");
	delay(1000);
	press_once(HOME_BUTTON_PIN);
	delay(20000);
	press_once(A_BUTTON_PIN);
	delay(9000);
	press_once(A_BUTTON_PIN);
	delay(2000);
	press_once(A_BUTTON_PIN);
	CLEAR_LCD();
}

void log_shiny()
{
	;
}

void do_horde()
{
	SET_TO_BEGINNING();
	lcd.print("horde");
	press_once(X_BUTTON_PIN);
	press_once(A_BUTTON_PIN);
	delay(2000);
	press_once(RIGHT_BUTTON_PIN);
	press_once(A_BUTTON_PIN);
	delay(1000);
	press_once(DOWN_BUTTON_PIN);
	press_once(A_BUTTON_PIN);
	press_once(A_BUTTON_PIN);
	delay(4500);
	long duration = check_screen(CATCH_TIMEOUT);
    
	if(duration == -1)
	{
		for (int i = 0; i < 50; i++)
			press_once(B_BUTTON_PIN);
		log_error(ERR_SWEET_SCENT_SEL);
		return;
	}
	
	encounter_count++;
	print_encounter_count();
	SET_TO_DEBUG();
	lcd.print(duration);
	while(is_between(duration, 13500, 14700) || duration > 16000)
		sound_buzzer();
	do_run_away();
	delay(1000);
}

void sound_buzzer()
{
	if(millis() % 1000 < 500)
		tone(BUZZER_PIN, 3000);
	else
		noTone(BUZZER_PIN);
}

void do_walking()
{
	while(1)
	{
		SET_TO_BEGINNING();
		lcd.print("walk ");
		long blackout_duration = walk_until_encounter();
		encounter_count++;
		print_encounter_count();
		if(is_shiny(blackout_duration))
		{
			try_catch();
			delay(500);
			continue;
		}
		else
		{
			do_run_away();
			delay(500);
		}
	}
}

void try_catch()
{
	SET_TO_BEGINNING();
	lcd.print("catch");
	throw_ball(QUICKBALL_PAGE, QUICKBALL_POS);
	if(!is_caught())
	{
		do_false_swipe();
		for(int i = 0; i < RETRY_ATTEMPT; i++)
		{
			throw_ball(BACKUPBALL_PAGE, BACKUPBALL_POS);
			if(is_caught())
				goto logging;
		}
		log_shiny();
		do_alarm();
	}
	// now it's time to send logging request
	logging:
	log_shiny();
	catch_retry_count = 0;
	encounter_count = 0;
}

void do_alarm()
{
	SET_TO_BEGINNING();
	lcd.print("crap ");
	while(1)
		sound_buzzer();
}

int is_caught()
{
	if(check_screen(CATCH_TIMEOUT) == -1)
	{
		catch_retry_count++;
		print_catch_info();
		return 0;
	}
	SET_TO_BEGINNING();
	lcd.print("cnfrm");
	if(nickname_enabled)
	{
		CLEAR_NAME_BUF();
		sprintf(name_buf, "test");
		give_nickname();
	}
	else
		skip_nickname();
	delay(1000);
	press_once(A_BUTTON_PIN);
	delay(5000);
	print_catch_info();
	return 1;
}

void skip_nickname()
{
	delay(1000);
	press_once(B_BUTTON_PIN);
	press_once(B_BUTTON_PIN);
}

void give_nickname()
{
	delay(1000);
	press_once(A_BUTTON_PIN);
	delay(3000);
	enter_nickname(name_buf);
	press_once(A_BUTTON_PIN);
	delay(5500);
}

void enter_nickname(char* s)
{
	for(int i = 0; i < 12; i++)
	{
		if(s[i] == 0)
		{
			select_enter();
			return;
		}

		enter_letter(s[i], i);
	}
}

void print_catch_info()
{
	lcd.setCursor(4, 1);
	lcd.print("C");
	lcd.print(shiny_caught_count);
	lcd.print("R");
	lcd.print(catch_retry_count);
}

void throw_ball(char page_index, char ball_index)
{
	delay(1000);
	press_once(DOWN_BUTTON_PIN);
	press_once(A_BUTTON_PIN);
	delay(BATTLE_BUTTON_PINPRESS_DELAY);
	press_once(RIGHT_BUTTON_PIN);
	press_once(A_BUTTON_PIN);
	delay(BATTLE_BUTTON_PINPRESS_DELAY);
	reset_page();
	goto_page_start(page_index);
	select_ball(ball_index);
	delay(BATTLE_BUTTON_PINPRESS_DELAY);
	press_once(A_BUTTON_PIN);
}

void reset_page()
{
	unsigned long now = millis();
	while(millis() - now <= 2000)
		press_once(LEFT_BUTTON_PIN);
	for(int i=0; i<3; i++)
		press_once(UP_BUTTON_PIN);
}

void select_ball(char ball_index)
{
	unsigned char row = ball_index / 2;
	unsigned char col = ball_index - 2 * row;
    
	for(int i=0; i<row; i++)
		press_once(DOWN_BUTTON_PIN);
    
	for(int i=0; i<col; i++)
		press_once(RIGHT_BUTTON_PIN);
    
	press_once(A_BUTTON_PIN);
}

void goto_page_start(char page_index)
{
	switch(page_index)
	{
		case 0: return;
            
		case 1:
        press_once(RIGHT_BUTTON_PIN);
        press_once(RIGHT_BUTTON_PIN);
        delay(BATTLE_BUTTON_PINPRESS_DELAY);
        press_once(LEFT_BUTTON_PIN);
        break;
            
		case 2:
        press_once(RIGHT_BUTTON_PIN);
        press_once(RIGHT_BUTTON_PIN);
        delay(BATTLE_BUTTON_PINPRESS_DELAY);
        press_once(RIGHT_BUTTON_PIN);
        delay(BATTLE_BUTTON_PINPRESS_DELAY);
        press_once(LEFT_BUTTON_PIN);
        break;
            
		default: return;
	}
}

int is_between(long val, long lower, long upper)
{
	return (val >= lower && val < upper);
}

int walk_until_encounter()
{
	unsigned long dark_start = 0;
	unsigned char screen_state = SCREEN_BRIGHT;
	
	while(1)
	{
		delay(CHECK_SCRREN_DELAY);
		if(screen_state == SCREEN_BRIGHT)
		{
			press(B_BUTTON_PIN);
			if(millis() % WALKING_BUTTON_PINPRESS_DURATION * 2 < WALKING_BUTTON_PINPRESS_DURATION)
			{
				release(RIGHT_BUTTON_PIN);
				press(LEFT_BUTTON_PIN);
			}
			else
			{
				release(LEFT_BUTTON_PIN);
				press(RIGHT_BUTTON_PIN);
			}
		}
        
		int curr = READ_LIGHT_SENSOR();
        
		lcd.setCursor(0, 1);
		lcd.print(curr);
		lcd.print(" ");
        
		curr < dark_threshold ? curr = DARK : curr = BRIGHT;
        
		if(screen_state == SCREEN_BRIGHT && curr == DARK)
		{
			screen_state = SCREEN_DARK;
			release_all();
			SET_TO_LAST();
			lcd.print("D");
			SET_TO_BEGINNING();
			lcd.print("batt ");
			dark_start = millis();
		}
		
		else if(screen_state == SCREEN_DARK && curr == BRIGHT)
		{
			unsigned long ret = millis() - dark_start;
			if(ret < 10000)
			{
				SET_TO_LAST();
				lcd.print("F");
				SET_TO_DEBUG();
				lcd.print(ret);
				lcd.print(" ");
				error_occurred = 1;
				continue;
			}
			SET_TO_LAST();
			lcd.print("L");
			SET_TO_DEBUG();
			lcd.print(ret);
			lcd.print(" ");
			if(error_occurred)
				log_error(ERR_WUE_PREMATURE_RETURN);
			return ret;
		}
	}
}

void do_fishing()
{
	SET_TO_BEGINNING();
	lcd.print("start");
	delay(200);
	press_once(Y_BUTTON_PIN);
	press_once(RIGHT_BUTTON_PIN);
	if(!is_bite())
		return;
	do_fishing_check_shiny();
}

void do_fishing_check_shiny()
{
	long blackout_duration;
	SET_TO_BEGINNING();
	lcd.print("batt ");
	blackout_duration = check_screen(CATCH_TIMEOUT);
	if(blackout_duration == -1)
	{
		press_ntimes(B_BUTTON_PIN, 50);
		log_error(ERR_FISHING_FALSE_BITE);
		return;
	}
	if(is_shiny(blackout_duration))
	{
		try_catch();
		return;
	}
	else
	{
		do_run_away();
		return;
	}
}

int is_bite()
{
	SET_TO_BEGINNING();
	lcd.print("cast ");
	int curr = 0;
	int max = 0;
	delay(3000);
	unsigned long start_time = millis();
	while(1)
	{
		curr = analogRead(AUDIO_IN_PIN); 
		if(curr > max)
		{
			max = curr;
			lcd.setCursor(0, 1);
			lcd.print(max);
			lcd.print(" ");
		}
		if(max >= 400) // 300
		{
			clear_debug_display();
			SET_TO_DEBUG();
			lcd.print("bite");
			press_once(A_BUTTON_PIN);
			encounter_count++;
			print_encounter_count();
			return 1;
		}
        
		if(millis() - start_time > FISHING_TIMEOUT)
		{
			clear_debug_display();
			SET_TO_DEBUG();
			lcd.print("timeout");
			press_once(A_BUTTON_PIN);
			encounter_count = 0;
			print_encounter_count();
			return 0;
		}
	}
}

void do_false_swipe()
{
	delay(100);
	press_once(UP_BUTTON_PIN);
	delay(100);
	press_once(A_BUTTON_PIN);
	delay(BATTLE_BUTTON_PINPRESS_DELAY);
	press_once(A_BUTTON_PIN);
	delay(CATCH_TIMEOUT);
}

void do_run_away()
{
	SET_TO_BEGINNING();
	lcd.print("flee ");
	while(1)
	{
		delay(100);
		press_once(DOWN_BUTTON_PIN);
		delay(100);
		press_once(RIGHT_BUTTON_PIN);
		delay(100);
		press_once(A_BUTTON_PIN);
		delay(100);
        
		if(check_screen(CATCH_TIMEOUT) == -1)
		{
			for(int i = 0; i < 10; i++)
				press_once(B_BUTTON_PIN);
			for(int i = 0; i < 10; i++)
				press_once(LEFT_BUTTON_PIN);
			for(int i = 0; i < 10; i++)
				press_once(UP_BUTTON_PIN);
			error_occurred = 1;
			continue;
		}
		else
		{
			if(error_occurred)
				log_error(ERR_FLEE_FAILED);
			return;
		}
	}
}

long check_screen(long timeout)
{
	unsigned long dark_start = 0;
	unsigned long timeout_start = millis();
	unsigned char screen_state = SCREEN_BRIGHT;
	while(1)
	{
		delay(CHECK_SCRREN_DELAY);
		
		if((timeout != NO_TIMEOUT) && (millis() - timeout_start > timeout))
		{
			return -1;
		}
		int curr = READ_LIGHT_SENSOR();
        
		lcd.setCursor(0, 1);
		lcd.print(curr);
		lcd.print(" ");
        
		curr <= dark_threshold ? curr = DARK : curr = BRIGHT;
        
		if(screen_state == SCREEN_BRIGHT && curr == DARK)
		{
			screen_state = SCREEN_DARK;
			SET_TO_LAST();
			lcd.print("D");
			dark_start = millis();
		}
		else if(screen_state == SCREEN_DARK && curr == BRIGHT)
		{
			unsigned long ret = millis() - dark_start;
			if(check_screen_delay_enabled && ret < 2400)
			{
				SET_TO_LAST();
				lcd.print("F");
				SET_TO_DEBUG();
				lcd.print(ret);
				lcd.print(" ");
				error_occurred = 1;
				continue;
			}
			SET_TO_LAST();
			lcd.print("L");
			SET_TO_DEBUG();
			lcd.print(ret);
			lcd.print(" ");
			if(error_occurred)
				log_error(ERR_CHECK_SCREEN_PREMATURE_RETURN);
			return ret;
		}
	}
}

void press(int16_t button)
{
	pinMode(button, OUTPUT);
	digitalWrite(button, BUTTON_PRESSED);
}

void release(int16_t button)
{
	pinMode(button, INPUT);
}

void release_all()
{
	release(A_BUTTON_PIN);
	release(B_BUTTON_PIN);
	release(X_BUTTON_PIN);
	release(Y_BUTTON_PIN);
	release(LEFT_BUTTON_PIN);
	release(RIGHT_BUTTON_PIN);
	release(DOWN_BUTTON_PIN);
	release(UP_BUTTON_PIN);
	release(HOME_BUTTON_PIN);
	release(DS_SELECT_BUTTON_PIN);
	release(DS_START_BUTTON_PIN);
	release(LEFT_SHOULDER_BUTTON_PIN);
	release(RIGHT_SHOULDER_BUTTON_PIN);
}

void do_mode(unsigned char mode)
{
	switch(mode)
	{
		case FISHING_MODE: do_fishing(); break;
		case WALKING_MODE: do_walking(); break;
		case HORDE_MODE: do_horde(); break;
		case CLONE_MODE: do_clone(); break;
		case EGG_MODE: cycle_for_egg(); break;
		case RELEASE_MODE: do_release(); break;
		case SR_MODE: do_SR(); break;
		default:
		CLEAR_LCD();
		SET_TO_BEGINNING();
		lcd.print("unknown mode");
		while(1);
	}
}

void clear_debug_display()
{
	lcd.setCursor(0, 1);
	lcd.print("    ");
	lcd.setCursor(9, 1);
	lcd.print("      ");
}

void print_encounter_count()
{
	lcd.setCursor(6, 0);
	lcd.print("count:");
	lcd.print(encounter_count);
	lcd.print("    ");
}

void press_ntimes(int button, int count)
{
	for(int i = 0; i < count; i++)
		press_once_fast(button);
}

void press_once(int button)
{
	delay(150);
	press(button);
	delay(100);
	release(button);
	delay(150);
}

void press_once_fast(int button)
{
	press(button);
	delay(50);
	release(button);
	delay(50);
}

void enter_letter(char letter, char letter_index)
{
	switch(letter)
	{
		case '!': goto_coord(0, 0, letter_index); return;
		case '1': goto_coord(0, 1, letter_index); return;
		case '2': goto_coord(0, 2, letter_index); return;
		case '+' : select_shift();
		case '3': goto_coord(0, 3, letter_index); return;
		case '-' : select_shift();
		case '4': goto_coord(0, 4, letter_index); return;
		case '*' : select_shift();
		case '5': goto_coord(0, 5, letter_index); return;
		case '/' : select_shift();
		case '6': goto_coord(0, 6, letter_index); return;
		case '=' : select_shift();
		case '7': goto_coord(0, 7, letter_index); return;
		case '%' : select_shift();
		case '8': goto_coord(0, 8, letter_index); return;
		case '(' : select_shift();
		case '9': goto_coord(0, 9, letter_index); return;
		case ')' : select_shift();
		case '0': goto_coord(0, 10, letter_index); return;

		case 'Q': select_shift();
		case 'q': goto_coord(1, 0, letter_index); return;

		case 'W': select_shift();
		case 'w': goto_coord(1, 1, letter_index); return;
        
		case 'E': select_shift();
		case 'e': goto_coord(1, 2, letter_index); return;

		case 'R': select_shift();
		case 'r': goto_coord(1, 3, letter_index); return;

		case 'T': select_shift();
		case 't': goto_coord(1, 4, letter_index); return;

		case 'Y': select_shift();
		case 'y': goto_coord(1, 5, letter_index); return;

		case 'U': select_shift();
		case 'u': goto_coord(1, 6, letter_index); return;

		case 'I': select_shift();
		case 'i': goto_coord(1, 7, letter_index); return;

		case 'O': select_shift();
		case 'o': goto_coord(1, 8, letter_index); return;

		case 'P': select_shift();
		case 'p': goto_coord(1, 9, letter_index); return;

		case 'A': select_shift();
		case 'a': goto_coord(2, 0, letter_index); return;

		case 'S': select_shift();
		case 's': goto_coord(2, 1, letter_index); return;
        
		case 'D': select_shift();
		case 'd': goto_coord(2, 2, letter_index); return;

		case 'F': select_shift();
		case 'f': goto_coord(2, 3, letter_index); return;

		case 'G': select_shift();
		case 'g': goto_coord(2, 4, letter_index); return;

		case 'H': select_shift();
		case 'h': goto_coord(2, 5, letter_index); return;

		case 'J': select_shift();
		case 'j': goto_coord(2, 6, letter_index); return;

		case 'K': select_shift();
		case 'k': goto_coord(2, 7, letter_index); return;

		case 'L': select_shift();
		case 'l': goto_coord(2, 8, letter_index); return;

		case '\'': goto_coord(2, 9, letter_index); return;

		case ':': goto_coord(2, 10, letter_index); return;

		case 'Z': select_shift();
		case 'z': goto_coord(3, 0, letter_index); return;

		case 'X': select_shift();
		case 'x': goto_coord(3, 1, letter_index); return;
        
		case 'C': select_shift();
		case 'c': goto_coord(3, 2, letter_index); return;

		case 'V': select_shift();
		case 'v': goto_coord(3, 3, letter_index); return;

		case 'B': select_shift();
		case 'b': goto_coord(3, 4, letter_index); return;

		case 'N': select_shift();
		case 'n': goto_coord(3, 5, letter_index); return;

		case 'M': select_shift();
		case 'm': goto_coord(3, 6, letter_index); return;

		case ',': goto_coord(3, 7, letter_index); return;

		case '.': goto_coord(3, 8, letter_index); return;

		case '?': goto_coord(3, 9, letter_index); return;

		case '"': goto_coord(3, 10, letter_index); return;

		case ' ': goto_coord(4, 0, letter_index); return;

		default: goto_coord(4, 0, letter_index); return;
	}
}

void goto_coord(char y, char x, char letter_index)
{
	press_ntimes(RIGHT_BUTTON_PIN, x);
	press_ntimes(DOWN_BUTTON_PIN, y);
	press_once_fast(A_BUTTON_PIN);
	if(letter_index >= 11)
		return;
	press_ntimes(UP_BUTTON_PIN, y);
	press_ntimes(LEFT_BUTTON_PIN, x);
}

void select_shift()
{
	press_once_fast(UP_BUTTON_PIN);
	press_once_fast(RIGHT_BUTTON_PIN);
	press_once_fast(A_BUTTON_PIN);
	press_once_fast(LEFT_BUTTON_PIN);
	press_once_fast(DOWN_BUTTON_PIN);
}

void select_enter()
{
	press_once_fast(LEFT_BUTTON_PIN);
	press_once_fast(DOWN_BUTTON_PIN);
	press_once_fast(A_BUTTON_PIN);
}

void print_cursor(char pos)
{
	switch(pos)
	{
		case 0:
		clear_cursor(pos);
		SET_TO_BEGINNING();
		lcd.print(">");
		break;

		case 1:
		clear_cursor(pos);
		lcd.setCursor(5, 0);
		lcd.print(">");
		break;

		case 2:
		clear_cursor(pos);
		lcd.setCursor(8, 0);
		lcd.print(">");
		break;

		case 3:
		clear_cursor(pos);
		lcd.setCursor(10, 0);
		lcd.print(">");
		break;

		case 4:
		clear_cursor(pos);
		lcd.setCursor(13, 0);
		lcd.print(">");
		break;

		default:
		clear_cursor(pos);
	}
}

void clear_cursor(char pos)
{
	if(pos != 0)
	{
		SET_TO_BEGINNING();
		lcd.print(" ");
	}
	if(pos != 1)
	{
		lcd.setCursor(5, 0);
		lcd.print(" ");
	}
	if(pos != 2)
	{
		lcd.setCursor(8, 0);
		lcd.print(" ");
	}
	if(pos != 3)
	{
		lcd.setCursor(10, 0);
		lcd.print(" ");
	}
	if(pos != 4)
	{
		lcd.setCursor(13, 0);
		lcd.print(" ");
	}
}

void print_nickname_en()
{
	lcd.setCursor(9, 1);
	lcd.print((int)nickname_enabled);
}

void print_DS()
{
	lcd.setCursor(6, 1);
	lcd.print((int)dark_threshold);
}

void print_mode(char mode)
{
	lcd.setCursor(1, 1);
	switch(mode)
	{
		case WALKING_MODE:
		lcd.print("walk");
		break;

		case FISHING_MODE:
		lcd.print("fish");
		break;

		case HORDE_MODE:
		lcd.print("hord");
		break;

		case CLONE_MODE:
		lcd.print("clon");
		break;

		case SR_MODE:
		lcd.print("SR  ");
		break;

		case EGG_MODE:
		lcd.print("egg ");
		break;

		case RELEASE_MODE:
		lcd.print("rlse");
		break;

		default:
		lcd.print("ERR ");
		break;
	}
}

void print_blackout_mode()
{
	lcd.setCursor(11, 1);
	switch(blackout_mode)
	{
		case ABILITY_NOT_STAT_MODIFYING:
		lcd.print("NS");
		break;

		case ABILITY_STAT_MODIFYING:
		lcd.print("S ");
		break;

		case ABILITY_DROUGHT:
		lcd.print("DR");
		break;

		case ALWAYS_CATCH:
		lcd.print("R1");
		break;

		case NEVER_CATCH:
		lcd.print("R0");
		break;

		default:
		lcd.print("ER");
		break;
	}
}

void do_SR()
{
	press(LEFT_BUTTON_PIN);
}

void c_pad_reset()
{
	analogWrite(C_PAD_X_PIN, C_PAD_DEFAULT_POTENTIAL);
	analogWrite(C_PAD_Y_PIN, C_PAD_DEFAULT_POTENTIAL);
}
