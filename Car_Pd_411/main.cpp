#include <iostream>
#include<conio.h>
#include<thread>
#include<chrono>
#include<Windows.h>
using namespace std::chrono_literals;
using std::cin;
using std::cout;
using std::endl;

#define Enter	13
#define Escape	27

#define MIN_TANK_VOLUME 25
#define MAX_TANK_VOLUME 125
class Tank
{
	const int VOLUME;
	double fuel_level;

public:

	int get_VOLUME()const
	{
		return VOLUME;
	}

	double get_fuel_level() const
	{
		return fuel_level;
	}

	void fill(double fuel)
	{
		if (fuel < 0)return;
		fuel_level += fuel;
		if (fuel_level > VOLUME)fuel_level = VOLUME;
	}

	double give_fuel(double amount)
	{
		fuel_level -= amount;
		if (fuel_level < 0)fuel_level = 0;
		return fuel_level;
	}

	Tank(int volume) :
		VOLUME
		(
			volume < MIN_TANK_VOLUME ? MIN_TANK_VOLUME :
			volume > MAX_TANK_VOLUME ? MAX_TANK_VOLUME :
			volume
		)
	{
		//this->VOLUME = volume;obliged-обязан облайдж левосторонеее и(относительно =) провостороннее значения
		this->fuel_level = 0;
		cout << "Tank is ready:\t" << this << endl;
	}
	~Tank()
	{
		cout << "Tank is over:\t" << this << endl;
	}

	void info() const
	{
		cout << "Volume:\t" << VOLUME << " liters\n";
		cout << "Fuel:\t" << get_fuel_level() << " liters\n";
	}

};


#define MIN_ENGINE_CONSUMPTION 5
#define MAX_ENGINE_CONSUMPTION 25
class Engine
{
	double consumption_per_second;
	bool is_started;

public:
	const double CONSUMPTION;

	double consumption_liters_per_second_at_speed(int speed_kmh) const
	{
		if (speed_kmh <= 0)
			return get_consumption_per_second(); // холостой ход
		return (CONSUMPTION * ((double)speed_kmh / 100.0)) / 3600.0;
	}

	double get_consumption_per_second() const
	{
		return consumption_per_second;
	}
	Engine(double consumption) :CONSUMPTION
	(
		consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		consumption
	)
	{
		consumption_per_second = CONSUMPTION * 3e-5;
		is_started = false;
		cout << "Engine is ready:\t" << this << endl;
	}
	~Engine()
	{
		cout << "Engine is over:\t" << this << endl;
	}
	void start()
	{
		is_started = true;
	}
	void stop()
	{
		is_started = false;
	}
	bool started()const
	{
		return is_started;
	}
	void info() const
	{
		cout << "Consumption per 100 km:\t" << CONSUMPTION << " liters.\n";
		cout << "Consumption per 1 second:\t" << get_consumption_per_second() << " liters.\n";
	}
};

#define MAX_SPEED_LOWER_LIMIT	 40
#define MAX_SPEED_UPPER_LIMIT	400
class Car
{
	Engine engine;
	Tank tank;
	const int MAX_SPEED;
	int speed;
	bool driver_inside;
	double target_distance_km; // цель км
	double distance_traveled_km; // пройдено км
	struct //Threads
	{
		std::thread panel_thread;
		std::thread engine_idle_threads;

	}threads;

public:

	Car(double consumption, int volume, int max_speed) :
		engine(consumption),
		tank(volume),
		speed(0),
		MAX_SPEED
		(
			max_speed < MAX_SPEED_LOWER_LIMIT ? MAX_SPEED_LOWER_LIMIT :
			max_speed > MAX_SPEED_UPPER_LIMIT ? MAX_SPEED_UPPER_LIMIT :
			max_speed
		)
	{
		driver_inside = false;
		cout << "Your car is redy:-)\t" << this << endl;
	}
	~Car()
	{
		cout << "Your car is over:-(\t" << this << endl;
	}
	void get_in()
	{
		driver_inside = true;
		//panel();
		threads.panel_thread = std::thread(&Car::panel, this);
	}
	void get_out()
	{
		driver_inside = false;
		if (threads.panel_thread.joinable())threads.panel_thread.join();
		system("CLS");
		cout << "Wall street" << endl;
	}
	void start()
	{
		if (driver_inside && tank.get_fuel_level())
		{
			engine.start();
			threads.engine_idle_threads = std::thread(&Car::engine_idle, this);
		}
	}
	void stop()
	{
		engine.stop();
		if (threads.engine_idle_threads.joinable())threads.engine_idle_threads.join();
	}
	void control()
	{
		cout << "Press 'Enter' to get in" << endl;
		char key;
		do
		{
			key = 0;
			if (_kbhit())key = _getch();
			switch (key)
			{
			case Enter:
				if (driver_inside)get_out();
				else get_in();
				break;
			case 'F':
			case 'f':
				double amount;
				cout << "How much do you want? "; cin >> amount;
				tank.fill(amount);
				break;
			case'I'://зажигание
			case 'i':
				if (engine.started())stop();
				else start();
				break;
			case 'K':
			case 'k':
			{
				double distance;
				cout << "Enter distance to travel (km): ";
				cin >> distance;
				if (distance < 0) distance = 0;
				else
				{
					target_distance_km = distance;
					distance_traveled_km = 0.0;
				}
				break;
			}
			case '+':
			{
				speed += 10;
				if (speed > MAX_SPEED_UPPER_LIMIT) speed = MAX_SPEED_UPPER_LIMIT;
				break;
			}
			case '-':
			{
				speed -= 10;
				if (speed < 0) speed = 0;
			}
			break;
			case '0':
			{
				speed = 0;
			}
			break;
			case Escape:
				stop();
				get_out();
			}
			if (tank.get_fuel_level() == 0 && threads.engine_idle_threads.joinable())stop();
			std::this_thread::sleep_for(50ms);
		} while (key != Escape);
	}
	void engine_idle()
	{
		while (engine.started() && tank.give_fuel(engine.get_consumption_per_second()))
		{
			//расход за текущую секунду (включая холостой)
			double fuel_needed_per_sec = engine.consumption_liters_per_second_at_speed(speed);
			if (speed <= 0)
				fuel_needed_per_sec = engine.get_consumption_per_second();

			//-топливо
			double fuel_after = tank.give_fuel(fuel_needed_per_sec);

			// новый пройденный путь
			double km_this_second = static_cast<double>(speed) / 3600.0;
			distance_traveled_km += km_this_second;

			//проверка на достижение цели
			if (target_distance_km > 0.0 && distance_traveled_km >= target_distance_km)
			{
				distance_traveled_km = target_distance_km;
				engine.stop();
				speed = 0;
				break;
			}
			std::this_thread::sleep_for(1s);
		}
			std::this_thread::sleep_for(1s);
	}
	void panel()const
	{
		while (driver_inside)
		{
			system("CLS");
			cout << "Fuel level:\t" << tank.get_fuel_level() << " liters.";
			if (tank.get_fuel_level() < 5)
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, 0x4F);
				cout << " LOW FUEL";
				SetConsoleTextAttribute(hConsole, 0x07);
			}
			cout << endl;
			cout << "Engine is " << (engine.started() ? "started" : "stopped") << endl;
			cout << "Speed: " << speed << " km/h" << endl;

			if (target_distance_km > 0.0)
			{
				double remaining = target_distance_km - distance_traveled_km;
				if (remaining < 0) remaining = 0;
				cout << "Target: " << target_distance_km << " km. Passed: " << distance_traveled_km << " km. Remaining: " << remaining << " km." << endl;
			}
			else
			{
				cout << "No target distance set. Press 'K' to set kms." << endl;
			}

			std::this_thread::sleep_for(100ms);
		}
	}
	void info()const
	{
		engine.info();
		tank.info();
		cout << "Max speed:\t" << MAX_SPEED << " km/h\n";
	}
};

//#define ENGINE_CHECK
//#define TANK_CHECK

void main()
{
	setlocale(LC_ALL, "");
#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif //ENGINE_CHECK

#ifdef  TANK_CHECK

	Tank tank(-30);
	tank.info();
	double fuel;
	do
	{
		cout << "Введите объем топлива"; cin >> fuel;
		tank.fill(fuel);
		tank.info();
	} while (fuel > 0);
#endif // TANK_CHECK

	Car bmw(10, 80, 270);
	bmw.info();
	bmw.control();
}
